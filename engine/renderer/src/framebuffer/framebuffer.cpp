/**
 * @file framebuffer.cpp
 * @brief Implementation of Poko Framebuffer.
 */

#include "poko/render/framebuffer.h"
#include "backend/diligent/internal/framebuffer_impl.h"
#include "backend/diligent/internal/texture_impl.h"

#include <new>

namespace poko::render {

Framebuffer::Framebuffer() : impl_(new (std::nothrow) Impl()) {}

Framebuffer::~Framebuffer() {
    delete impl_;
}

FramebufferErrorCode Framebuffer::initialize(Device* device, const FramebufferDesc& desc) {
    if (!impl_) {
        return FramebufferErrorCode::Unknown;
    }

    if (!device) {
        impl_->last_error = "Device is null";
        return FramebufferErrorCode::InvalidDevice;
    }

    if (!device->is_valid()) {
        impl_->last_error = "Device is invalid";
        return FramebufferErrorCode::InvalidDevice;
    }

    if (!desc.name) {
        impl_->last_error = "Name is null";
        return FramebufferErrorCode::InvalidName;
    }

    if (desc.color_attachment_count == 0) {
        impl_->last_error = "No color attachments specified";
        return FramebufferErrorCode::NoColorAttachments;
    }

    if (!impl_->initialize(device, desc)) {
        return FramebufferErrorCode::CreationFailed;
    }

    impl_->last_error.clear();
    return FramebufferErrorCode::Success;
}

void Framebuffer::release() {
    if (impl_) {
        impl_->~Impl();
        new (impl_) Impl();
    }
}

bool Framebuffer::is_valid() const {
    return impl_ != nullptr && impl_->color_rtvs[0] != nullptr;
}

Device* Framebuffer::get_device() const {
    return impl_ ? impl_->device : nullptr;
}

const char* Framebuffer::get_name() const {
    return impl_ ? (impl_->desc.name ? impl_->desc.name : "") : "";
}

uint32_t Framebuffer::get_color_attachment_count() const {
    return impl_ ? impl_->desc.color_attachment_count : 0;
}

Texture* Framebuffer::get_color_attachment(uint32_t index) const {
    if (!impl_ || index >= impl_->desc.color_attachment_count) {
        return nullptr;
    }
    return impl_->color_attachments[index];
}

Texture* Framebuffer::get_depth_stencil_attachment() const {
    return impl_ ? impl_->depth_stencil_attachment : nullptr;
}

uint32_t Framebuffer::get_width() const {
    return impl_ ? impl_->width : 0;
}

uint32_t Framebuffer::get_height() const {
    return impl_ ? impl_->height : 0;
}

const char* Framebuffer::get_last_error() const {
    if (!impl_) {
        return "Framebuffer implementation is null";
    }
    return impl_->last_error.c_str();
}

} // namespace poko::render
