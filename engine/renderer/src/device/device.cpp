/**
 * @file device.cpp
 * @brief Implementation of Poko Device.
 */

#include "poko/render/device.h"
#include "backend/diligent/internal/device_impl.h"

#include <new>

namespace poko::render {

Device::Device() : impl_(new (std::nothrow) Impl()) {}

Device::~Device() {
    delete impl_;
}

BackendType Device::backend_type() const {
    return impl_ ? impl_->backend_type : BackendType::Unknown;
}

uint32_t Device::api_major_version() const {
    return impl_ ? impl_->api_major : 0;
}

uint32_t Device::api_minor_version() const {
    return impl_ ? impl_->api_minor : 0;
}

bool Device::is_valid() const {
    return impl_ != nullptr && impl_->device != nullptr;
}

bool Device::initialize(void* hwnd, uint32_t width, uint32_t height) {
    return impl_ && impl_->initialize(hwnd, width, height);
}

const char* Device::get_last_error() const {
    if (!impl_) {
        return "Device implementation is null";
    }
    return impl_->last_error.c_str();
}

} // namespace poko::render
