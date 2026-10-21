#include "poko/platform/WindowsWindow.hpp"
#include "poko/core/Logger.hpp"
#include <stdexcept>

namespace poko::platform {

static const char* WINDOW_CLASS_NAME = "PokoXWindow";

WindowsWindow::WindowsWindow(const WindowConfig& config)
    : config_(config), width_(config.width), height_(config.height) {
    
    // Register window class
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = window_proc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.lpszClassName = WINDOW_CLASS_NAME;

    if (!RegisterClassEx(&wc)) {
        core::Logger::instance().error("Failed to register window class");
        return;
    }

    // Create window
    DWORD style = WS_OVERLAPPEDWINDOW;
    if (config.resizable) {
        style |= WS_THICKFRAME | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;
    }

    RECT rect = {0, 0, config.width, config.height};
    AdjustWindowRect(&rect, style, FALSE);

    hwnd_ = CreateWindowExA(
        0,
        WINDOW_CLASS_NAME,
        config.title.c_str(),
        style,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top,
        nullptr, nullptr,
        GetModuleHandle(nullptr),
        this
    );

    if (!hwnd_) {
        core::Logger::instance().error("Failed to create window");
        return;
    }

    // Store instance pointer
    SetWindowLongPtrA(static_cast<HWND>(hwnd_), GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    if (!on_create()) {
        DestroyWindow(static_cast<HWND>(hwnd_));
        hwnd_ = nullptr;
        return;
    }

    ShowWindow(static_cast<HWND>(hwnd_), SW_SHOW);
    UpdateWindow(static_cast<HWND>(hwnd_));

    core::Logger::instance().info("Window created successfully");
}

WindowsWindow::~WindowsWindow() {
    if (hwnd_) {
        DestroyWindow(static_cast<HWND>(hwnd_));
        hwnd_ = nullptr;
    }
    UnregisterClassA(WINDOW_CLASS_NAME, GetModuleHandle(nullptr));
}

bool WindowsWindow::process_messages() {
    MSG msg = {};
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            return false;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return !should_close_;
}

void WindowsWindow::request_close() {
    should_close_ = true;
    PostMessage(static_cast<HWND>(hwnd_), WM_CLOSE, 0, 0);
}

void WindowsWindow::set_resize_callback(WindowEventCallback callback) {
    resize_callback_ = std::move(callback);
}

LRESULT CALLBACK WindowsWindow::window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    WindowsWindow* window = nullptr;
    
    if (msg == WM_NCCREATE) {
        CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lparam);
        window = reinterpret_cast<WindowsWindow*>(cs->lpCreateParams);
        SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
        return DefWindowProcA(hwnd, msg, wparam, lparam);
    }
    
    window = reinterpret_cast<WindowsWindow*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));
    
    if (window) {
        switch (msg) {
            case WM_DESTROY:
                window->on_destroy();
                PostQuitMessage(0);
                return 0;
                
            case WM_SIZE:
                if (wparam == SIZE_RESTORED || wparam == SIZE_MAXIMIZED || wparam == SIZE_MINIMIZED) {
                    int width = LOWORD(lparam);
                    int height = HIWORD(lparam);
                    window->on_resize(width, height);
                }
                return 0;
                
            case WM_CLOSE:
                window->request_close();
                return 0;
        }
    }
    
    return DefWindowProcA(hwnd, msg, wparam, lparam);
}

bool WindowsWindow::on_create() {
    core::Logger::instance().info("Window initialization complete");
    return true;
}

void WindowsWindow::on_destroy() {
    core::Logger::instance().info("Window destroyed");
    hwnd_ = nullptr;
}

void WindowsWindow::on_resize(int new_width, int new_height) {
    width_ = new_width;
    height_ = new_height;
    
    if (resize_callback_) {
        resize_callback_(new_width, new_height);
    }
}

} // namespace poko::platform
