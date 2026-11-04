/**
 * @file framebuffer_impl.h
 * @brief Diligent Core implementation of Framebuffer.
 *
 * This header is private to the backend. It contains Diligent types and
 * implementation details. Public headers never include this.
 */

#ifndef POKO_RENDER_BACKEND_DILIGENT_FRAMEBUFFER_IMPL_H
#define POKO_RENDER_BACKEND_DILIGENT_FRAMEBUFFER_IMPL_H

#include "poko/render/framebuffer.h"
#include "poko/render/texture.h"

#include "Texture.h"
#include "TextureView.h"
#include "texture_impl.h"
#include <array>

namespace poko::render {

/**
 * @brief Diligent-specific implementation of Framebuffer.
 */
struct Framebuffer::Impl {
    std::array<Diligent::ITextureView*, 8> color_rtvs;  // Render target views
    Diligent::ITextureView* depth_dsv = nullptr;  // Depth stencil view
    std::array<Texture*, 8> color_attachments;  // Non-owning references
    Texture* depth_stencil_attachment = nullptr;  // Non-owning reference
    FramebufferDesc desc;
    Device* device = nullptr;  // Non-owning reference for validation
    uint32_t width = 0;
    uint32_t height = 0;
    std::string last_error;  // Last error message

    Impl();
    ~Impl();

    /**
     * @brief Initializes the framebuffer from a description.
     *
     * @param device Poko device.
     * @param desc Framebuffer description.
     * @return true on success.
     */
    bool initialize(Device* device, const FramebufferDesc& desc);
};

// Backend helper functions (friend to public classes)
inline Framebuffer::Impl* get_framebuffer_impl(Framebuffer* framebuffer) {
    return framebuffer ? framebuffer->impl_ : nullptr;
}

} // namespace poko::render

#endif // POKO_RENDER_BACKEND_DILIGENT_FRAMEBUFFER_IMPL_H
