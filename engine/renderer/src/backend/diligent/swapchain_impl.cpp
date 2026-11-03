/**
 * @file swapchain_impl.cpp
 * @brief Diligent Core implementation of SwapChain.
 */

#include "backend/diligent/internal/swapchain_impl.h"
#include "backend/diligent/internal/device_impl.h"

namespace poko::render {

SwapChain::Impl::Impl() = default;

SwapChain::Impl::~Impl() = default;

bool SwapChain::Impl::initialize(Device* device, void* hwnd, const SwapChainDesc& desc) {
    (void)hwnd;  // Swap chain is created by Device::initialize via Diligent
    (void)desc;  // M1: descriptor not yet used; swap chain already created
    
    auto device_impl = get_device_impl(device);
    if (!device_impl) {
        return false;
    }

    this->device = device_impl->device;
    this->context = device_impl->context;
    this->swap_chain = device_impl->swap_chain;
    this->desc = desc;

    return this->device != nullptr && this->context != nullptr && this->swap_chain != nullptr;
}

} // namespace poko::render
