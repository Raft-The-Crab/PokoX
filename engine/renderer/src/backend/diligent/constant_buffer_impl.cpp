/**
 * @file constant_buffer_impl.cpp
 * @brief Diligent Core implementation of ConstantBuffer.
 */

#include "backend/diligent/internal/constant_buffer_impl.h"
#include "backend/diligent/internal/device_impl.h"

#include <cstdio>
#include <cstring>

namespace poko::render {

ConstantBuffer::Impl::~Impl() {
    if (buffer) {
        buffer->Release();
        buffer = nullptr;
    }
}

bool ConstantBuffer::Impl::initialize(Device* device, const ConstantBufferDesc& desc) {
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
        last_error = "ConstantBuffer must have a name";
        return false;
    }
    if (desc.size == 0) {
        last_error = "Invalid size: 0";
        return false;
    }
    // Validate std140 alignment (must be multiple of 16 bytes for OpenGL)
    if (desc.size % 16 != 0) {
        last_error = "Size must be a multiple of 16 bytes for std140 layout";
        return false;
    }
    // Validate buffer size limit (typical max)
    if (desc.size > 256 * 1024 * 1024) {  // 256 MB
        last_error = "Size exceeds maximum (256 MB)";
        return false;
    }

    // Copy initial data if provided (but note: dynamic buffers cannot have initial data in Diligent)
    if (desc.data && desc.size > 0) {
        data.resize(desc.size);
        std::memcpy(data.data(), desc.data, desc.size);
    }

    // Create Diligent buffer with DYNAMIC usage without CPU access flags
    // DYNAMIC buffers are mapped for updates, not updated via UpdateBuffer()
    Diligent::BufferDesc bd;
    bd.Name = desc.name ? desc.name : "Poko ConstantBuffer";
    bd.Size = desc.size;
    bd.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
    if (desc.allow_shader_resource_view) {
        // M13: raw-mode buffer so it can back a shader storage buffer view
        bd.BindFlags |= Diligent::BIND_SHADER_RESOURCE;
        bd.Mode = Diligent::BUFFER_MODE_RAW;
    }
    bd.Usage = Diligent::USAGE_DYNAMIC;
    bd.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

    Diligent::BufferData buffer_data;
    buffer_data.pData = nullptr;  // Dynamic buffers cannot have initial data
    buffer_data.DataSize = 0;

    device_impl->device->CreateBuffer(bd, &buffer_data, &buffer);

    if (!buffer) {
        last_error = "Failed to create constant buffer";
        return false;
    }

    last_error.clear();
    return true;
}

bool ConstantBuffer::Impl::update(const void* data_ptr, uint32_t size, uint32_t offset) {
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

    // Map the buffer for writing (DISCARD flag tells GPU we're writing the entire buffer)
    void* mapped_data = nullptr;
    device_impl->context->MapBuffer(buffer, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD, mapped_data);

    if (!mapped_data) {
        last_error = "Failed to map buffer for update";
        return false;
    }

    // Copy data to mapped region
    std::memcpy(static_cast<uint8_t*>(mapped_data) + offset, data_ptr, size);

    // Unmap the buffer
    device_impl->context->UnmapBuffer(buffer, Diligent::MAP_WRITE);
    last_error.clear();
    return true;
}

} // namespace poko::render
