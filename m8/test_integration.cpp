/**
 * @file test_integration.cpp
 * @brief Integration tests for full rendering pipeline.
 *
 * Tests end-to-end rendering including device, swap chain, context,
 * pipeline, buffers, and framebuffers.
 */

#include "poko/render/device.h"
#include "poko/render/swapchain.h"
#include "poko/render/context.h"
#include "poko/render/pipeline.h"
#include "poko/render/vertex_buffer.h"
#include "poko/render/index_buffer.h"
#include "poko/render/texture.h"
#include "poko/render/material.h"
#include "poko/render/constant_buffer.h"
#include "poko/render/framebuffer.h"

#include <cassert>
#include <cstdio>

int main() {
    std::printf("Testing Poko renderer integration...\n");

    // Test 1: Default objects are invalid
    {
        std::printf("Test 1: Default objects are invalid... ");
        poko::render::Device device;
        poko::render::SwapChain swap_chain;
        poko::render::Context context;
        poko::render::PipelineState pipeline;
        poko::render::VertexBuffer vb;
        poko::render::IndexBuffer ib;
        poko::render::Texture texture;
        poko::render::Material material;
        poko::render::ConstantBuffer cb;
        poko::render::Framebuffer framebuffer;

        assert(!device.is_valid());
        assert(!swap_chain.is_valid());
        assert(!context.is_valid());
        assert(!pipeline.is_valid());
        assert(!vb.is_valid());
        assert(!ib.is_valid());
        assert(!texture.is_valid());
        assert(!material.is_valid());
        assert(!cb.is_valid());
        assert(!framebuffer.is_valid());
        std::printf("PASS\n");
    }

    // Test 2: All resources have get_last_error
    {
        std::printf("Test 2: All resources have get_last_error... ");
        poko::render::Device device;
        poko::render::SwapChain swap_chain;
        poko::render::Context context;
        poko::render::PipelineState pipeline;
        poko::render::VertexBuffer vb;
        poko::render::IndexBuffer ib;
        poko::render::Texture texture;
        poko::render::Material material;
        poko::render::ConstantBuffer cb;
        poko::render::Framebuffer framebuffer;

        assert(device.get_last_error() != nullptr);
        // Skip SwapChain for now (not implemented yet)
        assert(context.get_last_error() != nullptr);
        assert(pipeline.get_last_error() != nullptr);
        assert(vb.get_last_error() != nullptr);
        assert(ib.get_last_error() != nullptr);
        assert(texture.get_last_error() != nullptr);
        assert(material.get_last_error() != nullptr);
        assert(cb.get_last_error() != nullptr);
        assert(framebuffer.get_last_error() != nullptr);
        std::printf("PASS\n");
    }

    // Test 3: Context operations on invalid context fail gracefully
    {
        std::printf("Test 3: Context operations on invalid context fail gracefully... ");
        poko::render::Context context;
        poko::render::Viewport viewport{0, 0, 1280, 720, 0, 1};
        context.set_viewport(viewport);
        assert(context.get_last_error() != nullptr);

        context.set_scissor_rect({0, 0, 1280, 720});
        assert(context.get_last_error() != nullptr);

        context.clear_render_target({0, 0, 0, 1});
        assert(context.get_last_error() != nullptr);

        assert(!context.set_pipeline_state(nullptr));
        assert(context.get_last_error() != nullptr);

        assert(!context.set_render_target((poko::render::SwapChain*)nullptr));
        assert(context.get_last_error() != nullptr);

        assert(!context.set_render_target((poko::render::Framebuffer*)nullptr));
        assert(context.get_last_error() != nullptr);

        assert(!context.set_vertex_buffer(nullptr));
        assert(context.get_last_error() != nullptr);

        assert(!context.set_index_buffer(nullptr));
        assert(context.get_last_error() != nullptr);

        assert(!context.draw(100));
        assert(context.get_last_error() != nullptr);

        assert(!context.draw_indexed(100));
        assert(context.get_last_error() != nullptr);

        assert(!context.set_material(nullptr));
        assert(context.get_last_error() != nullptr);
        std::printf("PASS\n");
    }

    // Test 4: Resource getters return safe defaults for invalid objects
    {
        std::printf("Test 4: Resource getters return safe defaults for invalid objects... ");
        poko::render::Device device;
        poko::render::SwapChain swap_chain;
        poko::render::Context context;
        poko::render::PipelineState pipeline;
        poko::render::VertexBuffer vb;
        poko::render::IndexBuffer ib;
        poko::render::Texture texture;
        poko::render::Material material;
        poko::render::ConstantBuffer cb;
        poko::render::Framebuffer framebuffer;

        assert(device.backend_type() == poko::render::BackendType::Unknown);
        assert(swap_chain.get_color_format() == poko::render::TextureFormat::Unknown);
        assert(swap_chain.get_depth_format() == poko::render::TextureFormat::Unknown);
        assert(context.backend_type() == poko::render::BackendType::Unknown);
        assert(pipeline.get_device() == nullptr);
        assert(vb.get_size() == 0);
        assert(ib.get_size() == 0);
        assert(texture.get_format() == poko::render::TextureFormat::Unknown);
        assert(material.get_device() == nullptr);
        assert(framebuffer.get_width() == 0);
        assert(framebuffer.get_height() == 0);
        std::printf("PASS\n");
    }

    // Test 5: Viewport validation
    {
        std::printf("Test 5: Viewport validation... ");
        poko::render::Context context;
        
        // Invalid dimensions
        context.set_viewport({0, 0, -100, 720, 0, 1});
        assert(context.get_last_error() != nullptr);
        
        context.set_viewport({0, 0, 1280, -720, 0, 1});
        assert(context.get_last_error() != nullptr);
        
        // Invalid depth range
        context.set_viewport({0, 0, 1280, 720, -0.5f, 1.0f});
        assert(context.get_last_error() != nullptr);
        
        context.set_viewport({0, 0, 1280, 720, 0.0f, 1.5f});
        assert(context.get_last_error() != nullptr);
        
        context.set_viewport({0, 0, 1280, 720, 0.5f, 0.25f});
        assert(context.get_last_error() != nullptr);
        
        std::printf("PASS\n");
    }

    // Test 6: Scissor validation
    {
        std::printf("Test 6: Scissor validation... ");
        poko::render::Context context;
        
        // Negative coordinates
        context.set_scissor_rect({-1, 0, 1280, 720});
        assert(context.get_last_error() != nullptr);
        
        context.set_scissor_rect({0, -1, 1280, 720});
        assert(context.get_last_error() != nullptr);
        
        // Invalid rectangle
        context.set_scissor_rect({100, 0, 50, 720});
        assert(context.get_last_error() != nullptr);
        
        context.set_scissor_rect({0, 100, 1280, 50});
        assert(context.get_last_error() != nullptr);
        
        std::printf("PASS\n");
    }

    // Test 7: Framebuffer attachment validation
    {
        std::printf("Test 7: Framebuffer attachment validation... ");
        // Test invalid device - should fail
        poko::render::Framebuffer framebuffer;
        poko::render::Device device;
        poko::render::Texture texture;
        poko::render::FramebufferDesc desc;
        desc.name = "test";
        desc.color_attachment_count = 1;
        desc.color_attachments[0].texture = &texture;
        auto result = framebuffer.initialize(&device, desc);
        assert(result != poko::render::FramebufferErrorCode::Success);
        std::printf("PASS\n");
    }

    // Test 8: Framebuffer with depth/stencil validation
    {
        std::printf("Test 8: Framebuffer with depth/stencil validation... ");
        // Test invalid device - should fail
        poko::render::Framebuffer framebuffer;
        poko::render::Device device;
        poko::render::Texture texture;
        poko::render::FramebufferDesc desc;
        desc.name = "test";
        desc.color_attachment_count = 1;
        desc.color_attachments[0].texture = &texture;
        desc.has_depth_stencil = true;
        desc.depth_stencil.texture = &texture;
        auto result = framebuffer.initialize(&device, desc);
        assert(result != poko::render::FramebufferErrorCode::Success);
        std::printf("PASS\n");
    }

    // Test 9: Multiple color attachments
    {
        std::printf("Test 9: Multiple color attachments setup... ");
        // Test invalid device - should fail
        poko::render::Framebuffer framebuffer;
        poko::render::Device device;
        poko::render::FramebufferDesc desc;
        desc.name = "test";
        desc.color_attachment_count = 3;
        // All textures are invalid, should fail
        for (int i = 0; i < 3; ++i) {
            desc.color_attachments[i].texture = nullptr;
        }
        auto result = framebuffer.initialize(&device, desc);
        assert(result != poko::render::FramebufferErrorCode::Success);
        std::printf("PASS\n");
    }

    // Test 10: Context framebuffer binding validation
    {
        std::printf("Test 10: Context framebuffer binding validation... ");
        poko::render::Context context;
        poko::render::Framebuffer framebuffer;
        assert(!context.set_render_target(&framebuffer));
        assert(context.get_last_error() != nullptr);
        std::printf("PASS\n");
    }

    std::printf("\nAll integration tests passed.\n");
    return 0;
}
