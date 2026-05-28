#include "moody/Loggr.hpp"

#include <filesystem>
#include <iomanip>
#include <iostream>

namespace moody {

Loggr::Loggr(
    const std::string& basepath,
    const std::string& dirname,
    const std::string& filename,
    bool coutEnabled,
    bool append,
    bool coloring,
    bool milliseconds
)
    :
    basepath(basepath),
    dirname(dirname),
    filename(filename),
    coutEnabled(coutEnabled),
    append(append),
    coloring(coloring),
    milliseconds(milliseconds)
{
    const std::filesystem::path root =
        std::filesystem::path(this->basepath) /
        this->dirname;

    if (!std::filesystem::exists(root)) {
        std::filesystem::create_directories(root);
    }

    const std::filesystem::path fullpath = root / this->filename;

    file.open(
        fullpath,
        append ? std::ios::app : std::ios::out
    );

    running = true;
    workerThread = std::thread(&Loggr::worker, this);
}

Loggr::~Loggr()
{
    {
        std::lock_guard<std::mutex> lock(mutex);
        running = false;
    }

    cv.notify_one();

    if (workerThread.joinable()) {
        workerThread.join();
    }

    flush();
}

void Loggr::set_level(Level level)
{
    minLevel = level;
}

void Loggr::set_milliseconds(bool enabled)
{
    milliseconds = enabled;
}

void Loggr::set_file_mode(FileMode mode)
{
    std::lock_guard<std::mutex> lock(fileMutex);
    fileMode = mode;
}

void Loggr::flush()
{
    std::lock_guard<std::mutex> lock(fileMutex);

    if (file.is_open()) {
        file.flush();
    }

    for (auto& [_, stream] : moduleFiles) {
        if (stream.is_open()) {
            stream.flush();
        }
    }

    std::cout.flush();
}

void Loggr::log(
    Level level,
    const std::string& module,
    const std::string& msg,
    const FileSourceInfo& src
)
{
    if (level < minLevel) {
        return;
    }

    LogEvent event;
    event.level = level;
    event.module = module;
    event.message = msg;
    event.source = src;
    event.timestamp = std::chrono::system_clock::now();

    push(std::move(event));
}

void Loggr::worker()
{
    std::queue<LogEvent> localQueue;

    while (true) {
        {
            std::unique_lock<std::mutex> lock(mutex);

            cv.wait(lock, [this] {
                return !inbox.empty() || !running;
            });

            if (!running && inbox.empty()) {
                break;
            }

            std::swap(localQueue, inbox);
        }

        while (!localQueue.empty()) {
            const auto& event = localQueue.front();

            const std::string formatted = format(event);

            write_file(formatted, event.module);
            write_console(formatted, event.level);

            localQueue.pop();
        }

        flush();
    }
}

void Loggr::push(LogEvent event)
{
    {
        std::lock_guard<std::mutex> lock(mutex);
        inbox.push(std::move(event));
    }

    cv.notify_one();
}

std::string Loggr::format(const LogEvent& event)
{
    std::ostringstream oss;

    format_timestamp(oss, event.timestamp);
    format_level(oss, event.level);
    format_module(oss, event.module);
    format_source(oss, event.source);
    format_message(oss, event.message);
    format_options(oss, event.options);

    oss << '\n';

    return oss.str();
}

void Loggr::format_timestamp(
    std::ostringstream& oss,
    const std::chrono::system_clock::time_point& timestamp
)
{
    const auto time =
        std::chrono::system_clock::to_time_t(timestamp);

    std::tm tm{};

#ifdef _WIN32
    localtime_s(&tm, &time);
#else
    localtime_r(&time, &tm);
#endif

    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");

    if (milliseconds) {
        const auto ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                timestamp.time_since_epoch()
            ) % 1000;

        oss << "."
            << std::setfill('0')
            << std::setw(3)
            << ms.count();
    }

    oss << ' ';
}

