#include "poko/core/Result.hpp"

#include <unordered_map>
#include <sstream>

namespace poko::core {

std::string Error::to_string() const {
    static const std::unordered_map<ErrorCode, const char*> error_names = {
        // General errors
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
        // System errors
        {ErrorCode::PermissionDenied, "PermissionDenied"},
        {ErrorCode::SystemError, "SystemError"},
        // Network errors
        {ErrorCode::NetworkDisconnected, "NetworkDisconnected"},
        {ErrorCode::NetworkTimeout, "NetworkTimeout"},
        {ErrorCode::NetworkUnreachable, "NetworkUnreachable"},
        // File system errors
        {ErrorCode::FileNotFound, "FileNotFound"},
        {ErrorCode::FileInUse, "FileInUse"},
        {ErrorCode::DirectoryNotFound, "DirectoryNotFound"},
        {ErrorCode::PathTooLong, "PathTooLong"},
        {ErrorCode::DiskFull, "DiskFull"},
        // Graphics errors
        {ErrorCode::GraphicsDeviceLost, "GraphicsDeviceLost"},
        {ErrorCode::GraphicsInitializationFailed, "GraphicsInitializationFailed"},
        {ErrorCode::ShaderCompilationFailed, "ShaderCompilationFailed"},
        // Audio errors
        {ErrorCode::AudioDeviceNotFound, "AudioDeviceNotFound"},
        {ErrorCode::AudioInitializationFailed, "AudioInitializationFailed"},
        // Input errors
        {ErrorCode::InputDeviceNotFound, "InputDeviceNotFound"},
        {ErrorCode::InputInitializationFailed, "InputInitializationFailed"},
        // Scripting errors
        {ErrorCode::ScriptCompilationFailed, "ScriptCompilationFailed"},
        {ErrorCode::ScriptRuntimeError, "ScriptRuntimeError"},
        {ErrorCode::ScriptSecurityViolation, "ScriptSecurityViolation"},
        // Asset errors
        {ErrorCode::AssetLoadFailed, "AssetLoadFailed"},
        {ErrorCode::AssetCorrupted, "AssetCorrupted"},
        {ErrorCode::AssetVersionMismatch, "AssetVersionMismatch"},
        // Validation errors
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
        oss << "[" << category_names.at(category_) << "] " << it->second;
    } else {
        oss << "[" << category_names.at(category_) << "] UnknownError";
    }
    
    if (!message_.empty()) {
        oss << ": " << message_;
    }
    
    return oss.str();
}

void Error::capture_stack_trace() {
    // Platform-specific stack trace capture
    // This is a placeholder - full implementation would use:
    // - Windows: StackWalk64, CaptureStackBackTrace
    // - Linux: backtrace from execinfo.h
    // - macOS: backtrace from execinfo.h
    // For now, this is a no-op in Stage 1
}

} // namespace poko::core
