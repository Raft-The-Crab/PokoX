/**
 * @file material_impl.cpp
 * @brief Diligent Core implementation of Material.
 */

#include "backend/diligent/internal/material_impl.h"
#include "backend/diligent/internal/device_impl.h"
#include "backend/diligent/internal/texture_impl.h"
#include "backend/diligent/internal/pipeline_impl.h"
#include "backend/diligent/internal/constant_buffer_impl.h"

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
        last_error = "Device is invalid";
        return false;
    }

    auto device_impl = get_device_impl(device);
    if (!device_impl || !device_impl->device) {
        last_error = "Failed to get device implementation";
        return false;
    }

    this->device = device;
    this->desc = desc;

    // Validate inputs
    if (!desc.pipeline || !desc.pipeline->is_valid()) {
        last_error = "Material requires a valid pipeline";
        return false;
    }
    // M7: Allow materials with only constant buffers or only textures or both
    if (desc.texture_count == 0 && desc.constant_buffer_count == 0) {
        last_error = "Material must have at least one texture or constant buffer";
        return false;
    }
    if (desc.texture_count > 32) {
        last_error = "Texture count exceeds maximum (32)";
        return false;
    }
    if (desc.constant_buffer_count > 16) {
        last_error = "Constant buffer count exceeds maximum (16)";
        return false;
    }
    if (desc.texture_count > 0 && !desc.texture_bindings) {
        last_error = "Invalid texture bindings array";
        return false;
    }
    if (desc.constant_buffer_count > 0 && !desc.constant_buffer_bindings) {
        last_error = "Invalid constant buffer bindings array";
        return false;
    }
    if (!desc.name) {
        last_error = "Material must have a name";
        return false;
    }

    // Store texture references and shader variable names
    textures.resize(desc.texture_count);
    shader_variables.resize(desc.texture_count);
    array_indices.resize(desc.texture_count);

    for (uint32_t i = 0; i < desc.texture_count; ++i) {
        const auto& binding = desc.texture_bindings[i];
        if (!binding.texture || !binding.texture->is_valid()) {
            last_error = "Invalid texture at binding index";
            return false;
        }
        if (!binding.shader_variable) {
            last_error = "Missing shader variable name at binding index";
            return false;
        }
        // Validate array index
        if (binding.array_index >= 256) {
            last_error = "Array index exceeds maximum (255)";
            return false;
        }
        // Validate device compatibility
        auto texture_device = binding.texture->get_device();
        if (texture_device && texture_device != device) {
            last_error = "Texture created from different device";
            return false;
        }
        textures[i] = binding.texture;
        shader_variables[i] = binding.shader_variable;
        array_indices[i] = binding.array_index;
    }

    // M7: Store constant buffer references and shader variable names
    constant_buffers.resize(desc.constant_buffer_count);
    cb_shader_variables.resize(desc.constant_buffer_count);

    for (uint32_t i = 0; i < desc.constant_buffer_count; ++i) {
        const auto& binding = desc.constant_buffer_bindings[i];
        if (!binding.buffer || !binding.buffer->is_valid()) {
            last_error = "Invalid constant buffer at binding index";
            return false;
        }
        if (!binding.shader_variable) {
            last_error = "Missing shader variable name at binding index";
            return false;
        }
        // Validate device compatibility
        auto cb_device = binding.buffer->get_device();
        if (cb_device && cb_device != device) {
            last_error = "Constant buffer created from different device";
            return false;
        }
        constant_buffers[i] = binding.buffer;
        cb_shader_variables[i] = binding.shader_variable;
    }

    // Get pipeline implementation
    auto pipeline_impl = desc.pipeline->impl_;
    if (!pipeline_impl || !pipeline_impl->pipeline) {
        last_error = "Invalid pipeline implementation";
        return false;
    }

    // Validate pipeline device compatibility
    if (pipeline_impl->device && pipeline_impl->device != device) {
        last_error = "Pipeline created from different device";
        return false;
    }

    // Create SRB from pipeline's resource signature
    // Use the explicit resource signature if available, otherwise use the pipeline's implicit one
    if (pipeline_impl->resource_signature) {
        pipeline_impl->resource_signature->CreateShaderResourceBinding(&srb, true);
    } else {
        pipeline_impl->pipeline->CreateShaderResourceBinding(&srb, true);
    }

    if (!srb) {
        last_error = "Failed to create SRB from pipeline";
        return false;
    }

    // Bind all textures to SRB
    for (uint32_t i = 0; i < desc.texture_count; ++i) {
        auto texture_impl = textures[i]->impl_;
        if (!texture_impl) {
            last_error = "Invalid texture implementation at binding index";
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
            last_error = "Failed to create texture SRV at binding index";
            return false;
        }

        // Bind texture to SRB variable
        auto* texture_var = srb->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, shader_variables[i].c_str());
        if (texture_var) {
            texture_var->Set(texture_impl->srv);
        }
        // Note: Missing shader variable is not an error - may be unused in shader
    }

    // M7: Bind all constant buffers to SRB
    for (uint32_t i = 0; i < desc.constant_buffer_count; ++i) {
        auto cb_impl = constant_buffers[i]->impl_;
        if (!cb_impl) {
            last_error = "Invalid constant buffer implementation at binding index";
            return false;
        }

        // Bind constant buffer to SRB variable
        auto* cb_var = srb->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, cb_shader_variables[i].c_str());
        if (cb_var) {
            cb_var->Set(cb_impl->buffer);
        }
        // Note: Missing shader variable is not an error - may be unused in shader
    }

    last_error.clear();
    return true;
}

} // namespace poko::render
