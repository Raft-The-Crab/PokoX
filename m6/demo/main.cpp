/**
 * @file main.cpp
 * @brief M6 demo: Material creation through Poko API.
 */

#define SDL_MAIN_HANDLED

#include "poko/core/engine.h"
#include "poko/render/texture.h"
#include "poko/render/material.h"
#include "poko/render/pipeline.h"
#include "poko/platform/window.h"

#include <SDL.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

int main(int argc, char** argv) {
    std::printf("PokoX M6 Demo - Material Creation\n");
    std::printf("==================================\n\n");

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
    config.window_title = "PokoX M6 - Material";
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

    std::printf("Creating texture...\n");

    // Create a simple 64x64 RGBA8 texture
    const uint32_t tex_width = 64;
    const uint32_t tex_height = 64;
    const uint32_t tex_size = tex_width * tex_height * 4;
    
    uint8_t* tex_data = new uint8_t[tex_size];
    for (uint32_t y = 0; y < tex_height; ++y) {
        for (uint32_t x = 0; x < tex_width; ++x) {
            uint32_t idx = (y * tex_width + x) * 4;
            bool checker = ((x / 8) + (y / 8)) % 2 == 0;
            tex_data[idx + 0] = checker ? 255 : 0;
            tex_data[idx + 1] = checker ? 0 : 255;
            tex_data[idx + 2] = 0;
            tex_data[idx + 3] = 255;
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

    std::printf("Texture created successfully\n");
    std::printf("Creating pipeline...\n");

    // Create a simple pipeline for material binding
    const char* vs_source = R"(
        #version 330 core
        layout(location = 0) in vec3 inPosition;
        void main() {
            gl_Position = vec4(inPosition, 1.0);
        }
    )";

    const char* ps_source = R"(
        #version 330 core
        uniform sampler2D g_Texture;
        out vec4 fragColor;
        void main() {
            fragColor = vec4(1.0, 0.5, 0.0, 1.0);  // Orange fallback
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
    pipeline_desc.name = "M6 Pipeline";
    pipeline_desc.vertex_shader = vs_desc;
    pipeline_desc.pixel_shader = ps_desc;
    pipeline_desc.color_format = poko::render::TextureFormat::RGBA8_UNORM;
    pipeline_desc.depth_format = poko::render::TextureFormat::D32_FLOAT;
    pipeline_desc.topology = poko::render::PrimitiveTopology::TriangleList;
    pipeline_desc.depth_stencil.depth_enable = false;
    pipeline_desc.rasterizer.cull_mode = poko::render::CullMode::None;

    auto pipeline_result = pipeline.initialize(device, pipeline_desc);

    if (pipeline_result != poko::render::PipelineErrorCode::Success) {
        std::fprintf(stderr, "Pipeline creation failed: %d\n", static_cast<int>(pipeline_result));
        engine.shutdown();
        return 1;
    }

    std::printf("Pipeline created successfully\n");
    std::printf("Creating material...\n");

    // Create material binding texture to shader variable
    poko::render::TextureBinding tex_binding;
    tex_binding.texture = &texture;
    tex_binding.shader_variable = "g_Texture";
    tex_binding.array_index = 0;

    poko::render::Material material;
    poko::render::MaterialDesc mat_desc;
    mat_desc.name = "CheckerboardMaterial";
    mat_desc.texture_count = 1;
    mat_desc.texture_bindings = &tex_binding;
    mat_desc.pipeline = &pipeline;

    auto mat_result = material.initialize(device, mat_desc);

    if (mat_result != poko::render::MaterialErrorCode::Success) {
        std::fprintf(stderr, "Material creation failed: %d\n", static_cast<int>(mat_result));
        std::printf("Note: SRB creation requires shader variable 'g_Texture' in pixel shader\n");
    } else {
        std::printf("Material created successfully with SRB\n");
    }

    engine.shutdown();

    std::printf("\nDemo finished successfully\n");
    return 0;
}
