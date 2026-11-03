/**
 * @file main.cpp
 * @brief M1 demo: Poko Engine initialization and basic rendering.
 *
 * This demonstrates the Poko Engine foundation:
 * - Engine initialization
 * - Window creation
 * - Graphics device initialization
 * - Basic frame loop with clear/present
 *
 * Automation flags:
 *   --frames N    exit after N frames (0 = run until closed)
 */

#include "poko/core/engine.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

int main(int argc, char** argv) {
    std::printf("PokoX M1 Demo\n");
    std::printf("================\n\n");

    uint64_t max_frames = 0;

    // Parse arguments
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--frames") == 0 && i + 1 < argc) {
            max_frames = static_cast<uint64_t>(std::atoi(argv[++i]));
        }
    }

    poko::core::EngineConfig config;
    config.window_width = 1280;
    config.window_height = 720;
    config.window_title = "PokoX M1 Demo";
    config.headless = false;
    config.max_frames = max_frames;

    poko::core::Engine engine;

    if (engine.initialize(config) != poko::core::ErrorCode::Success) {
        std::fprintf(stderr, "Failed to initialize engine\n");
        return 1;
    }

    std::printf("\nRunning engine");
    if (max_frames > 0) {
        std::printf(" (max %llu frames)", static_cast<unsigned long long>(max_frames));
    }
    std::printf("...\n");

    poko::core::ErrorCode exit_code = engine.run();

    engine.shutdown();

    std::printf("\nDemo finished with exit code: %d\n", static_cast<int>(exit_code));
    return static_cast<int>(exit_code);
}
