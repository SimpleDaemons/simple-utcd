/*
 * includes/simple_utcd/logger.hpp
 *
 * Copyright 2024 SimpleDaemons
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <string>
#include <memory>
#include <fstream>
#include <mutex>
#include <sstream>
#include <type_traits>
#include <utility>

namespace simple_utcd {

// winsock2.h defines ERROR as a macro. Keep the enumerator name.
#pragma push_macro("ERROR")
#undef ERROR
enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3
};
#pragma pop_macro("ERROR")

class Logger {
public:
    Logger();
    ~Logger();

    void set_level(LogLevel level);
    void set_log_file(const std::string& filename);
    void enable_console(bool enable);
    void enable_syslog(bool enable);
    void set_json_format(bool enable);
    bool is_json_format() const { return json_format_; }
    
    // Log rotation
    void set_max_log_size(size_t max_size_bytes);
    void set_max_log_files(size_t max_files);
    void enable_log_rotation(bool enable);
    bool should_rotate_log() const;
    void rotate_log();

    void debug(const std::string& message);
    void info(const std::string& message);
    void warn(const std::string& message);
    void error(const std::string& message);

    template<typename... Args>
    void debug(const std::string& format, Args&&... args) {
        log(LogLevel::DEBUG, render(format, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void info(const std::string& format, Args&&... args) {
        log(LogLevel::INFO, render(format, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void warn(const std::string& format, Args&&... args) {
        log(LogLevel::WARN, render(format, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void error(const std::string& format, Args&&... args) {
        log(LogLevel::ERROR, render(format, std::forward<Args>(args)...));
    }

private:
    LogLevel current_level_;
    std::string log_file_;
    std::unique_ptr<std::ofstream> file_stream_;
    bool console_enabled_;
    bool syslog_enabled_;
    bool json_format_;
    bool log_rotation_enabled_;
    size_t max_log_size_bytes_;
    size_t max_log_files_;
    size_t current_log_size_;
    std::mutex log_mutex_;

    void log(LogLevel level, const std::string& message);

    template<typename T>
    static void append_formatted(std::string& message, T&& value) {
        std::ostringstream rendered;
        rendered << std::forward<T>(value);
        const auto pos = message.find("{}");
        if (pos == std::string::npos) {
            if (!message.empty() && message.back() != ' ') {
                message.push_back(' ');
            }
            message += rendered.str();
            return;
        }
        message.replace(pos, 2, rendered.str());
    }

    template<typename T, typename... Rest>
    static void append_formatted(std::string& message, T&& value, Rest&&... rest) {
        append_formatted(message, std::forward<T>(value));
        append_formatted(message, std::forward<Rest>(rest)...);
    }

    template<typename... Args>
    static std::string render(const std::string& format, Args&&... args) {
        std::string message = format;
        append_formatted(message, std::forward<Args>(args)...);
        return message;
    }

    std::string level_to_string(LogLevel level);
    std::string get_timestamp();
    std::string format_json_log(LogLevel level, const std::string& message);
};

} // namespace simple_utcd
