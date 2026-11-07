/**
 * @file skinned_model.cpp
 * @brief GPU skinned model with CPU animation sampling.
 */

#include "poko/render/skinned_model.h"

#include "poko/render/mesh.h"
#include "poko/render/texture.h"
#include "poko/render/material.h"
#include "poko/render/pipeline.h"
#include "poko/render/constant_buffer.h"
#include "poko/render/texture_loader.h"
#include "poko/render/context.h"

#include "poko/math/vec3.h"
#include "poko/math/vec4.h"
#include "poko/math/quat.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <memory>
#include <vector>

namespace poko::render {

namespace {

// Vertex shader: 5 attributes matching the GltfPrimitive interleaved layout.
// Bone palette and object transform arrive through uniform blocks.
constexpr char kSkinnedVertexShader[] = R"(#version 430 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec4 aJoints;
layout(location = 4) in vec4 aWeights;

layout(std140) uniform g_Transforms {
    mat4 uViewProj;
    mat4 uWorld;
};

layout(std430) readonly buffer g_Bones {
    mat4 uBoneMatrices[64];
};

out vec3 vNormal;
out vec2 vTexCoord;

void main()
{
    int j0 = int(aJoints.x + 0.5);
    int j1 = int(aJoints.y + 0.5);
    int j2 = int(aJoints.z + 0.5);
    int j3 = int(aJoints.w + 0.5);

    mat4 skin = uBoneMatrices[j0] * aWeights.x
              + uBoneMatrices[j1] * aWeights.y
              + uBoneMatrices[j2] * aWeights.z
              + uBoneMatrices[j3] * aWeights.w;

    vec4 skinned = skin * vec4(aPosition, 1.0);
    vec4 worldPos = uWorld * skinned;
    gl_Position = uViewProj * worldPos;

    vNormal = normalize(mat3(uWorld) * (mat3(skin) * aNormal));
    vTexCoord = aTexCoord;
}
)";

constexpr char kSkinnedPixelShader[] = R"(#version 430 core

in vec3 vNormal;
in vec2 vTexCoord;

uniform sampler2D g_Texture;

out vec4 FragColor;

void main()
{
    vec4 tex = texture(g_Texture, vTexCoord);
    vec3 n = normalize(vNormal);
    vec3 lightDir = normalize(vec3(0.45, 0.85, 0.35));
    float diff = max(dot(n, lightDir), 0.0);
    float ambient = 0.38;
    FragColor = vec4(tex.rgb * (ambient + 0.75 * diff), tex.a);
}
)";

// Per-object constants: viewProj + world (std140: two mat4 = 128 bytes)
struct TransformConstants {
    math::Mat4 view_proj;
    math::Mat4 world;
};

// Bone palette: up to 64 joints (std140: 64 * mat4 = 4096 bytes)
struct BoneConstants {
    math::Mat4 bones[kMaxBones];
};

// Animated local TRS for one node
struct LocalPose {
    math::Vec3 translation{0.0f, 0.0f, 0.0f};
    math::Quat rotation{0.0f, 0.0f, 0.0f, 1.0f};
    math::Vec3 scale{1.0f, 1.0f, 1.0f};
};

math::Vec3 sample_vec3(const GltfAnimChannel& channel, float time) {
    const auto& times = channel.times;
    if (times.empty()) return math::Vec3(0.0f, 0.0f, 0.0f);
    auto value_at = [&](size_t key) -> math::Vec3 {
        const float* v = channel.values.data() + key * channel.value_stride;
        return math::Vec3(v[0], v[1], v[2]);
    };
    if (time <= times.front()) return value_at(0);
    if (time >= times.back()) return value_at(times.size() - 1);

    size_t lo = 0, hi = times.size() - 1;
    while (hi - lo > 1) {
        size_t mid = (lo + hi) / 2;
        if (times[mid] <= time) lo = mid; else hi = mid;
    }
    float t0 = times[lo], t1 = times[hi];
    float f = (t1 > t0) ? (time - t0) / (t1 - t0) : 0.0f;
    f = f < 0.0f ? 0.0f : (f > 1.0f ? 1.0f : f);

    if (channel.interp == GltfAnimInterp::Step) return value_at(lo);
    math::Vec3 a = value_at(lo), b = value_at(hi);
    return math::Vec3(a.x + (b.x - a.x) * f,
                      a.y + (b.y - a.y) * f,
                      a.z + (b.z - a.z) * f);
}

