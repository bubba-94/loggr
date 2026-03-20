#pragma once

// Output
#include <iostream>
#include <cstdint>
#include <string_view>
#include <queue>
#include <iomanip>
#include <vector>
#include <optional>
#include <map>
#include <utility>

// Files
#include <fstream>
#include <sstream>
#include <filesystem>

// Timing
#include <ctime>
#include <chrono>

// Thread safe
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>

#define CONSTRUCT_LOG(object, level, module, msg, ...) \
    (object).log(level, module, msg, {__FILE__, __LINE__}, ##__VA_ARGS__)

#define LOG_INFO(object, module, msg, ...) \
    CONSTRUCT_LOG(loggr, moody::Loggr::Level::INFO, module, msg, ##__VA_ARGS__)
    
#define LOG_TRACE(object, module, msg, ...) \
    CONSTRUCT_LOG(loggr, moody::Loggr::Level::TRACE, module, msg, ##__VA_ARGS__)
    
#define LOG_DEBUG(object, module, msg, ...) \
    CONSTRUCT_LOG(loggr, moody::Loggr::Level::DEBUG, module, msg, ##__VA_ARGS__)
    
#define LOG_WARN(object, module, msg, ...) \
    CONSTRUCT_LOG(loggr, moody::Loggr::Level::WARN, module, msg, ##__VA_ARGS__)
    
#define LOG_ERROR(object, module, msg, ...) \
    CONSTRUCT_LOG(loggr, moody::Loggr::Level::ERROR, module, msg, ##__VA_ARGS__)
    
#define LOG_FATAL(object, module, msg, ...) \
    CONSTRUCT_LOG(loggr, moody::Loggr::Level::FATAL, module, msg, ##__VA_ARGS__)

namespace moody{    
    class Loggr{
        public:
        
    // Map with Color
    enum Level : uint8_t {
        TRACE = 0,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        OFF
    };
    
    // Match ascii colors
    enum Color : uint8_t {
        DEFAULT     = 39,
        GREY        = 90,
        GREEN       = 32,
        CYAN        = 36,
        YELLOW      = 33,
        RED         = 31,
        BOLD_RED    = 91
    };
    
    // Push LogMessage to queue
    struct LogMessage{
        std::string message{};
        Level level{};
    };
    
    struct FileSourceInfo{
        std::string filename{};
        const int line{};
    };
    
    struct LogOptions{
        std::vector<std::string> values{};
    };

    /**
     * @brief Construct a new Loggr object
     * 
     * @details 
     * Creates an independent thread that is waiting for updates in queue.
     * 
     * @param basepath root dir
     * @param dirname project name
     * @param filename file output name
     * @param cout write to console or not, default == false;
     * @param append overwrite or not, default == false
     * @param coloring coloring or not to cout, default == false;
     */
    explicit Loggr( const std::string& basepath, 
                    const std::string& dirname,
                    const std::string& filename,
                    bool cout = false,
                    bool append = false,
                    bool coloring = false): 
                    m_basepath{basepath}, m_dirname{dirname}, m_cout{cout}, m_append{append}, m_coloring{coloring}
                    {
                        std::filesystem::path fullpath = m_basepath / m_dirname / filename;

                        if (!std::filesystem::exists(fullpath.parent_path())) {
                            std::filesystem::create_directories(fullpath.parent_path());
                        }

                        m_file.open(fullpath, m_append ? std::ios::app : std::ios::out);

                        // Start thread
                        m_running = true;
                        m_worker = std::thread(&Loggr::worker, this);
                    }

    ~Loggr(){
        {
            std::unique_lock<std::mutex>lock(m_mutex);
            m_running = false;
        }

        m_cv.notify_one();

        if(m_worker.joinable()){
            m_worker.join();
        }
    }

    // Support no variable arguments (USED FOR CONSOLE RUNTIME OUTPUT)
    void log(const Level level, const std::string& module, const std::string& msg, const FileSourceInfo& src){
        
        LogMessage log = construct(level, module, msg, src);

        // Push to queue
        push(log);
    }

    // Support multiple arguments
    template <typename... Args>
    void log(const Level level, const std::string& module, const std::string& msg, const FileSourceInfo& src, Args&&... args){
        // Create a string of address
        
        LogOptions log_optionals {};

        ((log_optionals.values.push_back(
            stringify(std::forward<Args>(args))
        )),... );

        LogMessage log = construct(level, module, msg, src, log_optionals);

        // Push to queue
        push(log);
    }

// Private methods
private:
    LogMessage construct(const Level level, const std::string& module, const std::string& output, const FileSourceInfo& src){
        LogMessage log;
        return info(level, module, output, src);
    }

    LogMessage construct(const Level level, const std::string& module, const std::string& output, const FileSourceInfo& src, const LogOptions& optionals){
        LogMessage log;

        switch (level){

            case INFO:     return log = info (level, module, output, src);
            case TRACE:    return log = trace(level, module, output, src, optionals);
            case DEBUG:    return log = debug(level, module, output, src, optionals);
            case WARN:     return log = warn (level, module, output, src, optionals);
            case ERROR:    return log = error(level, module, output, src, optionals);
            case FATAL:    return log = fatal(level, module, output, src, optionals);

            default:       return log = debug(level, module, output, src, optionals);
        }
    }

    LogMessage info(const Level level, const std::string& module, const std::string& output, const FileSourceInfo& src){
        LogMessage temp_log;
        temp_log.level = level;

        std::ostringstream oss;

        oss << timestamp()
            << toStr(level)
            << format_module(module)
            << format_filedesc(src)
            << format_msg(output)
            << "\n";

        temp_log.message = oss.str();

        if (m_cout && m_coloring){
            const std::string color = "\033[" + std::to_string(getColor(level)) + "m";
            const std::string reset = "\033[0m";
            std::cout << color << temp_log.message << reset;
        }
        else if(m_cout && !m_coloring){
            std::cout << temp_log.message;
        }

        // Wrapper with mutex
        return temp_log;
    }

    LogMessage trace(const Level level, const std::string& module, const std::string &output, const FileSourceInfo& src , const LogOptions& optionals){
        LogMessage temp_log;
        temp_log.level = level;

        std::ostringstream oss;
        
        oss << timestamp()
            << toStr(level)
            << format_module(module)
            << format_filedesc(src)
            << format_opts(optionals)
            << format_msg(output)
            << "\n";

        // Constructed message from stream
        temp_log.message = oss.str();

        if (m_cout && m_coloring){
            const std::string color = "\033[" + std::to_string(getColor(level)) + "m";
            const std::string reset = "\033[0m";
            std::cout << color << temp_log.message << reset;
        }
        else if(m_cout && !m_coloring){
            std::cout << temp_log.message;
        }

        // Wrapper with mutex
        return temp_log;
    }
    LogMessage debug(const Level level, const std::string& module, const std::string &output, const FileSourceInfo& src, const LogOptions& optionals){
        LogMessage temp_log;
        temp_log.level = level;
        
        std::ostringstream oss;
        
        oss << timestamp()
            << toStr(level)
            << format_module(module)
            << format_filedesc(src)
            << format_opts(optionals)
            << format_msg(output)
            << "\n";

        // Constructed message from stream
        temp_log.message = oss.str();

        if (m_cout && m_coloring){
            const std::string color = "\033[" + std::to_string(getColor(level)) + "m";
            const std::string reset = "\033[0m";
            std::cout << color << temp_log.message << reset;
        }
        else if(m_cout && !m_coloring){
            std::cout << temp_log.message;
        }

        // Wrapper with mutex
        return temp_log;
    }
    LogMessage warn(const Level level, const std::string& module, const std::string &output, const FileSourceInfo& src, const LogOptions& optionals){
        LogMessage temp_log;
        temp_log.level = level;

        std::ostringstream oss;
        
        oss << timestamp()
            << toStr(level)
            << format_module(module)
            << format_filedesc(src)
            << format_opts(optionals)
            << format_msg(output)
            << "\n";

        // Constructed message from stream
        temp_log.message = oss.str();

        if (m_cout && m_coloring){
            const std::string color = "\033[" + std::to_string(getColor(level)) + "m";
            const std::string reset = "\033[0m";
            std::cout << color << temp_log.message << reset;
        }
        else if(m_cout && !m_coloring){
            std::cout << temp_log.message;
        }

        // Wrapper with mutex
        return temp_log;
    }
    LogMessage error(const Level level, const std::string& module, const std::string &output, const FileSourceInfo& src, const LogOptions& optionals){
        LogMessage temp_log;
        temp_log.level = level;
        
        std::ostringstream oss;
        
        oss << timestamp()
            << toStr(level)
            << format_module(module)
            << format_filedesc(src)
            << format_opts(optionals)
            << format_msg(output)
            << "\n";

        // Constructed message from stream
        temp_log.message = oss.str();

        if (m_cout && m_coloring){
            const std::string color = "\033[" + std::to_string(getColor(level)) + "m";
            const std::string reset = "\033[0m";
            std::cout << color << temp_log.message << reset;
        }
        else if(m_cout && !m_coloring){
            std::cout << temp_log.message;
        }

        // Wrapper with mutex
        return temp_log;
    }
    LogMessage fatal(const Level level, const std::string& module, const std::string &output, const FileSourceInfo& src, const LogOptions& optionals){
        LogMessage temp_log;
        temp_log.level = level;
        
        std::ostringstream oss;
        
        oss << timestamp()
            << toStr(level)
            << format_module(module)
            << format_filedesc(src)
            << format_opts(optionals)
            << format_msg(output)
            << "\n";

        // Constructed message from stream
        temp_log.message = oss.str();

        if (m_cout && m_coloring){
            const std::string color = "\033[" + std::to_string(getColor(level)) + "m";
            const std::string reset = "\033[0m";
            std::cout << color << temp_log.message << reset;
        }
        else if(m_cout && !m_coloring){
            std::cout << temp_log.message;
        }

        // Wrapper with mutex
        return temp_log;
    }

    void worker(){
        std::unique_lock<std::mutex>lock(m_mutex);
        while (m_running){
            // Wait for cv
            m_cv.wait(lock, [this] {return !m_inbox.empty() || !m_running;});

            while(!m_inbox.empty()){
                // Read first message
                lock.unlock();
                m_file << m_inbox.front().message;
                lock.lock();

                m_inbox.pop();
            }
        }
    }

    void push(const LogMessage log_msg){
        {
            std::lock_guard<std::mutex>lock(m_mutex);
            m_inbox.push(log_msg);
        }
        m_cv.notify_one();
    }

    template <typename T>
    std::string stringify(T&& value){
        std::ostringstream oss;
        oss << value << " ";
        return oss.str();
    }

    std::string format_opts(const LogOptions& optionals){
        
        std::ostringstream oss;
        std::string prefix = "var:{";
        std::string suffix = "}";
        std::string midfix = ":";

        // Add prefix 
        oss << prefix;

        if (optionals.values.size() > 1){
            for (size_t i = 0; i < optionals.values.size(); ++i){
                if (i % 2 == 0){
                    oss << optionals.values[i];
                }
                else oss << midfix << optionals.values[i]; 
            }
            oss << suffix; 
        }
        else if (optionals.values.size() <= 1){
            for (size_t i = 0; i < optionals.values.size(); ++i){
              oss << optionals.values[i];
            }
            oss << suffix;
        }

        return oss.str();
    }
    std::string format_module(const std::string& module_str){
        int min_width = 12;
        size_t size = module_str.size();
        int calc_width = (min_width - size) / 2;

        std::ostringstream oss;
        std::cout << size;

        std::string prefix = "[";
        std::string suffix = "]";

        // Format string
        oss << prefix << std::setw(8) << module_str << suffix << " ";

        return oss.str();
    }
    std::string format_filedesc(const FileSourceInfo& src){
        std::ostringstream oss;
        oss << "in:" << src.filename << "(" << std::to_string(src.line) << ") ";
        return oss.str();
    }

    std::string format_msg(const std::string& output){
        std::ostringstream oss;
        std::string prefix = "msg:{";
        std::string suffix = "}";
        oss << prefix << output << suffix;
        return oss.str();
    }

    std::string timestamp() const {
        std::ostringstream stream;
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);

        stream << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S") << " ";

        return stream.str();
    }

    std::string_view toStr(const Level level){
        switch(level){
            case TRACE:     return "[TRACE] ";
            case DEBUG:     return "[DEBUG] ";
            case INFO:      return "[ INFO] ";
            case WARN:      return "[ WARN] ";
            case ERROR:     return "[ERROR] ";
            case FATAL:     return "[FATAL] ";
            default:        return "[-----] ";
        }
    }

    Color getColor(const Level level) const {
        switch (level){
            case TRACE:     return Color::GREY;
            case DEBUG:     return Color::CYAN;
            case INFO:      return Color::GREEN;
            case WARN:      return Color::YELLOW;
            case ERROR:     return Color::BOLD_RED;
            case FATAL:     return Color::RED;
            default:        return Color::DEFAULT;
        }
    }

// Member variables
private:

    // Threading
    std::queue<LogMessage> m_inbox;
    std::thread m_worker;
    std::atomic<bool> m_running {false};
    std::condition_variable m_cv;
    std::mutex m_mutex {};

    // File paths
    std::filesystem::path m_basepath {};
    std::filesystem::path m_dirname {};
    std::ofstream m_file{};

    // Logger configuration
    bool m_cout {false}; 
    bool m_append {false};
    bool m_coloring {false};
}; // class Loggr
}; // namespace Moody