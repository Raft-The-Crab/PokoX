/**
 * @file vertex_buffer.h
 * @brief Poko-owned vertex buffer abstraction.
 *
 * This header contains only Poko types. The Diligent Core implementation lives
 * behind the Pimpl pattern in backend/diligent/. Consumers never need to include
 * Diligent headers.
 *
 * M3 Scope: Supports vertex buffer creation with position attribute (Float3).
 * Other attributes and index buffers are deferred to future milestones.
 */

#ifndef POKO_RENDER_VERTEX_BUFFER_H
#define POKO_RENDER_VERTEX_BUFFER_H

#include "device.h"
#include <cstdint>

namespace poko::render {

/**
 * @brief Vertex attribute data format.
 */
enum class VertexFormat : uint32_t {
    Unknown = 0,
    Float = 1,      // 1x float (4 bytes)
    Float2 = 2,     // 2x float (8 bytes)
    Float3 = 3,     // 3x float (12 bytes) - M3: Position
    Float4 = 4,     // 4x float (16 bytes)
    Int = 5,        // 1x int32 (4 bytes)
    Int2 = 6,       // 2x int32 (8 bytes)
    Int3 = 7,       // 3x int32 (12 bytes)
    Int4 = 8,       // 4x int32 (16 bytes)
    UInt = 9,       // 1x uint32 (4 bytes)
    UInt2 = 10,     // 2x uint32 (8 bytes)
    UInt3 = 11,     // 3x uint32 (12 bytes)
    UInt4 = 12      // 4x uint32 (16 bytes)
};

/**
 * @brief Vertex attribute semantic (what the attribute represents).
 */
enum class VertexSemantic : uint32_t {
    Unknown = 0,
    Position = 1,   // M3: Vertex position
    Normal = 2,
    TexCoord = 3,
    Color = 4
};

/**
 * @brief Vertex attribute description.
 */
struct VertexAttribute {
    VertexSemantic semantic = VertexSemantic::Unknown;
    VertexFormat format = VertexFormat::Unknown;
    uint32_t offset = 0;  // Byte offset from vertex start
};

/**
 * @brief Vertex layout description.
 */
struct VertexLayout {
    const VertexAttribute* attributes = nullptr;
    uint32_t attribute_count = 0;
    uint32_t stride = 0;  // Total vertex size in bytes
};

/**
 * @brief Vertex buffer description.
 */
struct BufferDesc {
    const void* data = nullptr;  // Initial data (can be null)
    uint32_t size = 0;           // Buffer size in bytes
    uint32_t stride = 0;         // Size of each vertex in bytes
    const char* name = "VertexBuffer";
};

/**
 * @brief Poko error codes for buffer operations.
 */
enum class BufferErrorCode : int32_t {
    Success = 0,
    Unknown = -1,
    InvalidDevice = -2,
    InvalidData = -3,
    InvalidSize = -4,
    InvalidLayout = -5,
    InvalidStride = -6,
    InvalidOffset = -7,
    BufferCreationFailed = -8
};

/**
 * @brief Opaque handle to a vertex buffer.
 *
 * The implementation is private; consumers cannot depend on its layout.
 * Lifetime: Must not outlive the Device used to create it.
 */
class VertexBuffer {
public:
    VertexBuffer();
    ~VertexBuffer();

    // Non-copyable, non-movable (owns backend resources)
    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;
    VertexBuffer(VertexBuffer&&) = delete;
    VertexBuffer& operator=(VertexBuffer&&) = delete;

    /**
     * @brief Initializes the vertex buffer from a device and description.
     *
     * Data is copied during initialization; the caller may free their copy
     * after this call returns.
     *
     * @param device The graphics device. Must remain valid for buffer lifetime.
     * @param desc Buffer description.
     * @return BufferErrorCode::Success on success, error code on failure.
     */
    BufferErrorCode initialize(Device* device, const BufferDesc& desc);

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

#endif // POKO_RENDER_VERTEX_BUFFER_H