math::Quat sample_quat(const GltfAnimChannel& channel, float time) {
    const auto& times = channel.times;
    if (times.empty()) return math::Quat::identity();
    auto value_at = [&](size_t key) -> math::Quat {
        const float* v = channel.values.data() + key * channel.value_stride;
        return math::Quat(v[0], v[1], v[2], v[3]);
    };
    if (time <= times.front()) return value_at(0).normalized();
    if (time >= times.back()) return value_at(times.size() - 1).normalized();

    size_t lo = 0, hi = times.size() - 1;
    while (hi - lo > 1) {
        size_t mid = (lo + hi) / 2;
        if (times[mid] <= time) lo = mid; else hi = mid;
    }
    float t0 = times[lo], t1 = times[hi];
    float f = (t1 > t0) ? (time - t0) / (t1 - t0) : 0.0f;
    f = f < 0.0f ? 0.0f : (f > 1.0f ? 1.0f : f);

    if (channel.interp == GltfAnimInterp::Step) return value_at(lo).normalized();
    return math::Quat::slerp(value_at(lo).normalized(), value_at(hi).normalized(), f);
}

} // namespace

struct SkinnedModel::Impl {
    Device* device = nullptr;
    GltfAsset asset;
    SkinnedModelDesc desc;
    std::string name;
    std::string last_error;

    std::vector<std::unique_ptr<Mesh>> meshes;
    Texture texture;
    PipelineState pipeline;
    Material material;
    ConstantBuffer cb_transforms;
    ConstantBuffer cb_bones;

    math::Mat4 world = math::Mat4::identity();
    TransformConstants transform_data{};
    BoneConstants bone_data{};

    // Animation state
    int32_t animation_index = -1;
    float animation_time = 0.0f;
    std::vector<LocalPose> pose;          // Per-node animated local TRS
    std::vector<math::Mat4> bone_matrices; // jMaxBones palette
    bool bones_dirty = true;

    bool valid = false;
};

SkinnedModel::SkinnedModel() : impl_(new Impl()) {}

SkinnedModel::~SkinnedModel() {
    delete impl_;
}

