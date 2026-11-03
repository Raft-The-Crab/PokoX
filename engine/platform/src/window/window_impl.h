/**
 * @file window_impl.h
 * @brief SDL2 implementation of Poko Window.
 *
 * This header is private to the platform module. It contains SDL types and
 * implementation details. Public headers never include this.
 */

#ifndef POKO_PLATFORM_WINDOW_IMPL_H
#define POKO_PLATFORM_WINDOW_IMPL_H

#include "poko/platform/window.h"

#include <SDL.h>
#include <SDL_syswm.h>

#include <memory>

namespace poko::platform {

/**
 * @brief SDL2-specific implementation of Window.
 */
struct Window::Impl {
    SDL_Window* window = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;
    bool initialized = false;

    Impl();
    ~Impl();

    bool initialize(const WindowDesc& desc);
    bool poll_event(WindowEvent* out_event);
    void* native_handle() const;
    void shutdown();
};

} // namespace poko::platform

#endif // POKO_PLATFORM_WINDOW_IMPL_H
