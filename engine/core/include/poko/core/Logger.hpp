#pragma once

#include <string>
#include <string_view>
#include <mutex>
#include <fstream>
#include <memory>

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
 * @brief Log output format types
 */
enum class LogFormat {
    Text,    // Human-readable text format
    JSON,    // JSON format for structured logging
    CSV      // CSV format for log analysis
};

/**
 * @brief Thread-safe logger for PokoX engine
 *
 * Provides RAII-based logging with configurable severity levels,
 * file output, log rotation, and multiple format support.
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
     * @brief Set the log output format
     * @param format The format to use
     */
    void set_format(LogFormat format) noexcept;

    /**
     * @brief Get the current log format
     * @return Current log format
     */
    [[nodiscard]] LogFormat format() const noexcept { return format_; }

    /**
     * @brief Enable or disable async logging
     * @param async Enable async logging if true
     * @note Async logging uses a background thread - Stage 1 implementation is synchronous
     */
    void set_async(bool async) noexcept {
        // Async logging will be implemented in later tasks with proper thread pools
        (void)async; // Suppress unused parameter warning
    }

    /**
     * @brief Enable file output
     * @param filepath Path to the log file
     * @param max_size_mb Maximum file size before rotation (in MB)
     * @param max_files Maximum number of rotated files to keep
     * @return true if file output was enabled successfully
     */
    bool enable_file_output(const std::string& filepath, 
                            size_t max_size_mb = 10, 
                            size_t max_files = 5);

    /**
     * @brief Disable file output
     */
    void disable_file_output();

    /**
     * @brief Check if file output is enabled
     * @return true if file output is enabled
     */
    [[nodiscard]] bool file_output_enabled() const noexcept { 
        return file_stream_ != nullptr; 
    }

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

    /**
     * @brief Format a log message as JSON
     * @param level The log level
     * @param message The message to format
     * @return Formatted JSON string
     */
    [[nodiscard]] std::string format_json(LogLevel level, std::string_view message) const;

    /**
     * @brief Format a log message as CSV
     * @param level The log level
     * @param message The message to format
     * @return Formatted CSV string
     */
    [[nodiscard]] std::string format_csv(LogLevel level, std::string_view message) const;

    /**
     * @brief Check if log rotation is needed and perform it
     */
    void check_log_rotation();

    /**
     * @brief Rotate log files
     */
    void rotate_log_files();

    LogLevel level_ = LogLevel::Info;
    LogFormat format_ = LogFormat::Text;
    mutable std::mutex mutex_;
    
    // File output
    std::unique_ptr<std::ofstream> file_stream_;
    std::string file_path_;
    size_t max_file_size_ = 10 * 1024 * 1024; // 10 MB default
    size_t max_files_ = 5;
    size_t current_file_size_ = 0;
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