SkinnedModelError SkinnedModel::initialize(Device* device, const GltfAsset& asset,
                                           const SkinnedModelDesc& desc) {
    if (!device || !device->is_valid()) {
        impl_->last_error = "Device is invalid";
        return SkinnedModelError::InvalidDevice;
    }
    if (asset.primitives.empty()) {
        impl_->last_error = "Asset has no mesh primitives";
        return SkinnedModelError::InvalidAsset;
    }

    impl_->device = device;
    impl_->asset = asset;
    impl_->desc = desc;
    impl_->name = desc.name ? desc.name : "SkinnedModel";

    // ------------------------------------------------------------- meshes
    impl_->meshes.reserve(asset.primitives.size());
    for (const auto& prim : asset.primitives) {
        MeshDesc md = prim.make_mesh_desc();
        auto mesh = std::make_unique<Mesh>();
        MeshErrorCode rc = mesh->initialize(device, md);
        if (rc != MeshErrorCode::Success) {
            impl_->last_error = std::string("Mesh creation failed: ") + mesh->get_last_error();
            return SkinnedModelError::ResourceCreationFailed;
        }
        impl_->meshes.push_back(std::move(mesh));
    }

    // ------------------------------------------------------------ texture
    TextureDesc tex_desc;
    bool has_texture = false;
    if (desc.texture_path) {
        TextureLoader loader;
        if (loader.load(desc.texture_path, tex_desc) == TextureLoaderError::Success) {
            has_texture = true;
            tex_desc.data_size = tex_desc.width * tex_desc.height * 4;
        } else {
            impl_->last_error = std::string("Texture load failed: ") + loader.get_last_error();
            return SkinnedModelError::TextureLoadFailed;
        }
    }
    if (!has_texture) {
        // 1x1 white fallback
        static const uint8_t white[4] = {255, 255, 255, 255};
        tex_desc.width = 1;
        tex_desc.height = 1;
        tex_desc.format = TextureFormat::RGBA8_UNORM;
        tex_desc.usage = TextureUsage::ShaderResource;
        tex_desc.data = white;
        tex_desc.data_size = sizeof(white);
        tex_desc.name = "DefaultWhite";
    }
    tex_desc.type = TextureType::Texture2D;
    tex_desc.mip_levels = 1;
    if (impl_->texture.initialize(device, tex_desc) != TextureErrorCode::Success) {
        if (desc.texture_path) TextureLoader::cleanup(tex_desc);
        impl_->last_error = std::string("Texture creation failed: ") + impl_->texture.get_last_error();
        return SkinnedModelError::ResourceCreationFailed;
    }
    if (desc.texture_path) TextureLoader::cleanup(tex_desc);

    // ---------------------------------------------------------- pipeline
    // Vertex layout shared by all primitives (same interleaved format)
    const VertexLayout* prim_layout = impl_->meshes.front()->get_layout();

    ShaderResource resources[3];
    resources[0].name = "g_Texture";
    resources[0].type = ShaderResourceType::TextureSampler;
    resources[0].shader_stages = 2;  // pixel
    resources[0].array_size = 1;
    resources[1].name = "g_Transforms";
    resources[1].type = ShaderResourceType::ConstantBuffer;
    resources[1].shader_stages = 1;  // vertex
    resources[1].array_size = 1;
    resources[2].name = "g_Bones";
    resources[2].type = ShaderResourceType::StructuredBuffer;
    resources[2].shader_stages = 1;  // vertex
    resources[2].array_size = 1;

    ResourceSignatureDesc sig_desc;
    sig_desc.name = "SkinnedModelSignature";
    sig_desc.resources = resources;
    sig_desc.resource_count = 3;

    ShaderDesc vs_desc;
    vs_desc.type = ShaderType::Vertex;
    vs_desc.language = ShaderLanguage::GLSL;
    vs_desc.source = kSkinnedVertexShader;
    vs_desc.entry_point = "main";

    ShaderDesc ps_desc;
    ps_desc.type = ShaderType::Pixel;
    ps_desc.language = ShaderLanguage::GLSL;
    ps_desc.source = kSkinnedPixelShader;
    ps_desc.entry_point = "main";

    PipelineDesc pipe_desc;
    pipe_desc.name = (impl_->name + "_Pipeline").c_str();
    pipe_desc.vertex_shader = vs_desc;
    pipe_desc.pixel_shader = ps_desc;
    pipe_desc.color_format = TextureFormat::RGBA8_UNORM;
    pipe_desc.depth_format = TextureFormat::D32_FLOAT;
    pipe_desc.topology = PrimitiveTopology::TriangleList;
    pipe_desc.depth_stencil.depth_enable = true;
    pipe_desc.depth_stencil.depth_write_enable = true;
    pipe_desc.depth_stencil.depth_func = ComparisonFunc::Less;
    pipe_desc.rasterizer.cull_mode = CullMode::None;  // glTF doubleSided typical
    pipe_desc.vertex_layout = *prim_layout;
    pipe_desc.resource_signature = sig_desc;

    PipelineErrorCode prc = impl_->pipeline.initialize(device, pipe_desc);
    if (prc != PipelineErrorCode::Success) {
        impl_->last_error = std::string("Pipeline failed: ") + impl_->pipeline.get_last_error();
        return SkinnedModelError::ShaderCompilationFailed;
    }

    // ------------------------------------------------------- constant buffers
    ConstantBufferDesc cb_desc;
    cb_desc.data = nullptr;
    cb_desc.size = sizeof(TransformConstants);
    cb_desc.name = "SkinnedTransforms";
    if (impl_->cb_transforms.initialize(device, cb_desc) != ConstantBufferErrorCode::Success) {
        impl_->last_error = std::string("CB failed: ") + impl_->cb_transforms.get_last_error();
        return SkinnedModelError::ResourceCreationFailed;
    }

    cb_desc.size = sizeof(BoneConstants);
    cb_desc.name = "SkinnedBones";
    cb_desc.allow_shader_resource_view = true;  // SSBO backing for g_Bones
    if (impl_->cb_bones.initialize(device, cb_desc) != ConstantBufferErrorCode::Success) {
        impl_->last_error = std::string("CB failed: ") + impl_->cb_bones.get_last_error();
        return SkinnedModelError::ResourceCreationFailed;
    }

    // ------------------------------------------------------------ material
    TextureBinding tex_binding;
    tex_binding.texture = &impl_->texture;
    tex_binding.shader_variable = "g_Texture";
    tex_binding.array_index = 0;

    ConstantBufferBinding cb_bindings[1];
    cb_bindings[0].buffer = &impl_->cb_transforms;
    cb_bindings[0].shader_variable = "g_Transforms";
    ConstantBufferBinding sb_bindings[1];
    sb_bindings[0].buffer = &impl_->cb_bones;
    sb_bindings[0].shader_variable = "g_Bones";

    MaterialDesc mat_desc;
    mat_desc.name = (impl_->name + "_Material").c_str();
    mat_desc.pipeline = &impl_->pipeline;
    mat_desc.texture_count = 1;
    mat_desc.texture_bindings = &tex_binding;
    mat_desc.constant_buffer_count = 1;
    mat_desc.constant_buffer_bindings = cb_bindings;
    mat_desc.structured_buffer_count = 1;
    mat_desc.structured_buffer_bindings = sb_bindings;

    MaterialErrorCode mrc = impl_->material.initialize(device, mat_desc);
    if (mrc != MaterialErrorCode::Success) {
        impl_->last_error = std::string("Material failed: ") + impl_->material.get_last_error();
        return SkinnedModelError::ResourceCreationFailed;
    }

    // --------------------------------------------------- animation / pose
    impl_->pose.resize(asset.nodes.size());
    for (size_t i = 0; i < asset.nodes.size(); ++i) {
        const GltfNode& n = asset.nodes[i];
        impl_->pose[i].translation = n.translation;
        impl_->pose[i].rotation = n.rotation;
        impl_->pose[i].scale = n.scale;
    }
    impl_->bone_matrices.assign(kMaxBones, math::Mat4::identity());

    if (desc.animation) {
        int32_t idx = asset.find_animation(desc.animation);
        if (idx >= 0) {
            impl_->animation_index = idx;
            impl_->animation_time = 0.0f;
            impl_->bones_dirty = true;
        }
    }

    compute_bones();
    impl_->valid = true;
    impl_->last_error.clear();
    return SkinnedModelError::Success;
}

