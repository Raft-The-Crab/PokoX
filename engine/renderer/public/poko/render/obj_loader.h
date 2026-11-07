/**
 * @file obj_loader.h
 * @brief OBJ mesh loader.
 *
 * Loads Wavefront OBJ files into Poko Mesh objects. Supports position,
 * normal, and texcoord attributes. Converts to a unified vertex buffer
 * by deduplicating (position, normal, texcoord) tuples.
 *
 * Usage example:
 * @code
 * ObjLoader loader;
 * Mesh mesh;
 * MeshDesc desc;
 * if (loader.load("model.obj", device, desc)) {
 *     mesh.initialize(device, desc);
 * }
 * @endcode
 */

#ifndef POKO_RENDER_OBJ_LOADER_H
#define POKO_RENDER_OBJ_LOADER_H

#include "device.h"
#include "mesh.h"
#include <cstdint>
#include <string>

namespace poko::render {

/**
 * @brief OBJ loader error codes.
 */
enum class ObjLoaderError : int32_t {
    Success = 0,
    Unknown = -1,
    FileNotFound = -2,
    InvalidFile = -3,
    NoVertices = -4,
    NoFaces = -5,
    IndexOutOfRange = -6,
    BufferOverflow = -7
};

/**
 * @brief OBJ mesh loader.
 */
class ObjLoader {
public:
    ObjLoader();
    ~ObjLoader() = default;

    /**
     * @brief Loads an OBJ file and populates a MeshDesc.
     *
     * The returned MeshDesc owns dynamically allocated buffers; the caller
     * must free them after use (MeshDesc::cleanup() helper provided).
     *
     * @param filepath Path to the OBJ file.
     * @param device Graphics device (for buffer sizing).
     * @param out_desc Receives the mesh description.
     * @return ObjLoaderError::Success on success, error code on failure.
     */
    ObjLoaderError load(const char* filepath, Device* device, MeshDesc& out_desc);

    /**
     * @brief Cleans up dynamically allocated buffers in a MeshDesc.
     *
     * Call this after the buffers have been passed to Mesh::initialize()
     * if you no longer need the intermediate data.
     *
     * @param desc The MeshDesc to clean up.
     */
    static void cleanup(MeshDesc& desc);

    /**
     * @brief Returns the last error message.
     */
    const char* get_last_error() const;

private:
    std::string last_error_;
};

} // namespace poko::render

#endif // POKO_RENDER_OBJ_LOADER_H
