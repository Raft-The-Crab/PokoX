#pragma once

#include <string>
#include <functional>
#include <windows.h>
#include "poko/core/Result.hpp"

namespace poko::platform {

/**
 * @brief Window configuration parameters
 */
struct WindowConfig {
    std::string title;
    int width = 1280;
    int height = 720;
    bool resizable = true;
};

/**
 * @brief Window event callback type
 */
using WindowEventCallback = std::function<void(int width, int height)>;

/**
 * @brief Win32 window class
 *
 * Provides RAII-based Win32 window management.
 * Wraps Win32 API calls for window creation, message loop, and destruction.
 * Thread-safe: window operations should happen on the message thread.
 */
class WindowsWindow {
public:
    /**
     * @brief Construct a window
     * @param config Window configuration
     */
    explicit WindowsWindow(const WindowConfig& config);

    /**
     * @brief Destructor - destroys the window
     */
    ~WindowsWindow();

    // Delete copy and move operations
    WindowsWindow(const WindowsWindow&) = delete;
    WindowsWindow& operator=(const WindowsWindow&) = delete;
    WindowsWindow(WindowsWindow&&) = delete;
    WindowsWindow& operator=(WindowsWindow&&) = delete;

    /**
     * @brief Check if the window is valid
     * @return true if valid, false otherwise
     */
    [[nodiscard]] bool is_valid() const noexcept { return hwnd_ != nullptr; }

    /**
     * @brief Get the window handle
     * @return The HWND handle
     */
    [[nodiscard]] void* native_handle() const noexcept { return hwnd_; }

    /**
     * @brief Process window messages (event pump)
     * @return true if the window should continue running, false if quit requested
     */
    bool process_messages();

    /**
     * @brief Request window close
     */
    void request_close();

    /**
     * @brief Set resize callback
     * @param callback Function to call on resize
     */
    void set_resize_callback(WindowEventCallback callback);

    /**
     * @brief Get the window width
     * @return Current width
     */
    [[nodiscard]] int width() const noexcept { return width_; }

    /**
     * @brief Get the window height
     * @return Current height
     */
    [[nodiscard]] int height() const noexcept { return height_; }

private:
    /**
     * @brief Window procedure callback
     */
    static LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

    /**
     * @brief Handle window creation
     */
    bool on_create();

    /**
     * @brief Handle window destruction
     */
    void on_destroy();

    /**
     * @brief Handle window resize
     */
    void on_resize(int new_width, int new_height);

    void* hwnd_ = nullptr;
    WindowConfig config_;
    int width_ = 0;
    int height_ = 0;
    bool should_close_ = false;
    WindowEventCallback resize_callback_;
};

} // namespace poko::platform
