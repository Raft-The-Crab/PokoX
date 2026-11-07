/**
 * @file gltf_loader.cpp
 * @brief glTF 2.0 / GLB loader implementation.
 *
 * Parses the glTF JSON + BIN chunks, decodes accessors, assembles
 * interleaved vertex buffers, and extracts skins, materials, images,
 * and animation clips.
 */

#include "poko/render/gltf_loader.h"

#include <cstdio>
#include <cstring>
#include <cmath>
#include <fstream>
#include <map>
#include <memory>

namespace poko::render {

namespace {

// =========================================================================
// Minimal JSON parser (objects, arrays, strings, numbers, bools, null)
// =========================================================================

struct JsonValue {
    enum class Type : uint8_t { Null, Bool, Number, String, Array, Object };

    Type type = Type::Null;
    bool boolean = false;
    double number = 0.0;
    std::string str;
    std::vector<JsonValue> array;
    std::vector<std::pair<std::string, JsonValue>> object;

    const JsonValue* find(const char* key) const {
        if (type != Type::Object) return nullptr;
        for (const auto& kv : object) {
            if (kv.first == key) return &kv.second;
        }
        return nullptr;
    }

    const JsonValue& at(const char* key) const {
        static const JsonValue null_value;
        const JsonValue* v = find(key);
        return v ? *v : null_value;
    }

    std::string get_string(const char* key, const char* fallback = "") const {
        const JsonValue* v = find(key);
        return (v && v->type == Type::String) ? v->str : std::string(fallback);
    }

    double get_number(const char* key, double fallback = 0.0) const {
        const JsonValue* v = find(key);
        return (v && v->type == Type::Number) ? v->number : fallback;
    }

    int32_t get_int(const char* key, int32_t fallback = 0) const {
        return static_cast<int32_t>(get_number(key, fallback));
    }

    bool get_bool(const char* key, bool fallback = false) const {
        const JsonValue* v = find(key);
        return (v && v->type == Type::Bool) ? v->boolean : fallback;
    }
};

class JsonParser {
public:
    explicit JsonParser(const std::string& text) : s_(text) {}

    bool parse(JsonValue& out) {
        skip_ws();
        if (!parse_value(out)) return false;
        skip_ws();
        return pos_ == s_.size();
    }

    const char* error() const { return error_; }

private:
    const std::string& s_;
    size_t pos_ = 0;
    const char* error_ = "parse error";

    void skip_ws() {
        while (pos_ < s_.size() && (s_[pos_] == ' ' || s_[pos_] == '\t' ||
                                    s_[pos_] == '\n' || s_[pos_] == '\r')) {
            ++pos_;
        }
    }

    bool parse_value(JsonValue& out) {
        skip_ws();
        if (pos_ >= s_.size()) return fail("unexpected end");
        char c = s_[pos_];
        switch (c) {
            case '{': return parse_object(out);
            case '[': return parse_array(out);
            case '"': {
                out.type = JsonValue::Type::String;
                return parse_string(out.str);
            }
            case 't': return parse_literal("true", JsonValue::Type::Bool, true, out);
            case 'f': return parse_literal("false", JsonValue::Type::Bool, false, out);
            case 'n': return parse_literal("null", JsonValue::Type::Null, false, out);
            default:
                if (c == '-' || (c >= '0' && c <= '9')) return parse_number(out);
                return fail("unexpected character");
        }
    }

    bool parse_object(JsonValue& out) {
        out.type = JsonValue::Type::Object;
        ++pos_;  // '{'
        skip_ws();
        if (pos_ < s_.size() && s_[pos_] == '}') { ++pos_; return true; }
        while (true) {
            skip_ws();
            if (pos_ >= s_.size() || s_[pos_] != '"') return fail("expected object key");
            std::string key;
            if (!parse_string(key)) return false;
            skip_ws();
            if (pos_ >= s_.size() || s_[pos_] != ':') return fail("expected ':'");
            ++pos_;
            JsonValue value;
            if (!parse_value(value)) return false;
            out.object.emplace_back(std::move(key), std::move(value));
            skip_ws();
            if (pos_ >= s_.size()) return fail("unterminated object");
            if (s_[pos_] == ',') { ++pos_; continue; }
            if (s_[pos_] == '}') { ++pos_; return true; }
            return fail("expected ',' or '}'");
        }
    }

    bool parse_array(JsonValue& out) {
        out.type = JsonValue::Type::Array;
        ++pos_;  // '['
        skip_ws();
        if (pos_ < s_.size() && s_[pos_] == ']') { ++pos_; return true; }
        while (true) {
            JsonValue value;
            if (!parse_value(value)) return false;
            out.array.push_back(std::move(value));
            skip_ws();
            if (pos_ >= s_.size()) return fail("unterminated array");
            if (s_[pos_] == ',') { ++pos_; continue; }
            if (s_[pos_] == ']') { ++pos_; return true; }
            return fail("expected ',' or ']'");
        }
    }

