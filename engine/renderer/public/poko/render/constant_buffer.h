/**
 * @file constant_buffer.h
 * @brief Poko-owned constant buffer (uniform buffer) abstraction.
 *
 * This header contains only Poko types. The Diligent Core implementation lives
 * behind the Pimpl pattern in backend/diligent/. Consumers never need to include
 * Diligent headers.
 *
 * M7 Scope: Supports constant buffer creation and dynamic updates for shader
 * uniform data (matrices, vectors, etc.). Bind to shader stages through Context.
 *
 * Usage example:
 * @code
 * // Shader uniform data (must be std140 aligned)
 * struct ShaderConstants {
 *     float4x4 worldViewProj;
 *     float4 color;
 * };
 * 
 * ShaderConstants constants;
 * // ... fill constants ...
 * 
 * ConstantBuffer buffer;
 * ConstantBufferDesc desc;
 * desc.data = &constants;
 * desc.size = sizeof(constants);  // Must be multiple of 16
 * desc.name = "SceneConstants";
 * buffer.initialize(device, desc);
 *
 * // Update dynamically (e.g., per frame)
 * buffer.update(&new_constants, sizeof(new_constants), 0);
 * @endcode
 */

#ifndef POKO_RENDER_CONSTANT_BUFFER_H
#define POKO_RENDER_CONSTANT_BUFFER_H

#include "device.h"
#include <cstdint>

namespace poko::render {

/**
 * @brief Constant buffer description.
 */
struct ConstantBufferDesc {
    const void* data = nullptr;  // Initial data (can be null for dynamic buffers)
    uint32_t size = 0;           // Buffer size in bytes (must be multiple of 16 for std140)
    const char* name = "ConstantBuffer";
};

/**
 * @brief Poko error codes for constant buffer operations.
 */
enum class ConstantBufferErrorCode : int32_t {
    Success = 0,
    Unknown = -1,
    InvalidDevice = -2,
    InvalidData = -3,
    InvalidSize = -4,
    InvalidOffset = -5,
    BufferCreationFailed = -6,
    UpdateFailed = -7
};

/**
 * @brief Opaque handle to a constant buffer (uniform buffer).
 *
 * The implementation is private; consumers cannot depend on its layout.
 * Lifetime: Must not outlive the Device used to create it.
 *
 * Constant buffers are used to pass uniform data to shaders (matrices,
 * lighting parameters, material properties, etc.). They support dynamic
 * updates for frame-varying data.
 */
class ConstantBuffer {
public:
    ConstantBuffer();
    ~ConstantBuffer();

    // Non-copyable, non-movable (owns backend resources)
    ConstantBuffer(const ConstantBuffer&) = delete;
    ConstantBuffer& operator=(const ConstantBuffer&) = delete;
    ConstantBuffer(ConstantBuffer&&) = delete;
    ConstantBuffer& operator=(ConstantBuffer&&) = delete;

    /**
     * @brief Initializes the constant buffer from a device and description.
     *
     * Data is copied during initialization; the caller may free their copy
     * after this call returns. If data is null, the buffer is created with
     * undefined contents and must be updated before use.
     *
     * @param device The graphics device. Must remain valid for buffer lifetime.
     * @param desc Buffer description.
     * @return ConstantBufferErrorCode::Success on success, error code on failure.
     *
     * Error conditions (call get_last_error() for details):
     * - Device is null or invalid
     * - Buffer name is null
     * - Size is 0
     * - Size is not a multiple of 16 bytes (std140 layout requirement)
     * - Size exceeds maximum (256 MB)
     * - Buffer creation failed
     */
    ConstantBufferErrorCode initialize(Device* device, const ConstantBufferDesc& desc);

    /**
     * @brief Updates the buffer contents dynamically.
     *
     * The buffer must have been created with initialize(). The update is
     * immediate and affects subsequent draw calls.
     *
     * @param data Pointer to new data.
     * @param size Size of data in bytes.
     * @param offset Byte offset into the buffer (default 0).
     * @return ConstantBufferErrorCode::Success on success, error code on failure.
     */
    ConstantBufferErrorCode update(const void* data, uint32_t size, uint32_t offset = 0);

    /**
     * @brief Returns true if the buffer is valid.
     */
    bool is_valid() const;

    /**
     * @brief Returns the device that created this buffer.
     */
    Device* get_device() const;

    /**
     * @brief Returns the buffer size in bytes.
     */
    uint32_t get_size() const;

    /**
     * @brief Returns the buffer name.
     */
    const char* get_name() const;

    /**
     * @brief Returns the last error message from buffer operations.
     */
    const char* get_last_error() const;

private:
    struct Impl;
    Impl* impl_;
    
    // Allow Context and Material to access private impl_ for backend operations
    friend class Context;
    friend class Material;
};

} // namespace poko::render

#endif // POKO_RENDER_CONSTANT_BUFFER_H
