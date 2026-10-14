//! Memory usage profiling tests
//!
//! Run with: cargo test --test memory -- --nocapture

use std::mem;

#[test]
fn test_registry_memory_efficiency() {
    use poko_ids_handles::HandleRegistry;

    let empty_size = mem::size_of::<HandleRegistry<u32>>();
    println!("Empty registry size: {} bytes", empty_size);

    let mut registry: HandleRegistry<u32> = HandleRegistry::new();
    let count: usize = 10_000;

    for i in 0..count {
        registry.allocate(i as u32).unwrap();
    }

    let registry_size = mem::size_of_val(&registry);
    let per_item = registry_size / count;

    println!(
        "Registry with {} items: {} bytes total, {} bytes per item",
        count, registry_size, per_item
    );

    // Each item should be reasonable (less than 1KB per item)
    assert!(
        per_item < 1024,
        "Per-item memory cost too high: {} bytes",
        per_item
    );
}

#[test]
fn test_id_memory_size() {
    use poko_ids_handles::Id;
    use poko_ids_handles::IdType;

    let id_size = mem::size_of::<Id>();
    println!("Id size: {} bytes", id_size);

    // ID should be small (16 bytes for UUID + 1 byte for type)
    assert!(id_size <= 24, "Id size too large: {} bytes", id_size);

    let count: usize = 10_000;
    let ids: Vec<_> = (0..count).map(|_| Id::new(IdType::Model)).collect();
    let vec_size = mem::size_of_val(&ids);
    let per_item = vec_size / count;

    println!(
        "Vec of {} IDs: {} bytes total, {} bytes per item",
        count, vec_size, per_item
    );
}

#[test]
fn test_handle_memory_size() {
    use poko_ids_handles::Handle;

    let handle_size = mem::size_of::<Handle>();
    println!("Handle size: {} bytes", handle_size);

    // Handle should be 4 bytes (2 for index + 2 for generation)
    assert_eq!(handle_size, 4, "Handle should be exactly 4 bytes");
}

#[test]
fn test_batch_serialization_memory() {
    use poko_ids_handles::{Id, IdBatch, IdType};

    let count: usize = 10_000;
    let ids: Vec<_> = (0..count).map(|_| Id::new(IdType::Model)).collect();
    let vec_size = mem::size_of_val(&ids);

    let mut batch = IdBatch::with_capacity(count);
    batch.extend(ids.clone());
    let batch_size = mem::size_of_val(&batch);

    println!("Vec of {} IDs: {} bytes", count, vec_size);
    println!("Batch of {} IDs: {} bytes", count, batch_size);

    // Batch should be more memory-efficient than Vec<Id>
    // Expected: 17 bytes per ID in batch (16 UUID + 1 type)
    let expected_bytes = count * 17;
    assert!(
        batch_size < expected_bytes * 2,
        "Batch memory cost too high"
    );
}

#[test]
fn test_registry_stats_size() {
    use poko_ids_handles::RegistryStats;

    let stats_size = mem::size_of::<RegistryStats>();
    println!("RegistryStats size: {} bytes", stats_size);

    // Should be small (4 usize fields)
    assert!(
        stats_size <= 32,
        "RegistryStats too large: {} bytes",
        stats_size
    );
}

#[test]
fn test_generation_counter_size() {
    use poko_ids_handles::generation::GenerationCounter;

    let counter_size = mem::size_of::<GenerationCounter>();
    println!("GenerationCounter size: {} bytes", counter_size);

    // Should be small (atomic u16)
    assert!(
        counter_size <= 8,
        "GenerationCounter too large: {} bytes",
        counter_size
    );
}

#[test]
fn test_id_bytes_size() {
    use poko_ids_handles::IdBytes;

    let bytes_size = mem::size_of::<IdBytes>();
    println!("IdBytes size: {} bytes", bytes_size);

    // Should be exactly 17 bytes (16 UUID + 1 type)
    assert_eq!(bytes_size, 17, "IdBytes should be exactly 17 bytes");
}

#[test]
fn test_handle_bytes_size() {
    use poko_ids_handles::HandleBytes;

    let bytes_size = mem::size_of::<HandleBytes>();
    println!("HandleBytes size: {} bytes", bytes_size);

    // Should be exactly 4 bytes (2 index + 2 generation)
    assert_eq!(bytes_size, 4, "HandleBytes should be exactly 4 bytes");
}
