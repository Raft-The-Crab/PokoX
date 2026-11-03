/**
 * @file vertex_buffer.cpp
 * @brief Implementation of Poko VertexBuffer.
 */

#include "poko/render/vertex_buffer.h"
#include "backend/diligent/internal/vertex_buffer_impl.h"

#include <new>

namespace poko::render {

VertexBuffer::VertexBuffer() : impl_(new (std::nothrow) Impl()) {}

VertexBuffer::~VertexBuffer() {
    delete impl_;
}

BufferErrorCode VertexBuffer::initialize(Device* device, const BufferDesc& desc) {
    if (!impl_) {
        return BufferErrorCode::Unknown;
    }
    if (!device) {
        return BufferErrorCode::InvalidDevice;
    }
    if (!device->is_valid()) {
        return BufferErrorCode::InvalidDevice;
    }
    if (desc.size == 0) {
        return BufferErrorCode::InvalidSize;
    }
    
    return impl_->initialize(device, desc) ? BufferErrorCode::Success : BufferErrorCode::BufferCreationFailed;
}

bool VertexBuffer::is_valid() const {
    return impl_ != nullptr && impl_->buffer != nullptr;
}

Device* VertexBuffer::get_device() const {
    return impl_ ? impl_->device : nullptr;
}

} // namespace poko::render
