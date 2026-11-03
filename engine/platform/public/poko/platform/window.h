/**
 * @file window.h
 * @brief Poko-owned window abstraction.
 *
 * This header contains only Poko types. SDL2 implementation is hidden
 * behind the Pimpl pattern. Consumers never need to include SDL headers.
 */

#ifndef POKO_PLATFORM_WINDOW_H
#define POKO_PLATFORM_WINDOW_H

#include <cstdint>

namespace poko::platform {

/**
 * @brief Window event types.
 */
enum class WindowEventType : uint32_t {
    Unknown = 0,
    Quit = 1,
    Resize = 2,
    Close = 3,
    Minimize = 4,
    Restore = 5,
    FocusGained = 6,
    FocusLost = 7
};

/**
 * @brief Key codes (subset of common keys).
 */
enum class KeyCode : uint32_t {
    Unknown = 0,
    Escape = 1,
    Space = 2,
    W = 3,
    A = 4,
    S = 5,
    D = 6
};

/**
 * @brief Window event.
 */
struct WindowEvent {
    WindowEventType type = WindowEventType::Unknown;
    uint32_t width = 0;
    uint32_t height = 0;
    KeyCode key = KeyCode::Unknown;
    bool key_pressed = false;
};

/**
 * @brief Window configuration.
 */
struct WindowDesc {
    uint32_t width = 1280;
    uint32_t height = 720;
    const char* title = "PokoX Window";
    bool resizable = true;
};

/**
 * @brief Opaque handle to a window.
 *
 * The implementation is private; consumers cannot depend on its layout.
 */
class Window {
public:
    Window();
    ~Window();

    // Non-copyable, non-movable
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;

    /**
     * @brief Initializes the window.
     *
     * @param desc Window description.
     * @return true on success.
     */
    bool initialize(const WindowDesc& desc);

    /**
     * @brief Pumps events and returns the next event if available.
     *
     * @param out_event Receives the event.
     * @return true if an event was available.
     */
    bool poll_event(WindowEvent* out_event);

    /**
     * @brief Returns the native window handle (platform-specific).
     *
     * On Windows, this is an HWND. This is needed for graphics backend
     * initialization but should be used sparingly.
     */
    void* native_handle() const;

    /**
     * @brief Returns the current window width.
     */
    uint32_t width() const;

    /**
     * @brief Returns the current window height.
     */
    uint32_t height() const;

    /**
     * @brief Returns true if the window is valid.
     */
    bool is_valid() const;

    /**
     * @brief Destroys the window and shuts down SDL2.
     */
    void shutdown();

private:
    struct Impl;
    Impl* impl_;
};

} // namespace poko::platform

#endif // POKO_PLATFORM_WINDOW_H
