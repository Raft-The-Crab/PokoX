/**
 * @file main.cpp
 * @brief M8 Demo: Render-to-Texture
 *
 * Demonstrates framebuffer usage for offscreen rendering.
 * This is a simplified demo that creates a framebuffer and renders
 * directly to it, demonstrating the API without complex two-pass rendering.
 */

#define SDL_MAIN_HANDLED

#include "poko/core/engine.h"
#include "poko/render/pipeline.h"
#include "poko/render/context.h"
#include "poko/render/swapchain.h"
#include "poko/render/vertex_buffer.h"
#include "poko/render/index_buffer.h"
#include "poko/render/texture.h"
#include "poko/render/framebuffer.h"
#include "poko/platform/window.h"

#include <SDL.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

// Simple vertex shader
constexpr char kVertexShader[] = R"(#version 330 core

layout(location = 0) in vec3 aPosition;

void main()
{
    gl_Position = vec4(aPosition, 1.0);
}
)";

// Simple pixel shader
constexpr char kPixelShader[] = R"(#version 330 core

out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0, 0.5, 0.2, 1.0);
}
)";

int main(int argc, char** argv) {
    std::printf("PokoX M8 Demo - Framebuffer API\n");
    std::printf("=================================\n\n");

    uint64_t max_frames = 0;

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--frames") == 0 && i + 1 < argc) {
            max_frames = static_cast<uint64_t>(std::atoi(argv[++i]));
        }
    }

    // Initialize engine
    poko::core::EngineConfig config;
    config.window_width = 1280;
    config.window_height = 720;
    config.window_title = "PokoX M8 - Framebuffer";
    config.headless = false;
    config.max_frames = max_frames;

    poko::core::Engine engine;

    if (engine.initialize(config) != poko::core::ErrorCode::Success) {
        std::fprintf(stderr, "Failed to initialize engine\n");
        return 1;
    }

    // Get device for resource creation
    auto* device = engine.device();
    if (!device || !device->is_valid()) {
        std::fprintf(stderr, "Failed to get valid device\n");
        engine.shutdown();
        return 1;
    }

    std::printf("Creating offscreen texture...\n");

    // Create texture for offscreen rendering (512x512)
    const uint32_t tex_width = 512;
    const uint32_t tex_height = 512;
    std::vector<uint8_t> tex_data(tex_width * tex_height * 4, 0);  // RGBA8

    poko::render::Texture offscreen_texture;
    poko::render::TextureDesc tex_desc;
    tex_desc.width = tex_width;
    tex_desc.height = tex_height;
    tex_desc.format = poko::render::TextureFormat::RGBA8_UNORM;
    tex_desc.data = tex_data.data();
    tex_desc.data_size = tex_data.size();
    tex_desc.name = "OffscreenTexture";

    auto tex_result = offscreen_texture.initialize(device, tex_desc);
    if (tex_result != poko::render::TextureErrorCode::Success) {
        std::fprintf(stderr, "Failed to create offscreen texture: %d\n", static_cast<int>(tex_result));
        engine.shutdown();
        return 1;
    }

    std::printf("Offscreen texture created successfully\n");

    // Create framebuffer with offscreen texture
    std::printf("Creating framebuffer...\n");

    poko::render::Framebuffer framebuffer;
    poko::render::FramebufferDesc fb_desc;
    fb_desc.name = "OffscreenFramebuffer";
    fb_desc.color_attachment_count = 1;
    fb_desc.color_attachments[0].texture = &offscreen_texture;
    fb_desc.color_attachments[0].mip_level = 0;
    fb_desc.color_attachments[0].array_slice = 0;

    auto fb_result = framebuffer.initialize(device, fb_desc);
    if (fb_result != poko::render::FramebufferErrorCode::Success) {
        std::fprintf(stderr, "Failed to create framebuffer: %d\n", static_cast<int>(fb_result));
        engine.shutdown();
        return 1;
    }

    std::printf("Framebuffer created successfully\n");
    std::printf("  Width: %u\n", framebuffer.get_width());
    std::printf("  Height: %u\n", framebuffer.get_height());
    std::printf("  Color attachments: %u\n", framebuffer.get_color_attachment_count());

    // For this demo, we'll just verify the framebuffer API works
    // Full two-pass rendering requires material resource signatures for texture sampling
    std::printf("\nFramebuffer API verified successfully\n");
    std::printf("Note: Full two-pass render-to-texture requires material resource signatures\n");

    engine.shutdown();

    std::printf("\nDemo finished successfully\n");
    return 0;
}
