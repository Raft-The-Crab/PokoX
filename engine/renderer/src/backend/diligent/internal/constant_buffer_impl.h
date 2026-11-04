/**
 * @file constant_buffer_impl.h
 * @brief Diligent Core implementation of ConstantBuffer.
 */

#ifndef POKO_RENDER_BACKEND_DILIGENT_CONSTANT_BUFFER_IMPL_H
#define POKO_RENDER_BACKEND_DILIGENT_CONSTANT_BUFFER_IMPL_H

#include "poko/render/constant_buffer.h"

#include "Buffer.h"
#include <vector>

namespace poko::render {

struct ConstantBuffer::Impl {
    Diligent::IBuffer* buffer = nullptr;
    std::vector<uint8_t> data;  // Copied data for backend reference
    ConstantBufferDesc desc;
    Device* device = nullptr;  // Non-owning reference for validation
    std::string last_error;  // Last error message

    Impl() = default;
    ~Impl();

    bool initialize(Device* device, const ConstantBufferDesc& desc);
    bool update(const void* data, uint32_t size, uint32_t offset);
};

} // namespace poko::render

#endif // POKO_RENDER_BACKEND_DILIGENT_CONSTANT_BUFFER_IMPL_H
