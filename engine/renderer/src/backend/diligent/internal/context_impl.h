/**
 * @file context_impl.h
 * @brief Diligent Core implementation of Poko Context.
 *
 * This header is private to the backend. It contains Diligent types and
 * implementation details. Public headers never include this.
 */

#ifndef POKO_RENDER_BACKEND_DILIGENT_CONTEXT_IMPL_H
#define POKO_RENDER_BACKEND_DILIGENT_CONTEXT_IMPL_H

#include "poko/render/context.h"
#include "poko/render/framebuffer.h"

#include "DeviceContext.h"
#include "RenderDevice.h"
#include "SwapChain.h"
#include "Buffer.h"

#include <memory>

namespace poko::render {

class ConstantBuffer;

/**
 * @brief Diligent-specific implementation of Context.
 */
struct Context::Impl {
    Diligent::IDeviceContext* context = nullptr;
    Diligent::ISwapChain* swap_chain = nullptr;
    Diligent::VALUE_TYPE bound_index_format = Diligent::VT_UINT16;  // M4: Track bound index format
    poko::render::Device* poko_device = nullptr;  // Non-owning reference for validation
    BackendType backend_type = BackendType::Unknown;  // Backend type for this context
    std::string last_error;  // Last error message

    Impl();
    ~Impl();

    /**
     * @brief Initializes the context from a device and swap chain.
     *
     * @param device Poko device.
     * @param swap_chain Poko swap chain.
     * @return true on success.
     */
    bool initialize(Device* device, SwapChain* swap_chain);
};

// Backend helper functions (friend to public classes)
inline Context::Impl* get_context_impl(Context* context) {
    return context ? context->impl_ : nullptr;
}

inline Framebuffer::Impl* get_framebuffer_impl(Framebuffer* framebuffer);

} // namespace poko::render

#endif // POKO_RENDER_BACKEND_DILIGENT_CONTEXT_IMPL_H
