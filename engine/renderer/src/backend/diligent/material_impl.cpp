/**
 * @file material_impl.cpp
 * @brief Diligent Core implementation of Material.
 */

#include "backend/diligent/internal/material_impl.h"
#include "backend/diligent/internal/device_impl.h"
#include "backend/diligent/internal/texture_impl.h"
#include "backend/diligent/internal/pipeline_impl.h"

#include <cstdio>

namespace poko::render {

Material::Impl::~Impl() {
    if (srb) {
        srb->Release();
        srb = nullptr;
    }
}

bool Material::Impl::initialize(Device* device, const MaterialDesc& desc) {
    if (!device || !device->is_valid()) {
        std::fprintf(stderr, "Invalid device\n");
        return false;
    }

    auto device_impl = get_device_impl(device);
    if (!device_impl || !device_impl->device) {
        std::fprintf(stderr, "Invalid device implementation\n");
        return false;
    }

    this->device = device;
    this->desc = desc;

    // Validate inputs
    if (!desc.pipeline || !desc.pipeline->is_valid()) {
        std::fprintf(stderr, "Material requires a valid pipeline\n");
        return false;
    }
    if (desc.texture_count == 0) {
        std::fprintf(stderr, "Material must have at least one texture\n");
        return false;
    }
    if (desc.texture_count > 16) {
        std::fprintf(stderr, "Material texture count (%u) exceeds maximum (16)\n", desc.texture_count);
        return false;
    }
    if (!desc.texture_bindings) {
        std::fprintf(stderr, "Invalid texture bindings array\n");
        return false;
    }
    if (!desc.name) {
        std::fprintf(stderr, "Material must have a name\n");
        return false;
    }

    // Store texture references and shader variable names
    textures.resize(desc.texture_count);
    shader_variables.resize(desc.texture_count);
    array_indices.resize(desc.texture_count);
    
    for (uint32_t i = 0; i < desc.texture_count; ++i) {
        const auto& binding = desc.texture_bindings[i];
        if (!binding.texture || !binding.texture->is_valid()) {
            std::fprintf(stderr, "Invalid texture at binding index %u\n", i);
            return false;
        }
        if (!binding.shader_variable) {
            std::fprintf(stderr, "Missing shader variable name at binding index %u\n", i);
            return false;
        }
        // Validate device compatibility
        auto texture_device = binding.texture->get_device();
        if (texture_device && texture_device != device) {
            std::fprintf(stderr, "Texture at binding index %u created from different device\n", i);
            return false;
        }
        textures[i] = binding.texture;
        shader_variables[i] = binding.shader_variable;
        array_indices[i] = binding.array_index;
    }

    // Get pipeline implementation
    auto pipeline_impl = desc.pipeline->impl_;
    if (!pipeline_impl || !pipeline_impl->pipeline) {
        std::fprintf(stderr, "Invalid pipeline implementation\n");
        return false;
    }

    // Validate pipeline device compatibility
    if (pipeline_impl->device && pipeline_impl->device != device) {
        std::fprintf(stderr, "Pipeline created from different device\n");
        return false;
    }

    // Create SRB from pipeline's resource signature
    pipeline_impl->pipeline->CreateShaderResourceBinding(&srb, true);
    if (!srb) {
        std::fprintf(stderr, "Failed to create SRB from pipeline\n");
        return false;
    }

    // Bind all textures to SRB
    for (uint32_t i = 0; i < desc.texture_count; ++i) {
        auto texture_impl = textures[i]->impl_;
        if (!texture_impl) {
            std::fprintf(stderr, "Invalid texture implementation at binding index %u\n", i);
            return false;
        }

        // Create texture view if not already created
        if (!texture_impl->srv) {
            Diligent::TextureViewDesc srv_desc;
            srv_desc.ViewType = Diligent::TEXTURE_VIEW_SHADER_RESOURCE;
            srv_desc.TextureDim = Diligent::RESOURCE_DIM_TEX_2D;
            texture_impl->texture->CreateView(srv_desc, &texture_impl->srv);
        }

        if (!texture_impl->srv) {
            std::fprintf(stderr, "Failed to create texture SRV at binding index %u\n", i);
            return false;
        }

        // Bind texture to SRB variable
        auto* texture_var = srb->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, shader_variables[i].c_str());
        if (texture_var) {
            texture_var->Set(texture_impl->srv);
        } else {
            std::fprintf(stderr, "Warning: Shader variable '%s' not found in SRB (binding index %u)\n", 
                shader_variables[i].c_str(), i);
        }
    }

    return true;
}

} // namespace poko::render
