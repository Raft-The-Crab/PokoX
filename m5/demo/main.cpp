/**
 * @file main.cpp
 * @brief M5 demo: Texture creation through Poko API.
 */

#define SDL_MAIN_HANDLED

#include "poko/core/engine.h"
#include "poko/render/texture.h"
#include "poko/platform/window.h"

#include <SDL.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

int main(int argc, char** argv) {
    std::printf("PokoX M5 Demo - Texture Creation\n");
    std::printf("================================\n\n");

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
    config.window_title = "PokoX M5 - Texture";
    config.headless = false;
    config.max_frames = max_frames;

    poko::core::Engine engine;

    if (engine.initialize(config) != poko::core::ErrorCode::Success) {
        std::fprintf(stderr, "Failed to initialize engine\n");
        return 1;
    }

    // Get device for texture creation
    auto* device = engine.device();
    if (!device || !device->is_valid()) {
        std::fprintf(stderr, "Failed to get valid device\n");
        engine.shutdown();
        return 1;
    }

    std::printf("Creating texture...\n");

    // Create a simple 64x64 RGBA8 texture
    const uint32_t tex_width = 64;
    const uint32_t tex_height = 64;
    const uint32_t tex_size = tex_width * tex_height * 4;  // RGBA8 = 4 bytes per pixel
    
    // Create a simple pattern (checkerboard)
    uint8_t* tex_data = new uint8_t[tex_size];
    for (uint32_t y = 0; y < tex_height; ++y) {
        for (uint32_t x = 0; x < tex_width; ++x) {
            uint32_t idx = (y * tex_width + x) * 4;
            bool checker = ((x / 8) + (y / 8)) % 2 == 0;
            tex_data[idx + 0] = checker ? 255 : 0;     // R
            tex_data[idx + 1] = checker ? 255 : 0;     // G
            tex_data[idx + 2] = checker ? 255 : 0;     // B
            tex_data[idx + 3] = 255;                  // A
        }
    }

    poko::render::Texture texture;
    poko::render::TextureDesc tex_desc;
    tex_desc.type = poko::render::TextureType::Texture2D;
    tex_desc.width = tex_width;
    tex_desc.height = tex_height;
    tex_desc.format = poko::render::TextureFormat::RGBA8_UNORM;
    tex_desc.usage = poko::render::TextureUsage::ShaderResource;
    tex_desc.data = tex_data;
    tex_desc.data_size = tex_size;
    tex_desc.name = "CheckerboardTexture";

    auto result = texture.initialize(device, tex_desc);
    delete[] tex_data;

    if (result != poko::render::TextureErrorCode::Success) {
        std::fprintf(stderr, "Failed to create texture: %d\n", static_cast<int>(result));
        engine.shutdown();
        return 1;
    }

    std::printf("Texture created successfully (64x64 RGBA8)\n");
    std::printf("Note: Texture binding and sampling deferred to M6 (requires SRB management)\n");

    engine.shutdown();

    std::printf("\nDemo finished successfully\n");
    return 0;
}
