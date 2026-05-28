#pragma once

/**
 * @file Loggr.hpp
 * @brief Public API for Moody Loggr.
 */

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <fstream>
#include <map>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <filesystem>

#define CONSTRUCT_LOG(object, level, module, msg, ...) \
    (object).log(level, module, msg, {__FILE__, __LINE__}, ##__VA_ARGS__)

#define LOG_TRACE(object, module, msg, ...) \
    CONSTRUCT_LOG(object, moody::Loggr::Level::TRACE, module, msg, ##__VA_ARGS__)

#define LOG_DEBUG(object, module, msg, ...) \
    CONSTRUCT_LOG(object, moody::Loggr::Level::DEBUG, module, msg, ##__VA_ARGS__)

#define LOG_INFO(object, module, msg, ...) \
    CONSTRUCT_LOG(object, moody::Loggr::Level::INFO, module, msg, ##__VA_ARGS__)

#define LOG_WARN(object, module, msg, ...) \
    CONSTRUCT_LOG(object, moody::Loggr::Level::WARN, module, msg, ##__VA_ARGS__)

#define LOG_ERROR(object, module, msg, ...) \
    CONSTRUCT_LOG(object, moody::Loggr::Level::ERROR, module, msg, ##__VA_ARGS__)

#define LOG_FATAL(object, module, msg, ...) \
    CONSTRUCT_LOG(object, moody::Loggr::Level::FATAL, module, msg, ##__VA_ARGS__)

namespace moody {

/**
 * @brief Lightweight asynchronous logger.
 */
class Loggr {
public:
    /**
     * @brief Logging severity levels.
     */
    enum Level : uint8_t {
        TRACE = 0,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        OFF
    };

    /**
     * @brief File output strategy.
     */
    enum class FileMode : uint8_t {
        SingleFile,             ///< Write only to the main log file.
        PerModule,              ///< Write only to module-specific log files.
        SingleFileAndPerModule  ///< Write to both the main log and module logs.
    };

    /**
     * @brief ANSI terminal color codes.
     */
    enum Color : uint8_t {
        DEFAULT  = 39,
        GREY     = 90,
        GREEN    = 32,
        CYAN     = 36,
        YELLOW   = 33,
        RED      = 31,
        BOLD_RED = 91
    };

    /**
     * @brief Source file metadata.
     */
    struct FileSourceInfo {
        std::string filename{};
        int line{};
    };

    /**
     * @brief Optional key/value variables.
     */
    struct LogOptions {
        std::vector<std::string> values{};
    };

    /**
     * @brief Internal queued log event.
     */
    struct LogEvent {
        Level level{};
        std::string module{};
        std::string message{};
        FileSourceInfo source{};
        LogOptions options{};
        std::chrono::system_clock::time_point timestamp{};
    };

    /**
     * @brief Create logger instance.
     *
     * @param basepath Root logging directory.
     * @param dirname Project/application directory.
     * @param filename Main output log filename.
     * @param coutEnabled Enable console output.
     * @param append Append to existing files instead of overwriting.
     * @param coloring Enable ANSI console colors.
     * @param milliseconds Enable millisecond timestamp precision.
     */
    explicit Loggr(
        const std::string& basepath,
        const std::string& dirname,
        const std::string& filename,
        bool coutEnabled = false,
        bool append = false,
        bool coloring = false,
        bool milliseconds = false
    );

    /**
     * @brief Shutdown logger safely.
     */
    ~Loggr();

    Loggr(const Loggr&) = delete;
    Loggr& operator=(const Loggr&) = delete;
    Loggr(Loggr&&) = delete;
    Loggr& operator=(Loggr&&) = delete;

    /**
     * @brief Set minimum accepted log level.
     */
    void set_level(Level level);

    /**
     * @brief Enable or disable millisecond timestamps.
     */
    void set_milliseconds(bool enabled);

    /**
     * @brief Set file output mode.
     */
    void set_file_mode(FileMode mode);

    /**
     * @brief Flush active sinks manually.
     *
     * @threadsafe
     */
    void flush();

    /**
     * @brief Queue log message asynchronously.
     *
     * @threadsafe
     */
    void log(
        Level level,
        const std::string& module,
        const std::string& msg,
        const FileSourceInfo& src
    );

    /**
     * @brief Queue log message asynchronously with optional key/value variables.
     *
     * @threadsafe
     */
    template <typename... Args>
    void log(
        Level level,
        const std::string& module,
        const std::string& msg,
        const FileSourceInfo& src,
        Args&&... args
    )
    {
        if (level < minLevel) {
            return;
        }

        LogOptions optionals;

        (
            optionals.values.push_back(
                stringify(std::forward<Args>(args))
            ), ...
        );

        LogEvent event;
        event.level = level;
        event.module = module;
        event.message = msg;
        event.source = src;
        event.options = std::move(optionals);
        event.timestamp = std::chrono::system_clock::now();

        push(std::move(event));
    }

private:
    void worker();
    void push(LogEvent event);

    std::string format(const LogEvent& event);

    void format_timestamp(
        std::ostringstream& oss,
        const std::chrono::system_clock::time_point& timestamp
    );

    void format_level(std::ostringstream& oss, Level level);
    void format_module(std::ostringstream& oss, const std::string& module);
    void format_source(std::ostringstream& oss, const FileSourceInfo& src);
    void format_message(std::ostringstream& oss, const std::string& msg);
    void format_options(std::ostringstream& oss, const LogOptions& options);

    void write_file(const std::string& message, const std::string& module);
    void write_global_file(const std::string& message);
    void write_module_file(const std::string& message, const std::string& module);
    void write_console(const std::string& message, Level level);

    std::string sanitize_module_filename(const std::string& module) const;

    template <typename T>
    std::string stringify(T&& value)
    {
        std::ostringstream oss;
        oss << value;
        return oss.str();
    }

    Color get_color(Level level) const;

private:
    std::queue<LogEvent> inbox;

    std::thread workerThread;

    std::mutex mutex;
    std::condition_variable cv;
    std::atomic<bool> running{false};

    std::ofstream file;
    std::map<std::string, std::ofstream> moduleFiles;
    std::mutex fileMutex;

    std::string basepath;
    std::string dirname;
    std::string filename;

    bool coutEnabled{false};
    bool append{false};
    bool coloring{false};
    bool milliseconds{false};

    FileMode fileMode{FileMode::SingleFile};

    Level minLevel{TRACE};
};

} // namespace moody
