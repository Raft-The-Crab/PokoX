/**
 * @file pipeline.cpp
 * @brief Implementation of Poko PipelineState.
 */

#include "poko/render/pipeline.h"
#include "backend/diligent/internal/pipeline_impl.h"

#include <new>

namespace poko::render {

PipelineState::PipelineState() : impl_(new (std::nothrow) Impl()) {}

PipelineState::~PipelineState() {
    delete impl_;
}

PipelineErrorCode PipelineState::initialize(Device* device, const PipelineDesc& desc) {
    if (!impl_) {
        return PipelineErrorCode::Unknown;
    }
    if (!device) {
        impl_->last_error = "Device is null";
        return PipelineErrorCode::InvalidDevice;
    }
    if (!device->is_valid()) {
        impl_->last_error = "Device is invalid";
        return PipelineErrorCode::InvalidDevice;
    }

    // Validate formats
    if (desc.color_format == TextureFormat::Unknown) {
        impl_->last_error = "Invalid color format: Unknown";
        return PipelineErrorCode::IncompatibleFormat;
    }
    if (desc.depth_format == TextureFormat::Unknown) {
        impl_->last_error = "Invalid depth format: Unknown";
        return PipelineErrorCode::IncompatibleFormat;
    }

    if (!impl_->initialize(device, desc)) {
        return PipelineErrorCode::PipelineCreationFailed;
    }

    impl_->last_error.clear();
    return PipelineErrorCode::Success;
}

bool PipelineState::is_valid() const {
    return impl_ != nullptr && impl_->pipeline != nullptr;
}

const char* PipelineState::get_name() const {
    return impl_ ? (impl_->desc.name ? impl_->desc.name : "") : "";
}

Device* PipelineState::get_device() const {
    return impl_ ? impl_->device : nullptr;
}

const char* PipelineState::get_last_error() const {
    if (!impl_) {
        return "Pipeline implementation is null";
    }
    return impl_->last_error.c_str();
}

} // namespace poko::render
