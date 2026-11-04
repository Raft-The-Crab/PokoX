/**
 * @file material.h
 * @brief Poko-owned material abstraction.
 *
 * This header contains only Poko types. The Diligent Core implementation lives
 * behind the Pimpl pattern in backend/diligent/. Consumers never need to include
 * Diligent headers.
 *
 * M6/M7 Scope: Associates textures and constant buffers with shader resource
 * slots for binding through SRB. Supports multiple textures and constant buffers
 * per material with device compatibility validation.
 *
 * Usage example:
 * @code
 * // Create material with texture binding
 * Texture texture;
 * // ... initialize texture ...
 *
 * Material material;
 * MaterialDesc desc;
 * desc.pipeline = &pipeline;
 * 
 * TextureBinding tex_binding;
 * tex_binding.texture = &texture;
 * tex_binding.shader_variable = "g_Texture";
 * desc.texture_count = 1;
 * desc.texture_bindings = &tex_binding;
 * 
 * material.initialize(device, desc);
 *
 * // Bind material for drawing
 * context.set_material(&material);
 * @endcode
 */

#ifndef POKO_RENDER_MATERIAL_H
#define POKO_RENDER_MATERIAL_H

#include "texture.h"
#include "constant_buffer.h"
#include "device.h"
#include <cstdint>

namespace poko::render {

// Forward declaration
class PipelineState;

/**
 * @brief Texture binding description.
 */
struct TextureBinding {
    Texture* texture = nullptr;  // Texture to bind
    const char* shader_variable = nullptr;  // Shader variable name (e.g., "g_Texture")
    uint32_t array_index = 0;  // Array index for texture arrays
};

/**
 * @brief Constant buffer binding description.
 */
struct ConstantBufferBinding {
    ConstantBuffer* buffer = nullptr;  // Constant buffer to bind
    const char* shader_variable = nullptr;  // Shader variable name (e.g., "g_Transform")
};

/**
 * @brief Material description.
 */
struct MaterialDesc {
    const char* name = "Material";
    PipelineState* pipeline = nullptr;  // Pipeline for resource signature
    uint32_t texture_count = 0;  // Number of textures
    TextureBinding* texture_bindings = nullptr;  // Array of texture bindings
    uint32_t constant_buffer_count = 0;  // Number of constant buffers
    ConstantBufferBinding* constant_buffer_bindings = nullptr;  // Array of constant buffer bindings
};

/**
 * @brief Poko error codes for material operations.
 */
enum class MaterialErrorCode : int32_t {
    Success = 0,
    Unknown = -1,
    InvalidDevice = -2,
    InvalidTextures = -3,
    InvalidSlots = -4,
    InvalidCount = -5,
    MaterialCreationFailed = -6
};

/**
 * @brief Opaque handle to a material.
 *
 * The implementation is private; consumers cannot depend on its layout.
 * Lifetime: Must not outlive the Device used to create it.
 */
class Material {
public:
    Material();
    ~Material();

    // Non-copyable, non-movable (owns backend resources)
    Material(const Material&) = delete;
    Material& operator=(const Material&) = delete;
    Material(Material&&) = delete;
    Material& operator=(Material&&) = delete;

    /**
     * @brief Initializes the material from a device and description.
     *
     * @param device The graphics device. Must remain valid for material lifetime.
     * @param desc Material description.
     * @return MaterialErrorCode::Success on success, error code on failure.
     *
     * Error conditions (call get_last_error() for details):
     * - Device is null or invalid
     * - Material name is null
     * - Invalid pipeline
     * - No textures or constant buffers
     * - Texture count exceeds maximum (32)
     * - Constant buffer count exceeds maximum (16)
     * - Invalid texture bindings array
     * - Invalid constant buffer bindings array
     * - Invalid texture at binding index
     * - Missing shader variable name
     * - Array index exceeds maximum (255)
     * - Texture or constant buffer created from different device
     * - Invalid pipeline implementation
     * - Pipeline created from different device
     * - SRB creation failed
     * - Texture SRV creation failed
     */
    MaterialErrorCode initialize(Device* device, const MaterialDesc& desc);

    /**
     * @brief Returns true if the material is valid.
     */
    bool is_valid() const;

    /**
     * @brief Returns the device that created this material.
     */
    Device* get_device() const;

    /**
     * @brief Returns the material name.
     */
    const char* get_name() const;

    /**
     * @brief Returns the last error message from material operations.
     */
    const char* get_last_error() const;

private:
    struct Impl;
    Impl* impl_;
    
    // Allow Context to access private impl_ for backend operations
    friend class Context;
};

} // namespace poko::render

#endif // POKO_RENDER_MATERIAL_H
