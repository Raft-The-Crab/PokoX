/**
 * @file engine.h
 * @brief Poko Engine main lifecycle.
 *
 * Provides initialization, shutdown, and the main application loop.
 * Thread safety: Not thread-safe. All calls must originate from the same
 * thread that called initialize().
 */

#ifndef POKO_CORE_ENGINE_H
#define POKO_CORE_ENGINE_H

#include <cstdint>

namespace poko {
namespace render {
class Device;
class Context;
class SwapChain;
}
namespace platform {
class Window;
}
}

namespace poko::core {

/**
 * @brief Poko error codes.
 */
enum class ErrorCode : int32_t {
    Success = 0,
    Unknown = -1,
    InitFailed = -2,
    WindowInitFailed = -3,
    DeviceInitFailed = -4,
    SwapChainInitFailed = -5,
    ContextInitFailed = -6,
    InvalidState = -7
};

/**
 * @brief Engine configuration.
 */
struct EngineConfig {
    uint32_t window_width = 1280;
    uint32_t window_height = 720;
    const char* window_title = "PokoX Engine";
    bool headless = false;
    uint64_t max_frames = 0;  // 0 = unlimited (for automated testing)
};

/**
 * @brief Main engine class.
 *
 * Owns the platform window, graphics device, and manages the main loop.
 */
class Engine {
public:
    Engine();
    ~Engine();

    // Non-copyable, non-movable
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
    Engine(Engine&&) = delete;
    Engine& operator=(Engine&&) = delete;

    /**
     * @brief Initializes the engine.
     *
     * @param config Engine configuration.
     * @return ErrorCode::Success on success, error code on failure.
     */
    ErrorCode initialize(const EngineConfig& config);

    /**
     * @brief Runs the main loop.
     *
     * This blocks until the window is closed or quit is requested.
     * @return ErrorCode::Success on normal exit, error code on failure.
     */
    ErrorCode run();

    /**
     * @brief Shuts down the engine.
     *
     * Can be called multiple times; subsequent calls are no-ops.
     */
    void shutdown();

    /**
     * @brief Returns true if the engine is initialized.
     */
    bool is_initialized() const;

    /**
     * @brief Returns the graphics device for resource creation.
     *
     * The returned pointer is non-owning and must not outlive the engine.
     * This is exposed for demo/test use cases; production code would typically
     * have a dedicated resource manager.
     */
    render::Device* device();

    /**
     * @brief Returns the command context for rendering operations.
     *
     * The returned pointer is non-owning and must not outlive the engine.
     * This is exposed for demo/test use cases; production code would typically
     * have a dedicated frame manager.
     */
    render::Context* context();

    /**
     * @brief Returns the swap chain for presentation.
     *
     * The returned pointer is non-owning and must not outlive the engine.
     * This is exposed for demo/test use cases.
     */
    render::SwapChain* swap_chain();

    /**
     * @brief Returns the platform window.
     *
     * The returned pointer is non-owning and must not outlive the engine.
     * This is exposed for demo/test use cases.
     */
    platform::Window* window();

private:
    struct Impl;
    Impl* impl_;
};

} // namespace poko::core

#endif // POKO_CORE_ENGINE_H
