/**
 * @file pipeline_impl.cpp
 * @brief Diligent Core implementation of PipelineState.
 */

#include "backend/diligent/internal/pipeline_impl.h"
#include "backend/diligent/internal/device_impl.h"

#include <cstdio>
#include <vector>

namespace poko::render {

PipelineState::Impl::~Impl() {
    if (srb) {
        srb->Release();
        srb = nullptr;
    }
    if (pipeline) {
        pipeline->Release();
        pipeline = nullptr;
    }
}

bool PipelineState::Impl::initialize(Device* device, const PipelineDesc& desc) {
    if (!device || !device->is_valid()) {
        return false;
    }

    auto device_impl = get_device_impl(device);
    if (!device_impl || !device_impl->device) {
        return false;
    }

    this->device = device;
    this->desc = desc;

    // Copy shader sources for lifetime management
    if (desc.vertex_shader.source) {
        vertex_shader_source = desc.vertex_shader.source;
    }
    if (desc.pixel_shader.source) {
        pixel_shader_source = desc.pixel_shader.source;
    }

    // Validate inputs
    if (desc.vertex_shader.type != ShaderType::Vertex) {
        std::fprintf(stderr, "Invalid vertex shader type\n");
        return false;
    }
    if (desc.pixel_shader.type != ShaderType::Pixel) {
        std::fprintf(stderr, "Invalid pixel shader type\n");
        return false;
    }
    if (vertex_shader_source.empty() || pixel_shader_source.empty()) {
        std::fprintf(stderr, "Shader source is empty\n");
        return false;
    }
    if (!desc.name) {
        std::fprintf(stderr, "Pipeline must have a name\n");
        return false;
    }

    // Create vertex shader
    Diligent::ShaderCreateInfo shader_ci;
    shader_ci.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_GLSL_VERBATIM;
    shader_ci.EntryPoint = desc.vertex_shader.entry_point ? desc.vertex_shader.entry_point : "main";

    Diligent::IShader* vs = nullptr;
    shader_ci.Desc.Name = "Poko vertex shader";
    shader_ci.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
    shader_ci.Source = vertex_shader_source.c_str();
    device_impl->device->CreateShader(shader_ci, &vs);

    if (!vs) {
        std::fprintf(stderr, "Vertex shader compilation failed for pipeline '%s'\n", desc.name ? desc.name : "(unnamed)");
        return false;
    }

    // Create pixel shader
    Diligent::IShader* ps = nullptr;
    shader_ci.Desc.Name = "Poko pixel shader";
    shader_ci.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
    shader_ci.Source = pixel_shader_source.c_str();
    shader_ci.EntryPoint = desc.pixel_shader.entry_point ? desc.pixel_shader.entry_point : "main";
    device_impl->device->CreateShader(shader_ci, &ps);

    if (!ps) {
        std::fprintf(stderr, "Pixel shader compilation failed for pipeline '%s'\n", desc.name ? desc.name : "(unnamed)");
        vs->Release();
        return false;
    }

    // Map Poko formats to Diligent formats
    Diligent::TEXTURE_FORMAT color_format = Diligent::TEX_FORMAT_RGBA8_UNORM;
    Diligent::TEXTURE_FORMAT depth_format = Diligent::TEX_FORMAT_D32_FLOAT;

    switch (desc.color_format) {
        case TextureFormat::RGBA8_UNORM:
            color_format = Diligent::TEX_FORMAT_RGBA8_UNORM;
            break;
        case TextureFormat::RGBA8_SRGB:
            color_format = Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB;
            break;
        case TextureFormat::RG8_UNORM:
            color_format = Diligent::TEX_FORMAT_RG8_UNORM;
            break;
        case TextureFormat::R8_UNORM:
            color_format = Diligent::TEX_FORMAT_R8_UNORM;
            break;
        case TextureFormat::RGBA16_FLOAT:
            color_format = Diligent::TEX_FORMAT_RGBA16_FLOAT;
            break;
        case TextureFormat::RGBA32_FLOAT:
            color_format = Diligent::TEX_FORMAT_RGBA32_FLOAT;
            break;
        default:
            color_format = Diligent::TEX_FORMAT_RGBA8_UNORM;
            break;
    }

    switch (desc.depth_format) {
        case TextureFormat::D16_UNORM:
            depth_format = Diligent::TEX_FORMAT_D16_UNORM;
            break;
        case TextureFormat::D24_UNORM_S8_UINT:
            depth_format = Diligent::TEX_FORMAT_D24_UNORM_S8_UINT;
            break;
        case TextureFormat::D32_FLOAT:
            depth_format = Diligent::TEX_FORMAT_D32_FLOAT;
            break;
        case TextureFormat::D32_FLOAT_S8_UINT:
            depth_format = Diligent::TEX_FORMAT_D32_FLOAT_S8X24_UINT;
            break;
        default:
            depth_format = Diligent::TEX_FORMAT_UNKNOWN;
            break;
    }

    // Map Poko topology to Diligent topology
    Diligent::PRIMITIVE_TOPOLOGY topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    switch (desc.topology) {
        case PrimitiveTopology::TriangleList:
            topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            break;
        case PrimitiveTopology::TriangleStrip:
            topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            break;
        case PrimitiveTopology::LineList:
            topology = Diligent::PRIMITIVE_TOPOLOGY_LINE_LIST;
            break;
        case PrimitiveTopology::PointList:
            topology = Diligent::PRIMITIVE_TOPOLOGY_POINT_LIST;
            break;
        default:
            topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            break;
    }

    // Map Poko cull mode to Diligent cull mode
    Diligent::CULL_MODE cull_mode = Diligent::CULL_MODE_NONE;
    switch (desc.rasterizer.cull_mode) {
        case CullMode::None:
            cull_mode = Diligent::CULL_MODE_NONE;
            break;
        case CullMode::Front:
            cull_mode = Diligent::CULL_MODE_FRONT;
            break;
        case CullMode::Back:
            cull_mode = Diligent::CULL_MODE_BACK;
            break;
        default:
            cull_mode = Diligent::CULL_MODE_NONE;
            break;
    }

    // Map Poko blend factor to Diligent blend factor
    auto map_blend_factor = [](BlendFactor factor) -> Diligent::BLEND_FACTOR {
        switch (factor) {
            case BlendFactor::Zero: return Diligent::BLEND_FACTOR_ZERO;
            case BlendFactor::One: return Diligent::BLEND_FACTOR_ONE;
            case BlendFactor::SrcColor: return Diligent::BLEND_FACTOR_SRC_COLOR;
            case BlendFactor::InvSrcColor: return Diligent::BLEND_FACTOR_INV_SRC_COLOR;
            case BlendFactor::SrcAlpha: return Diligent::BLEND_FACTOR_SRC_ALPHA;
            case BlendFactor::InvSrcAlpha: return Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
            case BlendFactor::DestColor: return Diligent::BLEND_FACTOR_DEST_COLOR;
            case BlendFactor::InvDestColor: return Diligent::BLEND_FACTOR_INV_DEST_COLOR;
            case BlendFactor::DestAlpha: return Diligent::BLEND_FACTOR_DEST_ALPHA;
            case BlendFactor::InvDestAlpha: return Diligent::BLEND_FACTOR_INV_DEST_ALPHA;
            case BlendFactor::SrcAlphaSat: return Diligent::BLEND_FACTOR_SRC_ALPHA_SAT;
            default: return Diligent::BLEND_FACTOR_ONE;
        }
    };

    // Map Poko blend operation to Diligent blend operation
    auto map_blend_op = [](BlendOp op) -> Diligent::BLEND_OPERATION {
        switch (op) {
            case BlendOp::Add: return Diligent::BLEND_OPERATION_ADD;
            case BlendOp::Subtract: return Diligent::BLEND_OPERATION_SUBTRACT;
            case BlendOp::ReverseSubtract: return Diligent::BLEND_OPERATION_REV_SUBTRACT;
            case BlendOp::Min: return Diligent::BLEND_OPERATION_MIN;
            case BlendOp::Max: return Diligent::BLEND_OPERATION_MAX;
            default: return Diligent::BLEND_OPERATION_ADD;
        }
    };

    // Map Poko comparison function to Diligent comparison function
    auto map_comparison_func = [](ComparisonFunc func) -> Diligent::COMPARISON_FUNCTION {
        switch (func) {
            case ComparisonFunc::Never: return Diligent::COMPARISON_FUNC_NEVER;
            case ComparisonFunc::Less: return Diligent::COMPARISON_FUNC_LESS;
            case ComparisonFunc::Equal: return Diligent::COMPARISON_FUNC_EQUAL;
            case ComparisonFunc::LessEqual: return Diligent::COMPARISON_FUNC_LESS_EQUAL;
            case ComparisonFunc::Greater: return Diligent::COMPARISON_FUNC_GREATER;
            case ComparisonFunc::NotEqual: return Diligent::COMPARISON_FUNC_NOT_EQUAL;
            case ComparisonFunc::GreaterEqual: return Diligent::COMPARISON_FUNC_GREATER_EQUAL;
            case ComparisonFunc::Always: return Diligent::COMPARISON_FUNC_ALWAYS;
            default: return Diligent::COMPARISON_FUNC_LESS;
        }
    };

    // Build input layout from vertex layout (M3)
    std::vector<Diligent::LayoutElement> layout_elements;
    if (desc.vertex_layout.attributes && desc.vertex_layout.attribute_count > 0) {
        for (uint32_t i = 0; i < desc.vertex_layout.attribute_count; ++i) {
            const auto& attr = desc.vertex_layout.attributes[i];
            
            Diligent::LayoutElement element;
            element.InputIndex = 0;  // M3: Single input slot
            
            // Map semantic to slot (M3 simplified)
            const char* semantic_name = "ATTRIBUTE";
            switch (attr.semantic) {
                case VertexSemantic::Position:
                    semantic_name = "POSITION";
                    break;
                case VertexSemantic::Normal:
                    semantic_name = "NORMAL";
                    break;
                case VertexSemantic::TexCoord:
                    semantic_name = "TEXCOORD";
                    break;
                case VertexSemantic::Color:
                    semantic_name = "COLOR";
                    break;
                default:
                    semantic_name = "ATTRIBUTE";
                    break;
            }
            
            element.HLSLSemantic = semantic_name;
            element.InputIndex = 0;
            element.ValueType = Diligent::VT_FLOAT32;  // Default, will override below
            element.RelativeOffset = attr.offset;
            element.BufferSlot = 0;
            element.NumComponents = 1;  // Default, will override below
            element.IsNormalized = false;
            element.Frequency = Diligent::INPUT_ELEMENT_FREQUENCY_PER_VERTEX;
            
            // Map format
            switch (attr.format) {
                case VertexFormat::Float:
                    element.ValueType = Diligent::VT_FLOAT32;
                    element.NumComponents = 1;
                    break;
                case VertexFormat::Float2:
                    element.ValueType = Diligent::VT_FLOAT32;
                    element.NumComponents = 2;
                    break;
                case VertexFormat::Float3:
                    element.ValueType = Diligent::VT_FLOAT32;
                    element.NumComponents = 3;
                    break;
                case VertexFormat::Float4:
                    element.ValueType = Diligent::VT_FLOAT32;
                    element.NumComponents = 4;
                    break;
                case VertexFormat::Int:
                    element.ValueType = Diligent::VT_INT32;
                    element.NumComponents = 1;
                    break;
                case VertexFormat::Int2:
                    element.ValueType = Diligent::VT_INT32;
                    element.NumComponents = 2;
                    break;
                case VertexFormat::Int3:
                    element.ValueType = Diligent::VT_INT32;
                    element.NumComponents = 3;
                    break;
                case VertexFormat::Int4:
                    element.ValueType = Diligent::VT_INT32;
                    element.NumComponents = 4;
                    break;
                case VertexFormat::UInt:
                    element.ValueType = Diligent::VT_UINT32;
                    element.NumComponents = 1;
                    break;
                case VertexFormat::UInt2:
                    element.ValueType = Diligent::VT_UINT32;
                    element.NumComponents = 2;
                    break;
                case VertexFormat::UInt3:
                    element.ValueType = Diligent::VT_UINT32;
                    element.NumComponents = 3;
                    break;
                case VertexFormat::UInt4:
                    element.ValueType = Diligent::VT_UINT32;
                    element.NumComponents = 4;
                    break;
                default:
                    element.ValueType = Diligent::VT_FLOAT32;
                    element.NumComponents = 1;
                    break;
            }
            
            layout_elements.push_back(element);
        }
    }

    // Create pipeline state
    Diligent::GraphicsPipelineStateCreateInfo pso_ci;
    pso_ci.PSODesc.Name = desc.name ? desc.name : "Poko Pipeline";
    pso_ci.PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;
    pso_ci.GraphicsPipeline.NumRenderTargets = 1;
    pso_ci.GraphicsPipeline.RTVFormats[0] = color_format;
    pso_ci.GraphicsPipeline.DSVFormat = depth_format;
    pso_ci.GraphicsPipeline.PrimitiveTopology = topology;
    
    // Configure blend state
    pso_ci.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendEnable = desc.blend_state.blend_enable ? Diligent::True : Diligent::False;
    pso_ci.GraphicsPipeline.BlendDesc.RenderTargets[0].SrcBlend = map_blend_factor(desc.blend_state.src_blend);
    pso_ci.GraphicsPipeline.BlendDesc.RenderTargets[0].DestBlend = map_blend_factor(desc.blend_state.dest_blend);
    pso_ci.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendOp = map_blend_op(desc.blend_state.blend_op);
    pso_ci.GraphicsPipeline.BlendDesc.RenderTargets[0].SrcBlendAlpha = map_blend_factor(desc.blend_state.src_blend_alpha);
    pso_ci.GraphicsPipeline.BlendDesc.RenderTargets[0].DestBlendAlpha = map_blend_factor(desc.blend_state.dest_blend_alpha);
    pso_ci.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendOpAlpha = map_blend_op(desc.blend_state.blend_op_alpha);
    pso_ci.GraphicsPipeline.BlendDesc.RenderTargets[0].RenderTargetWriteMask = static_cast<Diligent::COLOR_MASK>(desc.blend_state.render_target_write_mask);
    
    // Configure depth stencil state
    pso_ci.GraphicsPipeline.DepthStencilDesc.DepthEnable = desc.depth_stencil.depth_enable ? Diligent::True : Diligent::False;
    pso_ci.GraphicsPipeline.DepthStencilDesc.DepthWriteEnable = desc.depth_stencil.depth_write_enable ? Diligent::True : Diligent::False;
    pso_ci.GraphicsPipeline.DepthStencilDesc.DepthFunc = map_comparison_func(desc.depth_stencil.depth_func);
    pso_ci.GraphicsPipeline.DepthStencilDesc.StencilEnable = desc.depth_stencil.stencil_enable ? Diligent::True : Diligent::False;
    pso_ci.GraphicsPipeline.DepthStencilDesc.StencilReadMask = desc.depth_stencil.stencil_read_mask;
    pso_ci.GraphicsPipeline.DepthStencilDesc.StencilWriteMask = desc.depth_stencil.stencil_write_mask;
    
    // Configure rasterizer state
    pso_ci.GraphicsPipeline.RasterizerDesc.CullMode = cull_mode;
    pso_ci.GraphicsPipeline.RasterizerDesc.FrontCounterClockwise = desc.rasterizer.front_counter_clockwise ? Diligent::True : Diligent::False;
    pso_ci.GraphicsPipeline.RasterizerDesc.DepthBias = desc.rasterizer.depth_bias;
    pso_ci.GraphicsPipeline.RasterizerDesc.DepthBiasClamp = desc.rasterizer.depth_bias_clamp;
    pso_ci.GraphicsPipeline.RasterizerDesc.SlopeScaledDepthBias = desc.rasterizer.slope_scaled_depth_bias;
    pso_ci.GraphicsPipeline.RasterizerDesc.DepthClipEnable = desc.rasterizer.depth_clip_enable ? Diligent::True : Diligent::False;
    pso_ci.GraphicsPipeline.RasterizerDesc.ScissorEnable = desc.rasterizer.scissor_enable ? Diligent::True : Diligent::False;
    pso_ci.GraphicsPipeline.RasterizerDesc.AntialiasedLineEnable = desc.rasterizer.antialiased_line_enable ? Diligent::True : Diligent::False;
    
    pso_ci.pVS = vs;
    pso_ci.pPS = ps;
    
    // Set input layout
    if (!layout_elements.empty()) {
        pso_ci.GraphicsPipeline.InputLayout.LayoutElements = layout_elements.data();
        pso_ci.GraphicsPipeline.InputLayout.NumElements = static_cast<uint32_t>(layout_elements.size());
    }

    device_impl->device->CreateGraphicsPipelineState(pso_ci, &pipeline);

    vs->Release();
    ps->Release();

    if (!pipeline) {
        std::fprintf(stderr, "Pipeline state creation failed for '%s'\n", desc.name ? desc.name : "(unnamed)");
        return false;
    }

    // Create shader resource binding (required by Diligent even when no resources used)
    pipeline->CreateShaderResourceBinding(&srb, true);
    if (!srb) {
        std::fprintf(stderr, "Shader resource binding creation failed\n");
        pipeline->Release();
        pipeline = nullptr;
        return false;
    }

    return true;
}

} // namespace poko::render
