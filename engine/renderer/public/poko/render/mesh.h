/**
 * @file mesh.h
 * @brief Mesh abstraction combining vertex/index buffers and layout.
 *
 * A Mesh owns a VertexBuffer, IndexBuffer, and VertexLayout, providing a
 * convenient single object for model rendering. Supports common vertex
 * formats (position, normal, texcoord, color) and both 16/32-bit indices.
 *
 * Usage example:
 * @code
 * Mesh mesh;
 * MeshDesc desc;
 * desc.vertex_data = vertices;
 * desc.vertex_count = vertex_count;
 * desc.index_data = indices;
 * desc.index_count = index_count;
 * desc.layout = &my_layout;
 * mesh.initialize(device, desc);
 *
 * // Bind for drawing
 * context.set_vertex_buffer(mesh.get_vertex_buffer());
 * context.set_index_buffer(mesh.get_index_buffer());
 * context.draw_indexed(mesh.get_index_count());
 * @endcode
 */

#ifndef POKO_RENDER_MESH_H
#define POKO_RENDER_MESH_H

#include "device.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include <cstdint>

namespace poko::render {

struct VertexLayout;

/**
 * @brief Mesh description.
 */
struct MeshDesc {
    const void* vertex_data = nullptr;
    uint32_t vertex_count = 0;
    uint32_t vertex_stride = 0;
    const void* index_data = nullptr;
    uint32_t index_count = 0;
    IndexFormat index_format = IndexFormat::UInt16;
    const VertexLayout* layout = nullptr;
    const char* name = "Mesh";
};

/**
 * @brief Mesh error codes.
 */
enum class MeshErrorCode : int32_t {
    Success = 0,
    Unknown = -1,
    InvalidDevice = -2,
    InvalidVertexData = -3,
    InvalidIndexData = -4,
    InvalidLayout = -5,
    BufferCreationFailed = -6
};

/**
 * @brief Mesh combining vertex/index buffers and layout.
 */
class Mesh {
public:
    Mesh();
    ~Mesh();

    // Non-copyable, non-movable
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&&) = delete;
    Mesh& operator=(Mesh&&) = delete;

    /**
     * @brief Initializes the mesh from a device and description.
     *
     * Creates internal vertex and index buffers. Data is copied during
     * initialization; the caller may free their copy after this call.
     *
     * @param device The graphics device.
     * @param desc Mesh description.
     * @return MeshErrorCode::Success on success, error code on failure.
     */
    MeshErrorCode initialize(Device* device, const MeshDesc& desc);

    /**
     * @brief Returns true if the mesh is valid.
     */
    bool is_valid() const;

    /**
     * @brief Returns the vertex buffer.
     */
    VertexBuffer* get_vertex_buffer();

    /**
     * @brief Returns the vertex buffer (const).
     */
    const VertexBuffer* get_vertex_buffer() const;

    /**
     * @brief Returns the index buffer.
     */
    IndexBuffer* get_index_buffer();

    /**
     * @brief Returns the index buffer (const).
     */
    const IndexBuffer* get_index_buffer() const;

    /**
     * @brief Returns the vertex layout.
     */
    const VertexLayout* get_layout() const;

    /**
     * @brief Returns the vertex count.
     */
    uint32_t get_vertex_count() const;

    /**
     * @brief Returns the index count.
     */
    uint32_t get_index_count() const;

    /**
     * @brief Returns the mesh name.
     */
    const char* get_name() const;

    /**
     * @brief Returns the last error message.
     */
    const char* get_last_error() const;

private:
    struct Impl;
    Impl* impl_;
};

} // namespace poko::render

#endif // POKO_RENDER_MESH_H
