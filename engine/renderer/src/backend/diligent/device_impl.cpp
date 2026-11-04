/**
 * @file device_impl.cpp
 * @brief Diligent Core implementation of Device.
 */

#include "backend/diligent/internal/device_impl.h"

#include <windows.h>

namespace poko::render {

Device::Impl::Impl() = default;

Device::Impl::~Impl() {
    // Release Diligent resources in reverse order of creation
    if (swap_chain) {
        swap_chain->Release();
        swap_chain = nullptr;
    }
    if (context) {
        context->Release();
        context = nullptr;
    }
    if (device) {
        device->Release();
        device = nullptr;
    }
    last_error.clear();
}

bool Device::Impl::initialize(void* hwnd, uint32_t width, uint32_t height) {
    if (!hwnd) {
        last_error = "Native window handle is null";
        return false;
    }
    if (width == 0 || height == 0) {
        last_error = "Invalid window dimensions (width or height is zero)";
        return false;
    }

    Diligent::IEngineFactoryOpenGL* factory = Diligent::GetEngineFactoryOpenGL();
    if (factory == nullptr) {
        last_error = "Failed to get OpenGL engine factory";
        return false;
    }

    Diligent::SwapChainDesc sc_desc;
    sc_desc.Width = width;
    sc_desc.Height = height;
    sc_desc.ColorBufferFormat = Diligent::TEX_FORMAT_RGBA8_UNORM;
    sc_desc.DepthBufferFormat = Diligent::TEX_FORMAT_D32_FLOAT;

    Diligent::EngineGLCreateInfo engine_ci;
    engine_ci.Window.hWnd = static_cast<HWND>(hwnd);

    factory->CreateDeviceAndSwapChainGL(engine_ci, &device, &context, sc_desc, &swap_chain);

    if (device == nullptr || context == nullptr || swap_chain == nullptr) {
        last_error = "Failed to create device, context, or swap chain";
        // Clean up any partially created resources
        if (swap_chain) {
            swap_chain->Release();
            swap_chain = nullptr;
        }
        if (context) {
            context->Release();
            context = nullptr;
        }
        if (device) {
            device->Release();
            device = nullptr;
        }
        return false;
    }

    const Diligent::RenderDeviceInfo& device_info = device->GetDeviceInfo();

    // Map Diligent device type to Poko backend type
    switch (device_info.Type) {
        case Diligent::RENDER_DEVICE_TYPE_GL:
            backend_type = BackendType::OpenGL;
            break;
        case Diligent::RENDER_DEVICE_TYPE_GLES:
            backend_type = BackendType::OpenGL_ES;
            break;
        default:
            backend_type = BackendType::Unknown;
            break;
    }

    api_major = device_info.APIVersion.Major;
    api_minor = device_info.APIVersion.Minor;

    // Populate device capabilities
    const Diligent::GraphicsAdapterInfo& adapter_info = device->GetAdapterInfo();
    caps.supports_compute = device_info.Features.ComputeShaders;
    caps.supports_geometry_shaders = device_info.Features.GeometryShaders;
    caps.supports_tessellation = device_info.Features.Tessellation;
    caps.max_texture_size = adapter_info.Texture.MaxTexture2DDimension;
    caps.max_texture_3d_size = adapter_info.Texture.MaxTexture3DDimension;
    caps.max_texture_array_layers = adapter_info.Texture.MaxTextureCubeDimension;
    caps.max_render_targets = 8;  // Diligent default max render targets
    caps.max_vertex_shader_inputs = 16;  // Diligent default max inputs
    caps.max_pixel_shader_inputs = 16;  // Diligent default max inputs

    last_error.clear();
    return true;
}

} // namespace poko::render
