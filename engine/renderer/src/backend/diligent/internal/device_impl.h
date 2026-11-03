/**
 * @file device_impl.h
 * @brief Diligent Core implementation of Poko Device.
 *
 * This header is private to the backend. It contains Diligent types and
 * implementation details. Public headers never include this.
 */

#ifndef POKO_RENDER_BACKEND_DILIGENT_DEVICE_IMPL_H
#define POKO_RENDER_BACKEND_DILIGENT_DEVICE_IMPL_H

#include "poko/render/device.h"

#include "RenderDevice.h"
#include "DeviceContext.h"
#include "SwapChain.h"
#include "EngineFactoryOpenGL.h"

#include <memory>

namespace poko::render {

/**
 * @brief Diligent-specific implementation of Device.
 */
struct Device::Impl {
    Diligent::IRenderDevice* device = nullptr;
    Diligent::IDeviceContext* context = nullptr;
    Diligent::ISwapChain* swap_chain = nullptr;
    BackendType backend_type = BackendType::Unknown;
    uint32_t api_major = 0;
    uint32_t api_minor = 0;
    std::string last_error;

    Impl();
    ~Impl();

    /**
     * @brief Initializes the device from a native window handle.
     *
     * @param hwnd Native window handle (HWND on Windows).
     * @param width Initial width.
     * @param height Initial height.
     * @return true on success.
     */
    bool initialize(void* hwnd, uint32_t width, uint32_t height);
};

// Backend helper functions (friend to public classes)
inline Device::Impl* get_device_impl(Device* device) {
    return device ? device->impl_ : nullptr;
}

} // namespace poko::render

#endif // POKO_RENDER_BACKEND_DILIGENT_DEVICE_IMPL_H
