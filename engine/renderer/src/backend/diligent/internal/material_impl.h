/**
 * @file material_impl.h
 * @brief Diligent Core implementation of Material.
 */

#ifndef POKO_RENDER_BACKEND_DILIGENT_MATERIAL_IMPL_H
#define POKO_RENDER_BACKEND_DILIGENT_MATERIAL_IMPL_H

#include "poko/render/material.h"

#include "ShaderResourceBinding.h"
#include <vector>
#include <string>

namespace poko::render {

struct Material::Impl {
    Diligent::IShaderResourceBinding* srb = nullptr;
    std::vector<Texture*> textures;  // Non-owning references
    std::vector<std::string> shader_variables;  // Shader variable names
    std::vector<uint32_t> array_indices;  // Array indices
    std::vector<ConstantBuffer*> constant_buffers;  // Non-owning references (M7)
    std::vector<std::string> cb_shader_variables;  // Constant buffer shader variable names (M7)
    MaterialDesc desc;
    Device* device = nullptr;
    std::string last_error;  // Last error message

    Impl() = default;
    ~Impl();

    bool initialize(Device* device, const MaterialDesc& desc);
};

} // namespace poko::render

#endif // POKO_RENDER_BACKEND_DILIGENT_MATERIAL_IMPL_H
