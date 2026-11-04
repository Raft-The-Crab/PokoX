/**
 * @file swapchain.h
 * @brief Poko-owned swap chain abstraction.
 *
 * This header contains only Poko types. The Diligent Core implementation lives
 * behind the Pimpl pattern in backend/diligent/. Consumers never need to include
 * Diligent headers.
 *
 * The SwapChain manages the presentation surface (e.g., window backbuffer) and
 * is responsible for presenting rendered frames to the screen.
 *
 * Usage example:
 * @code
 * SwapChain swap_chain;
 * SwapChainDesc desc;
 * desc.width = 1280;
 * desc.height = 720;
 * desc.color_format = TextureFormat::RGBA8_UNORM;
 * desc.depth_format = TextureFormat::D32_FLOAT;
 * swap_chain.initialize(device, window_handle, desc);
 *
 * // Present rendered frame
 * swap_chain.present();
 *
 * // Resize on window resize
 * swap_chain.resize(new_width, new_height);
 * @endcode
 */

#ifndef POKO_RENDER_SWAPCHAIN_H
#define POKO_RENDER_SWAPCHAIN_H

#include "device.h"
#include <cstdint>

namespace poko::render {

/**
 * @brief Swap chain configuration.
 */
struct SwapChainDesc {
    uint32_t width = 1280;
    uint32_t height = 720;
    TextureFormat color_format = TextureFormat::RGBA8_UNORM;
    TextureFormat depth_format = TextureFormat::D32_FLOAT;
};

/**
 * @brief Opaque handle to a swap chain.
 *
 * The implementation is private; consumers cannot depend on its layout.
 */
class SwapChain {
public:
    SwapChain();
    ~SwapChain();

    // Non-copyable, non-movable (owns backend resources)
    SwapChain(const SwapChain&) = delete;
    SwapChain& operator=(const SwapChain&) = delete;
    SwapChain(SwapChain&&) = delete;
    SwapChain& operator=(SwapChain&&) = delete;

    /**
     * @brief Returns the current swap chain description.
     */
    SwapChainDesc get_desc() const;

    /**
     * @brief Resizes the swap chain to new dimensions.
     *
     * @param width New width in pixels.
     * @param height New height in pixels.
     * @return true on success, false on failure.
     */
    bool resize(uint32_t width, uint32_t height);

    /**
     * @brief Presents the current back buffer.
     */
    void present();

    /**
     * @brief Returns true if the swap chain is valid.
     */
    bool is_valid() const;

    /**
     * @brief Returns the device that created this swap chain.
     */
    Device* get_device() const;

    /**
     * @brief Returns the color format of the swap chain.
     */
    TextureFormat get_color_format() const;

    /**
     * @brief Returns the depth format of the swap chain.
     */
    TextureFormat get_depth_format() const;

    /**
     * @brief Initializes the swap chain from a device and window.
     *
     * @param device The graphics device.
     * @param hwnd Native window handle (HWND on Windows, void* on other platforms).
     *            Platform-specific. Lifetime: must remain valid for the lifetime of the swap chain.
     * @param desc Swap chain description.
     * @return true on success.
     */
    bool initialize(Device* device, void* hwnd, const SwapChainDesc& desc);

private:
    struct Impl;
    Impl* impl_;
    
    // Allow backend implementation to access private impl_
    friend SwapChain::Impl* get_swapchain_impl(SwapChain*);
};

} // namespace poko::render

#endif // POKO_RENDER_SWAPCHAIN_H
