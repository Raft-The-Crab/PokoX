/**
 * @file engine_impl.cpp
 * @brief Engine implementation.
 */

#include "engine_impl.h"

#include <SDL.h>
#include <cstdio>

namespace poko::core {

Engine::Impl::Impl() = default;

Engine::Impl::~Impl() {
    shutdown();
}

bool Engine::Impl::initialize(const EngineConfig& config) {
    if (initialized) {
        return true;
    }

    max_frames = config.max_frames;

    // Initialize window
    platform::WindowDesc window_desc;
    window_desc.width = config.window_width;
    window_desc.height = config.window_height;
    window_desc.title = config.window_title;
    window_desc.resizable = true;

    if (!window.initialize(window_desc)) {
        std::fprintf(stderr, "Failed to initialize window\n");
        return false;
    }

    if (config.headless) {
        initialized = true;
        return true;
    }

    // Initialize graphics device
    void* hwnd = window.native_handle();
    if (!hwnd) {
        std::fprintf(stderr, "Failed to get native window handle\n");
        window.shutdown();
        return false;
    }

    if (!device.initialize(hwnd, config.window_width, config.window_height)) {
        std::fprintf(stderr, "Failed to initialize graphics device\n");
        window.shutdown();
        return false;
    }

    // Initialize swap chain
    render::SwapChainDesc sc_desc;
    sc_desc.width = config.window_width;
    sc_desc.height = config.window_height;
    sc_desc.color_format = render::TextureFormat::RGBA8_UNORM;
    sc_desc.depth_format = render::TextureFormat::D32_FLOAT;

    if (!swap_chain.initialize(&device, hwnd, sc_desc)) {
        std::fprintf(stderr, "Failed to initialize swap chain\n");
        // Device destructor will clean up Diligent resources
        window.shutdown();
        return false;
    }

    // Initialize context
    if (!context.initialize(&device, &swap_chain)) {
        std::fprintf(stderr, "Failed to initialize context\n");
        // Device and swap chain destructors will clean up Diligent resources
        window.shutdown();
        return false;
    }

    initialized = true;
    std::printf("Poko Engine initialized\n");
    std::printf("  Backend: %s\n",
        device.backend_type() == render::BackendType::OpenGL ? "OpenGL" : "Unknown");
    std::printf("  API: %u.%u\n", device.api_major_version(), device.api_minor_version());
    std::printf("  Window: %ux%u\n", window.width(), window.height());

    return true;
}

int Engine::Impl::run() {
    if (!initialized) {
        return 1;
    }

    running = true;
    uint64_t frame = 0;

    while (running) {
        platform::WindowEvent event;
        while (window.poll_event(&event)) {
            switch (event.type) {
                case platform::WindowEventType::Quit:
                case platform::WindowEventType::Close:
                    running = false;
                    break;
                case platform::WindowEventType::Resize:
                    if (swap_chain.is_valid()) {
                        swap_chain.resize(event.width, event.height);
                    }
                    break;
                default:
                    break;
            }
        }

        if (!running) {
            break;
        }

        // Skip rendering if not ready (defensive)
        if (!context.is_valid() || !swap_chain.is_valid()) {
            std::fprintf(stderr, "Warning: renderer not valid, skipping frame %llu\n", static_cast<unsigned long long>(frame));
            ++frame;
            SDL_Delay(16);
            continue;
        }

        // Basic frame: clear and present
        render::ClearColor clear_color{0.05f, 0.10f, 0.20f, 1.0f};
        context.clear_render_target(clear_color);
        swap_chain.present();

        ++frame;

        // Frame limit for automated testing
        if (max_frames > 0 && frame >= max_frames) {
            running = false;
        }

        // Simple frame limiter (~60 FPS)
        SDL_Delay(16);
    }

    std::printf("Engine shutdown after %llu frames\n", static_cast<unsigned long long>(frame));
    return 0;
}

void Engine::Impl::shutdown() {
    if (!initialized) {
        return;
    }

    running = false;
    
    // Shutdown in reverse order of initialization
    window.shutdown();
    
    // Renderer objects will be cleaned up by their destructors when Engine is destroyed
    // Re-initialization is not supported, so no need to reset them to default state
    
    initialized = false;
}

} // namespace poko::core
