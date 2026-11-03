/**
 * @file pipeline.h
 * @brief Poko-owned graphics pipeline state abstraction.
 *
 * This header contains only Poko types. The Diligent Core implementation lives
 * behind the Pimpl pattern in backend/diligent/. Consumers never need to include
 * Diligent headers.
 *
 * M2/M3 Scope: Supports GLSL shaders on the OpenGL backend only. Other shader
 * languages and backends are deferred to future milestones.
 */

#ifndef POKO_RENDER_PIPELINE_H
#define POKO_RENDER_PIPELINE_H

#include "device.h"
#include "vertex_buffer.h"
#include <cstdint>

namespace poko::render {

/**
 * @brief Shader language enumeration.
 *
 * M2 supports only GLSL on the OpenGL backend. Other languages are reserved
 * for future backend support.
 */
enum class ShaderLanguage : uint32_t {
    Unknown = 0,
    GLSL = 1,
    HLSL = 2,
    MSL = 3
};

/**
 * @brief Shader type enumeration.
 */
enum class ShaderType : uint32_t {
    Unknown = 0,
    Vertex = 1,
    Pixel = 2,
    Geometry = 3,
    Hull = 4,
    Domain = 5,
    Compute = 6
};

/**
 * @brief Rasterizer cull mode.
 */
enum class CullMode : uint32_t {
    None = 0,
    Front = 1,
    Back = 2
};

/**
 * @brief Primitive topology.
 */
enum class PrimitiveTopology : uint32_t {
    TriangleList = 0,
    TriangleStrip = 1,
    LineList = 2,
    PointList = 3
};

/**
 * @brief Blend factor enumeration.
 */
enum class BlendFactor : uint32_t {
    Zero = 0,
    One = 1,
    SrcColor = 2,
    InvSrcColor = 3,
    SrcAlpha = 4,
    InvSrcAlpha = 5,
    DestColor = 6,
    InvDestColor = 7,
    DestAlpha = 8,
    InvDestAlpha = 9,
    SrcAlphaSat = 10
};

/**
 * @brief Blend operation enumeration.
 */
enum class BlendOp : uint32_t {
    Add = 0,
    Subtract = 1,
    ReverseSubtract = 2,
    Min = 3,
    Max = 4
};

/**
 * @brief Blend state description.
 */
struct BlendStateDesc {
    bool blend_enable = false;
    BlendFactor src_blend = BlendFactor::One;
    BlendFactor dest_blend = BlendFactor::Zero;
    BlendOp blend_op = BlendOp::Add;
    BlendFactor src_blend_alpha = BlendFactor::One;
    BlendFactor dest_blend_alpha = BlendFactor::Zero;
    BlendOp blend_op_alpha = BlendOp::Add;
    uint8_t render_target_write_mask = 0xF;  // RGBA all enabled
};

/**
 * @brief Comparison function enumeration.
 */
enum class ComparisonFunc : uint32_t {
    Never = 0,
    Less = 1,
    Equal = 2,
    LessEqual = 3,
    Greater = 4,
    NotEqual = 5,
    GreaterEqual = 6,
    Always = 7
};

/**
 * @brief Texture address mode enumeration.
 */
enum class TextureAddressMode : uint32_t {
    Wrap = 0,
    Mirror = 1,
    Clamp = 2,
    Border = 3
};

/**
 * @brief Texture filter mode enumeration.
 */
enum class TextureFilterMode : uint32_t {
    Point = 0,
    Linear = 1,
    Anisotropic = 2
};

/**
 * @brief Depth stencil state description.
 */
struct DepthStencilDesc {
    bool depth_enable = false;
    bool depth_write_enable = false;
    ComparisonFunc depth_func = ComparisonFunc::Less;
    bool stencil_enable = false;
    uint8_t stencil_read_mask = 0xFF;
    uint8_t stencil_write_mask = 0xFF;
};

/**
 * @brief Sampler state description.
 *
 * Defines texture sampling behavior. For OpenGL with combined texture-samplers,
 * the sampler state is typically applied to the texture resource itself.
 */
struct SamplerDesc {
    TextureFilterMode min_filter = TextureFilterMode::Linear;
    TextureFilterMode mag_filter = TextureFilterMode::Linear;
    TextureFilterMode mip_filter = TextureFilterMode::Linear;
    TextureAddressMode address_u = TextureAddressMode::Wrap;
    TextureAddressMode address_v = TextureAddressMode::Wrap;
    TextureAddressMode address_w = TextureAddressMode::Wrap;
    float mip_lod_bias = 0.0f;
    float max_anisotropy = 16.0f;
    ComparisonFunc comparison_func = ComparisonFunc::Never;
    float border_color[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    float min_lod = -1000.0f;
    float max_lod = 1000.0f;
};

/**
 * @brief Rasterizer state description.
 */
struct RasterizerDesc {
    CullMode cull_mode = CullMode::None;
    bool front_counter_clockwise = false;
    float depth_bias = 0.0f;
    float depth_bias_clamp = 0.0f;
    float slope_scaled_depth_bias = 0.0f;
    bool depth_clip_enable = true;
    bool scissor_enable = false;
    bool antialiased_line_enable = false;
};

/**
 * @brief Shader resource type enumeration.
 *
 * Used to describe shader resources in resource signatures.
 * TextureSampler is the combined texture+sampler mode used in OpenGL.
 */
enum class ShaderResourceType : uint32_t {
    Unknown = 0,
    Texture = 1,
    Sampler = 2,
    TextureSampler = 3,
    ConstantBuffer = 4,
    StructuredBuffer = 5
};

/**
 * @brief Shader resource description for resource signature.
 *
 * Describes a single shader resource (texture, sampler, constant buffer, etc.)
 * that will be available for binding through materials.
 */
struct ShaderResource {
    const char* name = nullptr;              /// Shader variable name (e.g., "g_Texture")
    ShaderResourceType type = ShaderResourceType::TextureSampler;
    uint32_t shader_stages = 0;              /// Bitmask: 1=Vertex, 2=Pixel, 4=Geometry, etc.
    uint32_t array_size = 1;                 /// Array size for texture arrays (1 for single texture)
};

/**
 * @brief Resource signature description.
 *
 * Defines the set of shader resources that a pipeline can access.
 * Materials must bind textures to these resources by name.
 * Required for pipelines that use textures or other shader resources.
 */
struct ResourceSignatureDesc {
    const char* name = "ResourceSignature";
    const ShaderResource* resources = nullptr;
    uint32_t resource_count = 0;
};

/**
 * @brief Poko error codes for pipeline operations.
 */
enum class PipelineErrorCode : int32_t {
    Success = 0,
    Unknown = -1,
    InvalidDevice = -2,
    InvalidShaderSource = -3,
    InvalidShaderType = -4,
    InvalidEntryPoint = -5,
    InvalidPipelineDesc = -6,
    IncompatibleFormat = -7,
    ShaderCompilationFailed = -8,
    PipelineCreationFailed = -9
};

/**
 * @brief Shader description.
 *
 * Source is copied during PipelineState::initialize(); the caller may free
 * their copy after initialization completes.
 */
struct ShaderDesc {
    ShaderType type = ShaderType::Unknown;
    ShaderLanguage language = ShaderLanguage::GLSL;
    const char* source = nullptr;  // M2: GLSL 3.30 core string
    const char* entry_point = "main";
};

/**
 * @brief Graphics pipeline description.
 *
 * Combines shaders, vertex layout, render target formats, and rendering state
 * into a complete pipeline configuration. Requires a resource signature if
 * the pipeline uses textures or other shader resources.
 */
struct PipelineDesc {
    const char* name = "Pipeline";
    ShaderDesc vertex_shader;
    ShaderDesc pixel_shader;
    TextureFormat color_format = TextureFormat::RGBA8_UNORM;
    TextureFormat depth_format = TextureFormat::D32_FLOAT;
    PrimitiveTopology topology = PrimitiveTopology::TriangleList;
    BlendStateDesc blend_state;
    DepthStencilDesc depth_stencil;
    RasterizerDesc rasterizer;
    SamplerDesc sampler;                     /// M6: Default sampler state for textures
    VertexLayout vertex_layout;              /// M3: Vertex input layout
    ResourceSignatureDesc resource_signature; /// M6: Resource signature for textures/materials
};

/**
 * @brief Opaque handle to a graphics pipeline state.
 *
 * The implementation is private; consumers cannot depend on its layout.
 * Lifetime: Must not outlive the Device used to create it.
 */
class PipelineState {
public:
    PipelineState();
    ~PipelineState();

    // Non-copyable, non-movable (owns backend resources)
    PipelineState(const PipelineState&) = delete;
    PipelineState& operator=(const PipelineState&) = delete;
    PipelineState(PipelineState&&) = delete;
    PipelineState& operator=(PipelineState&&) = delete;

    /**
     * @brief Initializes the pipeline state from a device and description.
     *
     * Compiles shaders and creates the pipeline. Source strings are copied
     * internally; the caller may free them after this call returns.
     *
     * @param device The graphics device. Must remain valid for pipeline lifetime.
     * @param desc Pipeline description.
     * @return PipelineErrorCode::Success on success, error code on failure.
     */
    PipelineErrorCode initialize(Device* device, const PipelineDesc& desc);

    /**
     * @brief Returns true if the pipeline is valid.
     */
    bool is_valid() const;

    /**
     * @brief Returns the pipeline name.
     */
    const char* get_name() const;

    /**
     * @brief Returns the device that created this pipeline.
     */
    Device* get_device() const;

private:
    struct Impl;
    Impl* impl_;
    
    // Allow Context to access private impl_ for backend operations
    friend class Context;
    friend class Material;
};

} // namespace poko::render

#endif // POKO_RENDER_PIPELINE_H
