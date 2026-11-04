/**
 * @file texture_impl.h
 * @brief Diligent Core implementation of Texture.
 */

#ifndef POKO_RENDER_BACKEND_DILIGENT_TEXTURE_IMPL_H
#define POKO_RENDER_BACKEND_DILIGENT_TEXTURE_IMPL_H

#include "poko/render/texture.h"

#include "Texture.h"
#include <vector>

namespace poko::render {

struct Texture::Impl {
    Diligent::ITexture* texture = nullptr;
    Diligent::ITextureView* srv = nullptr;  // Shader resource view
    std::vector<uint8_t> data;  // Copied data for backend reference
    TextureDesc desc;
    Device* device = nullptr;  // Non-owning reference for validation
    std::string last_error;  // Last error message

    Impl() = default;
    ~Impl();

    bool initialize(Device* device, const TextureDesc& desc);
};

} // namespace poko::render

#endif // POKO_RENDER_BACKEND_DILIGENT_TEXTURE_IMPL_H
