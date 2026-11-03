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

#include <new>

namespace poko::render {

Context::Context() : impl_(new (std::nothrow) Impl()) {}

Context::~Context() {
    delete impl_;
}

void Context::set_viewport(const Viewport& viewport) {
    if (!impl_ || !impl_->context) {
        return;
    }

    Diligent::Viewport vp;
    vp.TopLeftX = viewport.x;
    vp.TopLeftY = viewport.y;
    vp.Width = viewport.width;
    vp.Height = viewport.height;
    vp.MinDepth = viewport.min_depth;
    vp.MaxDepth = viewport.max_depth;

    // Need width/height for reference; use reasonable defaults if swap chain not set
    uint32_t width = static_cast<uint32_t>(viewport.width > 0 ? viewport.width : 1280);
    uint32_t height = static_cast<uint32_t>(viewport.height > 0 ? viewport.height : 720);

    impl_->context->SetViewports(1, &vp, width, height);
}

void Context::clear_render_target(const ClearColor& color) {
    if (!impl_ || !impl_->context || !impl_->swap_chain) {
        return;
    }

    Diligent::ITextureView* rtv = impl_->swap_chain->GetCurrentBackBufferRTV();
    float clear_color[4] = {color.r, color.g, color.b, color.a};
    impl_->context->ClearRenderTarget(rtv, clear_color,
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

    // Validate device compatibility if we have a device reference
    // (For M2, we trust the caller provided compatible objects)

    impl_->context->SetPipelineState(pipeline_impl->pipeline);
    
    // Bind shader resource binding (required by Diligent)
    if (pipeline_impl->srb) {
        impl_->context->CommitShaderResources(pipeline_impl->srb,
            Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    }

    return true;
}

bool Context::set_render_target(SwapChain* swap_chain) {
    if (!impl_ || !impl_->context || !impl_->swap_chain) {
        return false;
    }
    
    if (!swap_chain || !swap_chain->is_valid()) {
        return false;
    }

    auto swap_chain_impl = get_swapchain_impl(swap_chain);
    if (!swap_chain_impl || !swap_chain_impl->swap_chain) {
        return false;
    }

    Diligent::ITextureView* rtv = swap_chain_impl->swap_chain->GetCurrentBackBufferRTV();
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
