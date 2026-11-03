/**
 * @file main.cpp
 * @brief M5 demo: Texture creation through Poko API.
 */

#define SDL_MAIN_HANDLED

#include "poko/core/engine.h"
#include "poko/render/texture.h"
#include "poko/render/pipeline.h"
#include "poko/render/context.h"
#include "poko/render/vertex_buffer.h"
#include "poko/platform/window.h"

#include <SDL.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

int main(int argc, char** argv) {
    std::printf("PokoX M5 Demo - Texture Creation\n");
    std::printf("================================\n\n");

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
    config.window_title = "PokoX M5 - Texture";
    config.headless = false;
    config.max_frames = max_frames;

    poko::core::Engine engine;

    if (engine.initialize(config) != poko::core::ErrorCode::Success) {
        std::fprintf(stderr, "Failed to initialize engine\n");
        return 1;
    }

    // Get device for texture creation
    auto* device = engine.device();
    if (!device || !device->is_valid()) {
        std::fprintf(stderr, "Failed to get valid device\n");
        engine.shutdown();
        return 1;
    }

    std::printf("Creating texture...\n");

    // Create a simple 64x64 RGBA8 texture
    const uint32_t tex_width = 64;
    const uint32_t tex_height = 64;
    const uint32_t tex_size = tex_width * tex_height * 4;  // RGBA8 = 4 bytes per pixel
    
    // Create a simple pattern (checkerboard)
    uint8_t* tex_data = new uint8_t[tex_size];
    for (uint32_t y = 0; y < tex_height; ++y) {
        for (uint32_t x = 0; x < tex_width; ++x) {
            uint32_t idx = (y * tex_width + x) * 4;
            bool checker = ((x / 8) + (y / 8)) % 2 == 0;
            tex_data[idx + 0] = checker ? 255 : 0;     // R
            tex_data[idx + 1] = checker ? 255 : 0;     // G
            tex_data[idx + 2] = checker ? 255 : 0;     // B
            tex_data[idx + 3] = 255;                  // A
        }
    }

    poko::render::Texture texture;
    poko::render::TextureDesc tex_desc;
    tex_desc.type = poko::render::TextureType::Texture2D;
    tex_desc.width = tex_width;
    tex_desc.height = tex_height;
    tex_desc.format = poko::render::TextureFormat::RGBA8_UNORM;
    tex_desc.usage = poko::render::TextureUsage::ShaderResource;
    tex_desc.data = tex_data;
    tex_desc.data_size = tex_size;
    tex_desc.name = "CheckerboardTexture";

    auto result = texture.initialize(device, tex_desc);
    delete[] tex_data;

    if (result != poko::render::TextureErrorCode::Success) {
        std::fprintf(stderr, "Failed to create texture: %d\n", static_cast<int>(result));
        engine.shutdown();
        return 1;
    }

    std::printf("Texture created successfully (64x64 RGBA8)\n");
    std::printf("Creating vertex buffer...\n");

    // Create vertex buffer for triangle
    const float triangle_vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };
    const uint32_t vertex_count = 3;
    const uint32_t vertex_size = 3 * sizeof(float);
    const uint32_t buffer_size = vertex_count * vertex_size;

    poko::render::VertexBuffer vertex_buffer;
    poko::render::BufferDesc vb_desc;
    vb_desc.data = triangle_vertices;
    vb_desc.size = buffer_size;
    vb_desc.stride = vertex_size;
    vb_desc.name = "TriangleVertexBuffer";

    auto vb_result = vertex_buffer.initialize(device, vb_desc);
    if (vb_result != poko::render::BufferErrorCode::Success) {
        std::fprintf(stderr, "Failed to create vertex buffer: %d\n", static_cast<int>(vb_result));
        engine.shutdown();
        return 1;
    }

    std::printf("Vertex buffer created successfully\n");

    // Define vertex layout
    poko::render::VertexAttribute layout_attr;
    layout_attr.semantic = poko::render::VertexSemantic::Position;
    layout_attr.format = poko::render::VertexFormat::Float3;
    layout_attr.offset = 0;

    poko::render::VertexLayout vertex_layout;
    vertex_layout.attributes = &layout_attr;
    vertex_layout.attribute_count = 1;
    vertex_layout.stride = vertex_size;

    std::printf("Creating pipeline...\n");

    // Create a simple pipeline for rendering with vertex layout
    const char* vs_source = R"(
        #version 330 core
        layout(location = 0) in vec3 inPosition;
        void main() {
            gl_Position = vec4(inPosition, 1.0);
        }
    )";

    const char* ps_source = R"(
        #version 330 core
        out vec4 fragColor;
        void main() {
            fragColor = vec4(1.0, 0.5, 0.0, 1.0);  // Orange
        }
    )";

    poko::render::ShaderDesc vs_desc;
    vs_desc.type = poko::render::ShaderType::Vertex;
    vs_desc.language = poko::render::ShaderLanguage::GLSL;
    vs_desc.source = vs_source;
    vs_desc.entry_point = "main";

    poko::render::ShaderDesc ps_desc;
    ps_desc.type = poko::render::ShaderType::Pixel;
    ps_desc.language = poko::render::ShaderLanguage::GLSL;
    ps_desc.source = ps_source;
    ps_desc.entry_point = "main";

    poko::render::PipelineState pipeline;
    poko::render::PipelineDesc pipeline_desc;
    pipeline_desc.name = "M5 Pipeline";
    pipeline_desc.vertex_shader = vs_desc;
    pipeline_desc.pixel_shader = ps_desc;
    pipeline_desc.color_format = poko::render::TextureFormat::RGBA8_UNORM;
    pipeline_desc.depth_format = poko::render::TextureFormat::D32_FLOAT;
    pipeline_desc.topology = poko::render::PrimitiveTopology::TriangleList;
    pipeline_desc.depth_stencil.depth_enable = false;
    pipeline_desc.rasterizer.cull_mode = poko::render::CullMode::None;
    pipeline_desc.vertex_layout = vertex_layout;  // Include vertex layout from start

    auto pipeline_result = pipeline.initialize(device, pipeline_desc);
    if (pipeline_result != poko::render::PipelineErrorCode::Success) {
        std::fprintf(stderr, "Pipeline creation failed: %d\n", static_cast<int>(pipeline_result));
        engine.shutdown();
        return 1;
    }

    std::printf("Pipeline created successfully\n");

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

    // Custom render loop
    bool running = true;
    uint64_t frame = 0;

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

        // Set render target (must be bound before clearing in OpenGL)
        context->set_render_target(swap_chain);

        // Clear with dark blue
        poko::render::ClearColor clear_color{0.05f, 0.10f, 0.20f, 1.0f};
        context->clear_render_target(clear_color);

        // Set pipeline
        context->set_pipeline_state(&pipeline);

        // Set vertex buffer
        context->set_vertex_buffer(&vertex_buffer);

        // Draw triangle
        context->draw(vertex_count);

        // Flush and present
        context->flush();
        swap_chain->present();

        ++frame;

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
    return 0;
}
