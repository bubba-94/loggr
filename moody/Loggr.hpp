#pragma once

// Output
#include <iostream>
#include <cstdint>
#include <string_view>
#include <queue>
#include <iomanip>
#include <array>
#include <optional>

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

    struct LogOptions{
        std::string filename{};
        const int line{};
        
        // Initialized in Log when T is present
        const void* var_address{};
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
                        std::cout << "Thread " << m_worker.get_id() << " constructed\n";
                    }

    ~Loggr(){
        {
            std::unique_lock<std::mutex>lock(m_mutex);
            m_running = false;
            std::cout << "Thread " << m_worker.get_id() << " destroyed\n";
        }

        m_cv.notify_one();

        if(m_worker.joinable()){
            std::cout << "Thread " << m_worker.get_id() << " joined\n";
            m_worker.join();
        }
    }

    void log(const Level level, const std::string& module, const std::string& msg, const LogOptions& opts){

        LogOptions log_opts = opts;
        LogMessage log = construct(level, module, msg, log_opts);

        // Push to queue
        push(log);
    }

    template <typename T>
    void log(const Level level, const std::string& module, const std::string& msg, const T& variable, const LogOptions& opts){
        // Create a string of address
        LogOptions log_opts = opts;

        log_opts.var_address = static_cast<const void*>(variable);

        LogMessage log = construct(level, module, msg, log_opts);

        // Push to queue
        push(log);
    }

// Private methods
private:
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

    LogMessage construct(const Level level, const std::string& module, const std::string& output, const LogOptions& opts){
        LogMessage log;

        switch (level){

            case TRACE:    return log = trace(level, module, output, opts);
            case DEBUG:    return log = debug(level, module, output, opts);
            case INFO:     return log = info (level, module, output, opts);
            case WARN:     return log = warn (level, module, output, opts);
            case ERROR:    return log = error(level, module, output, opts);
            case FATAL:    return log = fatal(level, module, output, opts);

            default:       return log = debug(level, module, output, opts);
        }
    }

    // All functions should create one message and later decide if printed to console or not
    LogMessage info(const Level level,const std::string& module, const std::string& output, const LogOptions& opts){
        LogMessage temp_log;
        if(m_cout){
            LogMessage console;
            if(m_coloring && m_cout){
                const std::string ascii = " \033[" + std::to_string(getColor(level)) + "m";
                console.message += ascii;
            }
            console.level = level;
            console.message += timestamp();
            console.message += toStr(level);
            console.message += " [" + module + "]";
            console.message += " log:";

            console.message += output;
            console.message += "\n";
            // Reset coloring
            console.message += "\033[0m";
            std::cout << console.message;
        }
        
        temp_log.level = level;
        temp_log.message += timestamp();
        temp_log.message += toStr(level);
        temp_log.message += " [" + module + "]";
        temp_log.message += " log:";
        temp_log.message += output;
        temp_log.message += "\n";
        // Reset coloring

        // Wrapper with mutex
        return temp_log;
    }

    LogMessage trace(const Level level, const std::string& module, const std::string &output, const LogOptions& opts){
        LogMessage temp_log;
        std::ostringstream oss;
        oss << opts.var_address;

        if(m_cout){
            LogMessage console;
            if(m_coloring && m_cout){
                const std::string ascii = " \033[" + std::to_string(getColor(level)) + "m";
                console.message += ascii;
            }
            console.level = level;
            console.message += timestamp();
            console.message += toStr(level);
            console.message += " [" + module + "] ";
            console.message += " file:" + opts.filename + "(" + std::to_string(opts.line) + ")";
            console.message += " @" + oss.str();
            console.message += " log:";
            console.message += output;
            console.message += "\n";
            // Reset coloring
            console.message += "\033[0m";
            std::cout << console.message;
        }

        if (m_cout){
            if(m_coloring){
                const std::string ascii = " \033[" + std::to_string(getColor(level)) + "m";
                temp_log.message += ascii;
            }
        }
        
        temp_log.level = level;
        temp_log.message += timestamp();
        temp_log.message += toStr(level);
        temp_log.message += " [" + module + "] ";
        temp_log.message += " file:" + opts.filename+ "(" + std::to_string(opts.line) + ")";        
        temp_log.message += " @" + oss.str();
        temp_log.message += " log:";
        temp_log.message += output;
        temp_log.message += "\n";
        // Reset coloring

        // Wrapper with mutex
        return temp_log;
    }
    LogMessage debug(const Level level, const std::string& module, const std::string &output, const LogOptions& opts){
        LogMessage temp_log;
        return temp_log;
    }
    LogMessage warn(const Level level, const std::string& module, const std::string &output, const LogOptions& opts){
        LogMessage temp_log;
        return temp_log;
    }
    LogMessage error(const Level level, const std::string& module, const std::string &output, const LogOptions& opts){
        LogMessage temp_log;
        return temp_log;
    }
    LogMessage fatal(const Level level, const std::string& module, const std::string &output, const LogOptions& opts){
        LogMessage temp_log;
        return temp_log;
    }

    void push(const LogMessage log_msg){
        {
            std::lock_guard<std::mutex>lock(m_mutex);
            m_inbox.push(log_msg);
        }
        m_cv.notify_one();
    }

    void print(LogMessage console_message){

    }

    std::string timestamp() const {
        std::ostringstream stream;
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);

        stream << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S") << " |";

        return stream.str();
    }

    std::string_view toStr(const Level level){
        switch(level){
            case TRACE:     return " [TRACE] |";
            case DEBUG:     return " [DEBUG] |";
            case INFO:      return " [INFO]  |";
            case WARN:      return " [WARN]  |";
            case ERROR:     return " [ERROR] |";
            case FATAL:     return " [FATAL] |";
            default:        return " [-----] |";
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
    std::queue<LogMessage> m_inbox;
    std::thread m_worker;
    std::atomic<bool> m_running {false};
    std::condition_variable m_cv;

    // File paths
    std::filesystem::path m_basepath {};
    std::filesystem::path m_dirname {};
    std::ofstream m_file{};

    // Logger configuration
    bool m_cout {false}; 
    bool m_append {false};
    bool m_coloring {false};

    // Threading
    std::mutex m_mutex {};
}; // class Loggr
}; // namespace Moody