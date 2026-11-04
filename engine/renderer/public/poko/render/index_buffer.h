/**
 * @file index_buffer.h
 * @brief Poko-owned index buffer abstraction.
 *
 * This header contains only Poko types. The Diligent Core implementation lives
 * behind the Pimpl pattern in backend/diligent/. Consumers never need to include
 * Diligent headers.
 *
 * M4 Scope: Supports 16-bit and 32-bit unsigned indices with full validation.
 *
 * Usage example:
 * @code
 * const uint16_t indices[] = {0, 1, 2};
 *
 * IndexBuffer buffer;
 * IndexBufferDesc desc;
 * desc.data = indices;
 * desc.size = sizeof(indices);
 * desc.index_count = 3;
 * desc.format = IndexFormat::UInt16;
 * desc.name = "TriangleIndexBuffer";
 * buffer.initialize(device, desc);
 *
 * // Draw indexed
 * context.set_index_buffer(&buffer);
 * context.draw_indexed(3);
 *
 * // Update indices dynamically
 * buffer.update(new_indices, new_size, 0);
 * @endcode
 */

#ifndef POKO_RENDER_INDEX_BUFFER_H
#define POKO_RENDER_INDEX_BUFFER_H

#include "device.h"
#include <cstdint>

namespace poko::render {

/**
 * @brief Index buffer data format.
 */
enum class IndexFormat : uint32_t {
    Unknown = 0,
    UInt16 = 1,  // 16-bit unsigned indices
    UInt32 = 2   // 32-bit unsigned indices
};

/**
 * @brief Index buffer description.
 */
struct IndexBufferDesc {
    const void* data = nullptr;  // Initial data (can be null)
    uint32_t size = 0;           // Buffer size in bytes
    uint32_t index_count = 0;     // Number of indices
    IndexFormat format = IndexFormat::UInt16;
    const char* name = "IndexBuffer";
};

/**
 * @brief Poko error codes for index buffer operations.
 */
enum class IndexBufferErrorCode : int32_t {
    Success = 0,
    Unknown = -1,
    InvalidDevice = -2,
    InvalidData = -3,
    InvalidSize = -4,
    InvalidFormat = -5,
    InvalidOffset = -6,
    BufferCreationFailed = -7,
    UpdateFailed = -8
};

/**
 * @brief Opaque handle to an index buffer.
 *
 * The implementation is private; consumers cannot depend on its layout.
 * Lifetime: Must not outlive the Device used to create it.
 */
class IndexBuffer {
public:
    IndexBuffer();
    ~IndexBuffer();

    // Non-copyable, non-movable (owns backend resources)
    IndexBuffer(const IndexBuffer&) = delete;
    IndexBuffer& operator=(const IndexBuffer&) = delete;
    IndexBuffer(IndexBuffer&&) = delete;
    IndexBuffer& operator=(IndexBuffer&&) = delete;

    /**
     * @brief Initializes the index buffer from a device and description.
     *
     * Data is copied during initialization; the caller may free their copy
     * after this call returns.
     *
     * @param device The graphics device. Must remain valid for buffer lifetime.
     * @param desc Buffer description.
     * @return IndexBufferErrorCode::Success on success, error code on failure.
     *
     * Error conditions (call get_last_error() for details):
     * - Device is null or invalid
     * - Buffer name is null
     * - Size is 0
     * - Null data pointer with non-zero size
     * - Invalid format (not UInt16 or UInt32)
     * - Index count is 0
     * - Size does not match expected size for format and index count
     * - Size exceeds maximum (256 MB)
     * - Buffer creation failed
     */
    IndexBufferErrorCode initialize(Device* device, const IndexBufferDesc& desc);

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
     * @brief Returns the index format.
     */
    IndexFormat get_format() const;

    /**
     * @brief Returns the buffer name.
     */
    const char* get_name() const;

    /**
     * @brief Returns the last error message from buffer operations.
     */
    const char* get_last_error() const;

    /**
     * @brief Updates the buffer contents dynamically.
     *
     * @param data Pointer to new data.
     * @param size Size of data in bytes.
     * @param offset Byte offset into the buffer (default 0).
     * @return IndexBufferErrorCode::Success on success, error code on failure.
     */
    IndexBufferErrorCode update(const void* data, uint32_t size, uint32_t offset = 0);

private:
    struct Impl;
    Impl* impl_;
    
    // Allow Context to access private impl_ for backend operations
    friend class Context;
};

} // namespace poko::render

#endif // POKO_RENDER_INDEX_BUFFER_H
