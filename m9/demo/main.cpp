/**
 * @file main.cpp
 * @brief M9 Demo: Advanced Drawing
 *
 * Demonstrates instanced rendering and indirect drawing for efficient
 * batched geometry and GPU-driven draw calls.
 */

#define SDL_MAIN_HANDLED

#include "poko/core/engine.h"
#include "poko/render/pipeline.h"
#include "poko/render/context.h"
#include "poko/render/swapchain.h"
#include "poko/render/vertex_buffer.h"
#include "poko/render/index_buffer.h"
#include "poko/platform/window.h"

#include <SDL.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

// Simple vertex shader for instanced rendering
constexpr char kVertexShader[] = R"(#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aInstancePos;  // Instance position
layout(location = 2) in vec3 aInstanceColor;  // Instance color

out vec3 vColor;

void main()
{
    vec3 worldPos = aPosition + aInstancePos;
    gl_Position = vec4(worldPos, 1.0);
    vColor = aInstanceColor;
}
)";

// Simple pixel shader
constexpr char kPixelShader[] = R"(#version 330 core

in vec3 vColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(vColor, 1.0);
}
)";

// Indirect draw arguments structure
struct DrawIndirectArgs {
    uint32_t vertex_count;
    uint32_t instance_count;
    uint32_t first_vertex;
    uint32_t first_instance;
};

