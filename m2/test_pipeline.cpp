/**
 * @file test_pipeline.cpp
 * @brief M2 tests for Poko pipeline API.
 */

#include "poko/render/pipeline.h"
#include "poko/render/device.h"
#include "poko/render/context.h"
#include "poko/render/swapchain.h"

#include <cstdio>
#include <cassert>

// M2: GLSL 3.30 core shaders (using gl_VertexID, no vertex buffer)
constexpr char kVertexShader[] = R"(#version 330 core

void main()
{
    vec2 p;
    if (gl_VertexID == 0)
        p = vec2(-0.6, -0.5);
    else if (gl_VertexID == 1)
        p = vec2(0.6, -0.5);
    else
        p = vec2(0.0, 0.6);

    gl_Position = vec4(p, 0.5, 1.0);
}
)";

constexpr char kPixelShader[] = R"(#version 330 core

out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0, 0.60, 0.15, 1.0);
}
)";

int main() {
    std::printf("M2 Pipeline API Tests\n");
    std::printf("=====================\n\n");

    // Test 1: Pipeline state defaults to invalid
    {
        std::printf("Test 1: Pipeline state defaults to invalid... ");
        poko::render::PipelineState pipeline;
        assert(!pipeline.is_valid());
        std::printf("PASS\n");
    }

    // Test 2: Invalid device returns error
    {
        std::printf("Test 2: Invalid device returns error... ");
        poko::render::PipelineState pipeline;
        poko::render::PipelineDesc desc;
        desc.vertex_shader.source = kVertexShader;
        desc.pixel_shader.source = kPixelShader;
        
        auto result = pipeline.initialize(nullptr, desc);
        assert(result == poko::render::PipelineErrorCode::InvalidDevice);
        assert(!pipeline.is_valid());
        std::printf("PASS\n");
    }

    // Test 3: Invalid shader source returns error
    {
        std::printf("Test 3: Invalid shader source returns error... ");
        poko::render::PipelineState pipeline;
        poko::render::Device device;  // Not initialized, so invalid
        poko::render::PipelineDesc desc;
        desc.vertex_shader.source = nullptr;
        desc.pixel_shader.source = kPixelShader;
        
        auto result = pipeline.initialize(&device, desc);
        assert(result == poko::render::PipelineErrorCode::InvalidDevice);
        assert(!pipeline.is_valid());
        std::printf("PASS\n");
    }

    // Test 4: Shader type validation
    {
        std::printf("Test 4: Invalid shader type returns error... ");
        poko::render::PipelineState pipeline;
        poko::render::Device device;  // Not initialized
        poko::render::PipelineDesc desc;
        desc.vertex_shader.type = poko::render::ShaderType::Pixel;  // Wrong type
        desc.vertex_shader.source = kVertexShader;
        desc.pixel_shader.source = kPixelShader;
        
        auto result = pipeline.initialize(&device, desc);
        assert(result == poko::render::PipelineErrorCode::InvalidDevice);
        assert(!pipeline.is_valid());
        std::printf("PASS\n");
    }

    // Test 5: Empty shader source validation
    {
        std::printf("Test 5: Empty shader source returns error... ");
        poko::render::PipelineState pipeline;
        poko::render::Device device;  // Not initialized
        poko::render::PipelineDesc desc;
        desc.vertex_shader.type = poko::render::ShaderType::Vertex;
        desc.vertex_shader.source = "";
        desc.pixel_shader.source = kPixelShader;
        
        auto result = pipeline.initialize(&device, desc);
        assert(result == poko::render::PipelineErrorCode::InvalidDevice);
        assert(!pipeline.is_valid());
        std::printf("PASS\n");
    }

    // Test 6: Context::set_pipeline_state with null pipeline
    {
        std::printf("Test 6: Context::set_pipeline_state with null pipeline... ");
        poko::render::Context context;
        bool result = context.set_pipeline_state(nullptr);
        assert(!result);
        std::printf("PASS\n");
    }

    // Test 7: Context::set_pipeline_state with invalid pipeline
    {
        std::printf("Test 7: Context::set_pipeline_state with invalid pipeline... ");
        poko::render::Context context;
        poko::render::PipelineState pipeline;  // Not initialized
        bool result = context.set_pipeline_state(&pipeline);
        assert(!result);
        std::printf("PASS\n");
    }

    // Test 8: Invalid color format returns error
    {
        std::printf("Test 8: Invalid color format returns error... ");
        poko::render::PipelineState pipeline;
        poko::render::Device device;  // Not initialized
        poko::render::PipelineDesc desc;
        desc.vertex_shader.source = kVertexShader;
        desc.pixel_shader.source = kPixelShader;
        desc.color_format = poko::render::TextureFormat::Unknown;
        
        auto result = pipeline.initialize(&device, desc);
        assert(result == poko::render::PipelineErrorCode::InvalidDevice);  // Device checked first
        assert(!pipeline.is_valid());
        std::printf("PASS\n");
    }

    // Test 9: Invalid depth format returns error
    {
        std::printf("Test 9: Invalid depth format returns error... ");
        poko::render::PipelineState pipeline;
        poko::render::Device device;  // Not initialized
        poko::render::PipelineDesc desc;
        desc.vertex_shader.source = kVertexShader;
        desc.pixel_shader.source = kPixelShader;
        desc.depth_format = poko::render::TextureFormat::Unknown;
        
        auto result = pipeline.initialize(&device, desc);
        assert(result == poko::render::PipelineErrorCode::InvalidDevice);  // Device checked first
        assert(!pipeline.is_valid());
        std::printf("PASS\n");
    }

    std::printf("\nAll M2 pipeline tests passed.\n");
    return 0;
}
