/**
 * @file pipeline_impl.h
 * @brief Diligent Core implementation of PipelineState.
 */

#ifndef POKO_RENDER_BACKEND_DILIGENT_PIPELINE_IMPL_H
#define POKO_RENDER_BACKEND_DILIGENT_PIPELINE_IMPL_H

#include "poko/render/pipeline.h"

#include "RenderDevice.h"
#include "PipelineState.h"
#include "Shader.h"

#include <string>

namespace poko::render {

struct PipelineState::Impl {
    Diligent::IPipelineState* pipeline = nullptr;
    Diligent::IShaderResourceBinding* srb = nullptr;
    std::string vertex_shader_source;
    std::string pixel_shader_source;
    PipelineDesc desc;
    Device* device = nullptr;  // Non-owning reference for validation

    Impl() = default;
    ~Impl();

    bool initialize(Device* device, const PipelineDesc& desc);
};

} // namespace poko::render

#endif // POKO_RENDER_BACKEND_DILIGENT_PIPELINE_IMPL_H
