/*
 * src/core/error_handler.cpp
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

#include "simple_utcd/error_handler.hpp"
#include "simple_utcd/logger.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>

namespace simple_utcd {

ErrorContext::ErrorContext(const std::string& comp, const std::string& func,
                          const std::string& f, int l, const std::string& desc,
                          ErrorSeverity sev)
    : component(comp)
    , function(func)
    , file(f)
    , line(l)
    , description(desc)
    , severity(sev)
{
    // Generate timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto tm = *std::localtime(&time_t);

    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    timestamp = ss.str();
}

DefaultErrorHandler::DefaultErrorHandler(bool enable_logging, ErrorSeverity min_log_level)
    : logging_enabled_(enable_logging)
    , min_log_level_(min_log_level)
    , error_counts_(4, 0) // Initialize for 4 severity levels
{
}

void DefaultErrorHandler::handle_error(const ErrorContext& context, const std::exception* exception) {
    // Update error statistics
    if (static_cast<int>(context.severity) < static_cast<int>(ErrorSeverity::CRITICAL) + 1) {
        error_counts_[static_cast<int>(context.severity)]++;
    }

    // Log error if enabled and meets minimum level
    if (logging_enabled_ && should_log(context.severity)) {
        log_error(context, exception);
    }

    // For critical errors, also output to stderr
    if (context.severity == ErrorSeverity::CRITICAL) {
        std::cerr << "CRITICAL ERROR: " << context.description
                  << " in " << context.component << "::" << context.function
                  << " at " << context.file << ":" << context.line << std::endl;
    }
}

bool DefaultErrorHandler::should_log(ErrorSeverity severity) const {
    return static_cast<int>(severity) >= static_cast<int>(min_log_level_);
}

std::vector<std::pair<ErrorSeverity, size_t>> DefaultErrorHandler::get_error_stats() const {
    std::vector<std::pair<ErrorSeverity, size_t>> stats;
    stats.reserve(4);

    stats.emplace_back(ErrorSeverity::INFO, error_counts_[0]);
    stats.emplace_back(ErrorSeverity::WARNING, error_counts_[1]);
    stats.emplace_back(ErrorSeverity::ERROR, error_counts_[2]);
    stats.emplace_back(ErrorSeverity::CRITICAL, error_counts_[3]);

    return stats;
}

void DefaultErrorHandler::reset_stats() {
    std::fill(error_counts_.begin(), error_counts_.end(), 0);
}

void DefaultErrorHandler::set_min_log_level(ErrorSeverity level) {
    min_log_level_ = level;
}

void DefaultErrorHandler::set_logging_enabled(bool enable) {
    logging_enabled_ = enable;
}

void DefaultErrorHandler::log_error(const ErrorContext& context, const std::exception* exception) {
    std::stringstream ss;
    ss << "[" << context.timestamp << "] "
       << severity_to_string(context.severity) << ": "
       << context.component << "::" << context.function
       << " (" << context.file << ":" << context.line << ") - "
       << context.description;

    if (exception) {
        ss << " - Exception: " << exception->what();
    }

    // For now, output to console. In a full implementation, this would
    // integrate with the logging system
    std::cout << ss.str() << std::endl;
}

std::string DefaultErrorHandler::severity_to_string(ErrorSeverity severity) const {
    switch (severity) {
        case ErrorSeverity::INFO: return "INFO";
        case ErrorSeverity::WARNING: return "WARNING";
        case ErrorSeverity::ERROR: return "ERROR";
        case ErrorSeverity::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

// Static member initialization
std::unique_ptr<ErrorHandler> ErrorHandlerManager::handler_ = nullptr;

void ErrorHandlerManager::set_handler(std::unique_ptr<ErrorHandler> handler) {
    handler_ = std::move(handler);
}

ErrorHandler& ErrorHandlerManager::get_handler() {
    if (!handler_) {
        initialize_default();
    }
    return *handler_;
}

void ErrorHandlerManager::initialize_default() {
    handler_ = std::make_unique<DefaultErrorHandler>();
}

} // namespace simple_utcd
