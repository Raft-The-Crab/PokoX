/**
 * @file framebuffer_impl.cpp
 * @brief Diligent Core implementation of Framebuffer.
 */

#include "backend/diligent/internal/framebuffer_impl.h"
#include "backend/diligent/internal/texture_impl.h"
#include "backend/diligent/internal/device_impl.h"

#include <new>

namespace poko::render {

Framebuffer::Impl::Impl() {
    color_attachments.fill(nullptr);
    color_rtvs.fill(nullptr);
}

Framebuffer::Impl::~Impl() {
    for (auto& rtv : color_rtvs) {
        if (rtv) {
            rtv->Release();
            rtv = nullptr;
        }
    }
    if (depth_dsv) {
        depth_dsv->Release();
        depth_dsv = nullptr;
    }
}

bool Framebuffer::Impl::initialize(Device* device, const FramebufferDesc& desc) {
    // Validate device
    if (!device || !device->is_valid()) {
        last_error = "Device is null or invalid";
        return false;
    }

    // Validate name
    if (!desc.name) {
        last_error = "Framebuffer name is null";
        return false;
    }

    // Validate color attachments
    if (desc.color_attachment_count == 0) {
        last_error = "Framebuffer must have at least one color attachment";
        return false;
    }

    if (desc.color_attachment_count > 8) {
        last_error = "Framebuffer cannot have more than 8 color attachments";
        return false;
    }

    // Get device implementation
    auto device_impl = get_device_impl(device);
    if (!device_impl || !device_impl->device) {
        last_error = "Failed to get device implementation";
        return false;
    }

    // Validate all color attachments
    uint32_t first_width = 0;
    uint32_t first_height = 0;

    for (uint32_t i = 0; i < desc.color_attachment_count; ++i) {
        Texture* texture = desc.color_attachments[i].texture;
        if (!texture || !texture->is_valid()) {
            last_error = "Color attachment texture is null or invalid";
            return false;
        }

        auto texture_impl = texture->impl_;
        if (!texture_impl || !texture_impl->texture) {
            last_error = "Failed to get texture implementation";
            return false;
        }

        // Check device compatibility
        if (texture_impl->device != device) {
            last_error = "Color attachment texture created from different device";
            return false;
        }

        // Get texture properties
        uint32_t width = texture_impl->desc.width;
        uint32_t height = texture_impl->desc.height;

        // First attachment defines reference resolution
        if (i == 0) {
            first_width = width;
            first_height = height;
        } else {
            // Check resolution match
            if (width != first_width || height != first_height) {
                last_error = "Color attachments have mismatched resolution";
                return false;
            }
        }

        color_attachments[i] = texture;
    }

    // Validate depth/stencil attachment if present
    if (desc.has_depth_stencil) {
        Texture* texture = desc.depth_stencil.texture;
        if (!texture || !texture->is_valid()) {
            last_error = "Depth/stencil attachment texture is null or invalid";
            return false;
        }

        auto texture_impl = texture->impl_;
        if (!texture_impl || !texture_impl->texture) {
            last_error = "Failed to get depth/stencil texture implementation";
            return false;
        }

        // Check device compatibility
        if (texture_impl->device != device) {
            last_error = "Depth/stencil attachment texture created from different device";
            return false;
        }

        // Check resolution match
        uint32_t width = texture_impl->desc.width;
        uint32_t height = texture_impl->desc.height;
        if (width != first_width || height != first_height) {
            last_error = "Depth/stencil attachment resolution does not match color attachments";
            return false;
        }

        depth_stencil_attachment = texture;
    }

    // Store description
    this->desc = desc;
    this->device = device;
    this->width = first_width;
    this->height = first_height;

    // Create render target views for color attachments
    for (uint32_t i = 0; i < desc.color_attachment_count; ++i) {
        auto texture_impl = color_attachments[i]->impl_;
        color_rtvs[i] = texture_impl->texture->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET);
        if (!color_rtvs[i]) {
            last_error = "Failed to create render target view for color attachment";
            return false;
        }
        color_rtvs[i]->AddRef();
    }

    // Create depth stencil view if present
    if (desc.has_depth_stencil) {
        auto texture_impl = depth_stencil_attachment->impl_;
        depth_dsv = texture_impl->texture->GetDefaultView(Diligent::TEXTURE_VIEW_DEPTH_STENCIL);
        if (!depth_dsv) {
            last_error = "Failed to create depth stencil view";
            return false;
        }
        depth_dsv->AddRef();
    }

    last_error.clear();
    return true;
}

} // namespace poko::render
