/**
 * @file obj_loader.cpp
 * @brief OBJ loader implementation.
 */

#include "poko/render/obj_loader.h"
#include "poko/render/vertex_buffer.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

namespace poko::render {

namespace {

struct VertexKey {
    int v;
    int vt;
    int vn;

    bool operator<(const VertexKey& other) const {
        if (v != other.v) return v < other.v;
        if (vt != other.vt) return vt < other.vt;
        return vn < other.vn;
    }
};

} // namespace

ObjLoader::ObjLoader() {}

ObjLoaderError ObjLoader::load(const char* filepath, Device* device, MeshDesc& out_desc) {
    out_desc = MeshDesc{};  // Zero-initialize

    if (!filepath) {
        last_error_ = "Filepath is null";
        return ObjLoaderError::FileNotFound;
    }

    std::FILE* file = std::fopen(filepath, "r");
    if (!file) {
        last_error_ = std::string("Failed to open file: ") + filepath;
        return ObjLoaderError::FileNotFound;
    }

    std::vector<float> positions;
    std::vector<float> texcoords;
    std::vector<float> normals;
    std::vector<VertexKey> face_indices;

    char line[1024];
    while (std::fgets(line, sizeof(line), file)) {
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') {
            continue;
        }

        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "v") {
            float x, y, z;
            if (iss >> x >> y >> z) {
                positions.push_back(x);
                positions.push_back(y);
                positions.push_back(z);
            }
        } else if (token == "vt") {
            float u, v;
            if (iss >> u >> v) {
                texcoords.push_back(u);
                texcoords.push_back(v);
            }
        } else if (token == "vn") {
            float x, y, z;
            if (iss >> x >> y >> z) {
                normals.push_back(x);
                normals.push_back(y);
                normals.push_back(z);
            }
        } else if (token == "f") {
            // Parse face indices (v/vt/vn or v//vn or v)
            for (int i = 0; i < 3; ++i) {
                std::string face_str;
                if (!(iss >> face_str)) break;

                VertexKey key = {0, 0, 0};

                // Parse v/vt/vn format (replace '/' with space for easy parsing)
                std::string face_str_fixed = face_str;
                std::replace(face_str_fixed.begin(), face_str_fixed.end(), '/', ' ');
                std::istringstream fss(face_str_fixed);

                int vi = 0, ti = 0, ni = 0;
                fss >> vi >> ti >> ni;

                // OBJ indices are 1-based; convert to 0-based
                key.v = (vi > 0) ? vi - 1 : ((vi < 0) ? (static_cast<int>(positions.size() / 3) + vi) : 0);
                key.vt = (ti > 0) ? ti - 1 : ((ti < 0) ? (static_cast<int>(texcoords.size() / 2) + ti) : 0);
                key.vn = (ni > 0) ? ni - 1 : ((ni < 0) ? (static_cast<int>(normals.size() / 3) + ni) : 0);

                face_indices.push_back(key);
            }
        }
    }

    std::fclose(file);

    if (positions.empty()) {
        last_error_ = "No vertices found in OBJ file";
        return ObjLoaderError::NoVertices;
    }

    if (face_indices.empty()) {
        last_error_ = "No faces found in OBJ file";
        return ObjLoaderError::NoFaces;
    }

    // Build vertex buffer by deduplicating (v, vt, vn) tuples
    std::map<VertexKey, uint32_t> vertex_map;
    std::vector<float> vertex_data;
    std::vector<uint32_t> index_data;

    bool has_texcoords = !texcoords.empty();
    bool has_normals = !normals.empty();

    for (const VertexKey& key : face_indices) {
        // Validate indices
        if (key.v < 0 || key.v * 3 >= static_cast<int>(positions.size())) {
            last_error_ = "Vertex index out of range";
            return ObjLoaderError::IndexOutOfRange;
        }
        if (has_texcoords && (key.vt < 0 || key.vt * 2 >= static_cast<int>(texcoords.size()))) {
            last_error_ = "Texcoord index out of range";
            return ObjLoaderError::IndexOutOfRange;
        }
        if (has_normals && (key.vn < 0 || key.vn * 3 >= static_cast<int>(normals.size()))) {
            last_error_ = "Normal index out of range";
            return ObjLoaderError::IndexOutOfRange;
        }

        auto it = vertex_map.find(key);
        if (it != vertex_map.end()) {
            index_data.push_back(it->second);
        } else {
            uint32_t index = static_cast<uint32_t>(vertex_data.size() / (3 + (has_texcoords ? 2 : 0) + (has_normals ? 3 : 0)));

            // Position
            vertex_data.push_back(positions[key.v * 3 + 0]);
            vertex_data.push_back(positions[key.v * 3 + 1]);
            vertex_data.push_back(positions[key.v * 3 + 2]);

            // Texcoord
            if (has_texcoords) {
                vertex_data.push_back(texcoords[key.vt * 2 + 0]);
                vertex_data.push_back(texcoords[key.vt * 2 + 1]);
            }

            // Normal
            if (has_normals) {
                vertex_data.push_back(normals[key.vn * 3 + 0]);
                vertex_data.push_back(normals[key.vn * 3 + 1]);
                vertex_data.push_back(normals[key.vn * 3 + 2]);
            }

            vertex_map[key] = index;
            index_data.push_back(index);
        }
    }

    // Allocate buffers
    size_t vertex_bytes = vertex_data.size() * sizeof(float);
    float* vertex_buffer = new float[vertex_data.size()];
    std::memcpy(vertex_buffer, vertex_data.data(), vertex_bytes);

    size_t index_bytes = index_data.size() * sizeof(uint32_t);
    uint32_t* index_buffer = new uint32_t[index_data.size()];
    std::memcpy(index_buffer, index_data.data(), index_bytes);

    // Build vertex layout
    static VertexAttribute attrs[4];
    uint32_t offset = 0;
    uint32_t attr_count = 0;

    attrs[attr_count].semantic = VertexSemantic::Position;
    attrs[attr_count].format = VertexFormat::Float3;
    attrs[attr_count].offset = offset;
    attrs[attr_count].input_slot = 0;
    attrs[attr_count].instance_data_step_rate = 0;
    offset += 12;
    attr_count++;

    if (has_texcoords) {
        attrs[attr_count].semantic = VertexSemantic::TexCoord;
        attrs[attr_count].format = VertexFormat::Float2;
        attrs[attr_count].offset = offset;
        attrs[attr_count].input_slot = 0;
        attrs[attr_count].instance_data_step_rate = 0;
        offset += 8;
        attr_count++;
    }

    if (has_normals) {
        attrs[attr_count].semantic = VertexSemantic::Normal;
        attrs[attr_count].format = VertexFormat::Float3;
        attrs[attr_count].offset = offset;
        attrs[attr_count].input_slot = 0;
        attrs[attr_count].instance_data_step_rate = 0;
        offset += 12;
        attr_count++;
    }

    static VertexLayout layout;
    layout.attributes = attrs;
    layout.attribute_count = attr_count;
    layout.stride = offset;

    // Populate MeshDesc
    out_desc.vertex_data = vertex_buffer;
    out_desc.vertex_count = static_cast<uint32_t>(vertex_data.size() / (offset / sizeof(float)));
    out_desc.vertex_stride = offset;
    out_desc.index_data = index_buffer;
    out_desc.index_count = static_cast<uint32_t>(index_data.size());
    out_desc.index_format = IndexFormat::UInt32;
    out_desc.layout = &layout;
    out_desc.name = filepath;

    (void)device;  // May be used for size validation in future

    return ObjLoaderError::Success;
}

void ObjLoader::cleanup(MeshDesc& desc) {
    if (desc.vertex_data) {
        delete[] static_cast<const float*>(desc.vertex_data);
        desc.vertex_data = nullptr;
    }
    if (desc.index_data) {
        delete[] static_cast<const uint32_t*>(desc.index_data);
        desc.index_data = nullptr;
    }
    desc.layout = nullptr;
}

const char* ObjLoader::get_last_error() const {
    return last_error_.c_str();
}

} // namespace poko::render
