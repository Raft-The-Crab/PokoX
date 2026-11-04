/**
 * @file framebuffer.h
 * @brief Poko-owned framebuffer abstraction for render-to-texture.
 *
 * This header contains only Poko types. The Diligent Core implementation lives
 * behind the Pimpl pattern in backend/diligent/. Consumers never need to include
 * Diligent headers.
 *
 * Framebuffers enable offscreen rendering (render-to-texture) and multiple
 * render targets (MRT) for advanced rendering techniques like deferred shading,
 * post-processing, and shadow mapping.
 *
 * Usage example:
 * @code
 * // Create a texture for offscreen rendering
 * Texture texture;
 * TextureDesc tex_desc;
 * tex_desc.width = 512;
 * tex_desc.height = 512;
 * tex_desc.format = TextureFormat::RGBA8_UNORM;
 * texture.initialize(device, tex_desc);
 *
 * // Create framebuffer with the texture
 * Framebuffer framebuffer;
 * FramebufferDesc fb_desc;
 * fb_desc.name = "OffscreenBuffer";
 * fb_desc.color_attachment_count = 1;
 * fb_desc.color_attachments[0].texture = &texture;
 * framebuffer.initialize(device, fb_desc);
 *
 * // Render to framebuffer
 * context.set_render_target(&framebuffer);
 * context.clear_render_target({0, 0, 0, 1});
 * // ... draw commands ...
 *
 * // Switch back to swap chain
 * context.set_render_target(swap_chain);
 * @endcode
 */

#ifndef POKO_RENDER_FRAMEBUFFER_H
#define POKO_RENDER_FRAMEBUFFER_H

#include "device.h"
#include "texture.h"
#include <cstdint>

namespace poko::render {

/**
 * @brief Framebuffer attachment description.
 */
struct FramebufferAttachment {
    Texture* texture = nullptr;      // Texture to attach (color or depth/stencil)
    uint32_t mip_level = 0;           // Mip level to attach
    uint32_t array_slice = 0;         // Array slice (for texture arrays)
};

/**
 * @brief Framebuffer description.
 */
struct FramebufferDesc {
    const char* name = nullptr;       // Debug name
    FramebufferAttachment color_attachments[8];  // Up to 8 color render targets
    uint32_t color_attachment_count = 0;         // Number of color attachments
    FramebufferAttachment depth_stencil;          // Depth/stencil attachment
    bool has_depth_stencil = false;               // Whether depth/stencil is attached
};

/**
 * @brief Framebuffer error codes.
 */
enum class FramebufferErrorCode {
    Success = 0,
    InvalidDevice,
    InvalidName,
    NoColorAttachments,
    InvalidAttachment,
    AttachmentResolutionMismatch,
    AttachmentSampleCountMismatch,
    CreationFailed,
    Unknown
};

/**
 * @brief Opaque handle to a framebuffer.
 *
 * Framebuffers allow rendering to textures (render-to-texture) and support
 * multiple render targets (MRT) for advanced rendering techniques.
 */
class Framebuffer {
public:
    Framebuffer();
    ~Framebuffer();

    // Non-copyable, non-movable (owns backend resources)
    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;
    Framebuffer(Framebuffer&&) = delete;
    Framebuffer& operator=(Framebuffer&&) = delete;

    /**
     * @brief Initializes the framebuffer from a description.
     *
     * All color attachments must have the same resolution and sample count.
     * Depth/stencil attachment must match resolution of color attachments.
     *
     * @param device The graphics device. Must remain valid for framebuffer lifetime.
     * @param desc Framebuffer description.
     * @return FramebufferErrorCode::Success on success, error code on failure.
     *
     * Error conditions (call get_last_error() for details):
     * - Device is null or invalid
     * - Name is null
     * - No color attachments specified
     * - Attachment texture is null or invalid
     * - Attachments have mismatched resolutions
     * - Attachments have mismatched sample counts
     * - Framebuffer creation failed
     */
    FramebufferErrorCode initialize(Device* device, const FramebufferDesc& desc);

    /**
     * @brief Releases framebuffer resources.
     */
    void release();

    /**
     * @brief Returns true if the framebuffer is valid.
     */
    bool is_valid() const;

    /**
     * @brief Returns the device that created this framebuffer.
     */
    Device* get_device() const;

    /**
     * @brief Returns the framebuffer name.
     */
    const char* get_name() const;

    /**
     * @brief Returns the number of color attachments.
     */
    uint32_t get_color_attachment_count() const;

    /**
     * @brief Returns a color attachment texture by index.
     *
     * @param index Attachment index (0-based).
     * @return Texture pointer, or nullptr if index is invalid.
     */
    Texture* get_color_attachment(uint32_t index) const;

    /**
     * @brief Returns the depth/stencil attachment texture.
     *
     * @return Texture pointer, or nullptr if no depth/stencil attached.
     */
    Texture* get_depth_stencil_attachment() const;

    /**
     * @brief Returns the framebuffer width (from first color attachment).
     */
    uint32_t get_width() const;

    /**
     * @brief Returns the framebuffer height (from first color attachment).
     */
    uint32_t get_height() const;

    /**
     * @brief Returns the last error message from framebuffer operations.
     */
    const char* get_last_error() const;

private:
    struct Impl;
    Impl* impl_;

    // Allow backend implementation to access private impl_
    friend Framebuffer::Impl* get_framebuffer_impl(Framebuffer*);
};

} // namespace poko::render

#endif // POKO_RENDER_FRAMEBUFFER_H
