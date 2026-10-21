#include "poko/core/Result.hpp"

#include <unordered_map>

namespace poko::core {

std::string Error::to_string() const {
    static const std::unordered_map<ErrorCode, const char*> error_names = {
        {ErrorCode::Success, "Success"},
        {ErrorCode::InvalidArgument, "InvalidArgument"},
        {ErrorCode::OutOfMemory, "OutOfMemory"},
        {ErrorCode::ResourceNotFound, "ResourceNotFound"},
        {ErrorCode::PermissionDenied, "PermissionDenied"},
        {ErrorCode::OperationFailed, "OperationFailed"},
        {ErrorCode::NotInitialized, "NotInitialized"},
        {ErrorCode::AlreadyInitialized, "AlreadyInitialized"},
        {ErrorCode::InvalidState, "InvalidState"},
        {ErrorCode::Timeout, "Timeout"},
        {ErrorCode::InvalidData, "InvalidData"},
        {ErrorCode::UnknownError, "UnknownError"}
    };

    auto it = error_names.find(code_);
    if (it != error_names.end()) {
        if (message_.empty()) {
            return std::string(it->second);
        }
        return std::string(it->second) + ": " + message_;
    }
    return message_.empty() ? "UnknownError" : message_;
}

} // namespace poko::core
