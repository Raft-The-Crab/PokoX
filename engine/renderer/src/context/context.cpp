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

#include <new>

namespace poko::render {

Context::Context() : impl_(new (std::nothrow) Impl()) {}

Context::~Context() {
    delete impl_;
}

void Context::set_viewport(const Viewport& viewport) {
    if (!impl_ || !impl_->context) {
        std::fprintf(stderr, "Context: set_viewport called on invalid context\n");
        return;
    }

    // Validate viewport dimensions
    if (viewport.width <= 0 || viewport.height <= 0) {
        std::fprintf(stderr, "Context: Invalid viewport dimensions (%.0f x %.0f)\n", 
            viewport.width, viewport.height);
        return;
    }
    if (viewport.min_depth < 0.0f || viewport.min_depth > 1.0f) {
        std::fprintf(stderr, "Context: Invalid min_depth (%.2f), must be in [0,1]\n", viewport.min_depth);
        return;
    }
    if (viewport.max_depth < 0.0f || viewport.max_depth > 1.0f) {
        std::fprintf(stderr, "Context: Invalid max_depth (%.2f), must be in [0,1]\n", viewport.max_depth);
        return;
    }
    if (viewport.min_depth >= viewport.max_depth) {
        std::fprintf(stderr, "Context: min_depth (%.2f) must be less than max_depth (%.2f)\n",
            viewport.min_depth, viewport.max_depth);
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
}

void Context::clear_render_target(const ClearColor& color) {
    if (!impl_ || !impl_->context) {
        std::fprintf(stderr, "Context: clear_render_target called on invalid context\n");
        return;
    }
    
    if (!impl_->swap_chain) {
        std::fprintf(stderr, "Context: clear_render_target called without swap chain bound\n");
        return;
    }

    Diligent::ITextureView* rtv = impl_->swap_chain->GetCurrentBackBufferRTV();
    if (!rtv) {
        std::fprintf(stderr, "Context: Failed to get back buffer RTV for clear\n");
        return;
    }
    
    float clear_color[4] = {color.r, color.g, color.b, color.a};
    impl_->context->ClearRenderTarget(rtv, clear_color,
        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
}

void Context::clear_depth(float depth, uint8_t stencil) {
    if (!impl_ || !impl_->context) {
        std::fprintf(stderr, "Context: clear_depth called on invalid context\n");
        return;
    }
    
    if (!impl_->swap_chain) {
        std::fprintf(stderr, "Context: clear_depth called without swap chain bound\n");
        return;
    }

    Diligent::ITextureView* dsv = impl_->swap_chain->GetDepthBufferDSV();
    if (!dsv) {
        // Not an error - may not have depth buffer
        return;
    }
    
    impl_->context->ClearDepthStencil(dsv, Diligent::CLEAR_DEPTH_FLAG, depth, stencil,
        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
}

bool Context::set_pipeline_state(PipelineState* pipeline) {
    if (!impl_ || !impl_->context) {
        return false;
    }
    
    if (!pipeline || !pipeline->is_valid()) {
        return false;
    }

    auto pipeline_impl = pipeline->impl_;
    if (!pipeline_impl || !pipeline_impl->pipeline) {
        return false;
    }

    // Validate device compatibility
    if (impl_->poko_device && pipeline_impl->device) {
        if (impl_->poko_device != pipeline_impl->device) {
            std::fprintf(stderr, "Context: Pipeline '%s' created from different device\n", 
                pipeline_impl->desc.name ? pipeline_impl->desc.name : "(unnamed)");
            return false;
        }
    }

    impl_->context->SetPipelineState(pipeline_impl->pipeline);
    
    // Bind shader resource binding (required by Diligent)
    if (pipeline_impl->srb) {
        impl_->context->CommitShaderResources(pipeline_impl->srb,
            Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    }

    return true;
}

bool Context::set_render_target(SwapChain* swap_chain) {
    if (!impl_ || !impl_->context) {
        std::fprintf(stderr, "Context: set_render_target called on invalid context\n");
        return false;
    }
    
    if (!swap_chain || !swap_chain->is_valid()) {
        std::fprintf(stderr, "Context: set_render_target called with invalid swap chain\n");
        return false;
    }

    auto swap_chain_impl = get_swapchain_impl(swap_chain);
    if (!swap_chain_impl || !swap_chain_impl->swap_chain) {
        std::fprintf(stderr, "Context: Failed to get swap chain implementation\n");
        return false;
    }

    Diligent::ITextureView* rtv = swap_chain_impl->swap_chain->GetCurrentBackBufferRTV();
    if (!rtv) {
        std::fprintf(stderr, "Context: Failed to get back buffer RTV\n");
        return false;
    }
    
    impl_->context->SetRenderTargets(1, &rtv, nullptr,
        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    return true;
}

bool Context::set_vertex_buffer(VertexBuffer* buffer) {
    if (!impl_ || !impl_->context) {
        return false;
    }
    
    if (!buffer || !buffer->is_valid()) {
        return false;
    }

    auto buffer_impl = buffer->impl_;
    if (!buffer_impl || !buffer_impl->buffer) {
        return false;
    }

    // Validate device compatibility
    if (impl_->poko_device && buffer_impl->device) {
        if (impl_->poko_device != buffer_impl->device) {
            std::fprintf(stderr, "Context: VertexBuffer '%s' created from different device\n", 
                buffer_impl->desc.name ? buffer_impl->desc.name : "(unnamed)");
            return false;
        }
    }

    // Bind vertex buffer to slot 0 with stride from descriptor
    uint32_t stride = buffer_impl->desc.stride > 0 ? buffer_impl->desc.stride : 12;  // Default to Float3
    
    Diligent::IBuffer* buffer_list[] = {buffer_impl->buffer};
    uint64_t strides[] = {static_cast<uint64_t>(stride)};
    
    impl_->context->SetVertexBuffers(0, 1, buffer_list, strides, 
        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);

    return true;
}

bool Context::set_index_buffer(IndexBuffer* buffer) {
    if (!impl_ || !impl_->context) {
        return false;
    }
    
    if (!buffer || !buffer->is_valid()) {
        return false;
    }

    auto buffer_impl = buffer->impl_;
    if (!buffer_impl || !buffer_impl->buffer) {
        return false;
    }

    // Validate device compatibility
    if (impl_->poko_device && buffer_impl->device) {
        if (impl_->poko_device != buffer_impl->device) {
            std::fprintf(stderr, "Context: IndexBuffer '%s' created from different device\n", 
                buffer_impl->desc.name ? buffer_impl->desc.name : "(unnamed)");
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

    return true;
}

bool Context::draw(uint32_t vertex_count) {
    if (!impl_ || !impl_->context) {
        return false;
    }

    Diligent::DrawAttribs draw_attribs{vertex_count, Diligent::DRAW_FLAG_NONE};
    impl_->context->Draw(draw_attribs);
    return true;
}

bool Context::draw_indexed(uint32_t index_count, uint32_t start_index) {
    if (!impl_ || !impl_->context) {
        return false;
    }

    Diligent::DrawIndexedAttribs draw_attribs;
    draw_attribs.IndexType = impl_->bound_index_format;  // M4: Use tracked format
    draw_attribs.NumIndices = index_count;
    draw_attribs.FirstIndexLocation = start_index;
    draw_attribs.Flags = Diligent::DRAW_FLAG_NONE;
    
    impl_->context->DrawIndexed(draw_attribs);
    return true;
}

bool Context::set_texture(Texture* texture, uint32_t slot) {
    // M6: Texture binding should go through materials for proper SRB management
    // For simplicity, we'll defer direct texture binding and require set_material
    (void)texture;
    (void)slot;
    std::fprintf(stderr, "M6: Use set_material() for texture binding (SRB management)\n");
    return false;
}

bool Context::set_material(Material* material) {
    if (!impl_ || !impl_->context) {
        return false;
    }
    
    if (!material || !material->is_valid()) {
        return false;
    }

    auto material_impl = material->impl_;
    if (!material_impl || !material_impl->srb) {
        return false;
    }

    // Validate device compatibility
    if (impl_->poko_device && material_impl->device) {
        if (impl_->poko_device != material_impl->device) {
            std::fprintf(stderr, "Context: Material '%s' created from different device\n", 
                material_impl->desc.name ? material_impl->desc.name : "(unnamed)");
            return false;
        }
    }

    // Commit shader resources from material's SRB
    impl_->context->CommitShaderResources(material_impl->srb,
        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

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

bool Context::initialize(Device* device, SwapChain* swap_chain) {
    return impl_ && impl_->initialize(device, swap_chain);
}

} // namespace poko::render
