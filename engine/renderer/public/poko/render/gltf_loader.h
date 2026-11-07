/**
 * @file gltf_loader.h
 * @brief glTF 2.0 / GLB mesh and animation loader.
 *
 * Loads binary glTF (.glb) assets into CPU-side structures describing
 * meshes (interleaved vertex data), materials, images, skins (joint
 * hierarchies + inverse bind matrices), and animation clips (TRS channels).
 *
 * The loader performs no GPU work; pass the resulting GltfAsset to
 * SkinnedModel (or build Mesh objects manually) for rendering.
 *
 * Supported subset:
 * - GLB container (JSON + BIN chunks); .gltf with embedded base64 buffers
 * - Mesh primitives with POSITION/NORMAL/TEXCOORD_0/JOINTS_0/WEIGHTS_0
 * - Accessors: SCALAR/VEC2/VEC3/VEC4/MAT4, component types 5120-5126,
 *   normalized and non-normalized, interleaved bufferViews
 * - Materials: baseColorFactor, baseColorTexture, metallic/roughness,
 *   doubleSided
 * - Images: bufferView-embedded (raw bytes retained; decode is the
 *   caller's responsibility — see SkinnedModel texture_path option)
 * - Skins: joints + inverseBindMatrices
 * - Animations: translation/rotation/scale channels, LINEAR and STEP
 *   interpolation, node TRS or matrix transforms
 *
 * Usage example:
 * @code
 * GltfLoader loader;
 * GltfAsset asset;
 * if (loader.load("character.glb", asset) == GltfLoaderError::Success) {
 *     SkinnedModel model;
 *     model.initialize(device, asset, desc);
 * }
 * @endcode
 */

#ifndef POKO_RENDER_GLTF_LOADER_H
#define POKO_RENDER_GLTF_LOADER_H

#include "device.h"
#include "vertex_buffer.h"
#include "mesh.h"
#include "poko/math/vec3.h"
#include "poko/math/vec4.h"
#include "poko/math/quat.h"
#include "poko/math/mat4.h"
#include <cstdint>
#include <string>
#include <vector>

namespace poko::render {

/**
 * @brief glTF loader error codes.
 */
enum class GltfLoaderError : int32_t {
    Success = 0,
    Unknown = -1,
    FileNotFound = -2,
    InvalidFile = -3,
    UnsupportedVersion = -4,
    ParseError = -5,
    MissingBuffer = -6,
    UnsupportedAccessor = -7,
    OutOfRange = -8
};

/**
 * @brief A mesh primitive with a fixed interleaved vertex layout.
 *
 * All primitives use the same vertex format so a single pipeline can draw
 * any of them:
 *   offset  0: POSITION  Float3
 *   offset 12: NORMAL    Float3
 *   offset 24: TEXCOORD0 Float2
 *   offset 32: JOINTS0   Float4 (indices as floats, 0-3 influences)
 *   offset 48: WEIGHTS0  Float4
 * Stride: 64 bytes (16 floats per vertex).
 *
 * Attributes missing from the source glTF are filled with defaults
 * (normal = +Z, uv = 0, joints = 0, weights = (1,0,0,0)).
 */
struct GltfPrimitive {
    std::string name;
    std::vector<float> vertices;   // Interleaved vertex data
    uint32_t vertex_count = 0;
    std::vector<uint32_t> indices;
    int32_t material = -1;         // Index into GltfAsset::materials
    bool skinned = false;

