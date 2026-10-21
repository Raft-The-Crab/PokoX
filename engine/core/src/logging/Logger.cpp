#include "poko/core/Logger.hpp"

#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>

namespace poko::core {

Logger& Logger::instance() noexcept {
    static Logger instance;
    return instance;
}

void Logger::set_level(LogLevel level) noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    level_ = level;
}

void Logger::set_format(LogFormat format) noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    format_ = format;
}

bool Logger::enable_file_output(const std::string& filepath, 
                                size_t max_size_mb, 
                                size_t max_files) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    file_path_ = filepath;
    max_file_size_ = max_size_mb * 1024 * 1024;
    max_files_ = max_files;
    current_file_size_ = 0;
    
    file_stream_ = std::make_unique<std::ofstream>(filepath, std::ios::app);
    
    if (!file_stream_->is_open()) {
        file_stream_.reset();
        return false;
    }
    
    // Note: File size check requires std::filesystem which may not be available
    // on all platforms. This is a simplified implementation for Stage 1.
    
    return true;
}

void Logger::disable_file_output() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (file_stream_) {
        file_stream_->close();
        file_stream_.reset();
    }
    file_path_.clear();
    current_file_size_ = 0;
}

void Logger::log(LogLevel level, std::string_view message) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (level < level_) {
        return;
    }
    
    std::string formatted;
    switch (format_) {
        case LogFormat::JSON:
            formatted = format_json(level, message);
            break;
        case LogFormat::CSV:
            formatted = format_csv(level, message);
            break;
        case LogFormat::Text:
        default:
            formatted = format_message(level, message);
            break;
    }
    
    // Always output to stderr
    std::cerr << formatted << std::endl;
    
    // Output to file if enabled
    if (file_stream_ && file_stream_->is_open()) {
        *file_stream_ << formatted << std::endl;
        file_stream_->flush();
        
        current_file_size_ += formatted.size() + 1; // +1 for newline
        
        check_log_rotation();
    }
}

const char* Logger::level_to_string(LogLevel level) noexcept {
    switch (level) {
        case LogLevel::Error:   return "ERROR";
        case LogLevel::Warning: return "WARN";
        case LogLevel::Info:    return "INFO";
        case LogLevel::Debug:   return "DEBUG";
        case LogLevel::Trace:   return "TRACE";
        default:               return "UNKNOWN";
    }
}

std::string Logger::format_message(LogLevel level, std::string_view message) const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::tm tm;
    #ifdef _WIN32
        localtime_s(&tm, &time_t);
    #else
        tm = *std::localtime(&time_t);
    #endif
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S")
        << " [" << level_to_string(level) << "] "
        << message;
    
    return oss.str();
}

std::string Logger::format_json(LogLevel level, std::string_view message) const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::tm tm;
    #ifdef _WIN32
        localtime_s(&tm, &time_t);
    #else
        tm = *std::localtime(&time_t);
    #endif
    
    std::ostringstream oss;
    oss << "{\"timestamp\":\"" << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S")
        << "\",\"level\":\"" << level_to_string(level)
        << "\",\"message\":\"" << message << "\"}";
    
    return oss.str();
}

std::string Logger::format_csv(LogLevel level, std::string_view message) const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::tm tm;
    #ifdef _WIN32
        localtime_s(&tm, &time_t);
    #else
        tm = *std::localtime(&time_t);
    #endif
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S")
        << "," << level_to_string(level)
        << ",\"" << message << "\"";
    
    return oss.str();
}

void Logger::check_log_rotation() {
    if (current_file_size_ >= max_file_size_) {
        rotate_log_files();
    }
}

void Logger::rotate_log_files() {
    // Note: Full log rotation requires std::filesystem
    // This is a placeholder for Stage 1 - will be implemented with platform-specific APIs in later tasks
    if (file_stream_) {
        file_stream_->close();
        file_stream_ = std::make_unique<std::ofstream>(file_path_, std::ios::app);
        current_file_size_ = 0;
    }
}

ScopedLogLevel::ScopedLogLevel(Logger& logger, LogLevel new_level) noexcept
    : logger_(logger), previous_level_(logger.level()) {
    logger_.set_level(new_level);
}

ScopedLogLevel::~ScopedLogLevel() noexcept {
    logger_.set_level(previous_level_);
}

} // namespace poko::core