    bool parse_string(std::string& out) {
        ++pos_;  // '"'
        out.clear();
        while (pos_ < s_.size()) {
            char c = s_[pos_++];
            if (c == '"') return true;
            if (c == '\\') {
                if (pos_ >= s_.size()) return fail("bad escape");
                char e = s_[pos_++];
                switch (e) {
                    case '"': out += '"'; break;
                    case '\\': out += '\\'; break;
                    case '/': out += '/'; break;
                    case 'b': out += '\b'; break;
                    case 'f': out += '\f'; break;
                    case 'n': out += '\n'; break;
                    case 'r': out += '\r'; break;
                    case 't': out += '\t'; break;
                    case 'u': {
                        if (pos_ + 4 > s_.size()) return fail("bad unicode escape");
                        unsigned code = 0;
                        for (int i = 0; i < 4; ++i) {
                            char h = s_[pos_++];
                            code <<= 4;
                            if (h >= '0' && h <= '9') code += h - '0';
                            else if (h >= 'a' && h <= 'f') code += h - 'a' + 10;
                            else if (h >= 'A' && h <= 'F') code += h - 'A' + 10;
                            else return fail("bad unicode escape");
                        }
                        // Encode as UTF-8 (BMP only; surrogate pairs kept simple)
                        if (code < 0x80) {
                            out += static_cast<char>(code);
                        } else if (code < 0x800) {
                            out += static_cast<char>(0xC0 | (code >> 6));
                            out += static_cast<char>(0x80 | (code & 0x3F));
                        } else {
                            out += static_cast<char>(0xE0 | (code >> 12));
                            out += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
                            out += static_cast<char>(0x80 | (code & 0x3F));
                        }
                        break;
                    }
                    default: return fail("bad escape");
                }
            } else {
                out += c;
            }
        }
        return fail("unterminated string");
    }

    bool parse_number(JsonValue& out) {
        size_t start = pos_;
        if (s_[pos_] == '-') ++pos_;
        while (pos_ < s_.size() &&
               ((s_[pos_] >= '0' && s_[pos_] <= '9') || s_[pos_] == '.' ||
                s_[pos_] == 'e' || s_[pos_] == 'E' || s_[pos_] == '+' || s_[pos_] == '-')) {
            ++pos_;
        }
        try {
            out.type = JsonValue::Type::Number;
            out.number = std::stod(s_.substr(start, pos_ - start));
            return true;
        } catch (...) {
            return fail("bad number");
        }
    }

    bool parse_literal(const char* lit, JsonValue::Type type, bool boolean, JsonValue& out) {
        size_t len = std::strlen(lit);
        if (s_.compare(pos_, len, lit) != 0) return fail("bad literal");
        pos_ += len;
        out.type = type;
        out.boolean = boolean;
        return true;
    }

    bool fail(const char* msg) {
        error_ = msg;
        return false;
    }
};

// =========================================================================
// glTF accessor decoding
// =========================================================================

struct AccessorView {
    const uint8_t* data = nullptr;
    uint32_t count = 0;
    uint32_t components = 0;
    uint32_t component_size = 0;
    uint32_t component_type = 0;  // glTF componentType
    bool normalized = false;
    uint32_t stride = 0;          // Bytes between elements (bufferView stride or packed)
};

struct BufferView {
    uint32_t offset = 0;
    uint32_t length = 0;
    uint32_t stride = 0;
    uint32_t buffer = 0;
};

struct Accessor {
    uint32_t buffer_view = 0xFFFFFFFF;
    uint32_t byte_offset = 0;
    uint32_t count = 0;
    uint32_t component_type = 0;
    std::string type;
    bool normalized = false;
    std::vector<float> sparse_values;  // unused placeholder
};

uint32_t type_components(const std::string& type) {
    if (type == "SCALAR") return 1;
    if (type == "VEC2") return 2;
    if (type == "VEC3") return 3;
    if (type == "VEC4") return 4;
    if (type == "MAT2") return 4;
    if (type == "MAT3") return 9;
    if (type == "MAT4") return 16;
    return 0;
}

uint32_t component_type_size(uint32_t ct) {
    switch (ct) {
        case 5120: case 5121: return 1;
        case 5122: case 5123: return 2;
        case 5125: case 5126: return 4;
        default: return 0;
    }
}

class GltfDocument {
public:
    const JsonValue* root = nullptr;
    std::vector<uint8_t> bin_chunk;
    std::string json_text;
    std::vector<BufferView> buffer_views;
    std::vector<Accessor> accessors;
    bool ok = false;
    std::string error;
};

bool load_glb(const char* filepath, GltfDocument& doc, std::string& error) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        error = std::string("Failed to open file: ") + filepath;
        return false;
    }
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());
    if (data.size() < 12) {
        error = "File too small for GLB header";
        return false;
    }
    uint32_t magic = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
    if (magic != 0x46546C67) {  // 'glTF' little-endian
        error = "Not a GLB file (bad magic)";
        return false;
    }
    uint32_t version = data[4] | (data[5] << 8) | (data[6] << 16) | (data[7] << 24);
    if (version != 2) {
        error = "Unsupported GLB version";
        return false;
    }

    size_t pos = 12;
    std::string json_text;
    while (pos + 8 <= data.size()) {
        uint32_t length = data[pos] | (data[pos + 1] << 8) | (data[pos + 2] << 16) | (data[pos + 3] << 24);
        uint32_t type = data[pos + 4] | (data[pos + 5] << 8) | (data[pos + 6] << 16) | (data[pos + 7] << 24);
        pos += 8;
        if (pos + length > data.size()) {
            error = "Chunk extends past end of file";
            return false;
        }
        if (type == 0x4E4F534A) {  // 'JSON'
            json_text.assign(reinterpret_cast<const char*>(data.data() + pos), length);
        } else if (type == 0x004E4942) {  // 'BIN'
            doc.bin_chunk.assign(data.begin() + pos, data.begin() + pos + length);
        }
        // Chunk data is padded to a 4-byte boundary; chunkLength excludes
        // the padding, so align the read position before the next chunk.
        pos += length;
        pos = (pos + 3) & ~static_cast<size_t>(3);
    }
    if (json_text.empty()) {
        error = "GLB has no JSON chunk";
        return false;
    }

    JsonParser parser(json_text);
    JsonValue root;
    if (!parser.parse(root)) {
        error = std::string("JSON parse error: ") + parser.error();
        return false;
    }
    doc.json_text = std::move(json_text);
    doc.root = new JsonValue(std::move(root));
    return true;
}

