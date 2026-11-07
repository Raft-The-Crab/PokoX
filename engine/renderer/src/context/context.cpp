/**
 * @file context.cpp
 * @brief Implementation of Poko Context.
 */

#include "poko/render/context.h"
#include "backend/diligent/internal/context_impl.h"
#include "backend/diligent/internal/pipeline_impl.h"
#include "backend/diligent/internal/vertex_buffer_impl.h"
#include "backend/diligent/internal/index_buffer_impl.h"
#include "backend/diligent/internal/swapchain_impl.h"
#include "backend/diligent/internal/texture_impl.h"
#include "backend/diligent/internal/material_impl.h"
#include "backend/diligent/internal/framebuffer_impl.h"

#include <new>
#include <string>

namespace poko::render {

Context::Context() : impl_(new (std::nothrow) Impl()) {}

Context::~Context() {
    delete impl_;
}

void Context::set_viewport(const Viewport& viewport) {
    if (!impl_ || !impl_->context) {
        impl_->last_error = "Context is invalid";
        return;
    }

    // Validate viewport dimensions
    if (viewport.width <= 0 || viewport.height <= 0) {
        impl_->last_error = "Invalid viewport dimensions: width and height must be positive";
        return;
    }
    if (viewport.min_depth < 0.0f || viewport.min_depth > 1.0f) {
        impl_->last_error = "Invalid min_depth: must be in [0, 1]";
        return;
    }
    if (viewport.max_depth < 0.0f || viewport.max_depth > 1.0f) {
        impl_->last_error = "Invalid max_depth: must be in [0, 1]";
        return;
    }
    if (viewport.min_depth >= viewport.max_depth) {
        impl_->last_error = "min_depth must be less than max_depth";
        return;
    }

    Diligent::Viewport vp;
    vp.TopLeftX = viewport.x;
    vp.TopLeftY = viewport.y;
    vp.Width = viewport.width;
    vp.Height = viewport.height;
    vp.MinDepth = viewport.min_depth;
    vp.MaxDepth = viewport.max_depth;

    uint32_t width = static_cast<uint32_t>(viewport.width);
    uint32_t height = static_cast<uint32_t>(viewport.height);

    impl_->context->SetViewports(1, &vp, width, height);
    impl_->last_error.clear();
}

void Context::set_scissor_rect(const ScissorRect& scissor) {
    if (!impl_ || !impl_->context) {
        impl_->last_error = "Context is invalid";
        return;
    }

    // Validate scissor rectangle
    if (scissor.left < 0 || scissor.top < 0) {
        impl_->last_error = "Invalid scissor: left and top must be non-negative";
        return;
    }
    if (scissor.right <= scissor.left || scissor.bottom <= scissor.top) {
        impl_->last_error = "Invalid scissor: right must be > left, bottom must be > top";
        return;
    }

    Diligent::Rect rect;
    rect.left = scissor.left;
    rect.top = scissor.top;
    rect.right = scissor.right;
    rect.bottom = scissor.bottom;

    impl_->context->SetScissorRects(1, &rect, 0, 0);
    impl_->last_error.clear();
}

void Context::clear_render_target(const ClearColor& color) {
    if (!impl_ || !impl_->context) {
        impl_->last_error = "Context is invalid";
        return;
    }

    if (!impl_->swap_chain) {
        impl_->last_error = "No swap chain bound for clear_render_target";
        return;
    }

    Diligent::ITextureView* rtv = impl_->swap_chain->GetCurrentBackBufferRTV();
    if (!rtv) {
        impl_->last_error = "Failed to get back buffer RTV";
        return;
    }

    float clear_color[4] = {color.r, color.g, color.b, color.a};
    impl_->context->ClearRenderTarget(rtv, clear_color,
        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    impl_->last_error.clear();
}

void Context::clear_depth(float depth, uint8_t stencil) {
    if (!impl_ || !impl_->context) {
        impl_->last_error = "Context is invalid";
        return;
    }

    if (!impl_->swap_chain) {
        impl_->last_error = "No swap chain bound for clear_depth";
        return;
    }

    Diligent::ITextureView* dsv = impl_->swap_chain->GetDepthBufferDSV();
    if (!dsv) {
        // Not an error - may not have depth buffer
        return;
    }

    impl_->context->ClearDepthStencil(dsv, Diligent::CLEAR_DEPTH_FLAG, depth, stencil,
        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    impl_->last_error.clear();
}

bool Context::set_pipeline_state(PipelineState* pipeline) {
    if (!impl_ || !impl_->context) {
        impl_->last_error = "Context is invalid";
        return false;
    }

    if (!pipeline || !pipeline->is_valid()) {
        impl_->last_error = "Pipeline is invalid";
        return false;
    }

    auto pipeline_impl = pipeline->impl_;
    if (!pipeline_impl || !pipeline_impl->pipeline) {
        impl_->last_error = "Pipeline implementation is invalid";
        return false;
    }

    // Validate device compatibility
    if (impl_->poko_device && pipeline_impl->device) {
        if (impl_->poko_device != pipeline_impl->device) {
            impl_->last_error = "Pipeline created from different device";
            return false;
        }
    }

    impl_->context->SetPipelineState(pipeline_impl->pipeline);

    // Bind shader resource binding (required by Diligent)
    if (pipeline_impl->srb) {
        impl_->context->CommitShaderResources(pipeline_impl->srb,
            Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    }

    impl_->last_error.clear();
    return true;
}

bool Context::set_render_target(SwapChain* swap_chain) {
    if (!impl_ || !impl_->context) {
        impl_->last_error = "Context is invalid";
        return false;
    }

    if (!swap_chain || !swap_chain->is_valid()) {
        impl_->last_error = "Swap chain is invalid";
        return false;
    }

    auto swap_chain_impl = get_swapchain_impl(swap_chain);
    if (!swap_chain_impl || !swap_chain_impl->swap_chain) {
        impl_->last_error = "Failed to get swap chain implementation";
        return false;
    }

    Diligent::ITextureView* rtv = swap_chain_impl->swap_chain->GetCurrentBackBufferRTV();
    if (!rtv) {
        impl_->last_error = "Failed to get back buffer RTV";
        return false;
    }

    Diligent::ITextureView* dsv = swap_chain_impl->swap_chain->GetDepthBufferDSV();
    impl_->context->SetRenderTargets(1, &rtv, dsv,
        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    impl_->last_error.clear();
    return true;
}

bool Context::set_render_target(Framebuffer* framebuffer) {
    if (!impl_ || !impl_->context) {
        impl_->last_error = "Context is invalid";
        return false;
    }

    if (!framebuffer || !framebuffer->is_valid()) {
        impl_->last_error = "Framebuffer is invalid";
        return false;
    }

    auto framebuffer_impl = get_framebuffer_impl(framebuffer);
    if (!framebuffer_impl) {
        impl_->last_error = "Failed to get framebuffer implementation";
        return false;
    }

    // Validate device compatibility
    if (impl_->poko_device && framebuffer_impl->device) {
        if (impl_->poko_device != framebuffer_impl->device) {
            impl_->last_error = "Framebuffer created from different device";
            return false;
        }
    }

    // Set render targets using framebuffer's views
    uint32_t num_rtvs = framebuffer_impl->desc.color_attachment_count;
    impl_->context->SetRenderTargets(num_rtvs, framebuffer_impl->color_rtvs.data(),
        framebuffer_impl->depth_dsv,
        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    impl_->last_error.clear();
    return true;
}

bool Context::set_vertex_buffer(VertexBuffer* buffer, uint32_t slot) {
    if (!impl_ || !impl_->context) {
        impl_->last_error = "Context is invalid";
        return false;
    }

    if (!buffer || !buffer->is_valid()) {
        impl_->last_error = "Vertex buffer is invalid";
        return false;
    }

    auto buffer_impl = buffer->impl_;
    if (!buffer_impl || !buffer_impl->buffer) {
        impl_->last_error = "Vertex buffer implementation is invalid";
        return false;
    }

    // Validate device compatibility
    if (impl_->poko_device && buffer_impl->device) {
        if (impl_->poko_device != buffer_impl->device) {
            impl_->last_error = "Vertex buffer created from different device";
            return false;
        }
    }

    // Validate slot (M9)
    if (slot >= 16) {
        impl_->last_error = "Vertex buffer slot must be < 16";
        return false;
    }

    // Bind vertex buffer to specified slot with stride from descriptor
    uint32_t stride = buffer_impl->desc.stride > 0 ? buffer_impl->desc.stride : 12;  // Default to Float3

    Diligent::IBuffer* buffer_list[] = {buffer_impl->buffer};
    uint64_t strides[] = {static_cast<uint64_t>(stride)};

    impl_->context->SetVertexBuffers(slot, 1, buffer_list, strides,
        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
        slot == 0 ? Diligent::SET_VERTEX_BUFFERS_FLAG_RESET : Diligent::SET_VERTEX_BUFFERS_FLAG_NONE);
    impl_->last_error.clear();
    return true;
}

bool Context::set_index_buffer(IndexBuffer* buffer) {
    if (!impl_ || !impl_->context) {
        impl_->last_error = "Context is invalid";
        return false;
    }

    if (!buffer || !buffer->is_valid()) {
        impl_->last_error = "Index buffer is invalid";
        return false;
    }

    auto buffer_impl = buffer->impl_;
    if (!buffer_impl || !buffer_impl->buffer) {
        impl_->last_error = "Index buffer implementation is invalid";
        return false;
    }

    // Validate device compatibility
    if (impl_->poko_device && buffer_impl->device) {
        if (impl_->poko_device != buffer_impl->device) {
            impl_->last_error = "Index buffer created from different device";
            return false;
        }
    }

    // Map Poko format to Diligent format
    Diligent::VALUE_TYPE format = Diligent::VT_UINT16;
    switch (buffer_impl->desc.format) {
        case IndexFormat::UInt16:
            format = Diligent::VT_UINT16;
            break;
        case IndexFormat::UInt32:
            format = Diligent::VT_UINT32;
            break;
        default:
            format = Diligent::VT_UINT16;
            break;
    }

    impl_->bound_index_format = format;  // M4: Remember for draw_indexed
    impl_->context->SetIndexBuffer(buffer_impl->buffer, 0,
        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    impl_->last_error.clear();
    return true;
}

bool Context::draw(uint32_t vertex_count) {
    if (!impl_ || !impl_->context) {
        impl_->last_error = "Context is invalid";
        return false;
    }
    if (vertex_count == 0) {
        impl_->last_error = "Vertex count must be > 0";
        return false;
    }

    Diligent::DrawAttribs draw_attribs{vertex_count, Diligent::DRAW_FLAG_NONE};
    impl_->context->Draw(draw_attribs);
    impl_->last_error.clear();
    return true;
}

bool Context::draw_indexed(uint32_t index_count, uint32_t start_index) {
    if (!impl_ || !impl_->context) {
        impl_->last_error = "Context is invalid";
        return false;
    }
    if (index_count == 0) {
        impl_->last_error = "Index count must be > 0";
        return false;
    }

    Diligent::DrawIndexedAttribs draw_attribs;
    draw_attribs.IndexType = impl_->bound_index_format;
    draw_attribs.NumIndices = index_count;
    draw_attribs.FirstIndexLocation = start_index;
    draw_attribs.Flags = Diligent::DRAW_FLAG_NONE;

    impl_->context->DrawIndexed(draw_attribs);
    impl_->last_error.clear();
    return true;
}

bool Context::draw_instanced(uint32_t vertex_count, uint32_t instance_count, uint32_t start_instance) {
    if (!impl_ || !impl_->context) {
        impl_->last_error = "Context is invalid";
        return false;
    }
    if (vertex_count == 0) {
        impl_->last_error = "Vertex count must be > 0";
        return false;
    }
    if (instance_count == 0) {
        impl_->last_error = "Instance count must be > 0";
        return false;
    }

    Diligent::DrawAttribs draw_attribs;
    draw_attribs.NumVertices = vertex_count;
    draw_attribs.NumInstances = instance_count;
    draw_attribs.FirstInstanceLocation = start_instance;
    draw_attribs.Flags = Diligent::DRAW_FLAG_NONE;

    impl_->context->Draw(draw_attribs);
    impl_->last_error.clear();
    return true;
}

bool Context::draw_indexed_instanced(uint32_t index_count, uint32_t instance_count,
                                     uint32_t start_index, uint32_t start_instance) {
    if (!impl_ || !impl_->context) {
        impl_->last_error = "Context is invalid";
        return false;
    }
    if (index_count == 0) {
        impl_->last_error = "Index count must be > 0";
        return false;
    }
    if (instance_count == 0) {
        impl_->last_error = "Instance count must be > 0";
        return false;
    }

    Diligent::DrawIndexedAttribs draw_attribs;
    draw_attribs.IndexType = impl_->bound_index_format;
    draw_attribs.NumIndices = index_count;
    draw_attribs.NumInstances = instance_count;
    draw_attribs.FirstIndexLocation = start_index;
    draw_attribs.FirstInstanceLocation = start_instance;
    draw_attribs.Flags = Diligent::DRAW_FLAG_NONE;

    impl_->context->DrawIndexed(draw_attribs);
    impl_->last_error.clear();
    return true;
}

bool Context::draw_indirect(VertexBuffer* indirect_buffer, uint32_t offset) {
    if (!impl_ || !impl_->context) {
        impl_->last_error = "Context is invalid";
        return false;
    }

    if (!indirect_buffer || !indirect_buffer->is_valid()) {
        impl_->last_error = "Indirect buffer is invalid";
        return false;
    }

    auto buffer_impl = indirect_buffer->impl_;
    if (!buffer_impl || !buffer_impl->buffer) {
        impl_->last_error = "Indirect buffer implementation is invalid";
        return false;
    }

    // Validate device compatibility
    if (impl_->poko_device && buffer_impl->device) {
        if (impl_->poko_device != buffer_impl->device) {
            impl_->last_error = "Indirect buffer created from different device";
            return false;
        }
    }

    // Validate offset alignment (must be 4-byte aligned)
    if (offset % 4 != 0) {
        impl_->last_error = "Indirect buffer offset must be 4-byte aligned";
        return false;
    }

    // Set indirect buffer as vertex buffer (required by Diligent for indirect arguments)
    impl_->context->SetVertexBuffers(0, 1, &buffer_impl->buffer, nullptr,
        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
        Diligent::SET_VERTEX_BUFFERS_FLAG_NONE);

    // Create indirect draw arguments
    Diligent::DrawIndirectAttribs draw_attribs;
    draw_attribs.Flags = Diligent::DRAW_FLAG_NONE;
    draw_attribs.DrawArgsOffset = offset;

    impl_->context->DrawIndirect(draw_attribs);
    impl_->last_error.clear();
    return true;
}

bool Context::draw_indexed_indirect(VertexBuffer* indirect_buffer, uint32_t offset) {
    if (!impl_ || !impl_->context) {
        impl_->last_error = "Context is invalid";
        return false;
    }

    if (!indirect_buffer || !indirect_buffer->is_valid()) {
        impl_->last_error = "Indirect buffer is invalid";
        return false;
    }

    auto buffer_impl = indirect_buffer->impl_;
    if (!buffer_impl || !buffer_impl->buffer) {
        impl_->last_error = "Indirect buffer implementation is invalid";
        return false;
    }

    // Validate device compatibility
    if (impl_->poko_device && buffer_impl->device) {
        if (impl_->poko_device != buffer_impl->device) {
            impl_->last_error = "Indirect buffer created from different device";
            return false;
        }
    }

    // Validate offset alignment (must be 4-byte aligned)
    if (offset % 4 != 0) {
        impl_->last_error = "Indirect buffer offset must be 4-byte aligned";
        return false;
    }

    // Set indirect buffer as vertex buffer (required by Diligent for indirect arguments)
    impl_->context->SetVertexBuffers(0, 1, &buffer_impl->buffer, nullptr,
        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
        Diligent::SET_VERTEX_BUFFERS_FLAG_NONE);

    // Create indexed indirect draw arguments
    Diligent::DrawIndexedIndirectAttribs draw_attribs;
    draw_attribs.IndexType = impl_->bound_index_format;
    draw_attribs.Flags = Diligent::DRAW_FLAG_NONE;
    draw_attribs.DrawArgsOffset = offset;

    impl_->context->DrawIndexedIndirect(draw_attribs);
    impl_->last_error.clear();
    return true;
}

bool Context::set_texture(Texture* texture, uint32_t slot) {
    // M6: Texture binding should go through materials for proper SRB management
    // For simplicity, we'll defer direct texture binding and require set_material
    (void)texture;
    (void)slot;
    impl_->last_error = "Use set_material() for texture binding (SRB management)";
    return false;
}

bool Context::set_material(Material* material) {
    if (!impl_ || !impl_->context) {
        impl_->last_error = "Context is invalid";
        return false;
    }

    if (!material || !material->is_valid()) {
        impl_->last_error = "Material is invalid";
        return false;
    }

    auto material_impl = material->impl_;
    if (!material_impl || !material_impl->srb) {
        impl_->last_error = "Material implementation is invalid";
        return false;
    }

    // Validate device compatibility
    if (impl_->poko_device && material_impl->device) {
        if (impl_->poko_device != material_impl->device) {
            impl_->last_error = "Material created from different device";
            return false;
        }
    }

    // Commit shader resources from material's SRB
    impl_->context->CommitShaderResources(material_impl->srb,
        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    impl_->last_error.clear();
    return true;
}

void Context::flush() {
    if (impl_ && impl_->context) {
        impl_->context->Flush();
    }
}

bool Context::is_valid() const {
    return impl_ != nullptr && impl_->context != nullptr;
}

BackendType Context::backend_type() const {
    return impl_ ? impl_->backend_type : BackendType::Unknown;
}

const char* Context::get_last_error() const {
    if (!impl_) {
        return "Context implementation is null";
    }
    return impl_->last_error.c_str();
}

bool Context::initialize(Device* device, SwapChain* swap_chain) {
    return impl_ && impl_->initialize(device, swap_chain);
}

} // namespace poko::render
