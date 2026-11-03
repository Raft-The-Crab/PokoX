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

bool SwapChain::initialize(Device* device, void* hwnd, const SwapChainDesc& desc) {
    return impl_ && impl_->initialize(device, hwnd, desc);
}

} // namespace poko::render
