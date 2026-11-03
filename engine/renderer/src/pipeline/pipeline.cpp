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
        return PipelineErrorCode::InvalidDevice;
    }
    if (!device->is_valid()) {
        return PipelineErrorCode::InvalidDevice;
    }
    
    return impl_->initialize(device, desc) ? PipelineErrorCode::Success : PipelineErrorCode::PipelineCreationFailed;
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

} // namespace poko::render
