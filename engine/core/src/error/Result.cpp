#include "poko/core/Result.hpp"

#include <unordered_map>
#include <sstream>

namespace poko::core {

std::string Error::to_string() const {
    // PRODUCTION: Simple error name + message
    // NON-PRODUCTION: Includes category prefix
    
    static const std::unordered_map<ErrorCode, const char*> error_names = {
        // General errors (PRODUCTION)
        {ErrorCode::Success, "Success"},
        {ErrorCode::InvalidArgument, "InvalidArgument"},
        {ErrorCode::OutOfMemory, "OutOfMemory"},
        {ErrorCode::OperationFailed, "OperationFailed"},
        {ErrorCode::NotInitialized, "NotInitialized"},
        {ErrorCode::AlreadyInitialized, "AlreadyInitialized"},
        {ErrorCode::InvalidState, "InvalidState"},
        {ErrorCode::Timeout, "Timeout"},
        {ErrorCode::InvalidData, "InvalidData"},
        {ErrorCode::UnknownError, "UnknownError"},
        // System errors (NON-PRODUCTION)
        {ErrorCode::PermissionDenied, "PermissionDenied"},
        {ErrorCode::SystemError, "SystemError"},
        // Network errors (NON-PRODUCTION)
        {ErrorCode::NetworkDisconnected, "NetworkDisconnected"},
        {ErrorCode::NetworkTimeout, "NetworkTimeout"},
        {ErrorCode::NetworkUnreachable, "NetworkUnreachable"},
        // File system errors (NON-PRODUCTION)
        {ErrorCode::FileNotFound, "FileNotFound"},
        {ErrorCode::FileInUse, "FileInUse"},
        {ErrorCode::DirectoryNotFound, "DirectoryNotFound"},
        {ErrorCode::PathTooLong, "PathTooLong"},
        {ErrorCode::DiskFull, "DiskFull"},
        // Graphics errors (NON-PRODUCTION)
        {ErrorCode::GraphicsDeviceLost, "GraphicsDeviceLost"},
        {ErrorCode::GraphicsInitializationFailed, "GraphicsInitializationFailed"},
        {ErrorCode::ShaderCompilationFailed, "ShaderCompilationFailed"},
        // Audio errors (NON-PRODUCTION)
        {ErrorCode::AudioDeviceNotFound, "AudioDeviceNotFound"},
        {ErrorCode::AudioInitializationFailed, "AudioInitializationFailed"},
        // Input errors (NON-PRODUCTION)
        {ErrorCode::InputDeviceNotFound, "InputDeviceNotFound"},
        {ErrorCode::InputInitializationFailed, "InputInitializationFailed"},
        // Scripting errors (NON-PRODUCTION)
        {ErrorCode::ScriptCompilationFailed, "ScriptCompilationFailed"},
        {ErrorCode::ScriptRuntimeError, "ScriptRuntimeError"},
        {ErrorCode::ScriptSecurityViolation, "ScriptSecurityViolation"},
        // Asset errors (NON-PRODUCTION)
        {ErrorCode::AssetLoadFailed, "AssetLoadFailed"},
        {ErrorCode::AssetCorrupted, "AssetCorrupted"},
        {ErrorCode::AssetVersionMismatch, "AssetVersionMismatch"},
        // Validation errors (NON-PRODUCTION)
        {ErrorCode::ValidationFailed, "ValidationFailed"},
        {ErrorCode::SchemaViolation, "SchemaViolation"}
    };

    static const std::unordered_map<ErrorCategory, const char*> category_names = {
        {ErrorCategory::General, "General"},
        {ErrorCategory::System, "System"},
        {ErrorCategory::Network, "Network"},
        {ErrorCategory::FileSystem, "FileSystem"},
        {ErrorCategory::Graphics, "Graphics"},
        {ErrorCategory::Audio, "Audio"},
        {ErrorCategory::Input, "Input"},
        {ErrorCategory::Scripting, "Scripting"},
        {ErrorCategory::Asset, "Asset"},
        {ErrorCategory::Validation, "Validation"}
    };

    std::ostringstream oss;
    
    auto it = error_names.find(code_);
    if (it != error_names.end()) {
        // NON-PRODUCTION: Include category prefix
        oss << "[" << category_names.at(category_) << "] " << it->second;
        // PRODUCTION: oss << it->second;
    } else {
        oss << "[" << category_names.at(category_) << "] UnknownError";
        // PRODUCTION: oss << "UnknownError";
    }
    
    if (!message_.empty()) {
        oss << ": " << message_;
    }
    
    return oss.str();
}

void Error::capture_stack_trace() {
    // NON-PRODUCTION: Platform-specific stack trace capture
    // PRODUCTION: No-op (stack traces add overhead)
    // This is a placeholder - full implementation would use:
    // - Windows: StackWalk64, CaptureStackBackTrace
    // - Linux: backtrace from execinfo.h
    // - macOS: backtrace from execinfo.h
}

} // namespace poko::core
