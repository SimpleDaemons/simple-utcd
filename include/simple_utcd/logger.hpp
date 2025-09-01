#pragma once

#include <string>
#include <memory>
#include <fstream>
#include <mutex>

namespace simple_utcd {

enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3
};

class Logger {
public:
    Logger();
    ~Logger();
    
    void set_level(LogLevel level);
    void set_log_file(const std::string& filename);
    void enable_console(bool enable);
    void enable_syslog(bool enable);
    
    void debug(const std::string& message);
    void info(const std::string& message);
    void warn(const std::string& message);
    void error(const std::string& message);
    
    template<typename... Args>
    void debug(const std::string& format, Args&&... args) {
        log(LogLevel::DEBUG, format, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void info(const std::string& format, Args&&... args) {
        log(LogLevel::INFO, format, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void warn(const std::string& format, Args&&... args) {
        log(LogLevel::WARN, format, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void error(const std::string& format, Args&&... args) {
        log(LogLevel::ERROR, format, std::forward<Args>(args)...);
    }

private:
    LogLevel current_level_;
    std::string log_file_;
    std::unique_ptr<std::ofstream> file_stream_;
    bool console_enabled_;
    bool syslog_enabled_;
    std::mutex log_mutex_;
    
    void log(LogLevel level, const std::string& message);
    
    template<typename... Args>
    void log(LogLevel level, const std::string& format, Args&&... args) {
        // Simple string formatting - in a real implementation, you'd use fmt or similar
        std::string message = format; // Placeholder for actual formatting
        log(level, message);
    }
    
    std::string level_to_string(LogLevel level);
    std::string get_timestamp();
};

} // namespace simple_utcd
