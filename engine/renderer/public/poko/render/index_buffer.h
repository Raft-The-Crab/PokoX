/**
 * @file index_buffer.h
 * @brief Poko-owned index buffer abstraction.
 *
 * This header contains only Poko types. The Diligent Core implementation lives
 * behind the Pimpl pattern in backend/diligent/. Consumers never need to include
 * Diligent headers.
 *
 * M4 Scope: Supports 16-bit and 32-bit unsigned indices. Other formats are
 * deferred to future milestones.
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
    BufferCreationFailed = -6
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
     */
    IndexBufferErrorCode initialize(Device* device, const IndexBufferDesc& desc);

    /**
     * @brief Returns true if the buffer is valid.
     */
    bool is_valid() const;

private:
    struct Impl;
    Impl* impl_;
    
    // Allow Context to access private impl_ for backend operations
    friend class Context;
};

} // namespace poko::render

#endif // POKO_RENDER_INDEX_BUFFER_H
