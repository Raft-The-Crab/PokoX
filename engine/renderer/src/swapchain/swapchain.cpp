/**
 * @file swapchain.cpp
 * @brief Implementation of Poko SwapChain.
 */

#include "poko/render/swapchain.h"
#include "backend/diligent/internal/swapchain_impl.h"

#include <new>

namespace poko::render {

SwapChain::SwapChain() : impl_(new (std::nothrow) Impl()) {}

SwapChain::~SwapChain() {
    delete impl_;
}

SwapChainDesc SwapChain::get_desc() const {
    return impl_ ? impl_->desc : SwapChainDesc{};
}

bool SwapChain::resize(uint32_t width, uint32_t height) {
    if (!impl_ || !impl_->swap_chain) {
        std::fprintf(stderr, "SwapChain: resize called on invalid swap chain\n");
        return false;
    }
    if (width == 0 || height == 0) {
        std::fprintf(stderr, "SwapChain: Invalid resize dimensions (%u x %u)\n", width, height);
        return false;
    }
    // Validate device is still valid
    if (impl_->poko_device && !impl_->poko_device->is_valid()) {
        std::fprintf(stderr, "SwapChain: Cannot resize - device is invalid\n");
        return false;
    }
    impl_->swap_chain->Resize(width, height);
    impl_->desc.width = width;
    impl_->desc.height = height;
    return true;
}

void SwapChain::present() {
    if (impl_ && impl_->swap_chain) {
        impl_->swap_chain->Present();
    }
}

bool SwapChain::is_valid() const {
    return impl_ != nullptr && impl_->swap_chain != nullptr;
}

Device* SwapChain::get_device() const {
    return impl_ ? impl_->poko_device : nullptr;
}

bool SwapChain::initialize(Device* device, void* hwnd, const SwapChainDesc& desc) {
    return impl_ && impl_->initialize(device, hwnd, desc);
}

} // namespace poko::render