    /**
     * @brief Fills a MeshDesc pointing into this primitive's data.
     *
     * The returned MeshDesc references memory owned by this primitive;
     * it remains valid until the GltfAsset is destroyed or modified.
     */
    MeshDesc make_mesh_desc() const;
};

/**
 * @brief glTF material (PBR metallic-roughness subset).
 */
struct GltfMaterial {
    std::string name;
    math::Vec4 base_color_factor{1.0f, 1.0f, 1.0f, 1.0f};
    int32_t base_color_texture = -1;  // Index into GltfAsset::images
    float metallic_factor = 1.0f;
    float roughness_factor = 1.0f;
    bool double_sided = false;
};

/**
 * @brief Embedded image. Pixel data is NOT decoded; `data` holds the raw
 * encoded bytes (PNG/JPEG) for the caller to decode.
 */
struct GltfImage {
    std::string name;
    std::string mime_type;
    std::vector<uint8_t> data;
};

/**
 * @brief Skin: joint node indices and inverse bind matrices.
 */
struct GltfSkin {
    std::string name;
    std::vector<int32_t> joints;              // Node indices
    std::vector<math::Mat4> inverse_bind_matrices;
};

/**
 * @brief Animation channel target path.
 */
enum class GltfAnimPath : uint32_t {
    Translation = 0,
    Rotation = 1,
    Scale = 2,
    Weights = 3   // Morph targets; parsed but ignored by the sampler
};

/**
 * @brief Animation interpolation mode.
 */
enum class GltfAnimInterp : uint32_t {
    Linear = 0,
    Step = 1,
    CubicSpline = 2  // Parsed but falls back to linear
};

/**
 * @brief A single animated node property (one sampler).
 */
struct GltfAnimChannel {
    int32_t node = -1;
    GltfAnimPath path = GltfAnimPath::Translation;
    GltfAnimInterp interp = GltfAnimInterp::Linear;
    std::vector<float> times;    // Seconds
    std::vector<float> values;   // path-dependent: 3 (T), 4 (R), 3 (S)
    uint32_t value_stride = 0;   // Floats per keyframe
};

/**
 * @brief An animation clip.
 */
struct GltfAnimation {
    std::string name;
    float duration = 0.0f;
    std::vector<GltfAnimChannel> channels;
};

/**
 * @brief A glTF node (joint, mesh attachment, or plain transform node).
 */
struct GltfNode {
    std::string name;
    math::Vec3 translation{0.0f, 0.0f, 0.0f};
    math::Quat rotation{0.0f, 0.0f, 0.0f, 1.0f};
    math::Vec3 scale{1.0f, 1.0f, 1.0f};
    bool has_matrix = false;
    math::Mat4 matrix = math::Mat4::identity();
    int32_t mesh = -1;          // Index into scene meshes (resolved to primitives)
    int32_t skin = -1;
    std::vector<int32_t> children;
    int32_t parent = -1;
};

/**
 * @brief Parsed glTF asset (CPU-side, no GPU resources).
 */
struct GltfAsset {
    std::vector<GltfPrimitive> primitives;
    std::vector<GltfMaterial> materials;
    std::vector<GltfImage> images;
    std::vector<GltfSkin> skins;
    std::vector<GltfAnimation> animations;
    std::vector<GltfNode> nodes;
    std::vector<int32_t> scene_roots;

    /**
     * @brief Computes the local transform matrix for a node.
     */
    math::Mat4 node_local_matrix(int32_t node_index) const;

    /**
     * @brief Computes world-space matrices for all nodes via DFS from roots.
     */
    std::vector<math::Mat4> compute_global_transforms() const;

    /**
     * @brief Finds an animation clip by name (case-insensitive).
     * @return Index into animations, or -1.
     */
    int32_t find_animation(const char* name) const;
};

/**
 * @brief glTF / GLB loader.
 */
class GltfLoader {
public:
    GltfLoader();
    ~GltfLoader() = default;

    /**
     * @brief Loads a .glb (or .gltf with data: URI buffers) file.
     *
     * @param filepath Path to the glTF file.
     * @param out_asset Receives the parsed asset.
     * @return GltfLoaderError::Success on success, error code on failure.
     */
    GltfLoaderError load(const char* filepath, GltfAsset& out_asset);

    /**
     * @brief Returns the last error message.
     */
    const char* get_last_error() const;

private:
    std::string last_error_;
};

} // namespace poko::render

#endif // POKO_RENDER_GLTF_LOADER_H
