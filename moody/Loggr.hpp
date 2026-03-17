#pragma once

// Output
#include <iostream>
#include <cstdint>
#include <string_view>
#include <queue>

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

namespace moody{
class Loggr{
public:
    enum class LevelSeverity : uint8_t {DEBUG = 0, INFO, WARNING, ERROR, FATAL};
    enum class LevelColor: uint8_t {GREY = 0, BLUE, GREEN, YELLOW, RED};

    struct LogMessage{
        std::string message;
        LevelSeverity sev;
        LevelColor color;
    };

    /**
     * @brief Construct a new Loggr object
     * 
     * @param basepath root dir
     * @param dirname project name
     * @param filename file output name
     * @param cout write to console or not, default == false;
     * @param append overwrite or not, default == false
     */
    explicit Loggr(  const std::string& basepath, 
                    const std::string& dirname,
                    const std::string& filename,
                    bool cout = false, 
                    bool append = false): 
                    m_basepath{basepath}, m_dirname{dirname}, m_cout{cout}, m_append{append}
                    {
                        std::filesystem::path fullpath = m_basepath / m_dirname / filename;

                        if (!std::filesystem::exists(fullpath.parent_path())) {
                            std::filesystem::create_directories(fullpath.parent_path());
                        }

                        m_file.open(fullpath, m_append ? std::ios::app : std::ios::out);
                    }

    ~Loggr(){
    }
// Public methods                                 
public:
    void msg(const std::string& str, LevelSeverity sev){
        // Construct a LogMessage
        LogMessage msg;
        msg.message += timestamp();
        msg.message += logLevel(sev);
        msg.message += str;
        msg.sev = sev;
        msg.color = getColor(sev);
        msg.message += "\n";

        // Wrapper with mutex
        push(msg);
    }

    void write(){
        std::lock_guard<std::mutex>lock(m_mutex);
        if (m_inbox.empty()){
            return;
        }

        // Write all logs
        while(!m_inbox.empty()){

            // Read first message
            m_file << m_inbox.front().message;

            m_inbox.pop();
        }
    }
    
    void push(const LogMessage log_msg){
        std::lock_guard<std::mutex>lock(m_mutex);
        m_inbox.push(log_msg);
    }

// Private methods
private:
    std::string timestamp() const {
        std::ostringstream stream;
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);

        stream << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S") << " ";

        return stream.str();
    }
    std::string_view logLevel(const LevelSeverity sev) const {
        switch (sev){
            case LevelSeverity::DEBUG: return "[DEBUG] ";
            case LevelSeverity::INFO:   return "[INFO] ";
            case LevelSeverity::WARNING: return "[WARNING] ";
            case LevelSeverity::ERROR: return "[ERROR] ";
            case LevelSeverity::FATAL: return "[FATAL] ";
        }
        
        return "[UNKNOWN]";
    }
    LevelColor getColor(const LevelSeverity sev) const {
        switch (sev){
            case LevelSeverity::DEBUG: return LevelColor::GREY;
            case LevelSeverity::INFO:   return LevelColor::BLUE;
            case LevelSeverity::WARNING: return LevelColor::GREEN;
            case LevelSeverity::ERROR: return LevelColor::YELLOW;
            case LevelSeverity::FATAL: return LevelColor::RED;
        }
        return LevelColor::GREY;
    }

// Member variables
private:
    std::queue<LogMessage> m_inbox;
    std::thread m_worker;

    // File paths
    std::filesystem::path m_basepath {};
    std::filesystem::path m_dirname {};
    std::ofstream m_file{};

    // Logger configuration
    bool m_cout {false}; 
    bool m_append {false};

    // Threading
    std::mutex m_mutex {};
}; // class Loggr
}; // namespace Moody