//! Concurrent access pattern tests
//!
//! Run with: cargo test --test concurrent -- --nocapture

use std::sync::{Arc, Barrier};
use std::thread;
use poko_ids_handles::{GenerationCounter, Id, IdType};

#[test]
fn test_generation_counter_concurrent() {
    let counter = Arc::new(GenerationCounter::new());
    let num_threads = 8;
    let increments_per_thread = 10_000;
    let barrier = Arc::new(Barrier::new(num_threads));

    let mut handles = Vec::new();

    for _ in 0..num_threads {
        let counter_clone = Arc::clone(&counter);
        let barrier_clone = Arc::clone(&barrier);

        let handle = thread::spawn(move || {
            barrier_clone.wait();

            let mut local_sum = 0u16;
            for _ in 0..increments_per_thread {
                local_sum = local_sum.wrapping_add(counter_clone.increment());
            }
            local_sum
        });

        handles.push(handle);
    }

    let results: Vec<_> = handles.into_iter().map(|h| h.join().unwrap()).collect();
    let _total_increments: u32 = results.iter().map(|&v| v as u32).sum();

    println!("Concurrent increments: {} threads × {} = {} total",
             num_threads, increments_per_thread, num_threads * increments_per_thread);
    println!("Final counter value: {}", counter.get());

    // Verify all threads completed
    assert_eq!(results.len(), num_threads);
}

#[test]
fn test_id_generation_concurrent() {
    use std::sync::Mutex;
    use std::collections::HashSet;

    let ids = Arc::new(Mutex::new(HashSet::new()));
    let num_threads = 8;
    let ids_per_thread = 1_000;
    let barrier = Arc::new(Barrier::new(num_threads));

    let mut handles = Vec::new();

    for _ in 0..num_threads {
        let ids_clone = Arc::clone(&ids);
        let barrier_clone = Arc::clone(&barrier);

        let handle = thread::spawn(move || {
            barrier_clone.wait();

            let mut local_ids = Vec::new();
            for _ in 0..ids_per_thread {
                local_ids.push(Id::new(IdType::Model));
            }

            let mut ids = ids_clone.lock().unwrap();
            for id in local_ids {
                ids.insert(id);
            }
        });

        handles.push(handle);
    }

    for handle in handles {
        handle.join().unwrap();
    }

    let ids = ids.lock().unwrap();
    println!("Generated {} unique IDs from {} threads", ids.len(), num_threads);

    // Verify all IDs are unique
    assert_eq!(ids.len(), num_threads * ids_per_thread);
}

#[test]
fn test_multiple_registries_concurrent() {
    use poko_ids_handles::HandleRegistry;

    let num_threads = 4;
    let items_per_thread = 5_000;
    let barrier = Arc::new(Barrier::new(num_threads));

    let mut handles = Vec::new();

    for _ in 0..num_threads {
        let barrier_clone = Arc::clone(&barrier);

        let handle = thread::spawn(move || {
            let mut registry = HandleRegistry::new();
            barrier_clone.wait();

            let start = std::time::Instant::now();
            let registry_handles: Vec<_> = (0..items_per_thread)
                .map(|i| registry.allocate(i as u32).unwrap())
                .collect();
            let duration = start.elapsed();

            let stats = registry.stats();
            (registry_handles.len(), duration, stats)
        });

        handles.push(handle);
    }

    let results: Vec<_> = handles.into_iter().map(|h| h.join().unwrap()).collect();

    println!("Concurrent registry operations:");
    for (i, (count, duration, stats)) in results.iter().enumerate() {
        println!("  Thread {}: {} items in {:?} (live={}, free={})",
                 i, count, duration, stats.live_count, stats.free_count);
    }

    // Verify all threads completed successfully
    assert_eq!(results.len(), num_threads);
    for (count, _, _) in results {
        assert_eq!(count, items_per_thread);
    }
}

#[test]
fn test_generation_pool_concurrent() {
    use poko_ids_handles::GenerationPool;
    use std::sync::Mutex;

    let pool = Arc::new(Mutex::new(GenerationPool::with_capacity(10_000)));
    let num_threads = 4;
    let items_per_thread = 2_500;
    let barrier = Arc::new(Barrier::new(num_threads));

    let mut handles = Vec::new();

    for _ in 0..num_threads {
        let pool_clone = Arc::clone(&pool);
        let barrier_clone = Arc::clone(&barrier);

        let handle = thread::spawn(move || {
            barrier_clone.wait();

            let mut generations = Vec::new();
            for _ in 0..items_per_thread {
                let mut pool = pool_clone.lock().unwrap();
                if let Some(gen) = pool.next() {
                    generations.push(gen);
                }
            }
            generations.len()
        });

        handles.push(handle);
    }

    let results: Vec<_> = handles.into_iter().map(|h| h.join().unwrap()).collect();
    let total: usize = results.iter().sum();

    println!("Concurrent pool operations: {} total generations retrieved", total);
    assert_eq!(total, num_threads * items_per_thread);
}

#[test]
fn test_id_serialization_concurrent() {
    use std::sync::Mutex;

    let serialized = Arc::new(Mutex::new(Vec::new()));
    let num_threads = 4;
    let ids_per_thread = 2_500;
    let barrier = Arc::new(Barrier::new(num_threads));

    let mut handles = Vec::new();

    for _ in 0..num_threads {
        let serialized_clone = Arc::clone(&serialized);
        let barrier_clone = Arc::clone(&barrier);

        let handle = thread::spawn(move || {
            barrier_clone.wait();

            let mut local_strings = Vec::new();
            for _ in 0..ids_per_thread {
                let id = Id::new(IdType::Model);
                local_strings.push(id.to_string());
            }

            let mut serialized = serialized_clone.lock().unwrap();
            serialized.extend(local_strings);
        });

        handles.push(handle);
    }

    for handle in handles {
        handle.join().unwrap();
    }

    let serialized = serialized.lock().unwrap();
    println!("Concurrent serialization: {} IDs serialized", serialized.len());

    // Verify all IDs can be deserialized
    for s in serialized.iter() {
        let id = Id::parse_str(s).unwrap();
        assert!(id.is_type(IdType::Model));
    }

    assert_eq!(serialized.len(), num_threads * ids_per_thread);
}

#[test]
fn test_registry_isolation_concurrent() {
    use poko_ids_handles::HandleRegistry;

    // Verify that separate registries don't interfere
    let num_threads = 4;
    let barrier = Arc::new(Barrier::new(num_threads));

    let mut handles = Vec::new();

    for thread_id in 0..num_threads {
        let barrier_clone = Arc::clone(&barrier);

        let handle = thread::spawn(move || {
            let mut registry = HandleRegistry::new();
            barrier_clone.wait();

            // Each thread creates objects with its own values
            let start = thread_id * 1000;
            let end = start + 1000;

            for i in start..end {
                registry.allocate(i).unwrap();
            }

            // Verify no cross-contamination
            assert_eq!(registry.len(), 1000);
            assert!(!registry.contains(|&v| v < start || v >= end));

            registry.len()
        });

        handles.push(handle);
    }

    let results: Vec<_> = handles.into_iter().map(|h| h.join().unwrap()).collect();

    println!("Registry isolation test: {} threads", results.len());
    for (i, count) in results.iter().enumerate() {
        println!("  Thread {}: {} objects", i, count);
    }

    assert_eq!(results.len(), num_threads);
    for count in results {
        assert_eq!(count, 1000);
    }
}
