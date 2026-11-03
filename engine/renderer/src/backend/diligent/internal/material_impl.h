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
    MaterialDesc desc;
    Device* device = nullptr;

    Impl() = default;
    ~Impl();

    bool initialize(Device* device, const MaterialDesc& desc);
};

} // namespace poko::render

#endif // POKO_RENDER_BACKEND_DILIGENT_MATERIAL_IMPL_H
