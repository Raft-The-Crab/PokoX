/**
 * @file main.cpp
 * @brief M7 demo: Constant buffer creation and update validation.
 */

#define SDL_MAIN_HANDLED

#include "poko/core/engine.h"
#include "poko/render/constant_buffer.h"
#include "poko/render/vertex_buffer.h"
#include "poko/render/pipeline.h"
#include "poko/render/context.h"
#include "poko/platform/window.h"

#include <SDL.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

int main(int argc, char** argv) {
    std::printf("PokoX M7 Demo - Constant Buffers\n");
    std::printf("==================================\n\n");

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
    config.window_title = "PokoX M7 - Constant Buffers";
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

    std::printf("Creating constant buffer...\n");

    // Create constant buffer (64 bytes for 4x4 matrix)
    const uint32_t cb_size = 64;  // 4x4 float matrix = 16 floats = 64 bytes
    float initial_data[16] = {0};
    for (int i = 0; i < 16; ++i) {
        initial_data[i] = 0.0f;
    }
    initial_data[0] = initial_data[5] = initial_data[10] = initial_data[15] = 1.0f;  // Identity

    poko::render::ConstantBuffer constant_buffer;
    poko::render::ConstantBufferDesc cb_desc;
    cb_desc.data = nullptr;  // Dynamic buffers cannot have initial data
    cb_desc.size = cb_size;
    cb_desc.name = "TestConstantBuffer";

    auto cb_result = constant_buffer.initialize(device, cb_desc);
    if (cb_result != poko::render::ConstantBufferErrorCode::Success) {
        std::fprintf(stderr, "Failed to create constant buffer: %d\n", static_cast<int>(cb_result));
        engine.shutdown();
        return 1;
    }

    std::printf("Constant buffer created successfully\n");

    // Test update functionality
    std::printf("Testing constant buffer update...\n");
    auto update_result = constant_buffer.update(initial_data, cb_size, 0);
    if (update_result != poko::render::ConstantBufferErrorCode::Success) {
        std::fprintf(stderr, "Failed to update constant buffer: %d\n", static_cast<int>(update_result));
        engine.shutdown();
        return 1;
    }
    std::printf("Constant buffer update successful\n");

    // Test multiple updates
    for (int i = 0; i < 10; ++i) {
        initial_data[0] = static_cast<float>(i);
        update_result = constant_buffer.update(initial_data, cb_size, 0);
        if (update_result != poko::render::ConstantBufferErrorCode::Success) {
            std::fprintf(stderr, "Failed update %d: %d\n", i, static_cast<int>(update_result));
            engine.shutdown();
            return 1;
        }
    }
    std::printf("Multiple updates successful\n");

    engine.shutdown();

    std::printf("\nDemo finished successfully\n");
    return 0;
}