bool SkinnedModel::is_valid() const {
    return impl_->valid;
}

const char* SkinnedModel::get_last_error() const {
    return impl_->last_error.c_str();
}

void SkinnedModel::set_world_matrix(const math::Mat4& world) {
    impl_->world = world;
}

const math::Mat4& SkinnedModel::get_world_matrix() const {
    return impl_->world;
}

void SkinnedModel::set_animation(const char* name) {
    impl_->animation_index = impl_->asset.find_animation(name);
    impl_->animation_time = 0.0f;
    compute_bones();
}

void SkinnedModel::set_animation_time(float time_seconds) {
    impl_->animation_time = time_seconds;
    compute_bones();
}

float SkinnedModel::get_animation_time() const {
    return impl_->animation_time;
}

const char* SkinnedModel::get_animation_name() const {
    if (impl_->animation_index < 0 ||
        impl_->animation_index >= static_cast<int32_t>(impl_->asset.animations.size())) {
        return "";
    }
    return impl_->asset.animations[impl_->animation_index].name.c_str();
}

uint32_t SkinnedModel::get_animation_count() const {
    return static_cast<uint32_t>(impl_->asset.animations.size());
}

const char* SkinnedModel::get_animation_name_at(uint32_t index) const {
    if (index >= impl_->asset.animations.size()) return "";
    return impl_->asset.animations[index].name.c_str();
}

uint32_t SkinnedModel::get_bone_count() const {
    if (impl_->asset.skins.empty()) return 0;
    return static_cast<uint32_t>(impl_->asset.skins[0].joints.size());
}

uint32_t SkinnedModel::get_primitive_count() const {
    return static_cast<uint32_t>(impl_->meshes.size());
}

void SkinnedModel::update(float delta_time) {
    if (impl_->animation_index < 0) return;
    const GltfAnimation& anim =
        impl_->asset.animations[impl_->animation_index];
    if (anim.duration <= 0.0f) return;

    float speed = impl_->desc.playback_speed > 0.0f ? impl_->desc.playback_speed : 1.0f;
    impl_->animation_time += delta_time * speed;
    if (impl_->desc.loop) {
        impl_->animation_time = std::fmod(impl_->animation_time, anim.duration);
    } else if (impl_->animation_time > anim.duration) {
        impl_->animation_time = anim.duration;
    }
    compute_bones();
}

