/**
 * @file device.h
 * @brief Poko-owned graphics device abstraction.
 *
 * This header contains only Poko types. The Diligent Core implementation lives
 * behind the Pimpl pattern in backend/diligent/. Consumers never need to include
 * Diligent headers.
 */

#ifndef POKO_RENDER_DEVICE_H
#define POKO_RENDER_DEVICE_H

#include <cstdint>

namespace poko::render {

/**
 * @brief Graphics backend type.
 */
enum class BackendType : uint32_t {
    Unknown = 0,
    OpenGL = 1,
    OpenGL_ES = 2,
    Vulkan = 3,
    Direct3D11 = 4,
    Direct3D12 = 5,
    Metal = 6
};

/**
 * @brief Texture format enumeration (subset of common formats).
 */
enum class TextureFormat : uint32_t {
    Unknown = 0,
    RGBA8_UNORM = 1,
    RGBA8_SRGB = 2,
    D32_FLOAT = 3
};

/**
 * @brief Opaque handle to a graphics device.
 *
 * The implementation is private; consumers cannot depend on its layout.
 */
class Device {
public:
    Device();
    ~Device();

    // Non-copyable, non-movable (owns backend resources)
    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;
    Device(Device&&) = delete;
    Device& operator=(Device&&) = delete;

    /**
     * @brief Returns the backend type of this device.
     */
    BackendType backend_type() const;

    /**
     * @brief Returns the API major version.
     */
    uint32_t api_major_version() const;

    /**
     * @brief Returns the API minor version.
     */
    uint32_t api_minor_version() const;

    /**
     * @brief Returns true if the device is valid (backend initialized successfully).
     */
    bool is_valid() const;

    /**
     * @brief Initializes the graphics device from a native window handle.
     *
     * @param hwnd Native window handle (HWND on Windows, void* on other platforms).
     *            Platform-specific. Lifetime: must remain valid for the lifetime of the device.
     * @param width Initial window width.
     * @param height Initial window height.
     * @return true on success.
     */
    bool initialize(void* hwnd, uint32_t width, uint32_t height);

private:
    struct Impl;
    Impl* impl_;
    
    // Allow backend implementation to access private impl_
    friend Device::Impl* get_device_impl(Device*);
};

} // namespace poko::render

#endif // POKO_RENDER_DEVICE_H
