#pragma once

#include <string>
#include <variant>
#include <system_error>

namespace poko::core {

/**
 * @brief Error codes for PokoX operations
 *
 * These error codes provide explicit, actionable diagnostics for failures.
 * Each error maps to a specific failure condition with a clear recovery path.
 */
enum class ErrorCode {
    Success = 0,
    InvalidArgument,
    OutOfMemory,
    ResourceNotFound,
    PermissionDenied,
    OperationFailed,
    NotInitialized,
    AlreadyInitialized,
    InvalidState,
    Timeout,
    InvalidData,
    UnknownError
};

/**
 * @brief Detailed error information
 *
 * Combines an error code with a descriptive message for debugging.
 * Follows RAII principles and is move-only for efficiency.
 */
class Error {
public:
    /**
     * @brief Construct an error with code and message
     * @param code The error code
     * @param message Human-readable error description
     */
    explicit Error(ErrorCode code = ErrorCode::UnknownError, std::string message = "")
        : code_(code), message_(std::move(message)) {}

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
     * @brief Check if this represents a success (no error)
     * @return true if success, false otherwise
     */
    [[nodiscard]] bool is_success() const noexcept { return code_ == ErrorCode::Success; }

    /**
     * @brief Convert to string representation
     * @return String representation of the error
     */
    [[nodiscard]] std::string to_string() const;

private:
    ErrorCode code_;
    std::string message_;
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
