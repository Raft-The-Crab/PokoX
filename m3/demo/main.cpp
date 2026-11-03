/**
 * @file main.cpp
 * @brief M4 demo: Render a triangle with vertex and index buffers through Poko renderer API.
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

// M4: GLSL 3.30 core shaders (reads position from vertex buffer)
constexpr char kVertexShader[] = R"(#version 330 core

layout(location = 0) in vec3 aPosition;

void main()
{
    gl_Position = vec4(aPosition, 1.0);
}
)";

constexpr char kPixelShader[] = R"(#version 330 core

out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0, 0.60, 0.15, 1.0);
}
)";

int main(int argc, char** argv) {
    std::printf("PokoX M2 Demo - Triangle through Poko API\n");
    std::printf("===========================================\n\n");

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
    config.window_title = "PokoX M2 - Triangle";
    config.headless = false;
    config.max_frames = max_frames;

    poko::core::Engine engine;

    if (engine.initialize(config) != poko::core::ErrorCode::Success) {
        std::fprintf(stderr, "Failed to initialize engine\n");
        return 1;
    }

    // Get device for pipeline creation
    auto* device = engine.device();
    if (!device || !device->is_valid()) {
        std::fprintf(stderr, "Failed to get valid device\n");
        engine.shutdown();
        return 1;
    }

    std::printf("Creating pipeline...\n");

    // Define vertex data (M4: 3 vertices for triangle)
    const float triangle_vertices[] = {
        -0.6f, -0.5f, 0.5f,  // Vertex 0
         0.6f, -0.5f, 0.5f,  // Vertex 1
         0.0f,  0.6f, 0.5f   // Vertex 2
    };

    const uint32_t vertex_count = 3;
    const uint32_t vertex_size = 3 * sizeof(float);  // Float3

    // Create vertex buffer
    poko::render::VertexBuffer vertex_buffer;
    poko::render::BufferDesc vb_desc;
    vb_desc.data = triangle_vertices;
    vb_desc.size = vertex_count * vertex_size;
    vb_desc.stride = vertex_size;  // M3: Set stride explicitly
    vb_desc.name = "TriangleVertexBuffer";

    auto vb_result = vertex_buffer.initialize(device, vb_desc);
    if (vb_result != poko::render::BufferErrorCode::Success) {
        std::fprintf(stderr, "Failed to create vertex buffer: %d\n", static_cast<int>(vb_result));
        engine.shutdown();
        return 1;
    }

    std::printf("Vertex buffer created successfully\n");

    // M4: Create index buffer (3 indices for triangle)
    const uint16_t triangle_indices[] = {0, 1, 2};
    const uint32_t index_count = 3;
    const uint32_t index_size = index_count * sizeof(uint16_t);

    poko::render::IndexBuffer index_buffer;
    poko::render::IndexBufferDesc ib_desc;
    ib_desc.data = triangle_indices;
    ib_desc.size = index_size;
    ib_desc.format = poko::render::IndexFormat::UInt16;
    ib_desc.name = "TriangleIndexBuffer";

    auto ib_result = index_buffer.initialize(device, ib_desc);
    if (ib_result != poko::render::IndexBufferErrorCode::Success) {
        std::fprintf(stderr, "Failed to create index buffer: %d\n", static_cast<int>(ib_result));
        engine.shutdown();
        return 1;
    }

    std::printf("Index buffer created successfully\n");

    // Define vertex layout
    poko::render::VertexAttribute layout_attr;
    layout_attr.semantic = poko::render::VertexSemantic::Position;
    layout_attr.format = poko::render::VertexFormat::Float3;
    layout_attr.offset = 0;

    poko::render::VertexLayout vertex_layout;
    vertex_layout.attributes = &layout_attr;
    vertex_layout.attribute_count = 1;
    vertex_layout.stride = vertex_size;

    // Create pipeline state
    poko::render::PipelineState pipeline;
    poko::render::PipelineDesc pipeline_desc;
    pipeline_desc.name = "M3 Triangle Pipeline";
    pipeline_desc.vertex_shader.type = poko::render::ShaderType::Vertex;
    pipeline_desc.vertex_shader.language = poko::render::ShaderLanguage::GLSL;
    pipeline_desc.vertex_shader.source = kVertexShader;
    pipeline_desc.vertex_shader.entry_point = "main";
    pipeline_desc.pixel_shader.type = poko::render::ShaderType::Pixel;
    pipeline_desc.pixel_shader.language = poko::render::ShaderLanguage::GLSL;
    pipeline_desc.pixel_shader.source = kPixelShader;
    pipeline_desc.pixel_shader.entry_point = "main";
    pipeline_desc.color_format = poko::render::TextureFormat::RGBA8_UNORM;
    pipeline_desc.depth_format = poko::render::TextureFormat::D32_FLOAT;
    pipeline_desc.topology = poko::render::PrimitiveTopology::TriangleList;
    pipeline_desc.depth_enable = false;
    pipeline_desc.cull_mode = poko::render::CullMode::None;
    pipeline_desc.vertex_layout = vertex_layout;  // M3: Vertex input layout

    auto result = pipeline.initialize(device, pipeline_desc);
    if (result != poko::render::PipelineErrorCode::Success) {
        std::fprintf(stderr, "Failed to create pipeline: %d\n", static_cast<int>(result));
        engine.shutdown();
        return 1;
    }

    std::printf("Pipeline created successfully\n");

    // Get context for rendering
    auto* context = engine.context();
    if (!context || !context->is_valid()) {
        std::fprintf(stderr, "Failed to get valid context\n");
        engine.shutdown();
        return 1;
    }

    std::printf("\nRunning custom frame loop");
    if (max_frames > 0) {
        std::printf(" (max %llu frames)", static_cast<unsigned long long>(max_frames));
    }
    std::printf("...\n");

    // M2: Custom frame loop that uses Poko pipeline API to render triangle
    // This bypasses Engine::run() to demonstrate direct Poko rendering API usage
    bool running = true;
    uint64_t frame = 0;
    
    auto* window = engine.window();
    if (!window) {
        std::fprintf(stderr, "Failed to get window\n");
        engine.shutdown();
        return 1;
    }

    while (running) {
        // Poll events
        poko::platform::WindowEvent event;
        while (window->poll_event(&event)) {
            if (event.type == poko::platform::WindowEventType::Quit ||
                event.type == poko::platform::WindowEventType::Close) {
                running = false;
                break;
            }
        }
        
        if (!running) {
            break;
        }

        // Render frame
        poko::render::Viewport viewport;
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(window->width());
        viewport.height = static_cast<float>(window->height());
        viewport.min_depth = 0.0f;
        viewport.max_depth = 1.0f;
        
        context->set_viewport(viewport);
        
        // Set render target before clearing (required by OpenGL backend)
        auto* swap_chain = engine.swap_chain();
        if (swap_chain && swap_chain->is_valid()) {
            context->set_render_target(swap_chain);
        }
        
        poko::render::ClearColor clear_color{0.05f, 0.10f, 0.20f, 1.0f};
        context->clear_render_target(clear_color);
        
        // Bind vertex buffer, index buffer, and pipeline
        if (context->set_vertex_buffer(&vertex_buffer) && 
            context->set_index_buffer(&index_buffer) &&
            context->set_pipeline_state(&pipeline)) {
            context->draw_indexed(index_count);  // M4: Draw indexed
        }
        
        context->flush();
        
        // Present the frame
        if (swap_chain && swap_chain->is_valid()) {
            swap_chain->present();
        }
        
        ++frame;
        
        if (max_frames > 0 && frame >= max_frames) {
            break;
        }
        
        // Small delay to avoid busy-waiting
        SDL_Delay(16);
    }

    std::printf("Rendered %llu frames\n", static_cast<unsigned long long>(frame));

    engine.shutdown();

    std::printf("\nDemo finished successfully\n");
    return 0;
}
