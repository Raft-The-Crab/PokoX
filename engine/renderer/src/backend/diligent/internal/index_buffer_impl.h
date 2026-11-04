/**
 * @file index_buffer_impl.h
 * @brief Diligent Core implementation of IndexBuffer.
 */

#ifndef POKO_RENDER_BACKEND_DILIGENT_INDEX_BUFFER_IMPL_H
#define POKO_RENDER_BACKEND_DILIGENT_INDEX_BUFFER_IMPL_H

#include "poko/render/index_buffer.h"

#include "Buffer.h"
#include <vector>

namespace poko::render {

struct IndexBuffer::Impl {
    Diligent::IBuffer* buffer = nullptr;
    std::vector<uint8_t> data;  // Copied data for backend reference
    IndexBufferDesc desc;
    Device* device = nullptr;  // Non-owning reference for validation

    Impl() = default;
    ~Impl();

    bool initialize(Device* device, const IndexBufferDesc& desc);
    bool update(const void* data, uint32_t size, uint32_t offset);
};

} // namespace poko::render

#endif // POKO_RENDER_BACKEND_DILIGENT_INDEX_BUFFER_IMPL_H
