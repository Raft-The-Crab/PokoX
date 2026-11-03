/**
 * @file constant_buffer.cpp
 * @brief Implementation of Poko ConstantBuffer.
 */

#include "poko/render/constant_buffer.h"
#include "backend/diligent/internal/constant_buffer_impl.h"

#include <new>

namespace poko::render {

ConstantBuffer::ConstantBuffer() : impl_(new (std::nothrow) Impl()) {}

ConstantBuffer::~ConstantBuffer() {
    delete impl_;
}

ConstantBufferErrorCode ConstantBuffer::initialize(Device* device, const ConstantBufferDesc& desc) {
    if (!impl_) {
        return ConstantBufferErrorCode::Unknown;
    }
    if (!device) {
        return ConstantBufferErrorCode::InvalidDevice;
    }
    if (!device->is_valid()) {
        return ConstantBufferErrorCode::InvalidDevice;
    }
    if (desc.size == 0) {
        return ConstantBufferErrorCode::InvalidSize;
    }
    
    return impl_->initialize(device, desc) ? ConstantBufferErrorCode::Success : ConstantBufferErrorCode::BufferCreationFailed;
}

ConstantBufferErrorCode ConstantBuffer::update(const void* data, uint32_t size, uint32_t offset) {
    if (!impl_) {
        return ConstantBufferErrorCode::Unknown;
    }
    if (!data) {
        return ConstantBufferErrorCode::InvalidData;
    }
    if (size == 0) {
        return ConstantBufferErrorCode::InvalidSize;
    }
    if (offset >= (impl_->desc.size > 0 ? impl_->desc.size : 1)) {
        return ConstantBufferErrorCode::InvalidOffset;
    }
    
    return impl_->update(data, size, offset) ? ConstantBufferErrorCode::Success : ConstantBufferErrorCode::UpdateFailed;
}

bool ConstantBuffer::is_valid() const {
    return impl_ != nullptr && impl_->buffer != nullptr;
}

Device* ConstantBuffer::get_device() const {
    return impl_ ? impl_->device : nullptr;
}

uint32_t ConstantBuffer::get_size() const {
    return impl_ ? impl_->desc.size : 0;
}

} // namespace poko::render
