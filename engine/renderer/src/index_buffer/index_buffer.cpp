/**
 * @file index_buffer.cpp
 * @brief Implementation of Poko IndexBuffer.
 */

#include "poko/render/index_buffer.h"
#include "backend/diligent/internal/index_buffer_impl.h"

#include <new>

namespace poko::render {

IndexBuffer::IndexBuffer() : impl_(new (std::nothrow) Impl()) {}

IndexBuffer::~IndexBuffer() {
    delete impl_;
}

IndexBufferErrorCode IndexBuffer::initialize(Device* device, const IndexBufferDesc& desc) {
    if (!impl_) {
        return IndexBufferErrorCode::Unknown;
    }
    if (!device) {
        return IndexBufferErrorCode::InvalidDevice;
    }
    if (!device->is_valid()) {
        return IndexBufferErrorCode::InvalidDevice;
    }
    if (desc.size == 0) {
        return IndexBufferErrorCode::InvalidSize;
    }
    if (desc.format != IndexFormat::UInt16 && desc.format != IndexFormat::UInt32) {
        return IndexBufferErrorCode::InvalidFormat;
    }
    
    return impl_->initialize(device, desc) ? IndexBufferErrorCode::Success : IndexBufferErrorCode::BufferCreationFailed;
}

bool IndexBuffer::is_valid() const {
    return impl_ != nullptr && impl_->buffer != nullptr;
}

Device* IndexBuffer::get_device() const {
    return impl_ ? impl_->device : nullptr;
}

uint32_t IndexBuffer::get_size() const {
    return impl_ ? impl_->desc.size : 0;
}

IndexFormat IndexBuffer::get_format() const {
    return impl_ ? impl_->desc.format : IndexFormat::UInt16;
}

const char* IndexBuffer::get_name() const {
    return impl_ ? (impl_->desc.name ? impl_->desc.name : "") : "";
}

IndexBufferErrorCode IndexBuffer::update(const void* data, uint32_t size, uint32_t offset) {
    if (!impl_) {
        return IndexBufferErrorCode::Unknown;
    }
    if (!data) {
        return IndexBufferErrorCode::InvalidData;
    }
    if (size == 0) {
        return IndexBufferErrorCode::InvalidSize;
    }
    if (offset >= (impl_->desc.size > 0 ? impl_->desc.size : 1)) {
        return IndexBufferErrorCode::InvalidOffset;
    }
    
    return impl_->update(data, size, offset) ? IndexBufferErrorCode::Success : IndexBufferErrorCode::UpdateFailed;
}

} // namespace poko::render
