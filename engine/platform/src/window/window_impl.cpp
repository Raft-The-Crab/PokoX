/**
 * @file window_impl.cpp
 * @brief SDL2 implementation of Window.
 */

#include "window_impl.h"

#include <cstdio>

namespace poko::platform {

Window::Impl::Impl() = default;

Window::Impl::~Impl() {
    shutdown();
}

bool Window::Impl::initialize(const WindowDesc& desc) {
    if (initialized) {
        return true; // Already initialized
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }

    uint32_t flags = SDL_WINDOW_SHOWN;
    if (desc.resizable) {
        flags |= SDL_WINDOW_RESIZABLE;
    }

    window = SDL_CreateWindow(
        desc.title ? desc.title : "PokoX Window",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        static_cast<int>(desc.width),
        static_cast<int>(desc.height),
        flags
    );

    if (window == nullptr) {
        std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }

    SDL_GetWindowSize(window, reinterpret_cast<int*>(&width), reinterpret_cast<int*>(&height));
    initialized = true;
    return true;
}

bool Window::Impl::poll_event(WindowEvent* out_event) {
    if (!initialized || !out_event) {
        return false;
    }

    SDL_Event event;
    if (SDL_PollEvent(&event) == 0) {
        return false;
    }

    // Map SDL event to Poko event
    switch (event.type) {
        case SDL_QUIT:
            out_event->type = WindowEventType::Quit;
            break;

        case SDL_WINDOWEVENT:
            switch (event.window.event) {
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    out_event->type = WindowEventType::Resize;
                    out_event->width = static_cast<uint32_t>(event.window.data1);
                    out_event->height = static_cast<uint32_t>(event.window.data2);
                    width = out_event->width;
                    height = out_event->height;
                    break;
                case SDL_WINDOWEVENT_CLOSE:
                    out_event->type = WindowEventType::Close;
                    break;
                case SDL_WINDOWEVENT_MINIMIZED:
                    out_event->type = WindowEventType::Minimize;
                    break;
                case SDL_WINDOWEVENT_RESTORED:
                    out_event->type = WindowEventType::Restore;
                    break;
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    out_event->type = WindowEventType::FocusGained;
                    break;
                case SDL_WINDOWEVENT_FOCUS_LOST:
                    out_event->type = WindowEventType::FocusLost;
                    break;
                default:
                    out_event->type = WindowEventType::Unknown;
                    break;
            }
            break;

        case SDL_KEYDOWN:
            out_event->type = WindowEventType::Unknown;
            out_event->key_pressed = true;
            switch (event.key.keysym.sym) {
                case SDLK_ESCAPE: out_event->key = KeyCode::Escape; break;
                case SDLK_SPACE: out_event->key = KeyCode::Space; break;
                case SDLK_w: out_event->key = KeyCode::W; break;
                case SDLK_a: out_event->key = KeyCode::A; break;
                case SDLK_s: out_event->key = KeyCode::S; break;
                case SDLK_d: out_event->key = KeyCode::D; break;
                default: out_event->key = KeyCode::Unknown; break;
            }
            break;

        case SDL_KEYUP:
            out_event->type = WindowEventType::Unknown;
            out_event->key_pressed = false;
            switch (event.key.keysym.sym) {
                case SDLK_ESCAPE: out_event->key = KeyCode::Escape; break;
                case SDLK_SPACE: out_event->key = KeyCode::Space; break;
                case SDLK_w: out_event->key = KeyCode::W; break;
                case SDLK_a: out_event->key = KeyCode::A; break;
                case SDLK_s: out_event->key = KeyCode::S; break;
                case SDLK_d: out_event->key = KeyCode::D; break;
                default: out_event->key = KeyCode::Unknown; break;
            }
            break;

        default:
            out_event->type = WindowEventType::Unknown;
            break;
    }

    return true;
}

void* Window::Impl::native_handle() const {
    if (!window) {
        return nullptr;
    }

    SDL_SysWMinfo wm_info;
    SDL_VERSION(&wm_info.version);
    if (SDL_GetWindowWMInfo(window, &wm_info) != SDL_TRUE) {
        return nullptr;
    }

#ifdef _WIN32
    if (wm_info.subsystem == SDL_SYSWM_WINDOWS) {
        return wm_info.info.win.window;
    }
#endif

    return nullptr;
}

void Window::Impl::shutdown() {
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    if (initialized) {
        SDL_Quit();
        initialized = false;
    }
}

} // namespace poko::platform
