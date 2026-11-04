/**
 * @file index_buffer_impl.cpp
 * @brief Diligent Core implementation of IndexBuffer.
 */

#include "backend/diligent/internal/index_buffer_impl.h"
#include "backend/diligent/internal/device_impl.h"

#include <cstdio>
#include <cstring>

namespace poko::render {

IndexBuffer::Impl::~Impl() {
    if (buffer) {
        buffer->Release();
        buffer = nullptr;
    }
}

bool IndexBuffer::Impl::initialize(Device* device, const IndexBufferDesc& desc) {
    if (!device || !device->is_valid()) {
        last_error = "Device is invalid";
        return false;
    }

    auto device_impl = get_device_impl(device);
    if (!device_impl || !device_impl->device) {
        last_error = "Failed to get device implementation";
        return false;
    }

    this->device = device;
    this->desc = desc;

    // Validate inputs
    if (!desc.name) {
        last_error = "IndexBuffer must have a name";
        return false;
    }
    if (desc.size == 0) {
        last_error = "Invalid size: 0";
        return false;
    }
    if (!desc.data && desc.size > 0) {
        last_error = "Null data pointer with non-zero size";
        return false;
    }
    if (desc.format != IndexFormat::UInt16 && desc.format != IndexFormat::UInt32) {
        last_error = "Invalid format";
        return false;
    }
    if (desc.index_count == 0) {
        last_error = "Invalid index count: 0";
        return false;
    }
    // Validate size matches format
    uint32_t expected_size = (desc.format == IndexFormat::UInt16) ? desc.index_count * 2 : desc.index_count * 4;
    if (desc.size != expected_size) {
        last_error = "Size does not match expected size for format and index count";
        return false;
    }
    // Validate buffer size limit (typical max)
    if (desc.size > 256 * 1024 * 1024) {  // 256 MB
        last_error = "Size exceeds maximum (256 MB)";
        return false;
    }

    // Copy data for lifetime management
    if (desc.data && desc.size > 0) {
        data.resize(desc.size);
        std::memcpy(data.data(), desc.data, desc.size);
    }

    // Create Diligent buffer
    Diligent::BufferDesc bd;
    bd.Name = desc.name ? desc.name : "Poko IndexBuffer";
    bd.Size = desc.size;
    bd.BindFlags = Diligent::BIND_INDEX_BUFFER;
    bd.Usage = Diligent::USAGE_IMMUTABLE;

    Diligent::BufferData buffer_data;
    buffer_data.pData = data.empty() ? nullptr : data.data();
    buffer_data.DataSize = desc.size;

    device_impl->device->CreateBuffer(bd, &buffer_data, &buffer);

    if (!buffer) {
        last_error = "Failed to create index buffer";
        return false;
    }

    last_error.clear();
    return true;
}

bool IndexBuffer::Impl::update(const void* data_ptr, uint32_t size, uint32_t offset) {
    if (!buffer) {
        last_error = "Cannot update uninitialized buffer";
        return false;
    }

    auto device_impl = get_device_impl(device);
    if (!device_impl || !device_impl->context) {
        last_error = "Failed to get device implementation";
        return false;
    }

    // Validate inputs
    if (!data_ptr) {
        last_error = "Null data pointer";
        return false;
    }
    if (size == 0) {
        last_error = "Invalid update size: 0";
        return false;
    }
    if (offset + size > desc.size) {
        last_error = "Update range exceeds buffer size";
        return false;
    }

    // Update buffer using context's UpdateBuffer
    device_impl->context->UpdateBuffer(buffer, offset, size, data_ptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    last_error.clear();
    return true;
}

} // namespace poko::render
