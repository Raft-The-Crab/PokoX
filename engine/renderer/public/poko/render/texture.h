/**
 * @file texture.h
 * @brief Poko-owned texture abstraction.
 *
 * This header contains only Poko types. The Diligent Core implementation lives
 * behind the Pimpl pattern in backend/diligent/. Consumers never need to include
 * Diligent headers.
 *
 * M5 Scope: Supports 2D textures with RGBA8 format. Other formats and texture
 * types are deferred to future milestones.
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

private:
    struct Impl;
    Impl* impl_;
    
    // Allow Context to access private impl_ for backend operations
    friend class Context;
    friend class Material;
};

} // namespace poko::render

#endif // POKO_RENDER_TEXTURE_H
