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
        std::fprintf(stderr, "Vertex shader compilation failed\n");
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
        std::fprintf(stderr, "Pixel shader compilation failed\n");
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
        default:
            color_format = Diligent::TEX_FORMAT_RGBA8_UNORM;
            break;
    }

    switch (desc.depth_format) {
        case TextureFormat::D32_FLOAT:
            depth_format = Diligent::TEX_FORMAT_D32_FLOAT;
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
    switch (desc.cull_mode) {
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
    pso_ci.GraphicsPipeline.DepthStencilDesc.DepthEnable = desc.depth_enable ? Diligent::True : Diligent::False;
    pso_ci.GraphicsPipeline.RasterizerDesc.CullMode = cull_mode;
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
        std::fprintf(stderr, "Pipeline state creation failed\n");
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
