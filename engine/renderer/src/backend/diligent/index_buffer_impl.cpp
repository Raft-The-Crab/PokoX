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
        std::fprintf(stderr, "Invalid index buffer size: 0\n");
        return false;
    }

    // Validate format
    if (desc.format != IndexFormat::UInt16 && desc.format != IndexFormat::UInt32) {
        std::fprintf(stderr, "Invalid index format: %d\n", static_cast<int>(desc.format));
        return false;
    }

    // Copy data if provided
    if (desc.data && desc.size > 0) {
        data.resize(desc.size);
        std::memcpy(data.data(), desc.data, desc.size);
    }

    // Create Diligent buffer
    Diligent::BufferDesc bd;
    bd.Name = desc.name ? desc.name : "Poko IndexBuffer";
    bd.Size = desc.size;
    bd.BindFlags = Diligent::BIND_INDEX_BUFFER;
    bd.Usage = Diligent::USAGE_IMMUTABLE;  // M4: Static for now

    Diligent::BufferData buffer_data;
    buffer_data.pData = data.empty() ? nullptr : data.data();
    buffer_data.DataSize = desc.size;

    device_impl->device->CreateBuffer(bd, &buffer_data, &buffer);

    if (!buffer) {
        std::fprintf(stderr, "Failed to create index buffer\n");
        return false;
    }

    return true;
}

} // namespace poko::render