int main(int argc, char** argv) {
    std::printf("PokoX M9 Demo - Advanced Drawing\n");
    std::printf("===================================\n\n");

    uint64_t max_frames = 0;

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--frames") == 0 && i + 1 < argc) {
            max_frames = static_cast<uint64_t>(std::atoi(argv[++i]));
        }
    }

    // Initialize engine
    poko::core::EngineConfig config;
    config.window_width = 1280;
    config.window_height = 720;
    config.window_title = "PokoX M9 - Advanced Drawing";
    config.headless = false;
    config.max_frames = max_frames;

    poko::core::Engine engine;

    if (engine.initialize(config) != poko::core::ErrorCode::Success) {
        std::fprintf(stderr, "Failed to initialize engine\n");
        return 1;
    }

    // Get device for resource creation
    auto* device = engine.device();
    if (!device || !device->is_valid()) {
        std::fprintf(stderr, "Failed to get valid device\n");
        engine.shutdown();
        return 1;
    }

    std::printf("Creating quad geometry...\n");

    // Create a simple quad (4 vertices)
    const float quad_vertices[] = {
        -0.1f, -0.1f, 0.0f,  // Bottom-left
         0.1f, -0.1f, 0.0f,  // Bottom-right
         0.1f,  0.1f, 0.0f,  // Top-right
        -0.1f,  0.1f, 0.0f   // Top-left
    };
    const uint32_t vertex_count = 4;
    const uint32_t vertex_size = 3 * sizeof(float);
    const uint32_t quad_size = vertex_count * vertex_size;

    poko::render::VertexBuffer quad_vb;
    poko::render::BufferDesc quad_desc;
    quad_desc.data = quad_vertices;
    quad_desc.size = quad_size;
    quad_desc.stride = vertex_size;
    quad_desc.name = "QuadVertexBuffer";

    auto quad_result = quad_vb.initialize(device, quad_desc);
    if (quad_result != poko::render::BufferErrorCode::Success) {
        std::fprintf(stderr, "Failed to create quad vertex buffer: %d\n", static_cast<int>(quad_result));
        engine.shutdown();
        return 1;
    }

    std::printf("Quad created successfully (4 vertices)\n");

    // Create instance data (positions and colors for 100 quads)
    const uint32_t instance_count = 100;
    std::vector<float> instance_data;
    instance_data.reserve(instance_count * 6);  // 3 floats position + 3 floats color per instance

    // Generate instances in a grid pattern
    uint32_t grid_size = 10;
    float spacing = 0.2f;
    float offset = -((grid_size - 1) * spacing) / 2.0f;

    for (uint32_t y = 0; y < grid_size; ++y) {
        for (uint32_t x = 0; x < grid_size; ++x) {
            // Position
            instance_data.push_back(offset + x * spacing);
            instance_data.push_back(offset + y * spacing);
            instance_data.push_back(0.0f);

            // Color (gradient from red to blue)
            float r = static_cast<float>(x) / (grid_size - 1);
            float g = static_cast<float>(y) / (grid_size - 1);
            float b = 1.0f - r;
            instance_data.push_back(r);
            instance_data.push_back(g);
            instance_data.push_back(b);
        }
    }

    const uint32_t instance_stride = 6 * sizeof(float);
    const uint32_t instance_size = instance_count * instance_stride;

    poko::render::VertexBuffer instance_vb;
    poko::render::BufferDesc instance_desc;
    instance_desc.data = instance_data.data();
    instance_desc.size = instance_size;
    instance_desc.stride = instance_stride;
    instance_desc.name = "InstanceVertexBuffer";

    auto instance_result = instance_vb.initialize(device, instance_desc);
    if (instance_result != poko::render::BufferErrorCode::Success) {
        std::fprintf(stderr, "Failed to create instance buffer: %d\n", static_cast<int>(instance_result));
        engine.shutdown();
        return 1;
    }

    std::printf("Instance data created successfully (%u instances)\n", instance_count);

    // Define vertex layout with instance attributes
    std::printf("Creating vertex layout with instance attributes...\n");

    poko::render::VertexAttribute attributes[3];
    // Slot 0: Vertex position (per-vertex)
    attributes[0].semantic = poko::render::VertexSemantic::Position;
    attributes[0].format = poko::render::VertexFormat::Float3;
    attributes[0].offset = 0;
    attributes[0].input_slot = 0;
    attributes[0].instance_data_step_rate = 0;

    // Slot 1: Instance position (per-instance)
    attributes[1].semantic = poko::render::VertexSemantic::Position;
    attributes[1].format = poko::render::VertexFormat::Float3;
    attributes[1].offset = 0;
    attributes[1].input_slot = 1;
    attributes[1].instance_data_step_rate = 1;

    // Slot 1: Instance color (per-instance)
    attributes[2].semantic = poko::render::VertexSemantic::Color;
    attributes[2].format = poko::render::VertexFormat::Float3;
    attributes[2].offset = 3 * sizeof(float);
    attributes[2].input_slot = 1;
    attributes[2].instance_data_step_rate = 1;

    poko::render::VertexLayout vertex_layout;
    vertex_layout.attributes = attributes;
    vertex_layout.attribute_count = 3;
    vertex_layout.stride = 0;  // Not used when multiple slots

    std::printf("Vertex layout created successfully\n");

    // Create pipeline for instanced rendering
    std::printf("Creating pipeline...\n");

    poko::render::ShaderDesc vs_desc;
    vs_desc.type = poko::render::ShaderType::Vertex;
    vs_desc.language = poko::render::ShaderLanguage::GLSL;
    vs_desc.source = kVertexShader;
    vs_desc.entry_point = "main";

    poko::render::ShaderDesc ps_desc;
    ps_desc.type = poko::render::ShaderType::Pixel;
    ps_desc.language = poko::render::ShaderLanguage::GLSL;
    ps_desc.source = kPixelShader;
    ps_desc.entry_point = "main";

    poko::render::PipelineState pipeline;
    poko::render::PipelineDesc pipeline_desc;
    pipeline_desc.name = "M9 InstancedPipeline";
    pipeline_desc.vertex_shader = vs_desc;
    pipeline_desc.pixel_shader = ps_desc;
    pipeline_desc.color_format = poko::render::TextureFormat::RGBA8_UNORM;
    pipeline_desc.depth_format = poko::render::TextureFormat::D32_FLOAT;
    pipeline_desc.topology = poko::render::PrimitiveTopology::TriangleList;
    pipeline_desc.depth_stencil.depth_enable = false;
    pipeline_desc.rasterizer.cull_mode = poko::render::CullMode::None;
    pipeline_desc.vertex_layout = vertex_layout;

    auto pipeline_result = pipeline.initialize(device, pipeline_desc);
    if (pipeline_result != poko::render::PipelineErrorCode::Success) {
        std::fprintf(stderr, "Pipeline creation failed: %d\n", static_cast<int>(pipeline_result));
        engine.shutdown();
        return 1;
    }

    std::printf("Pipeline created successfully\n");

    // Create indirect draw arguments buffer
    std::printf("Creating indirect draw arguments buffer...\n");

    DrawIndirectArgs indirect_args;
    indirect_args.vertex_count = vertex_count;
    indirect_args.instance_count = instance_count;
    indirect_args.first_vertex = 0;
    indirect_args.first_instance = 0;

    poko::render::VertexBuffer indirect_buffer;
    poko::render::BufferDesc indirect_desc;
    indirect_desc.data = &indirect_args;
    indirect_desc.size = sizeof(DrawIndirectArgs);
    indirect_desc.stride = sizeof(DrawIndirectArgs);
    indirect_desc.name = "IndirectArgsBuffer";

    auto indirect_result = indirect_buffer.initialize(device, indirect_desc);
    if (indirect_result != poko::render::BufferErrorCode::Success) {
        std::fprintf(stderr, "Failed to create indirect buffer: %d\n", static_cast<int>(indirect_result));
        engine.shutdown();
        return 1;
    }

    std::printf("Indirect buffer created successfully\n");

    // Get context and swap chain for rendering
    auto* context = engine.context();
    auto* swap_chain = engine.swap_chain();
    auto* window = engine.window();

    if (!context || !context->is_valid()) {
        std::fprintf(stderr, "Failed to get valid context\n");
        engine.shutdown();
        return 1;
    }
    if (!swap_chain || !swap_chain->is_valid()) {
        std::fprintf(stderr, "Failed to get valid swap chain\n");
        engine.shutdown();
        return 1;
    }
    if (!window) {
        std::fprintf(stderr, "Failed to get valid window\n");
        engine.shutdown();
        return 1;
    }

    std::printf("Starting render loop...\n");
    std::printf("Rendering %u instances of quad using instanced drawing\n", instance_count);

    // Custom render loop
    bool running = true;
    uint64_t frame = 0;
    bool use_indirect = false;  // Toggle between direct and indirect drawing

    while (running) {
        // Poll window events
        poko::platform::WindowEvent event;
        while (window->poll_event(&event)) {
            if (event.type == poko::platform::WindowEventType::Quit ||
                event.type == poko::platform::WindowEventType::Close) {
                running = false;
            }
        }

        if (!running) {
            break;
        }

        // Render frame
        uint32_t width = window->width();
        uint32_t height = window->height();

        poko::render::Viewport viewport;
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = static_cast<float>(width);
        viewport.height = static_cast<float>(height);
        viewport.min_depth = 0.0f;
        viewport.max_depth = 1.0f;
        context->set_viewport(viewport);

        // Set render target
        context->set_render_target(swap_chain);

        // Clear with dark background
        poko::render::ClearColor clear_color{0.1f, 0.1f, 0.15f, 1.0f};
        context->clear_render_target(clear_color);

        // Set pipeline
        context->set_pipeline_state(&pipeline);

        // Bind vertex buffers (slot 0 for geometry, slot 1 for instance data)
        context->set_vertex_buffer(&quad_vb, 0);
        context->set_vertex_buffer(&instance_vb, 1);

        // Draw using instanced rendering
        if (use_indirect) {
            // Toggle indirect drawing every 60 frames
            context->draw_indirect(&indirect_buffer, 0);
        } else {
            context->draw_instanced(vertex_count, instance_count);
        }

        // Flush and present
        context->flush();
        swap_chain->present();

        ++frame;

        // Toggle indirect drawing every 60 frames
        if (frame % 60 == 0) {
            use_indirect = !use_indirect;
        }

        // Frame limit for automated testing
        if (max_frames > 0 && frame >= max_frames) {
            running = false;
        }

        // Simple frame limiter (~60 FPS)
        SDL_Delay(16);
    }

    std::printf("Rendered %llu frames\n", static_cast<unsigned long long>(frame));

    engine.shutdown();

    std::printf("\nDemo finished successfully\n");
    std::printf("Instanced rendering: %u instances drawn with single draw call\n", instance_count);
    std::printf("Indirect drawing: GPU-driven draw calls (toggled every 60 frames)\n");
    return 0;
}
