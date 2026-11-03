/**
 * @file window.cpp
 * @brief Implementation of Poko Window.
 */

#include "poko/platform/window.h"
#include "window_impl.h"

#include <new>

namespace poko::platform {

Window::Window() : impl_(new (std::nothrow) Impl()) {}

Window::~Window() {
    if (impl_) {
        impl_->shutdown();
        delete impl_;
    }
}

bool Window::initialize(const WindowDesc& desc) {
    return impl_ && impl_->initialize(desc);
}

bool Window::poll_event(WindowEvent* out_event) {
    if (!impl_) {
        return false;
    }
    if (!out_event) {
        std::fprintf(stderr, "Window: poll_event called with null event pointer\n");
        return false;
    }
    return impl_->poll_event(out_event);
}

void* Window::native_handle() const {
    return impl_ ? impl_->native_handle() : nullptr;
}

uint32_t Window::width() const {
    return impl_ ? impl_->width : 0;
}

uint32_t Window::height() const {
    return impl_ ? impl_->height : 0;
}

bool Window::is_valid() const {
    return impl_ != nullptr && impl_->initialized;
}

void Window::shutdown() {
    if (impl_) {
        impl_->shutdown();
    }
}

} // namespace poko::platform
