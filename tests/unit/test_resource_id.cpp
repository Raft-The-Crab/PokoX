#include "poko/core/ResourceId.hpp"
#include <cassert>
#include <unordered_map>

using namespace poko::core;

void test_resource_id_default() {
    ResourceId id;
    assert(!id.is_valid());
    assert(id.value() == 0);
}

void test_resource_id_construct() {
    ResourceId id(42);
    assert(id.is_valid());
    assert(id.value() == 42);
}

void test_resource_id_equality() {
    ResourceId id1(5);
    ResourceId id2(5);
    ResourceId id3(6);
    
    assert(id1 == id2);
    assert(id1 != id3);
}

void test_asset_id_type_safety() {
    ResourceId rid(1);
    AssetId aid(1);
    
    // These should compile but are different types
    assert(rid.value() == aid.value());
    
    // Assignment should not compile (type safety)
    // rid = aid; // This would be a compile error
}

void test_texture_id() {
    TextureId tid(100);
    assert(tid.is_valid());
    assert(tid.value() == 100);
}

void test_mesh_id() {
    MeshId mid(200);
    assert(mid.is_valid());
    assert(mid.value() == 200);
}

void test_audio_id() {
    AudioId auid(300);
    assert(auid.is_valid());
    assert(auid.value() == 300);
}

void test_resource_id_unordered_map() {
    std::unordered_map<ResourceId, int> map;
    ResourceId id1(1);
    ResourceId id2(2);
    
    map[id1] = 100;
    map[id2] = 200;
    
    assert(map[id1] == 100);
    assert(map[id2] == 200);
}

void test_resource_id_invalid_static() {
    ResourceId id = ResourceId::invalid();
    assert(!id.is_valid());
}
