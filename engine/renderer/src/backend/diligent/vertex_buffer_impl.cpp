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
        return false;
    }

    auto device_impl = get_device_impl(device);
    if (!device_impl || !device_impl->device) {
        return false;
    }

    this->device = device;
    this->desc = desc;

    // Validate inputs
    if (desc.size == 0) {
        std::fprintf(stderr, "Invalid buffer size: 0\n");
        return false;
    }

    // Copy data if provided
    if (desc.data && desc.size > 0) {
        data.resize(desc.size);
        std::memcpy(data.data(), desc.data, desc.size);
    }

    // Create Diligent buffer
    Diligent::BufferDesc bd;
    bd.Name = desc.name ? desc.name : "Poko VertexBuffer";
    bd.Size = desc.size;
    bd.BindFlags = Diligent::BIND_VERTEX_BUFFER;
    bd.Usage = Diligent::USAGE_IMMUTABLE;  // M3: Static for now

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

} // namespace poko::render
