#include "poko/core/EntityId.hpp"
#include <cassert>
#include <unordered_map>

using namespace poko::core;

void test_entity_id_default() {
    EntityId id;
    assert(!id.is_valid());
    assert(id.index() == 0);
    assert(id.generation() == 0);
}

void test_entity_id_construct() {
    EntityId id(5, 10);
    assert(id.is_valid());
    assert(id.index() == 5);
    assert(id.generation() == 10);
}

void test_entity_id_equality() {
    EntityId id1(5, 10);
    EntityId id2(5, 10);
    EntityId id3(5, 11);
    
    assert(id1 == id2);
    assert(id1 != id3);
}

void test_entity_id_comparison() {
    EntityId id1(5, 10);
    EntityId id2(6, 10);
    EntityId id3(5, 11);
    
    assert(id1 < id2);
    assert(id1 < id3);
}

void test_entity_id_hash() {
    EntityId id1(5, 10);
    EntityId id2(5, 10);
    
    assert(id1.hash() == id2.hash());
}

void test_entity_id_unordered_map() {
    std::unordered_map<EntityId, int> map;
    EntityId id1(1, 1);
    EntityId id2(2, 2);
    
    map[id1] = 100;
    map[id2] = 200;
    
    assert(map[id1] == 100);
    assert(map[id2] == 200);
}

void test_entity_id_invalid_static() {
    EntityId id = EntityId::invalid();
    assert(!id.is_valid());
}
