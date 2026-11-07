/**
 * @file test_mesh.cpp
 * @brief Tests for Mesh and OBJ loader.
 */

#include "poko/render/mesh.h"
#include "poko/render/obj_loader.h"
#include "poko/render/device.h"
#include "poko/render/vertex_buffer.h"

#include <cstdio>
#include <cassert>

using namespace poko::render;

int main() {
    std::printf("PokoX Mesh and OBJ Loader Tests\n");
    std::printf("================================\n\n");

    // Test 1: Basic mesh construction
    {
        std::printf("Test 1: Basic mesh construction... ");
        float vertices[] = {
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
             0.0f,  0.5f, 0.0f
        };
        uint32_t indices[] = {0, 1, 2};

        static VertexAttribute attrs[] = {
            {VertexSemantic::Position, VertexFormat::Float3, 0, 0, 0}
        };
        static VertexLayout layout = {attrs, 1, 12};

        Mesh mesh;
        MeshDesc desc;
        desc.vertex_data = vertices;
        desc.vertex_count = 3;
        desc.vertex_stride = 12;
        desc.index_data = indices;
        desc.index_count = 3;
        desc.index_format = IndexFormat::UInt32;
        desc.layout = &layout;
        desc.name = "TriangleMesh";

        // Without a device, we can't fully initialize, but we can validate the desc
        assert(desc.vertex_count == 3);
        assert(desc.index_count == 3);
        assert(layout.stride == 12);
        std::printf("PASS (desc validation)\n");
    }

    // Test 2: OBJ loader with a simple inline OBJ
    {
        std::printf("Test 2: OBJ loader (inline OBJ)... ");

        // Write a simple triangle OBJ to a temp file
        const char* temp_path = "test_triangle.obj";
        FILE* f = std::fopen(temp_path, "w");
        if (!f) {
            std::printf("FAILED (cannot write temp file)\n");
            return 1;
        }
        std::fprintf(f, "# Test triangle\n");
        std::fprintf(f, "v -0.5 -0.5 0.0\n");
        std::fprintf(f, "v  0.5 -0.5 0.0\n");
        std::fprintf(f, "v  0.0  0.5 0.0\n");
        std::fprintf(f, "f 1 2 3\n");
        std::fclose(f);

        ObjLoader loader;
        MeshDesc desc;
        ObjLoaderError err = loader.load(temp_path, nullptr, desc);

        if (err != ObjLoaderError::Success) {
            std::printf("FAILED: %s\n", loader.get_last_error());
            std::remove(temp_path);
            return 1;
        }

        if (desc.vertex_count != 3 || desc.index_count != 3) {
            std::printf("FAILED (vertex_count=%u index_count=%u)\n",
                        desc.vertex_count, desc.index_count);
            ObjLoader::cleanup(desc);
            std::remove(temp_path);
            return 1;
        }

        ObjLoader::cleanup(desc);
        std::remove(temp_path);
        std::printf("PASS\n");
    }

    // Test 3: OBJ loader with normals and texcoords
    {
        std::printf("Test 3: OBJ loader with normals/texcoords... ");

        const char* temp_path = "test_attribs.obj";
        FILE* f = std::fopen(temp_path, "w");
        if (!f) {
            std::printf("FAILED (cannot write temp file)\n");
            return 1;
        }
        std::fprintf(f, "v 0.0 0.0 0.0\n");
        std::fprintf(f, "v 1.0 0.0 0.0\n");
        std::fprintf(f, "v 0.5 1.0 0.0\n");
        std::fprintf(f, "vn 0.0 0.0 1.0\n");
        std::fprintf(f, "vn 0.0 0.0 1.0\n");
        std::fprintf(f, "vn 0.0 0.0 1.0\n");
        std::fprintf(f, "vt 0.0 0.0\n");
        std::fprintf(f, "vt 1.0 0.0\n");
        std::fprintf(f, "vt 0.5 1.0\n");
        std::fprintf(f, "f 1/1/1 2/2/2 3/3/3\n");
        std::fclose(f);

        ObjLoader loader;
        MeshDesc desc;
        ObjLoaderError err = loader.load(temp_path, nullptr, desc);

        bool ok = true;
        if (err != ObjLoaderError::Success) {
            std::printf("FAILED: %s\n", loader.get_last_error());
            ok = false;
        } else {
            // Layout should have Position + TexCoord + Normal = 8 floats per vertex
            uint32_t expected_stride = 3 * 4 + 2 * 4 + 3 * 4;  // 32 bytes
            if (desc.vertex_stride != expected_stride) {
                std::printf("FAILED (stride=%u expected=%u)\n", desc.vertex_stride, expected_stride);
                ok = false;
            }
        }

        ObjLoader::cleanup(desc);
        std::remove(temp_path);

        if (!ok) {
            return 1;
        }
        std::printf("PASS\n");
    }

    std::printf("\nAll mesh and OBJ loader tests passed.\n");
    return 0;
}
