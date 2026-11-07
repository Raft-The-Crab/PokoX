/**
 * @file test_gltf_loader.cpp
 * @brief Tests for the glTF/GLB loader (M13).
 *
 * Builds a minimal GLB in a temp file: one triangle with POSITION and
 * indices, plus a node hierarchy and one animation channel, then verifies
 * the loader parses meshes, accessors, skins data, and animations.
 */

#include "poko/render/gltf_loader.h"

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

using namespace poko::render;
namespace math = poko::math;

namespace {

int failures = 0;

#define CHECK(cond)                                                        \
    do {                                                                   \
        if (!(cond)) {                                                     \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);    \
            ++failures;                                                    \
        }                                                                  \
    } while (0)

void write_u32(std::vector<uint8_t>& out, uint32_t v) {
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
}

void write_chunk(std::vector<uint8_t>& glb, uint32_t type,
                 const std::string& data) {
    write_u32(glb, static_cast<uint32_t>(data.size()));
    write_u32(glb, type);
    glb.insert(glb.end(), data.begin(), data.end());
    // Pad to 4-byte boundary with spaces (JSON) or zeros (BIN)
    while (glb.size() % 4 != 0) {
        glb.push_back(type == 0x4E4F534A ? ' ' : 0);
    }
}

// Builds a minimal GLB containing:
//  - 1 mesh, 1 primitive, 3 verts (POSITION only) + uint16 indices
//  - 1 node "MeshNode" referencing the mesh
//  - 1 animation "Test" translating MeshNode on X
std::string build_test_glb() {
    // BIN chunk: positions (36 bytes) + indices (6 bytes)
    std::vector<uint8_t> bin;
    auto push_floats = [&](std::initializer_list<float> vals) {
        for (float f : vals) {
            uint32_t u;
            std::memcpy(&u, &f, 4);
            bin.push_back(static_cast<uint8_t>(u & 0xFF));
            bin.push_back(static_cast<uint8_t>((u >> 8) & 0xFF));
            bin.push_back(static_cast<uint8_t>((u >> 16) & 0xFF));
            bin.push_back(static_cast<uint8_t>((u >> 24) & 0xFF));
        }
    };
    push_floats({0.f, 0.f, 0.f,   1.f, 0.f, 0.f,   0.f, 1.f, 0.f});
    // indices: 0,1,2 as uint16
    bin.push_back(0); bin.push_back(0);
    bin.push_back(1); bin.push_back(0);
    bin.push_back(2); bin.push_back(0);
    // animation times: 0.0, 1.0 (8 bytes) then values: translations x3 keys? keep 2 keys * vec3
    push_floats({0.f, 1.f});                    // times
    push_floats({0.f,0.f,0.f,  2.f,0.f,0.f});   // translations

    const std::string json = R"({
        "asset": {"version": "2.0"},
        "scene": 0,
        "scenes": [{"nodes": [0]}],
        "nodes": [
            {"name": "MeshNode", "mesh": 0, "translation": [0,0,0]}
        ],
        "meshes": [{
            "name": "Tri",
            "primitives": [{
                "attributes": {"POSITION": 0},
                "indices": 1,
                "mode": 4
            }]
        }],
        "accessors": [
            {"bufferView": 0, "componentType": 5126, "count": 3, "type": "VEC3"},
            {"bufferView": 1, "componentType": 5123, "count": 3, "type": "SCALAR"},
            {"bufferView": 2, "componentType": 5126, "count": 2, "type": "SCALAR"},
            {"bufferView": 3, "componentType": 5126, "count": 2, "type": "VEC3"}
        ],
        "bufferViews": [
            {"buffer": 0, "byteOffset": 0, "byteLength": 36},
            {"buffer": 0, "byteOffset": 36, "byteLength": 6},
            {"buffer": 0, "byteOffset": 42, "byteLength": 8},
            {"buffer": 0, "byteOffset": 50, "byteLength": 24}
        ],
        "buffers": [{"byteLength": 74}],
        "animations": [{
            "name": "Test",
            "samplers": [{"input": 2, "output": 3, "interpolation": "LINEAR"}],
            "channels": [{"sampler": 0, "target": {"node": 0, "path": "translation"}}]
        }]
    })";

    std::vector<uint8_t> glb;
    write_u32(glb, 0x46546C67);  // 'glTF'
    write_u32(glb, 2);
    write_u32(glb, 0);           // total length patched later
    write_chunk(glb, 0x4E4F534A, json);
    write_chunk(glb, 0x004E4942, std::string(reinterpret_cast<const char*>(bin.data()), bin.size()));
    // Patch total length
    uint32_t total = static_cast<uint32_t>(glb.size());
    glb[8] = static_cast<uint8_t>(total & 0xFF);
    glb[9] = static_cast<uint8_t>((total >> 8) & 0xFF);
    glb[10] = static_cast<uint8_t>((total >> 16) & 0xFF);
    glb[11] = static_cast<uint8_t>((total >> 24) & 0xFF);

    return std::string(reinterpret_cast<const char*>(glb.data()), glb.size());
}

} // namespace

