/**
 * @file context_impl.cpp
 * @brief Diligent Core implementation of Context.
 */

#include "backend/diligent/internal/context_impl.h"
#include "backend/diligent/internal/device_impl.h"
#include "backend/diligent/internal/swapchain_impl.h"

namespace poko::render {

Context::Impl::Impl() = default;

Context::Impl::~Impl() = default;

bool Context::Impl::initialize(Device* device, SwapChain* swap_chain) {
    if (!device || !device->is_valid()) {
        std::fprintf(stderr, "Invalid device\n");
        return false;
    }

    auto device_impl = get_device_impl(device);
    if (!device_impl || !device_impl->context) {
        std::fprintf(stderr, "Invalid device implementation\n");
        return false;
    }

    this->context = device_impl->context;
    this->poko_device = device;

    auto swapchain_impl = get_swapchain_impl(swap_chain);
    if (swapchain_impl) {
        this->swap_chain = swapchain_impl->swap_chain;
    }

    return this->context != nullptr;
}

} // namespace poko::render