void Loggr::format_level(std::ostringstream& oss, Level level)
{
    oss << '[';

    switch (level) {
        case TRACE: oss << "TRACE"; break;
        case DEBUG: oss << "DEBUG"; break;
        case INFO:  oss << "INFO";  break;
        case WARN:  oss << "WARN";  break;
        case ERROR: oss << "ERROR"; break;
        case FATAL: oss << "FATAL"; break;
        default:    oss << "-----"; break;
    }

    oss << "] ";
}

void Loggr::format_module(
    std::ostringstream& oss,
    const std::string& module
)
{
    oss << '[' << module << "] ";
}

void Loggr::format_source(
    std::ostringstream& oss,
    const FileSourceInfo& src
)
{
    const std::filesystem::path path(src.filename);

    oss << '['
        << path.filename().string()
        << ':'
        << src.line
        << "] ";
}

void Loggr::format_message(
    std::ostringstream& oss,
    const std::string& msg
)
{
    oss << "msg:{"
        << msg
        << "} ";
}

void Loggr::format_options(
    std::ostringstream& oss,
    const LogOptions& options
)
{
    if (options.values.empty()) {
        return;
    }

    oss << '<';

    for (size_t i = 0; i < options.values.size(); i += 2) {
        oss << options.values[i];

        if (i + 1 < options.values.size()) {
            oss << '=' << options.values[i + 1];
        }
        else {
            oss << "=<missing>";
        }

        if (i + 2 < options.values.size()) {
            oss << ", ";
        }
    }

    oss << "> ";
}

void Loggr::write_file(
    const std::string& message,
    const std::string& module
)
{
    std::lock_guard<std::mutex> lock(fileMutex);

    switch (fileMode) {
        case FileMode::SingleFile:
            write_global_file(message);
            break;

        case FileMode::PerModule:
            write_module_file(message, module);
            break;

        case FileMode::SingleFileAndPerModule:
            write_global_file(message);
            write_module_file(message, module);
            break;
    }
}

void Loggr::write_global_file(const std::string& message)
{
    if (file.is_open()) {
        file << message;
    }
}

void Loggr::write_module_file(
    const std::string& message,
    const std::string& module
)
{
    const std::filesystem::path moduleDir =
        std::filesystem::path(basepath) /
        dirname /
        "modules";

    if (!std::filesystem::exists(moduleDir)) {
        std::filesystem::create_directories(moduleDir);
    }

    const std::string safeModule =
        sanitize_module_filename(module);

    const std::filesystem::path modulePath =
        moduleDir / (safeModule + ".log");

    auto it = moduleFiles.find(safeModule);

    if (it == moduleFiles.end()) {
        std::ofstream stream(
            modulePath,
            append ? std::ios::app : std::ios::out
        );

        it = moduleFiles
            .emplace(safeModule, std::move(stream))
            .first;
    }

    if (it->second.is_open()) {
        it->second << message;
    }
}

void Loggr::write_console(
    const std::string& message,
    Level level
)
{
    if (!coutEnabled) {
        return;
    }

    if (coloring) {
        const std::string color =
            "\033[" +
            std::to_string(get_color(level)) +
            "m";

        const std::string reset = "\033[0m";

        std::cout << color << message << reset;
    }
    else {
        std::cout << message;
    }
}

std::string Loggr::sanitize_module_filename(
    const std::string& module
) const
{
    std::string result;

    for (char c : module) {
        if (
            (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            c == '_' ||
            c == '-'
        ) {
            result.push_back(c);
        }
        else {
            result.push_back('_');
        }
    }

    if (result.empty()) {
        return "UNKNOWN";
    }

    return result;
}

Loggr::Color Loggr::get_color(Level level) const
{
    switch (level) {
        case TRACE: return GREY;
        case DEBUG: return CYAN;
        case INFO:  return GREEN;
        case WARN:  return YELLOW;
        case ERROR: return BOLD_RED;
        case FATAL: return RED;
        default:    return DEFAULT;
    }
}

} // namespace moody
