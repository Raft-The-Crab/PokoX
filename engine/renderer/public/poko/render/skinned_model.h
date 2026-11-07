/**
 * @file skinned_model.h
 * @brief GPU skinned mesh model with animation playback.
 *
 * Wraps a parsed GltfAsset into render-ready resources: one Mesh per
 * primitive, a shared skinned-mesh pipeline, the base-color texture, and
 * constant buffers for the view-projection matrix, object transform, and
 * bone palette (up to 64 joints).
 *
 * Animation clips are sampled on the CPU each update(); bone matrices are
 * uploaded to the bone constant buffer before rendering.
 *
 * Usage example:
 * @code
 * GltfLoader loader;
 * GltfAsset asset;
 * loader.load("character.glb", asset);
 *
 * SkinnedModel model;
 * SkinnedModelDesc desc;
 * desc.name = "Hero";
 * desc.texture_path = "textures/character.tga";
 * desc.animation = "Walk";
 * model.initialize(device, asset, desc);
 *
 * // Per frame:
 * model.set_world_matrix(entity_world_matrix);
 * model.update(delta_time);
 * model.render(context, view_projection_matrix);
 * @endcode
 */

#ifndef POKO_RENDER_SKINNED_MODEL_H
#define POKO_RENDER_SKINNED_MODEL_H

#include "device.h"
#include "context.h"
#include "gltf_loader.h"
#include "poko/math/mat4.h"
#include <cstdint>
#include <string>
#include <vector>

namespace poko::render {

class Mesh;
class Texture;
class Material;
class PipelineState;
class ConstantBuffer;

/**
 * @brief Maximum number of bone influences per vertex (matches JOINTS_0/WEIGHTS_0).
 */
inline constexpr uint32_t kMaxSkinInfluences = 4;

/**
 * @brief Maximum number of joints supported by the bone palette.
 */
inline constexpr uint32_t kMaxBones = 64;

/**
 * @brief Skinned model description.
 */
struct SkinnedModelDesc {
    const char* name = "SkinnedModel";
    /**
     * @brief Optional path to a base-color texture (BMP/TGA via
     * TextureLoader). When null, a 1x1 white texture is used.
     */
    const char* texture_path = nullptr;
    /**
     * @brief Optional animation clip name to play on initialize.
     */
    const char* animation = nullptr;
    /**
     * @brief Whether the animation loops (default true).
     */
    bool loop = true;
    /**
     * @brief Animation playback speed multiplier (default 1.0).
     */
    float playback_speed = 1.0f;
};

/**
 * @brief Skinned model error codes.
 */
enum class SkinnedModelError : int32_t {
    Success = 0,
    Unknown = -1,
    InvalidDevice = -2,
    InvalidAsset = -3,
    ResourceCreationFailed = -4,
    TextureLoadFailed = -5,
    ShaderCompilationFailed = -6
};

/**
 * @brief GPU skinned model with animation playback.
 *
 * Non-copyable, non-movable. Owns all GPU resources; the GltfAsset may be
 * destroyed after initialize() returns.
 */
class SkinnedModel {
public:
    SkinnedModel();
    ~SkinnedModel();

    SkinnedModel(const SkinnedModel&) = delete;
    SkinnedModel& operator=(const SkinnedModel&) = delete;
    SkinnedModel(SkinnedModel&&) = delete;
    SkinnedModel& operator=(SkinnedModel&&) = delete;

    /**
     * @brief Initializes GPU resources from a parsed glTF asset.
     *
     * @param device Graphics device.
     * @param asset Parsed glTF asset (meshes, skin, animations).
     * @param desc Model description.
     * @return SkinnedModelError::Success on success.
     */
    SkinnedModelError initialize(Device* device, const GltfAsset& asset,
                                 const SkinnedModelDesc& desc);

    /**
     * @brief Returns true if the model is valid and renderable.
     */
    bool is_valid() const;

    /**
     * @brief Returns the last error message.
     */
    const char* get_last_error() const;

    /**
     * @brief Sets the world transform applied after skinning.
     */
    void set_world_matrix(const math::Mat4& world);

    /**
     * @brief Returns the current world matrix.
     */
    const math::Mat4& get_world_matrix() const;

    /**
     * @brief Advances animation and recomputes bone matrices.
     *
     * @param delta_time Seconds since last update.
     */
    void update(float delta_time);

    /**
     * @brief Sets the animation clip by name (case-insensitive).
     * Pass nullptr to return to bind pose.
     */
    void set_animation(const char* name);

    /**
     * @brief Sets animation playback time in seconds.
     */
    void set_animation_time(float time_seconds);

    /**
     * @brief Returns the current animation time in seconds.
     */
    float get_animation_time() const;

    /**
     * @brief Returns the active animation clip name, or "" for bind pose.
     */
    const char* get_animation_name() const;

    /**
     * @brief Returns the number of animation clips in the asset.
     */
    uint32_t get_animation_count() const;

    /**
     * @brief Returns the name of animation clip at index.
     */
    const char* get_animation_name_at(uint32_t index) const;

    /**
     * @brief Returns the number of joints in the skeleton.
     */
    uint32_t get_bone_count() const;

    /**
     * @brief Returns the number of mesh primitives.
     */
    uint32_t get_primitive_count() const;

    /**
     * @brief Renders all primitives with the given view-projection matrix.
     *
     * Updates the per-frame constant buffers and issues draw calls for
     * each primitive. The context must have a valid render target bound.
     *
     * @param context Command context.
     * @param view_projection Combined view-projection matrix.
     */
    void render(Context* context, const math::Mat4& view_projection);

private:
    struct Impl;
    Impl* impl_;

    /**
     * @brief Recomputes the bone palette from the current pose/time.
     */
    void compute_bones();
};

} // namespace poko::render

#endif // POKO_RENDER_SKINNED_MODEL_H