bool parse_document_json(GltfDocument& doc) {
    const JsonValue& root = *doc.root;

    // bufferViews
    const JsonValue& bvs = root.at("bufferViews");
    if (bvs.type == JsonValue::Type::Array) {
        for (const auto& bv : bvs.array) {
            BufferView view;
            view.offset = static_cast<uint32_t>(bv.get_number("byteOffset", 0));
            view.length = static_cast<uint32_t>(bv.get_number("byteLength", 0));
            view.stride = static_cast<uint32_t>(bv.get_number("byteStride", 0));
            view.buffer = static_cast<uint32_t>(bv.get_int("buffer", 0));
            doc.buffer_views.push_back(view);
        }
    }

    // accessors
    const JsonValue& accs = root.at("accessors");
    if (accs.type == JsonValue::Type::Array) {
        for (const auto& a : accs.array) {
            Accessor acc;
            const JsonValue* bv = a.find("bufferView");
            acc.buffer_view = (bv && bv->type == JsonValue::Type::Number)
                                  ? static_cast<uint32_t>(bv->number) : 0xFFFFFFFF;
            acc.byte_offset = static_cast<uint32_t>(a.get_number("byteOffset", 0));
            acc.count = static_cast<uint32_t>(a.get_number("count", 0));
            acc.component_type = static_cast<uint32_t>(a.get_number("componentType", 0));
            acc.type = a.get_string("type");
            acc.normalized = a.get_bool("normalized", false);
            if (!acc.type.empty() && acc.count > 0 &&
                component_type_size(acc.component_type) > 0 &&
                type_components(acc.type) > 0) {
                doc.accessors.push_back(acc);
            }
        }
    }
    return true;
}

// Reads one component as float from accessor data
inline float read_component(const uint8_t* p, uint32_t ct, bool normalized) {
    switch (ct) {
        case 5120: {  // byte
            int8_t v; std::memcpy(&v, p, 1);
            if (normalized) return std::max(-1.0f, v / 127.0f);
            return static_cast<float>(v);
        }
        case 5121: {  // ubyte
            uint8_t v = *p;
            if (normalized) return v / 255.0f;
            return static_cast<float>(v);
        }
        case 5122: {  // short
            int16_t v; std::memcpy(&v, p, 2);
            if (normalized) return std::max(-1.0f, v / 32767.0f);
            return static_cast<float>(v);
        }
        case 5123: {  // ushort
            uint16_t v; std::memcpy(&v, p, 2);
            if (normalized) return v / 65535.0f;
            return static_cast<float>(v);
        }
        case 5125: {  // uint
            uint32_t v; std::memcpy(&v, p, 4);
            return static_cast<float>(v);
        }
        case 5126: {  // float
            float v; std::memcpy(&v, p, 4);
            return v;
        }
        default: return 0.0f;
    }
}

