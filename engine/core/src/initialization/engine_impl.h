/**
 * @file engine_impl.h
 * @brief Engine implementation.
 *
 * Private header containing implementation details.
 */

#ifndef POKO_CORE_ENGINE_IMPL_H
#define POKO_CORE_ENGINE_IMPL_H

#include "poko/core/engine.h"
#include "poko/platform/window.h"
#include "poko/render/device.h"
#include "poko/render/swapchain.h"
#include "poko/render/context.h"

#include <memory>

namespace poko::core {

/**
 * @brief Engine implementation.
 */
struct Engine::Impl {
    platform::Window window;
    render::Device device;
    render::SwapChain swap_chain;
    render::Context context;
    bool initialized = false;
    bool running = false;
    uint64_t max_frames = 0;

    Impl();
    ~Impl();

    bool initialize(const EngineConfig& config);
    int run();
    void shutdown();
};

} // namespace poko::core

#endif // POKO_CORE_ENGINE_IMPL_H
