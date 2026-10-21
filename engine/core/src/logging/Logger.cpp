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

void Logger::log(LogLevel level, std::string_view message) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (level < level_) {
        return;
    }
    
    std::cerr << format_message(level, message) << std::endl;
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

ScopedLogLevel::ScopedLogLevel(Logger& logger, LogLevel new_level) noexcept
    : logger_(logger), previous_level_(logger.level()) {
    logger_.set_level(new_level);
}

ScopedLogLevel::~ScopedLogLevel() noexcept {
    logger_.set_level(previous_level_);
}

} // namespace poko::core