// Builds an AccessorView for an accessor index
bool make_accessor_view(const GltfDocument& doc, uint32_t index, AccessorView& out) {
    if (index >= doc.accessors.size()) return false;
    const Accessor& acc = doc.accessors[index];
    if (acc.buffer_view >= doc.buffer_views.size()) return false;
    const BufferView& bv = doc.buffer_views[acc.buffer_view];
    if (bv.buffer != 0 || bv.offset + bv.length > doc.bin_chunk.size()) return false;

    const uint8_t* base = doc.bin_chunk.data() + bv.offset + acc.byte_offset;
    uint32_t comps = type_components(acc.type);
    uint32_t csize = component_type_size(acc.component_type);

    out.data = base;
    out.count = acc.count;
    out.components = comps;
    out.component_size = csize;
    out.component_type = acc.component_type;
    out.normalized = acc.normalized;
    // Element stride: bufferView stride if interleaved, else packed
    out.stride = (bv.stride >= comps * csize) ? bv.stride : comps * csize;
    return true;
}

// Decodes an accessor into floats (count * components values)
bool read_accessor_floats(const GltfDocument& doc, uint32_t index,
                          std::vector<float>& out) {
    AccessorView view;
    if (!make_accessor_view(doc, index, view)) return false;
    out.resize(static_cast<size_t>(view.count) * view.components);
    for (uint32_t i = 0; i < view.count; ++i) {
        const uint8_t* p = view.data + static_cast<size_t>(i) * view.stride;
        for (uint32_t c = 0; c < view.components; ++c) {
            out[i * view.components + c] =
                read_component(p + c * view.component_size, view.component_type,
                               view.normalized);
        }
    }
    return true;
}

// Decodes an accessor into uint32 (for indices)
bool read_accessor_uints(const GltfDocument& doc, uint32_t index,
                         std::vector<uint32_t>& out) {
    AccessorView view;
    if (!make_accessor_view(doc, index, view)) return false;
    out.resize(view.count);
    for (uint32_t i = 0; i < view.count; ++i) {
        const uint8_t* p = view.data + static_cast<size_t>(i) * view.stride;
        switch (view.component_type) {
            case 5121: out[i] = *p; break;
            case 5123: { uint16_t v; std::memcpy(&v, p, 2); out[i] = v; break; }
            case 5125: { uint32_t v; std::memcpy(&v, p, 4); out[i] = v; break; }
            default: return false;
        }
    }
    return true;
}

// Decodes a MAT4 accessor into Mat4 list (16 floats each, column-major)
bool read_accessor_matrices(const GltfDocument& doc, uint32_t index,
                            std::vector<math::Mat4>& out) {
    std::vector<float> floats;
    if (!read_accessor_floats(doc, index, floats)) return false;
    if (floats.size() % 16 != 0) return false;
    out.resize(floats.size() / 16);
    for (size_t i = 0; i < out.size(); ++i) {
        std::memcpy(out[i].m, floats.data() + i * 16, 16 * sizeof(float));
    }
    return true;
}

} // namespace

// =========================================================================
// GltfAsset helpers
// =========================================================================

math::Mat4 GltfAsset::node_local_matrix(int32_t node_index) const {
    if (node_index < 0 || node_index >= static_cast<int32_t>(nodes.size())) {
        return math::Mat4::identity();
    }
    const GltfNode& node = nodes[node_index];
    if (node.has_matrix) {
        return node.matrix;
    }
    // T * R * S (column-major composition)
    math::Mat4 t = math::Mat4::translation(node.translation);
    math::Mat4 r = node.rotation.to_matrix();
    math::Mat4 s = math::Mat4::scale(node.scale);
    return t * r * s;
}

std::vector<math::Mat4> GltfAsset::compute_global_transforms() const {
    std::vector<math::Mat4> global(nodes.size(), math::Mat4::identity());
    std::vector<int32_t> stack;
    for (int32_t root : scene_roots) stack.push_back(root);
    while (!stack.empty()) {
        int32_t idx = stack.back();
        stack.pop_back();
        const GltfNode& node = nodes[idx];
        math::Mat4 local = node_local_matrix(idx);
        math::Mat4 parent_global = math::Mat4::identity();
        if (node.parent >= 0 && node.parent < static_cast<int32_t>(nodes.size())) {
            parent_global = global[node.parent];
        }
        global[idx] = parent_global * local;
        for (int32_t child : node.children) {
            stack.push_back(child);
        }
    }
    return global;
}

int32_t GltfAsset::find_animation(const char* name) const {
    if (!name) return -1;
    for (size_t i = 0; i < animations.size(); ++i) {
        const std::string& a = animations[i].name;
        const char* b = name;
        size_t j = 0;
        for (; j < a.size() && b[j]; ++j) {
            char ca = a[j] >= 'A' && a[j] <= 'Z' ? a[j] + 32 : a[j];
            char cb = b[j] >= 'A' && b[j] <= 'Z' ? b[j] + 32 : b[j];
            if (ca != cb) break;
        }
        if (j == a.size() && b[j] == '\0') return static_cast<int32_t>(i);
    }
    return -1;
}

// =========================================================================
// GltfLoader
// =========================================================================

GltfLoader::GltfLoader() {}

