#include "poko/core/EntityPool.hpp"
#include <cassert>
#include <vector>

using namespace poko::core;

void test_entity_pool_allocate() {
    EntityPool pool;
    
    EntityId id1 = pool.allocate();
    assert(id1.is_valid());
    assert(pool.is_alive(id1));
    assert(pool.alive_count() == 1);
}

void test_entity_pool_deallocate() {
    EntityPool pool;
    
    EntityId id1 = pool.allocate();
    assert(pool.is_alive(id1));
    
    bool deallocated = pool.deallocate(id1);
    assert(deallocated);
    assert(!pool.is_alive(id1));
    assert(pool.alive_count() == 0);
}

void test_entity_pool_stale_handle() {
    EntityPool pool;
    
    EntityId id1 = pool.allocate();
    (void)pool.allocate(); // Allocate another to test pool behavior
    
    pool.deallocate(id1);
    
    // id1 should now be stale
    assert(!pool.is_alive(id1));
    
    // Allocate a new entity - it should reuse the slot with new generation
    EntityId id3 = pool.allocate();
    assert(id3.index() == id1.index());
    assert(id3.generation() != id1.generation());
}

void test_entity_pool_capacity() {
    EntityPool pool;
    
    std::vector<EntityId> ids;
    for (int i = 0; i < 100; ++i) {
        EntityId id = pool.allocate();
        assert(id.is_valid());
        ids.push_back(id);
    }
    
    assert(pool.alive_count() == 100);
    
    for (auto id : ids) {
        pool.deallocate(id);
    }
    
    assert(pool.alive_count() == 0);
}

void test_entity_pool_full() {
    EntityPool pool;
    
    // The pool has a large capacity, so we won't actually fill it in this test
    // Just verify the API works
    assert(!pool.is_full());
    assert(pool.free_count() > 0);
}

void test_entity_pool_double_deallocate() {
    EntityPool pool;
    
    EntityId id1 = pool.allocate();
    
    bool first = pool.deallocate(id1);
    assert(first);
    
    bool second = pool.deallocate(id1);
    assert(!second); // Should fail - already deallocated
}
