/**
 * @file vertex_buffer_impl.cpp
 * @brief Diligent Core implementation of VertexBuffer.
 */

#include "backend/diligent/internal/vertex_buffer_impl.h"
#include "backend/diligent/internal/device_impl.h"

#include <cstdio>
#include <cstring>

namespace poko::render {

VertexBuffer::Impl::~Impl() {
    if (buffer) {
        buffer->Release();
        buffer = nullptr;
    }
}

bool VertexBuffer::Impl::initialize(Device* device, const BufferDesc& desc) {
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
        std::fprintf(stderr, "VertexBuffer must have a name\n");
        return false;
    }
    if (desc.size == 0) {
        std::fprintf(stderr, "VertexBuffer '%s': Invalid size: 0\n", desc.name);
        return false;
    }
    if (!desc.data && desc.size > 0) {
        std::fprintf(stderr, "VertexBuffer '%s': Null data pointer with non-zero size\n", desc.name);
        return false;
    }
    if (desc.stride == 0) {
        std::fprintf(stderr, "VertexBuffer '%s': Invalid stride: 0\n", desc.name);
        return false;
    }
    if (desc.stride > desc.size) {
        std::fprintf(stderr, "VertexBuffer '%s': Stride (%u) cannot exceed buffer size (%u)\n", desc.name, desc.stride, desc.size);
        return false;
    }
    if (desc.size % desc.stride != 0) {
        std::fprintf(stderr, "VertexBuffer '%s': Size (%u) is not a multiple of stride (%u)\n", desc.name, desc.size, desc.stride);
        return false;
    }
    // Validate buffer size limit (typical max)
    if (desc.size > 256 * 1024 * 1024) {  // 256 MB
        std::fprintf(stderr, "VertexBuffer '%s': Size (%u MB) exceeds maximum (256 MB)\n", desc.name, desc.size / (1024 * 1024));
        return false;
    }

    // Copy data for lifetime management
    if (desc.data && desc.size > 0) {
        data.resize(desc.size);
        std::memcpy(data.data(), desc.data, desc.size);
    }

    // Create Diligent buffer
    Diligent::BufferDesc bd;
    bd.Name = desc.name ? desc.name : "Poko VertexBuffer";
    bd.Size = desc.size;
    bd.BindFlags = Diligent::BIND_VERTEX_BUFFER;
    bd.Usage = Diligent::USAGE_IMMUTABLE;

    Diligent::BufferData buffer_data;
    buffer_data.pData = data.empty() ? nullptr : data.data();
    buffer_data.DataSize = desc.size;

    device_impl->device->CreateBuffer(bd, &buffer_data, &buffer);

    if (!buffer) {
        std::fprintf(stderr, "Failed to create vertex buffer\n");
        return false;
    }

    return true;
}

bool VertexBuffer::Impl::update(const void* data_ptr, uint32_t size, uint32_t offset) {
    if (!buffer) {
        std::fprintf(stderr, "VertexBuffer '%s': Cannot update uninitialized buffer\n", desc.name);
        return false;
    }

    auto device_impl = get_device_impl(device);
    if (!device_impl || !device_impl->context) {
        std::fprintf(stderr, "Invalid device implementation\n");
        return false;
    }

    // Validate inputs
    if (!data_ptr) {
        std::fprintf(stderr, "VertexBuffer '%s': Null data pointer\n", desc.name);
        return false;
    }
    if (size == 0) {
        std::fprintf(stderr, "VertexBuffer '%s': Invalid update size: 0\n", desc.name);
        return false;
    }
    if (offset + size > desc.size) {
        std::fprintf(stderr, "VertexBuffer '%s': Update range [offset=%u, size=%u] exceeds buffer size (%u)\n",
            desc.name, offset, size, desc.size);
        return false;
    }

    // Update buffer using context's UpdateBuffer
    device_impl->context->UpdateBuffer(buffer, offset, size, data_ptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    return true;
}

} // namespace poko::render
