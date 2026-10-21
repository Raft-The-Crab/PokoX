#pragma once

#include <string>
#include <string_view>
#include <mutex>

namespace poko::core {

/**
 * @brief Log severity levels
 *
 * Ordered from most to least severe.
 */
enum class LogLevel {
    Error,
    Warning,
    Info,
    Debug,
    Trace
};

/**
 * @brief Thread-safe logger for PokoX engine
 *
 * Provides RAII-based logging with configurable severity levels.
 * All logging operations are thread-safe via internal mutex.
 * Follows modern C++ best practices with no global state in the interface.
 */
class Logger {
public:
    /**
     * @brief Get the singleton logger instance
     * @return Reference to the logger instance
     */
    static Logger& instance() noexcept;

    /**
     * @brief Set the minimum log level
     * @param level Messages below this level will be suppressed
     */
    void set_level(LogLevel level) noexcept;

    /**
     * @brief Get the current minimum log level
     * @return Current minimum log level
     */
    [[nodiscard]] LogLevel level() const noexcept { return level_; }

    /**
     * @brief Log a message at specified level
     * @param level The severity level
     * @param message The message to log
     */
    void log(LogLevel level, std::string_view message);

    /**
     * @brief Convenience method for error logging
     * @param message The error message
     */
    void error(std::string_view message) { log(LogLevel::Error, message); }

    /**
     * @brief Convenience method for warning logging
     * @param message The warning message
     */
    void warning(std::string_view message) { log(LogLevel::Warning, message); }

    /**
     * @brief Convenience method for info logging
     * @param message The info message
     */
    void info(std::string_view message) { log(LogLevel::Info, message); }

    /**
     * @brief Convenience method for debug logging
     * @param message The debug message
     */
    void debug(std::string_view message) { log(LogLevel::Debug, message); }

    /**
     * @brief Convenience method for trace logging
     * @param message The trace message
     */
    void trace(std::string_view message) { log(LogLevel::Trace, message); }

    // Delete copy and move operations (singleton)
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

private:
    Logger() noexcept = default;
    ~Logger() = default;

    /**
     * @brief Convert log level to string
     * @param level The log level
     * @return String representation
     */
    [[nodiscard]] static const char* level_to_string(LogLevel level) noexcept;

    /**
     * @brief Format a log message with timestamp and level
     * @param level The log level
     * @param message The message to format
     * @return Formatted log string
     */
    [[nodiscard]] std::string format_message(LogLevel level, std::string_view message) const;

    LogLevel level_ = LogLevel::Info;
    mutable std::mutex mutex_;
};

/**
 * @brief RAII scoped log level changer
 *
 * Temporarily changes the log level for a scope,
 * then restores the previous level on destruction.
 */
class ScopedLogLevel {
public:
    /**
     * @brief Construct with new log level
     * @param logger Reference to logger
     * @param new_level The temporary log level
     */
    ScopedLogLevel(Logger& logger, LogLevel new_level) noexcept;
    
    /**
     * @brief Restore original log level on destruction
     */
    ~ScopedLogLevel() noexcept;

    // Delete copy and move
    ScopedLogLevel(const ScopedLogLevel&) = delete;
    ScopedLogLevel& operator=(const ScopedLogLevel&) = delete;
    ScopedLogLevel(ScopedLogLevel&&) = delete;
    ScopedLogLevel& operator=(ScopedLogLevel&&) = delete;

private:
    Logger& logger_;
    LogLevel previous_level_;
};

} // namespace poko::core
