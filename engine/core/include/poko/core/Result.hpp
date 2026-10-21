#pragma once

#include <string>
#include <variant>
#include <system_error>
#include <vector>

namespace poko::core {

/**
 * @brief Error categories for grouping related errors
 *
 * NON-PRODUCTION: Enhanced feature for better error handling strategies.
 * Basic version uses only error codes.
 */
enum class ErrorCategory {
    General = 0,
    System,
    Network,
    FileSystem,
    Graphics,
    Audio,
    Input,
    Scripting,
    Asset,
    Validation
};

/**
 * @brief Error codes for PokoX operations
 *
 * These error codes provide explicit, actionable diagnostics for failures.
 * Each error maps to a specific failure condition with a clear recovery path.
 *
 * NON-PRODUCTION: Basic version has 12 codes. Enhanced version has 40+ codes.
 */
enum class ErrorCode {
    Success = 0,
    // General errors (PRODUCTION)
    InvalidArgument,
    OutOfMemory,
    OperationFailed,
    NotInitialized,
    AlreadyInitialized,
    InvalidState,
    Timeout,
    InvalidData,
    UnknownError,
    // System errors (NON-PRODUCTION)
    PermissionDenied,
    SystemError,
    // Network errors (NON-PRODUCTION)
    NetworkDisconnected,
    NetworkTimeout,
    NetworkUnreachable,
    // File system errors (NON-PRODUCTION)
    FileNotFound,
    FileInUse,
    DirectoryNotFound,
    PathTooLong,
    DiskFull,
    // Graphics errors (NON-PRODUCTION)
    GraphicsDeviceLost,
    GraphicsInitializationFailed,
    ShaderCompilationFailed,
    // Audio errors (NON-PRODUCTION)
    AudioDeviceNotFound,
    AudioInitializationFailed,
    // Input errors (NON-PRODUCTION)
    InputDeviceNotFound,
    InputInitializationFailed,
    // Scripting errors (NON-PRODUCTION)
    ScriptCompilationFailed,
    ScriptRuntimeError,
    ScriptSecurityViolation,
    // Asset errors (NON-PRODUCTION)
    AssetLoadFailed,
    AssetCorrupted,
    AssetVersionMismatch,
    // Validation errors (NON-PRODUCTION)
    ValidationFailed,
    SchemaViolation
};

/**
 * @brief Stack trace entry for error context
 *
 * NON-PRODUCTION: Stack trace support - placeholder for platform-specific implementation.
 */
struct StackFrame {
    std::string function;
    std::string file;
    unsigned int line;
};

/**
 * @brief Detailed error information
 *
 * Combines an error code with a descriptive message, category,
 * and optional stack trace for debugging.
 * Follows RAII principles and is move-only for efficiency.
 *
 * NON-PRODUCTION: Enhanced version includes category and stack trace.
 * Production version uses only code and message.
 */
class Error {
public:
    /**
     * @brief Construct an error with code and message (PRODUCTION version)
     * @param code The error code
     * @param message Human-readable error description
     */
    explicit Error(ErrorCode code = ErrorCode::UnknownError, std::string message = "")
        : code_(code), message_(std::move(message)), category_(ErrorCategory::General) {}

    /**
     * @brief Construct an error with code, message, and category (NON-PRODUCTION)
     * @param code The error code
     * @param message Human-readable error description
     * @param category Error category (defaults to General)
     */
    explicit Error(ErrorCode code, std::string message, ErrorCategory category)
        : code_(code), message_(std::move(message)), category_(category) {}

    /**
     * @brief Get the error code
     * @return The error code
     */
    [[nodiscard]] ErrorCode code() const noexcept { return code_; }

    /**
     * @brief Get the error message
     * @return Human-readable error description
     */
    [[nodiscard]] const std::string& message() const noexcept { return message_; }

    /**
     * @brief Get the error category (NON-PRODUCTION)
     * @return The error category
     */
    [[nodiscard]] ErrorCategory category() const noexcept { return category_; }

    /**
     * @brief Check if this represents a success (no error)
     * @return true if success, false otherwise
     */
    [[nodiscard]] bool is_success() const noexcept { return code_ == ErrorCode::Success; }

    /**
     * @brief Get the stack trace (NON-PRODUCTION)
     * @return Vector of stack frames
     */
    [[nodiscard]] const std::vector<StackFrame>& stack_trace() const noexcept { return stack_trace_; }

    /**
     * @brief Set the stack trace (NON-PRODUCTION)
     * @param trace Stack trace to set
     */
    void set_stack_trace(std::vector<StackFrame> trace) {
        stack_trace_ = std::move(trace);
    }

    /**
     * @brief Capture current stack trace (NON-PRODUCTION - platform-dependent)
     */
    void capture_stack_trace();

    /**
     * @brief Convert to string representation
     * @return String representation of the error
     */
    [[nodiscard]] std::string to_string() const;

private:
    ErrorCode code_;
    std::string message_;
    ErrorCategory category_;
    std::vector<StackFrame> stack_trace_;
};

/**
 * @brief Result type for operations that can fail
 *
 * A discriminated union type that either contains a value T or an Error.
 * This provides explicit error handling without exceptions for critical paths.
 *
 * @tparam T The success value type
 */
template<typename T>
class Result {
public:
    /**
     * @brief Construct a successful result
     * @param value The success value
     */
    explicit Result(T value) : data_(std::move(value)) {}

    /**
     * @brief Construct a failed result
     * @param error The error information
     */
    explicit Result(Error error) : data_(std::move(error)) {}

    /**
     * @brief Check if the result contains a value
     * @return true if successful, false if error
     */
    [[nodiscard]] bool is_ok() const noexcept {
        return std::holds_alternative<T>(data_);
    }

    /**
     * @brief Check if the result contains an error
     * @return true if error, false if successful
     */
    [[nodiscard]] bool is_error() const noexcept {
        return !is_ok();
    }

    /**
     * @brief Get the success value
     * @return Reference to the value
     * @pre is_ok() must be true
     */
    [[nodiscard]] T& value() {
        return std::get<T>(data_);
    }

    /**
     * @brief Get the success value (const)
     * @return Const reference to the value
     * @pre is_ok() must be true
     */
    [[nodiscard]] const T& value() const {
        return std::get<T>(data_);
    }

    /**
     * @brief Get the error
     * @return Reference to the error
     * @pre is_error() must be true
     */
    [[nodiscard]] Error& error() {
        return std::get<Error>(data_);
    }

    /**
     * @brief Get the error (const)
     * @return Const reference to the error
     * @pre is_error() must be true
     */
    [[nodiscard]] const Error& error() const {
        return std::get<Error>(data_);
    }

    /**
     * @brief Get the value or a default if error
     * @param default_value Default value if error
     * @return The value if successful, otherwise default_value
     */
    [[nodiscard]] T value_or(T&& default_value) const {
        if (is_ok()) {
            return value();
        }
        return std::forward<T>(default_value);
    }

private:
    std::variant<T, Error> data_;
};

} // namespace poko::core
