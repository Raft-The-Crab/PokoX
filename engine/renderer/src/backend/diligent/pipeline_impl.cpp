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
    if (resource_signature) {
        resource_signature->Release();
        resource_signature = nullptr;
    }
    if (pipeline) {
        pipeline->Release();
        pipeline = nullptr;
    }
}

bool PipelineState::Impl::initialize(Device* device, const PipelineDesc& desc) {
    if (!device || !device->is_valid()) {
        last_error = "Device is invalid";
        return false;
    }

    auto device_impl = get_device_impl(device);
    if (!device_impl || !device_impl->device) {
        last_error = "Failed to get device implementation";
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
    if (!desc.name) {
        last_error = "Pipeline must have a name";
        return false;
    }
    if (desc.vertex_shader.type != ShaderType::Vertex) {
        last_error = "Invalid vertex shader type";
        return false;
    }
    if (desc.pixel_shader.type != ShaderType::Pixel) {
        last_error = "Invalid pixel shader type";
        return false;
    }
    if (vertex_shader_source.empty() || pixel_shader_source.empty()) {
        last_error = "Shader source is empty";
        return false;
    }
    if (!desc.vertex_shader.entry_point) {
        last_error = "Vertex shader entry point is null";
        return false;
    }
    if (!desc.pixel_shader.entry_point) {
        last_error = "Pixel shader entry point is null";
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
        last_error = "Vertex shader compilation failed";
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
        last_error = "Pixel shader compilation failed";
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

            // M9/M13: BufferSlot selects the vertex stream; InputIndex is the
            // shader attribute location. They are distinct concepts — several
            // attributes may share one buffer slot while needing unique
            // locations. An explicit input_index overrides the ordinal default.
            element.InputIndex = (attr.input_index == VertexAttribute::kAutoAttributeIndex)
                                     ? i
                                     : attr.input_index;

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
                case VertexSemantic::Joints:
                    semantic_name = "JOINTS";
                    break;
                case VertexSemantic::Weights:
                    semantic_name = "WEIGHTS";
                    break;
                default:
                    semantic_name = "ATTRIBUTE";
                    break;
            }

            element.HLSLSemantic = semantic_name;
            element.ValueType = Diligent::VT_FLOAT32;  // Default, will override below
            element.RelativeOffset = attr.offset;
            element.BufferSlot = attr.input_slot;  // M9: Buffer slot matches input slot
            element.NumComponents = 1;  // Default, will override below
            element.IsNormalized = false;

            // M9: Set frequency based on instance_data_step_rate
            if (attr.instance_data_step_rate > 0) {
                element.Frequency = Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE;
                element.InstanceDataStepRate = attr.instance_data_step_rate;
            } else {
                element.Frequency = Diligent::INPUT_ELEMENT_FREQUENCY_PER_VERTEX;
            }
            
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

    // Create resource signature if specified
    Diligent::IPipelineResourceSignature* resource_signature = nullptr;
    if (desc.resource_signature.resources && desc.resource_signature.resource_count > 0) {
        // Validate resource count limit
        if (desc.resource_signature.resource_count > 32) {
            last_error = "Resource signature has too many resources (> 32)";
            vs->Release();
            ps->Release();
            return false;
        }

        Diligent::PipelineResourceDesc resources[32];
        uint32_t res_count = 0;

        for (uint32_t i = 0; i < desc.resource_signature.resource_count && i < 32; ++i) {
            const auto& res = desc.resource_signature.resources[i];
            if (!res.name) {
                continue;
            }

            // Validate array size
            if (res.array_size == 0 || res.array_size > 256) {
                last_error = "Invalid array size (must be 1-256)";
                vs->Release();
                ps->Release();
                return false;
            }

            Diligent::SHADER_TYPE shader_type = Diligent::SHADER_TYPE_PIXEL;
            if (res.shader_stages & 1) shader_type |= Diligent::SHADER_TYPE_VERTEX;
            if (res.shader_stages & 2) shader_type |= Diligent::SHADER_TYPE_PIXEL;
            if (res.shader_stages & 4) shader_type |= Diligent::SHADER_TYPE_GEOMETRY;

            Diligent::SHADER_RESOURCE_TYPE res_type = Diligent::SHADER_RESOURCE_TYPE_TEXTURE_SRV;
            Diligent::SHADER_RESOURCE_VARIABLE_TYPE var_type = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE;
            Diligent::PIPELINE_RESOURCE_FLAGS flags = Diligent::PIPELINE_RESOURCE_FLAG_NONE;

            // M13: Map Poko resource types onto Diligent resource types so
            // constant buffers and storage buffers can be declared in the
            // resource signature (required for skinned-mesh bone palettes).
            switch (res.type) {
                case ShaderResourceType::Texture:
                    res_type = Diligent::SHADER_RESOURCE_TYPE_TEXTURE_SRV;
                    break;
                case ShaderResourceType::Sampler:
                    res_type = Diligent::SHADER_RESOURCE_TYPE_SAMPLER;
                    break;
                case ShaderResourceType::ConstantBuffer:
                    res_type = Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER;
                    break;
                case ShaderResourceType::StructuredBuffer:
                    res_type = Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV;
                    break;
                case ShaderResourceType::TextureSampler:
                default:
                    res_type = Diligent::SHADER_RESOURCE_TYPE_TEXTURE_SRV;
                    break;
            }

            // For combined texture-sampler in OpenGL
            if (res.type == ShaderResourceType::TextureSampler) {
                flags = Diligent::PIPELINE_RESOURCE_FLAG_COMBINED_SAMPLER;
            }

            resources[res_count].Name = res.name;
            resources[res_count].ShaderStages = shader_type;
            resources[res_count].ResourceType = res_type;
            resources[res_count].VarType = var_type;
            resources[res_count].ArraySize = res.array_size;
            resources[res_count].Flags = flags;
            ++res_count;
        }

        if (res_count == 0) {
            last_error = "Resource signature has no valid resources";
            vs->Release();
            ps->Release();
            return false;
        }

        Diligent::PipelineResourceSignatureDesc signature_desc;
        signature_desc.Name = desc.resource_signature.name ? desc.resource_signature.name : "Poko Resource Signature";
        signature_desc.Resources = resources;
        signature_desc.NumResources = res_count;
        signature_desc.UseCombinedTextureSamplers = true;  // Required for OpenGL

        device_impl->device->CreatePipelineResourceSignature(signature_desc, &resource_signature);
        if (!resource_signature) {
            last_error = "Failed to create resource signature";
            vs->Release();
            ps->Release();
            return false;
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
    
    // Use explicit resource signature if provided
    if (resource_signature) {
        pso_ci.ResourceSignaturesCount = 1;
        pso_ci.ppResourceSignatures = &resource_signature;
    }
    
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
        last_error = "Pipeline state creation failed";
        return false;
    }

    // Create shader resource binding from pipeline's resource signature
    // If we used an explicit resource signature, use that instead of the pipeline's implicit one
    if (resource_signature) {
        resource_signature->CreateShaderResourceBinding(&srb, true);
        // Keep the resource signature reference for later use by materials
        this->resource_signature = resource_signature;
    } else {
        pipeline->CreateShaderResourceBinding(&srb, true);
    }

    if (!srb) {
        last_error = "Shader resource binding creation failed";
        pipeline->Release();
        pipeline = nullptr;
        if (resource_signature) {
            resource_signature->Release();
        }
        return false;
    }

    return true;
}

} // namespace poko::render
