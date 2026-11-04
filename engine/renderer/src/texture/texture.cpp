/**
 * @file texture.cpp
 * @brief Implementation of Poko Texture.
 */

#include "poko/render/texture.h"
#include "backend/diligent/internal/texture_impl.h"

#include <new>

namespace poko::render {

Texture::Texture() : impl_(new (std::nothrow) Impl()) {}

Texture::~Texture() {
    delete impl_;
}

TextureErrorCode Texture::initialize(Device* device, const TextureDesc& desc) {
    if (!impl_) {
        return TextureErrorCode::Unknown;
    }
    if (!device) {
        return TextureErrorCode::InvalidDevice;
    }
    if (!device->is_valid()) {
        return TextureErrorCode::InvalidDevice;
    }
    if (desc.width == 0 || desc.height == 0) {
        return TextureErrorCode::InvalidSize;
    }
    if (desc.type != TextureType::Texture2D) {
        return TextureErrorCode::InvalidType;
    }
    
    return impl_->initialize(device, desc) ? TextureErrorCode::Success : TextureErrorCode::TextureCreationFailed;
}

bool Texture::is_valid() const {
    return impl_ != nullptr && impl_->texture != nullptr;
}

Device* Texture::get_device() const {
    return impl_ ? impl_->device : nullptr;
}

TextureFormat Texture::get_format() const {
    return (impl_ && impl_->texture) ? impl_->desc.format : TextureFormat::Unknown;
}

const char* Texture::get_name() const {
    return impl_ ? (impl_->desc.name ? impl_->desc.name : "") : "";
}

const char* Texture::get_last_error() const {
    if (!impl_) {
        return "Texture implementation is null";
    }
    return impl_->last_error.c_str();
}

} // namespace poko::render
