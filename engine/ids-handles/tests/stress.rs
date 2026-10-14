//! Stress tests for large-scale operations
//!
//! Run with: cargo test --test stress -- --nocapture

use poko_ids_handles::{HandleRegistry, Id, IdType};

#[test]
fn test_large_scale_allocation() {
    let mut registry: HandleRegistry<u32> = HandleRegistry::new();
    let count: usize = 50_000;

    let start = std::time::Instant::now();
    let handles: Vec<_> = (0..count)
        .map(|i| registry.allocate(i as u32).unwrap())
        .collect();
    let duration = start.elapsed();

    println!("Allocated {} handles in {:?}", count, duration);
    assert_eq!(handles.len(), count);
    assert_eq!(registry.len(), count);

    // Verify all handles are valid
    for handle in &handles {
        assert!(registry.is_valid(*handle));
    }
}

#[test]
fn test_large_scale_destruction_reuse() {
    let mut registry: HandleRegistry<u32> = HandleRegistry::new();
    let count: usize = 10_000;

    // Allocate
    let handles: Vec<_> = (0..count)
        .map(|_| registry.allocate(42).unwrap())
        .collect();

    // Remove every other
    let start = std::time::Instant::now();
    for (i, handle) in handles.iter().enumerate() {
        if i % 2 == 0 {
            registry.remove(*handle).unwrap();
        }
    }
    let duration = start.elapsed();

    println!("Removed {} handles in {:?}", count / 2, duration);
    assert_eq!(registry.len(), count / 2);

    // Reallocate to fill free slots
    let start = std::time::Instant::now();
    for _ in 0..(count / 2) {
        registry.allocate(99).unwrap();
    }
    let duration = start.elapsed();

    println!("Reallocated {} handles in {:?}", count / 2, duration);
    assert_eq!(registry.len(), count);
}

#[test]
fn test_generation_overflow_stress() {
    let mut registry: HandleRegistry<u32> = HandleRegistry::new();

    // Simulate many removal cycles to test generation wrapping
    let cycles = 1000;
    let start = std::time::Instant::now();

    for _ in 0..cycles {
        let handle = registry.allocate(42).unwrap();
        registry.remove(handle).unwrap();
        let new_handle = registry.allocate(99).unwrap();
        assert!(!registry.is_valid(handle));
        assert!(registry.is_valid(new_handle));
        registry.remove(new_handle).unwrap();
    }

    let duration = start.elapsed();
    println!("Completed {} generation cycles in {:?}", cycles, duration);
}

#[test]
fn test_id_batch_serialization_stress() {
    use poko_ids_handles::IdBatch;

    let count: usize = 50_000;
    let ids: Vec<_> = (0..count)
        .map(|_| Id::new(IdType::Model))
        .collect();

    let start = std::time::Instant::now();
    let mut batch = IdBatch::with_capacity(count);
    batch.extend(ids.clone());
    let duration = start.elapsed();

    println!("Batched {} IDs in {:?}", count, duration);
    assert_eq!(batch.len(), count);

    // Deserialize
    let start = std::time::Instant::now();
    let ids_out = batch.to_ids();
    let duration = start.elapsed();

    println!("Converted batch to {} IDs in {:?}", ids_out.len(), duration);
    assert_eq!(ids_out.len(), count);
}

#[test]
fn test_registry_capacity_stress() {
    let mut registry: HandleRegistry<u32> = HandleRegistry::new();
    let count: usize = 50_000;

    // Allocate up to high count
    let handles: Vec<_> = (0..count)
        .map(|_| registry.allocate(42).unwrap())
        .collect();

    println!("Successfully allocated {} handles", handles.len());
    assert_eq!(registry.len(), count);

    // Check stats
    let stats = registry.stats();
    println!("Registry stats: total_slots={}, live_count={}", stats.total_slots, stats.live_count);
}

#[test]
fn test_find_all_stress() {
    let mut registry: HandleRegistry<u32> = HandleRegistry::new();
    let count: usize = 20_000;

    // Allocate with pattern: even numbers
    for i in 0..count {
        registry.allocate(i as u32).unwrap();
    }

    let start = std::time::Instant::now();
    let even_handles = registry.find_all(|&v| v % 2 == 0);
    let duration = start.elapsed();

    println!("Found {} even values in {:?}", even_handles.len(), duration);
    assert_eq!(even_handles.len(), count / 2);
}

#[test]
fn test_retain_stress() {
    let mut registry: HandleRegistry<u32> = HandleRegistry::new();
    let count: usize = 20_000;

    for i in 0..count {
        registry.allocate(i as u32).unwrap();
    }

    let start = std::time::Instant::now();
    let removed = registry.retain(|&v| v % 2 == 0);
    let duration = start.elapsed();

    println!("Retained {} values, removed {} in {:?}", count / 2, removed.len(), duration);
    assert_eq!(removed.len(), count / 2);
    assert_eq!(registry.len(), count / 2);
}

#[test]
fn test_stats_tracking_stress() {
    let mut registry: HandleRegistry<u32> = HandleRegistry::new();
    let count: usize = 10_000;

    let handles: Vec<_> = (0..count)
        .map(|i| registry.allocate(i as u32).unwrap())
        .collect();

    let stats = registry.stats();
    assert_eq!(stats.live_count, count);
    assert_eq!(stats.free_count, 0);

    // Remove half
    for i in 0..(count / 2) {
        let _ = registry.remove(handles[i]);
    }

    let stats = registry.stats();
    println!("Stats after removal: live={}, free={}", stats.live_count, stats.free_count);
    assert!(stats.free_count > 0);
}

#[test]
fn test_concurrent_ids_creation() {
    use std::sync::{Arc, Mutex};
    use std::thread;

    let ids = Arc::new(Mutex::new(Vec::new()));
    let count = 1000;
    let threads = 4;

    let start = std::time::Instant::now();

    let mut handles = Vec::new();
    for _ in 0..threads {
        let ids_clone = Arc::clone(&ids);
        let handle = thread::spawn(move || {
            let mut local_ids = Vec::new();
            for _ in 0..count {
                local_ids.push(Id::new(IdType::Workspace));
            }
            let mut ids = ids_clone.lock().unwrap();
            ids.extend(local_ids);
        });
        handles.push(handle);
    }

    for handle in handles {
        handle.join().unwrap();
    }

    let duration = start.elapsed();
    let ids = ids.lock().unwrap();

    println!("Created {} IDs in {} threads in {:?}", ids.len(), threads, duration);
    assert_eq!(ids.len(), count * threads);

    // Verify all IDs are unique
    let unique: std::collections::HashSet<_> = ids.iter().collect();
    assert_eq!(unique.len(), ids.len());
}