GltfLoaderError GltfLoader::load(const char* filepath, GltfAsset& out_asset) {
    out_asset = GltfAsset{};
    if (!filepath) {
        last_error_ = "Filepath is null";
        return GltfLoaderError::FileNotFound;
    }

    GltfDocument doc;
    std::string error;
    if (!load_glb(filepath, doc, error)) {
        last_error_ = error;
        return error.find("version") != std::string::npos
                   ? GltfLoaderError::UnsupportedVersion
                   : GltfLoaderError::FileNotFound;
    }
    if (!parse_document_json(doc)) {
        last_error_ = doc.error.empty() ? "Failed to parse glTF JSON" : doc.error;
        delete doc.root;
        return GltfLoaderError::ParseError;
    }

    const JsonValue& root = *doc.root;
    const JsonValue& meshes_j = root.at("meshes");
    const JsonValue& nodes_j = root.at("nodes");
    const JsonValue& mats_j = root.at("materials");
    const JsonValue& imgs_j = root.at("images");
    const JsonValue& skins_j = root.at("skins");
    const JsonValue& anims_j = root.at("animations");
    const JsonValue& scenes_j = root.at("scenes");
    const JsonValue& scene_j = root.at("scene");

    // ------------------------------------------------------------------ nodes
    if (nodes_j.type == JsonValue::Type::Array) {
        for (const auto& nj : nodes_j.array) {
            GltfNode node;
            node.name = nj.get_string("name");

            const JsonValue& t = nj.at("translation");
            if (t.type == JsonValue::Type::Array && t.array.size() >= 3) {
                node.translation = math::Vec3(
                    static_cast<float>(t.array[0].number),
                    static_cast<float>(t.array[1].number),
                    static_cast<float>(t.array[2].number));
            }
            const JsonValue& r = nj.at("rotation");
            if (r.type == JsonValue::Type::Array && r.array.size() >= 4) {
                node.rotation = math::Quat(
                    static_cast<float>(r.array[0].number),
                    static_cast<float>(r.array[1].number),
                    static_cast<float>(r.array[2].number),
                    static_cast<float>(r.array[3].number));
            }
            const JsonValue& sc = nj.at("scale");
            if (sc.type == JsonValue::Type::Array && sc.array.size() >= 3) {
                node.scale = math::Vec3(
                    static_cast<float>(sc.array[0].number),
                    static_cast<float>(sc.array[1].number),
                    static_cast<float>(sc.array[2].number));
            }
            const JsonValue& m = nj.at("matrix");
            if (m.type == JsonValue::Type::Array && m.array.size() == 16) {
                node.has_matrix = true;
                for (int i = 0; i < 16; ++i) {
                    node.matrix.m[i] = static_cast<float>(m.array[i].number);
                }
            }
            node.mesh = nj.get_int("mesh", -1);
            node.skin = nj.get_int("skin", -1);
            const JsonValue& ch = nj.at("children");
            if (ch.type == JsonValue::Type::Array) {
                for (const auto& c : ch.array) {
                    if (c.type == JsonValue::Type::Number) {
                        node.children.push_back(static_cast<int32_t>(c.number));
                    }
                }
            }
            out_asset.nodes.push_back(std::move(node));
        }
        // Resolve parents
        for (size_t i = 0; i < out_asset.nodes.size(); ++i) {
            for (int32_t child : out_asset.nodes[i].children) {
                if (child >= 0 && child < static_cast<int32_t>(out_asset.nodes.size())) {
                    out_asset.nodes[child].parent = static_cast<int32_t>(i);
                }
            }
        }
    }

    // -------------------------------------------------------------- primitives
    // Map mesh index -> node skin (a mesh may be referenced by skinned nodes)
    std::map<int32_t, int32_t> mesh_to_skin;
    for (size_t ni = 0; ni < out_asset.nodes.size(); ++ni) {
        if (out_asset.nodes[ni].mesh >= 0 && out_asset.nodes[ni].skin >= 0) {
            mesh_to_skin[out_asset.nodes[ni].mesh] = out_asset.nodes[ni].skin;
        }
    }

    if (meshes_j.type == JsonValue::Type::Array) {
        for (size_t mesh_idx = 0; mesh_idx < meshes_j.array.size(); ++mesh_idx) {
            const auto& mesh_j = meshes_j.array[mesh_idx];
            const JsonValue& prims = mesh_j.at("primitives");
            if (prims.type != JsonValue::Type::Array) continue;

            int32_t skin_for_mesh = -1;
            auto it = mesh_to_skin.find(static_cast<int32_t>(mesh_idx));
            if (it != mesh_to_skin.end()) skin_for_mesh = it->second;

            for (const auto& prim : prims.array) {
                GltfPrimitive primitive;
                primitive.name = mesh_j.get_string("name");
                primitive.skinned = (skin_for_mesh >= 0);
                primitive.material = prim.get_int("material", -1);

                const JsonValue& attrs = prim.at("attributes");
                const JsonValue* pos_acc = attrs.find("POSITION");
                if (!pos_acc || pos_acc->type != JsonValue::Type::Number) {
                    continue;  // Skip primitives without positions
                }

                // Read POSITION to get vertex count
                std::vector<float> positions;
                if (!read_accessor_floats(doc, static_cast<uint32_t>(pos_acc->number), positions)) {
                    continue;
                }
                uint32_t vertex_count = static_cast<uint32_t>(positions.size() / 3);
                primitive.vertex_count = vertex_count;

                // Optional attributes
                auto read_attr = [&](const char* name,
                                     std::vector<float>& out) -> bool {
                    const JsonValue* acc = attrs.find(name);
                    if (!acc || acc->type != JsonValue::Type::Number) return false;
                    return read_accessor_floats(doc, static_cast<uint32_t>(acc->number), out);
                };

                std::vector<float> normals, uvs, joints, weights;
                bool has_normals = read_attr("NORMAL", normals);
                bool has_uvs = read_attr("TEXCOORD_0", uvs);
                bool has_joints = read_attr("JOINTS_0", joints);
                bool has_weights = read_attr("WEIGHTS_0", weights);

                // Indices
                const JsonValue* idx_acc = prim.find("indices");
                if (idx_acc && idx_acc->type == JsonValue::Type::Number) {
                    read_accessor_uints(doc, static_cast<uint32_t>(idx_acc->number),
                                        primitive.indices);
                } else {
                    // Non-indexed: sequential
                    primitive.indices.resize(vertex_count);
                    for (uint32_t i = 0; i < vertex_count; ++i) {
                        primitive.indices[i] = i;
                    }
                }

                // Assemble interleaved vertices (64-byte stride = 16 floats):
                // pos3 + norm3 + uv2 + joints4 + weights4
                primitive.vertices.resize(static_cast<size_t>(vertex_count) * 16);
                for (uint32_t v = 0; v < vertex_count; ++v) {
                    float* dst = primitive.vertices.data() + v * 16;
                    // Position
                    dst[0] = positions[v * 3 + 0];
                    dst[1] = positions[v * 3 + 1];
                    dst[2] = positions[v * 3 + 2];
                    // Normal (default +Z)
                    if (has_normals && (v * 3 + 2) < normals.size()) {
                        dst[3] = normals[v * 3 + 0];
                        dst[4] = normals[v * 3 + 1];
                        dst[5] = normals[v * 3 + 2];
                    } else {
                        dst[3] = 0.0f; dst[4] = 0.0f; dst[5] = 1.0f;
                    }
                    // Texcoord: glTF UV origin is top-left and texture uploads
                    // place the top image row at v=0, so V passes through
                    // unmodified (flipping would mirror the texture).
                    if (has_uvs && (v * 2 + 1) < uvs.size()) {
                        dst[6] = uvs[v * 2 + 0];
                        dst[7] = uvs[v * 2 + 1];
                    } else {
                        dst[6] = 0.0f; dst[7] = 0.0f;
                    }
                    // Joints
                    if (has_joints && (v * 4 + 3) < joints.size()) {
                        dst[8] = joints[v * 4 + 0];
                        dst[9] = joints[v * 4 + 1];
                        dst[10] = joints[v * 4 + 2];
                        dst[11] = joints[v * 4 + 3];
                    } else {
                        dst[8] = 0.0f; dst[9] = 0.0f; dst[10] = 0.0f; dst[11] = 0.0f;
                    }
                    // Weights (default all to joint 0)
                    if (has_weights && (v * 4 + 3) < weights.size()) {
                        dst[12] = weights[v * 4 + 0];
                        dst[13] = weights[v * 4 + 1];
                        dst[14] = weights[v * 4 + 2];
                        dst[15] = weights[v * 4 + 3];
                    } else {
                        dst[12] = 1.0f; dst[13] = 0.0f; dst[14] = 0.0f; dst[15] = 0.0f;
                    }
                }

                out_asset.primitives.push_back(std::move(primitive));
            }
        }
    }

    // ---------------------------------------------------------------- materials
    if (mats_j.type == JsonValue::Type::Array) {
        for (const auto& mj : mats_j.array) {
            GltfMaterial mat;
            mat.name = mj.get_string("name");
            const JsonValue& pbr = mj.at("pbrMetallicRoughness");
            const JsonValue& bcf = pbr.at("baseColorFactor");
            if (bcf.type == JsonValue::Type::Array && bcf.array.size() >= 4) {
                mat.base_color_factor = math::Vec4(
                    static_cast<float>(bcf.array[0].number),
                    static_cast<float>(bcf.array[1].number),
                    static_cast<float>(bcf.array[2].number),
                    static_cast<float>(bcf.array[3].number));
            }
            const JsonValue& bct = pbr.at("baseColorTexture");
            if (bct.type == JsonValue::Type::Object) {
                mat.base_color_texture = bct.get_int("index", -1);
            }
            mat.metallic_factor = static_cast<float>(pbr.get_number("metallicFactor", 1.0));
            mat.roughness_factor = static_cast<float>(pbr.get_number("roughnessFactor", 1.0));
            mat.double_sided = mj.get_bool("doubleSided", false);
            out_asset.materials.push_back(std::move(mat));
        }
    }

    // ------------------------------------------------------------------ images
    if (imgs_j.type == JsonValue::Type::Array) {
        for (const auto& ij : imgs_j.array) {
            GltfImage img;
            img.name = ij.get_string("name");
            img.mime_type = ij.get_string("mimeType");
            const JsonValue* bv = ij.find("bufferView");
            if (bv && bv->type == JsonValue::Type::Number) {
                uint32_t bvi = static_cast<uint32_t>(bv->number);
                if (bvi < doc.buffer_views.size()) {
                    const BufferView& view = doc.buffer_views[bvi];
                    if (view.buffer == 0 &&
                        view.offset + view.length <= doc.bin_chunk.size()) {
                        img.data.assign(doc.bin_chunk.begin() + view.offset,
                                        doc.bin_chunk.begin() + view.offset + view.length);
                    }
                }
            }
            out_asset.images.push_back(std::move(img));
        }
    }

    // ------------------------------------------------------------------- skins
    if (skins_j.type == JsonValue::Type::Array) {
        for (const auto& sj : skins_j.array) {
            GltfSkin skin;
            skin.name = sj.get_string("name");
            const JsonValue& joints = sj.at("joints");
            if (joints.type == JsonValue::Type::Array) {
                for (const auto& j : joints.array) {
                    if (j.type == JsonValue::Type::Number) {
                        skin.joints.push_back(static_cast<int32_t>(j.number));
                    }
                }
            }
            const JsonValue& ibm = sj.at("inverseBindMatrices");
            if (ibm.type == JsonValue::Type::Number) {
                read_accessor_matrices(doc, static_cast<uint32_t>(ibm.number),
                                       skin.inverse_bind_matrices);
            }
            // Pad IBM to joint count with identity if missing
            while (skin.inverse_bind_matrices.size() < skin.joints.size()) {
                skin.inverse_bind_matrices.push_back(math::Mat4::identity());
            }
            out_asset.skins.push_back(std::move(skin));
        }
    }

    // -------------------------------------------------------------- animations
    if (anims_j.type == JsonValue::Type::Array) {
        for (const auto& aj : anims_j.array) {
            GltfAnimation anim;
            anim.name = aj.get_string("name");

            const JsonValue& samplers = aj.at("samplers");
            const JsonValue& channels = aj.at("channels");

            // Pre-decode samplers
            struct SamplerData {
                std::vector<float> times;
                std::vector<float> values;
                GltfAnimInterp interp = GltfAnimInterp::Linear;
                uint32_t stride = 0;
            };
            std::vector<SamplerData> sampler_data;
            if (samplers.type == JsonValue::Type::Array) {
                for (const auto& smp : samplers.array) {
                    SamplerData sd;
                    const JsonValue* input = smp.find("input");
                    const JsonValue* output = smp.find("output");
                    if (input && output &&
                        input->type == JsonValue::Type::Number &&
                        output->type == JsonValue::Type::Number) {
                        read_accessor_floats(doc, static_cast<uint32_t>(input->number), sd.times);
                        read_accessor_floats(doc, static_cast<uint32_t>(output->number), sd.values);
                        std::string interp = smp.get_string("interpolation", "LINEAR");
                        if (interp == "STEP") sd.interp = GltfAnimInterp::Step;
                        else if (interp == "CUBIC_SPLINE") sd.interp = GltfAnimInterp::CubicSpline;
                        // stride derived from values/times ratio
                        if (!sd.times.empty()) {
                            sd.stride = static_cast<uint32_t>(sd.values.size() / sd.times.size());
                        }
                    }
                    sampler_data.push_back(std::move(sd));
                }
            }

            if (channels.type == JsonValue::Type::Array) {
                for (const auto& ch : channels.array) {
                    GltfAnimChannel channel;
                    const JsonValue& target = ch.at("target");
                    channel.node = target.get_int("node", -1);
                    std::string path = target.get_string("path");
                    if (path == "translation") channel.path = GltfAnimPath::Translation;
                    else if (path == "rotation") channel.path = GltfAnimPath::Rotation;
                    else if (path == "scale") channel.path = GltfAnimPath::Scale;
                    else channel.path = GltfAnimPath::Weights;

                    const JsonValue* sampler_idx = ch.find("sampler");
                    if (sampler_idx && sampler_idx->type == JsonValue::Type::Number) {
                        uint32_t si = static_cast<uint32_t>(sampler_idx->number);
                        if (si < sampler_data.size()) {
                            channel.times = std::move(sampler_data[si].times);
                            channel.values = std::move(sampler_data[si].values);
                            channel.interp = sampler_data[si].interp;
                            channel.value_stride = sampler_data[si].stride;

                            // CUBIC_SPLINE stores in-tangent, value, out-tangent
                            // per key (3x elements). The sampler only supports
                            // linear/step, so compact to the value keys and
                            // degrade to linear instead of indexing tangents.
                            if (channel.interp == GltfAnimInterp::CubicSpline &&
                                !channel.times.empty() && channel.value_stride > 0) {
                                uint32_t keys = static_cast<uint32_t>(channel.times.size());
                                uint32_t cubic_stride = channel.value_stride;  // 3 * components
                                uint32_t comps = cubic_stride / 3;
                                if (comps > 0 && cubic_stride == comps * 3 &&
                                    channel.values.size() >= static_cast<size_t>(keys) * cubic_stride) {
                                    std::vector<float> compact;
                                    compact.reserve(static_cast<size_t>(keys) * comps);
                                    for (uint32_t k = 0; k < keys; ++k) {
                                        const float* key = channel.values.data() +
                                            static_cast<size_t>(k) * cubic_stride + comps;
                                        compact.insert(compact.end(), key, key + comps);
                                    }
                                    channel.values = std::move(compact);
                                    channel.value_stride = comps;
                                    channel.interp = GltfAnimInterp::Linear;
                                }
                            }
                        }
                    }
                    if (!channel.times.empty() && channel.node >= 0) {
                        if (channel.times.back() > anim.duration) {
                            anim.duration = channel.times.back();
                        }
                        anim.channels.push_back(std::move(channel));
                    }
                }
            }
            if (anim.duration <= 0.0f && !anim.channels.empty()) {
                // Fallback: use last time of first channel
                anim.duration = anim.channels[0].times.back();
            }
            out_asset.animations.push_back(std::move(anim));
        }
    }

    // ------------------------------------------------------------------ scene
    if (scenes_j.type == JsonValue::Type::Array && !scenes_j.array.empty()) {
        int32_t scene_index = scene_j.type == JsonValue::Type::Number
                                  ? scene_j.get_int("scene", 0) : 0;
        if (scene_index < 0 || scene_index >= static_cast<int32_t>(scenes_j.array.size())) {
            scene_index = 0;
        }
        const JsonValue& scene = scenes_j.array[scene_index];
        const JsonValue& roots = scene.at("nodes");
        if (roots.type == JsonValue::Type::Array) {
            for (const auto& r : roots.array) {
                if (r.type == JsonValue::Type::Number) {
                    out_asset.scene_roots.push_back(static_cast<int32_t>(r.number));
                }
            }
        }
    }
    // Fallback: all parentless nodes are roots
    if (out_asset.scene_roots.empty()) {
        for (size_t i = 0; i < out_asset.nodes.size(); ++i) {
            if (out_asset.nodes[i].parent < 0) {
                out_asset.scene_roots.push_back(static_cast<int32_t>(i));
            }
        }
    }

    delete doc.root;
    doc.root = nullptr;

    if (out_asset.primitives.empty()) {
        last_error_ = "No valid mesh primitives found";
        return GltfLoaderError::InvalidFile;
    }

    last_error_.clear();
    return GltfLoaderError::Success;
}

