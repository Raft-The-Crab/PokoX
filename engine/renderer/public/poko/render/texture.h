/**
 * @file texture.h
 * @brief Poko-owned texture abstraction.
 *
 * This header contains only Poko types. The Diligent Core implementation lives
 * behind the Pimpl pattern in backend/diligent/. Consumers never need to include
 * Diligent headers.
 *
 * M5 Scope: Supports 2D textures with multiple formats (RGBA8, RG8, R8, RGBA16,
 * RGBA32, depth formats), dimension validation, and texture SRV creation.
 *
 * Usage example:
 * @code
 * // Create a texture from pixel data
 * uint8_t pixel_data[256 * 256 * 4];  // RGBA8
 * 
 * Texture texture;
 * TextureDesc desc;
 * desc.width = 256;
 * desc.height = 256;
 * desc.format = TextureFormat::RGBA8_UNORM;
 * desc.type = TextureType::Texture2D;
 * desc.data = pixel_data;
 * desc.data_size = sizeof(pixel_data);
 * desc.name = "MyTexture";
 * texture.initialize(device, desc);
 *
 * // Use in material binding (M6)
 * @endcode
 */

#ifndef POKO_RENDER_TEXTURE_H
#define POKO_RENDER_TEXTURE_H

#include "device.h"
#include <cstdint>

namespace poko::render {

/**
 * @brief Texture type.
 */
enum class TextureType : uint32_t {
    Unknown = 0,
    Texture2D = 1,
    TextureCube = 2
};

/**
 * @brief Texture usage flags.
 */
enum class TextureUsage : uint32_t {
    Unknown = 0,
    ShaderResource = 1,
    RenderTarget = 2,
    DepthStencil = 4
};

/**
 * @brief Texture description.
 */
struct TextureDesc {
    TextureType type = TextureType::Texture2D;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t depth = 1;  // For 3D textures, M5: unused
    uint32_t mip_levels = 1;  // M5: single mip level
    TextureFormat format = TextureFormat::RGBA8_UNORM;
    TextureUsage usage = TextureUsage::ShaderResource;
    const void* data = nullptr;  // Initial data (can be null)
    uint32_t data_size = 0;  // Size of initial data
    const char* name = "Texture";
};

/**
 * @brief Poko error codes for texture operations.
 */
enum class TextureErrorCode : int32_t {
    Success = 0,
    Unknown = -1,
    InvalidDevice = -2,
    InvalidData = -3,
    InvalidSize = -4,
    InvalidFormat = -5,
    InvalidType = -6,
    TextureCreationFailed = -7
};

/**
 * @brief Opaque handle to a texture.
 *
 * The implementation is private; consumers cannot depend on its layout.
 * Lifetime: Must not outlive the Device used to create it.
 */
class Texture {
public:
    Texture();
    ~Texture();

    // Non-copyable, non-movable (owns backend resources)
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&&) = delete;
    Texture& operator=(Texture&&) = delete;

    /**
     * @brief Initializes the texture from a device and description.
     *
     * Data is copied during initialization; the caller may free their copy
     * after this call returns.
     *
     * @param device The graphics device. Must remain valid for texture lifetime.
     * @param desc Texture description.
     * @return TextureErrorCode::Success on success, error code on failure.
     *
     * Error conditions (call get_last_error() for details):
     * - Device is null or invalid
     * - Texture name is null
     * - Invalid dimensions (width or height is 0)
     * - Invalid type (only Texture2D supported)
     * - Dimensions exceed maximum (16384 x 16384)
     * - Null data pointer with non-zero size
     * - Data size does not match expected size for dimensions
     * - Texture creation failed
     */
    TextureErrorCode initialize(Device* device, const TextureDesc& desc);

    /**
     * @brief Returns true if the texture is valid.
     */
    bool is_valid() const;

    /**
     * @brief Returns the device that created this texture.
     */
    Device* get_device() const;

    /**
     * @brief Returns the texture format.
     */
    TextureFormat get_format() const;

    /**
     * @brief Returns the texture name.
     */
    const char* get_name() const;

    /**
     * @brief Returns the last error message from texture operations.
     */
    const char* get_last_error() const;

private:
    struct Impl;
    Impl* impl_;
    
    // Allow Context to access private impl_ for backend operations
    friend class Context;
    friend class Material;
    friend class Framebuffer;
    
    // Allow backend helper functions to access private impl_
    friend Texture::Impl* get_texture_impl(Texture*);
};

} // namespace poko::render

#endif // POKO_RENDER_TEXTURE_H
