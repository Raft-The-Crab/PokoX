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
        std::fprintf(stderr, "Invalid device\n");
        return false;
    }

    auto device_impl = get_device_impl(device);
    if (!device_impl || !device_impl->device) {
        std::fprintf(stderr, "Invalid device implementation\n");
        return false;
    }

    this->device = device;
    this->desc = desc;

    // Validate inputs
    if (!desc.name) {
        std::fprintf(stderr, "ConstantBuffer must have a name\n");
        return false;
    }
    if (desc.size == 0) {
        std::fprintf(stderr, "ConstantBuffer '%s': Invalid size: 0\n", desc.name);
        return false;
    }
    // Validate std140 alignment (must be multiple of 16 bytes for OpenGL)
    if (desc.size % 16 != 0) {
        std::fprintf(stderr, "ConstantBuffer '%s': Size (%u) must be a multiple of 16 bytes for std140 layout\n", desc.name, desc.size);
        return false;
    }
    // Validate buffer size limit (typical max)
    if (desc.size > 256 * 1024 * 1024) {  // 256 MB
        std::fprintf(stderr, "ConstantBuffer '%s': Size (%u MB) exceeds maximum (256 MB)\n", desc.name, desc.size / (1024 * 1024));
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
    bd.Usage = Diligent::USAGE_DYNAMIC;
    bd.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

    Diligent::BufferData buffer_data;
    buffer_data.pData = nullptr;  // Dynamic buffers cannot have initial data
    buffer_data.DataSize = 0;

    device_impl->device->CreateBuffer(bd, &buffer_data, &buffer);

    if (!buffer) {
        std::fprintf(stderr, "Failed to create constant buffer\n");
        return false;
    }

    return true;
}

bool ConstantBuffer::Impl::update(const void* data_ptr, uint32_t size, uint32_t offset) {
    if (!buffer) {
        std::fprintf(stderr, "ConstantBuffer '%s': Cannot update uninitialized buffer\n", desc.name);
        return false;
    }

    auto device_impl = get_device_impl(device);
    if (!device_impl || !device_impl->context) {
        std::fprintf(stderr, "Invalid device implementation\n");
        return false;
    }

    // Validate inputs
    if (!data_ptr) {
        std::fprintf(stderr, "ConstantBuffer '%s': Null data pointer\n", desc.name);
        return false;
    }
    if (size == 0) {
        std::fprintf(stderr, "ConstantBuffer '%s': Invalid update size: 0\n", desc.name);
        return false;
    }
    if (offset + size > desc.size) {
        std::fprintf(stderr, "ConstantBuffer '%s': Update range [offset=%u, size=%u] exceeds buffer size (%u)\n",
            desc.name, offset, size, desc.size);
        return false;
    }

    // Map the buffer for writing (DISCARD flag tells GPU we're writing the entire buffer)
    void* mapped_data = nullptr;
    device_impl->context->MapBuffer(buffer, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD, mapped_data);
    
    if (!mapped_data) {
        std::fprintf(stderr, "ConstantBuffer '%s': Failed to map buffer for update\n", desc.name);
        return false;
    }

    // Copy data to mapped region
    std::memcpy(static_cast<uint8_t*>(mapped_data) + offset, data_ptr, size);

    // Unmap the buffer
    device_impl->context->UnmapBuffer(buffer, Diligent::MAP_WRITE);

    return true;
}

} // namespace poko::render