const char* GltfLoader::get_last_error() const {
    return last_error_.c_str();
}

// =========================================================================
// GltfPrimitive::make_mesh_desc
// =========================================================================

MeshDesc GltfPrimitive::make_mesh_desc() const {
    static const VertexAttribute kAttributes[5] = {
        {VertexSemantic::Position, VertexFormat::Float3, 0,  0, 0, VertexAttribute::kAutoAttributeIndex},
        {VertexSemantic::Normal,   VertexFormat::Float3, 12, 0, 0, VertexAttribute::kAutoAttributeIndex},
        {VertexSemantic::TexCoord, VertexFormat::Float2, 24, 0, 0, VertexAttribute::kAutoAttributeIndex},
        {VertexSemantic::Joints,   VertexFormat::Float4, 32, 0, 0, VertexAttribute::kAutoAttributeIndex},
        {VertexSemantic::Weights,  VertexFormat::Float4, 48, 0, 0, VertexAttribute::kAutoAttributeIndex},
    };
    // Stride is 64 bytes (16 floats): pos3 + norm3 + uv2 + joints4 + weights4.
    // The vertex buffer packs 16 floats per vertex.
    static VertexLayout layout;
    layout.attributes = kAttributes;
    layout.attribute_count = 5;
    layout.stride = 64;

    MeshDesc desc;
    desc.vertex_data = vertices.data();
    desc.vertex_count = vertex_count;
    desc.vertex_stride = 64;
    desc.index_data = indices.data();
    desc.index_count = static_cast<uint32_t>(indices.size());
    desc.index_format = IndexFormat::UInt32;
    desc.layout = &layout;
    desc.name = name.c_str();
    return desc;
}

} // namespace poko::render