int main() {
    // Write the test GLB to a temp file
    std::filesystem::path tmp = std::filesystem::temp_directory_path() / "poko_test.glb";
    {
        std::string glb = build_test_glb();
        std::ofstream out(tmp, std::ios::binary);
        out.write(glb.data(), static_cast<std::streamsize>(glb.size()));
    }

    GltfLoader loader;
    GltfAsset asset;
    GltfLoaderError rc = loader.load(tmp.string().c_str(), asset);
    CHECK(rc == GltfLoaderError::Success);
    if (rc != GltfLoaderError::Success) {
        std::printf("load error: %s\n", loader.get_last_error());
        return 1;
    }

    // Mesh data
    CHECK(asset.primitives.size() == 1);
    if (!asset.primitives.empty()) {
        const GltfPrimitive& prim = asset.primitives[0];
        CHECK(prim.vertex_count == 3);
        CHECK(prim.indices.size() == 3);
        CHECK(prim.indices[0] == 0 && prim.indices[2] == 2);
        CHECK(prim.vertices.size() == 3 * 16);
        // Position of vertex 1 should be (1,0,0)
        CHECK(prim.vertices[16 + 0] == 1.0f);
        CHECK(prim.vertices[16 + 1] == 0.0f);
        // Default normal +Z, weight on joint 0
        CHECK(prim.vertices[16 + 5] == 1.0f);   // normal z
        CHECK(prim.vertices[16 + 12] == 1.0f);  // weight0
        MeshDesc md = prim.make_mesh_desc();
        CHECK(md.vertex_stride == 64);
        CHECK(md.index_format == IndexFormat::UInt32);
    }

    // Nodes
    CHECK(asset.nodes.size() == 1);
    CHECK(asset.nodes[0].name == "MeshNode");
    CHECK(asset.scene_roots.size() == 1);

    // Global transforms: node has identity rotation, translation 0
    std::vector<math::Mat4> globals = asset.compute_global_transforms();
    CHECK(globals.size() == 1);

    // Animation
    CHECK(asset.animations.size() == 1);
    CHECK(asset.animations[0].name == "Test");
    CHECK(asset.animations[0].duration == 1.0f);
    CHECK(asset.find_animation("test") == 0);
    CHECK(asset.find_animation("missing") == -1);

    // Error paths
    GltfLoader bad;
    GltfAsset empty;
    CHECK(bad.load("does_not_exist.glb", empty) != GltfLoaderError::Success);

    std::filesystem::remove(tmp);

    if (failures == 0) {
        std::printf("test_gltf_loader: all checks passed\n");
        return 0;
    }
    std::printf("test_gltf_loader: %d failures\n", failures);
    return 1;
}
