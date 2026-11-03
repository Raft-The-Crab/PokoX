/**
 * @file swapchain_impl.h
 * @brief Diligent Core implementation of Poko SwapChain.
 *
 * This header is private to the backend. It contains Diligent types and
 * implementation details. Public headers never include this.
 */

#ifndef POKO_RENDER_BACKEND_DILIGENT_SWAPCHAIN_IMPL_H
#define POKO_RENDER_BACKEND_DILIGENT_SWAPCHAIN_IMPL_H

#include "poko/render/swapchain.h"

#include "SwapChain.h"
#include "RenderDevice.h"
#include "DeviceContext.h"

#include <memory>

namespace poko::render {

/**
 * @brief Diligent-specific implementation of SwapChain.
 */
struct SwapChain::Impl {
    Diligent::ISwapChain* swap_chain = nullptr;
    Diligent::IRenderDevice* device = nullptr;
    Diligent::IDeviceContext* context = nullptr;
    SwapChainDesc desc;
    poko::render::Device* poko_device = nullptr;  // Non-owning reference for validation

    Impl();
    ~Impl();

    /**
     * @brief Initializes the swap chain.
     *
     * @param device Poko device (contains Diligent device/context).
     * @param hwnd Native window handle.
     * @param desc Swap chain description.
     * @return true on success.
     */
    bool initialize(Device* device, void* hwnd, const SwapChainDesc& desc);
};

// Backend helper functions (friend to public classes)
inline SwapChain::Impl* get_swapchain_impl(SwapChain* swap_chain) {
    return swap_chain ? swap_chain->impl_ : nullptr;
}

} // namespace poko::render

#endif // POKO_RENDER_BACKEND_DILIGENT_SWAPCHAIN_IMPL_H
