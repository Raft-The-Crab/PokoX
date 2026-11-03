/**
 * @file material.cpp
 * @brief Implementation of Poko Material.
 */

#include "poko/render/material.h"
#include "poko/render/pipeline.h"
#include "backend/diligent/internal/material_impl.h"

#include <new>

namespace poko::render {

Material::Material() : impl_(new (std::nothrow) Impl()) {}

Material::~Material() {
    delete impl_;
}

MaterialErrorCode Material::initialize(Device* device, const MaterialDesc& desc) {
    if (!impl_) {
        return MaterialErrorCode::Unknown;
    }
    if (!device) {
        return MaterialErrorCode::InvalidDevice;
    }
    if (!device->is_valid()) {
        return MaterialErrorCode::InvalidDevice;
    }
    if (!desc.pipeline || !desc.pipeline->is_valid()) {
        return MaterialErrorCode::InvalidDevice;  // Pipeline required
    }
    if (desc.texture_count == 0) {
        return MaterialErrorCode::InvalidCount;
    }
    if (!desc.texture_bindings) {
        return MaterialErrorCode::InvalidTextures;
    }
    
    // Validate each texture binding
    for (uint32_t i = 0; i < desc.texture_count; ++i) {
        if (!desc.texture_bindings[i].texture) {
            return MaterialErrorCode::InvalidTextures;
        }
        if (!desc.texture_bindings[i].shader_variable) {
            return MaterialErrorCode::InvalidTextures;
        }
    }
    
    return impl_->initialize(device, desc) ? MaterialErrorCode::Success : MaterialErrorCode::MaterialCreationFailed;
}

bool Material::is_valid() const {
    return impl_ != nullptr && impl_->srb != nullptr;
}

} // namespace poko::render
