/**
 * @file vertex_buffer_impl.h
 * @brief Diligent Core implementation of VertexBuffer.
 */

#ifndef POKO_RENDER_BACKEND_DILIGENT_VERTEX_BUFFER_IMPL_H
#define POKO_RENDER_BACKEND_DILIGENT_VERTEX_BUFFER_IMPL_H

#include "poko/render/vertex_buffer.h"

#include "Buffer.h"
#include <vector>

namespace poko::render {

struct VertexBuffer::Impl {
    Diligent::IBuffer* buffer = nullptr;
    std::vector<uint8_t> data;  // Copied data for backend reference
    BufferDesc desc;
    Device* device = nullptr;  // Non-owning reference for validation

    Impl() = default;
    ~Impl();

    bool initialize(Device* device, const BufferDesc& desc);
};

} // namespace poko::render

#endif // POKO_RENDER_BACKEND_DILIGENT_VERTEX_BUFFER_IMPL_H