void SkinnedModel::render(Context* context, const math::Mat4& view_projection) {
    if (!impl_->valid || !context) return;

    impl_->transform_data.view_proj = view_projection;
    impl_->transform_data.world = impl_->world;

    impl_->cb_transforms.update(&impl_->transform_data,
                                sizeof(TransformConstants), 0);
    impl_->cb_bones.update(impl_->bone_data.bones,
                           kMaxBones * sizeof(math::Mat4), 0);

    context->set_pipeline_state(&impl_->pipeline);
    context->set_material(&impl_->material);

    for (const auto& mesh : impl_->meshes) {
        context->set_vertex_buffer(mesh->get_vertex_buffer(), 0);
        context->set_index_buffer(mesh->get_index_buffer());
        context->draw_indexed(mesh->get_index_count());
    }
}

void SkinnedModel::compute_bones() {
    Impl& impl = *impl_;
    const GltfAsset& asset = impl.asset;

    // Reset pose to rest TRS
    for (size_t i = 0; i < asset.nodes.size() && i < impl.pose.size(); ++i) {
        const GltfNode& n = asset.nodes[i];
        impl.pose[i].translation = n.translation;
        impl.pose[i].rotation = n.rotation;
        impl.pose[i].scale = n.scale;
    }

    // Apply animation channels
    if (impl.animation_index >= 0 &&
        impl.animation_index < static_cast<int32_t>(asset.animations.size())) {
        const GltfAnimation& anim = asset.animations[impl.animation_index];
        float t = impl.animation_time;
        for (const auto& ch : anim.channels) {
            if (ch.node < 0 || ch.node >= static_cast<int32_t>(impl.pose.size())) continue;
            LocalPose& p = impl.pose[ch.node];
            switch (ch.path) {
                case GltfAnimPath::Translation:
                    p.translation = sample_vec3(ch, t);
                    break;
                case GltfAnimPath::Rotation:
                    p.rotation = sample_quat(ch, t);
                    break;
                case GltfAnimPath::Scale: {
                    math::Vec3 s = sample_vec3(ch, t);
                    p.scale = s;
                    break;
                }
                case GltfAnimPath::Weights:
                    break;  // Morph targets unsupported
            }
        }
    }

    // Compute global transforms via iterative DFS honoring animated locals.
    size_t node_count = asset.nodes.size();
    std::vector<math::Mat4> global(node_count, math::Mat4::identity());
    std::vector<int32_t> stack;
    for (int32_t root : asset.scene_roots) stack.push_back(root);
    while (!stack.empty()) {
        int32_t idx = stack.back();
        stack.pop_back();
        const GltfNode& node = asset.nodes[idx];
        math::Mat4 local;
        if (node.has_matrix && impl.animation_index < 0) {
            local = node.matrix;
        } else {
            const LocalPose& p = impl.pose[idx];
            local = math::Mat4::translation(p.translation) *
                    p.rotation.to_matrix() *
                    math::Mat4::scale(p.scale);
        }
        math::Mat4 parent_global = math::Mat4::identity();
        if (node.parent >= 0 && node.parent < static_cast<int32_t>(node_count)) {
            parent_global = global[node.parent];
        }
        global[idx] = parent_global * local;
        for (int32_t child : node.children) stack.push_back(child);
    }

    // Build bone palette from the first skin
    for (uint32_t i = 0; i < kMaxBones; ++i) {
        impl.bone_data.bones[i] = math::Mat4::identity();
    }
    if (!asset.skins.empty()) {
        const GltfSkin& skin = asset.skins[0];
        uint32_t count = static_cast<uint32_t>(skin.joints.size());
        if (count > kMaxBones) count = kMaxBones;
        for (uint32_t i = 0; i < count; ++i) {
            int32_t joint_node = skin.joints[i];
            if (joint_node < 0 || joint_node >= static_cast<int32_t>(node_count)) continue;
            math::Mat4 ibm = (i < skin.inverse_bind_matrices.size())
                                 ? skin.inverse_bind_matrices[i]
                                 : math::Mat4::identity();
            impl.bone_data.bones[i] = global[joint_node] * ibm;
        }
    }
    impl.bones_dirty = false;
}

} // namespace poko::render
