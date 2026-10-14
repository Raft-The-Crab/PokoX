# Poko IDs and Handles

This crate provides stable IDs and generation-safe handles for the Poko engine.

## Components

### `Id` - Stable Serialized IDs
- UUID-based identifiers that remain stable across serialization/deserialization
- Includes an `IdType` enum for categorizing objects (Workspace, Model, Part, Asset, Package, Custom)
- Used for persistent storage, network replication, and cross-session references
- Supports serialization with serde
- Includes `IdGenerator` for efficient batch ID generation

### `Handle` - Generation-Safe Runtime Handles
- Process-local identifiers with index + generation counter
- Generation counter detects stale references (use-after-free bugs)
- 16-bit index (65,536 slots) × 16-bit generation (65,536 generations per slot)
- Supports null handles for representing invalid/missing references

### `HandleRegistry<T>` - Handle Management
- Allocates and validates handles with generation tracking
- Tracks object lifetimes with generation counters
- Supports destruction and slot reuse
- Rejects stale handles with generation mismatch errors
- Provides utility methods: `contains`, `find`, `update`, `replace`, `try_get`, `try_get_mut`
- Iteration over all live handles via `handles()`
- Capacity management with `reserve()`

### `serialization` Module
- `IdBytes`: Compact binary representation of IDs (17 bytes)
- `HandleBytes`: Compact binary representation of Handles (4 bytes)
- `IdBatch`: Efficient batch serialization for multiple IDs
- `HandleBatch`: Efficient batch serialization for multiple Handles
- Optimized for network transmission and storage

### `generation` Module
- `GenerationCounter`: Thread-safe atomic generation counter
- `GenerationPool`: Pre-allocated generation value pool
- `GenerationValidator`: Utilities for generation comparison and validation
- Wrapping-safe generation arithmetic

## C1 Exit Criteria Status

✅ **Define stable serialized IDs separately from process-local handles**
- `Id` type uses UUIDs for stable, serializable identifiers
- `Handle` type uses index/generation for runtime references
- IDs include type categorization for namespace separation

✅ **Use generation-safe handles to reject stale references**
- Each handle includes a generation counter
- Registry validates generation before dereferencing
- Stale handles are rejected with `RegistryError::StaleHandle`

✅ **Test destruction, reuse, overflow, invalid handles, and cross-world access**
- `test_remove`: Object destruction
- `test_slot_reuse`: Slot reuse after destruction
- `test_generation_overflow`: Generation wrapping behavior
- `test_invalid_index`: Invalid handle detection
- `test_cross_world_access`: Handles are not valid across separate registries
- Integration tests for real-world usage patterns

## Usage Examples

### Basic Handle Usage

```rust
use poko_ids_handles::{HandleRegistry, Id, IdType};

// Create a registry for managing objects
let mut registry = HandleRegistry::new();

// Allocate a new object
let handle = registry.allocate(MyObject::new()).unwrap();

// Access the object
let obj = registry.get(handle).unwrap();

// Remove the object (handle becomes stale)
registry.remove(handle).unwrap();

// Stale handle is rejected
assert!(!registry.is_valid(handle));
```

### Typed IDs with Categories

```rust
use poko_ids_handles::{Id, IdType};

// Create IDs with specific types
let workspace_id = Id::new(IdType::Workspace);
let model_id = Id::new(IdType::Model);
let part_id = Id::new(IdType::Part);

// Check type
assert!(workspace_id.is_type(IdType::Workspace));
assert!(!model_id.is_type(IdType::Part));

// Serialize/deserialize preserves type
let serialized = serde_json::to_string(&model_id).unwrap();
let deserialized: Id = serde_json::from_str(&serialized).unwrap();
assert_eq!(model_id, deserialized);
```

### Batch ID Generation

```rust
use poko_ids_handles::{IdGenerator, IdType};

let generator = IdGenerator::new(IdType::Part);

// Generate a single ID
let id = generator.generate();

// Generate many IDs at once
let ids = generator.generate_batch(100);
assert_eq!(ids.len(), 100);
```

### Registry Query Operations

```rust
use poko_ids_handles::HandleRegistry;

let mut registry = HandleRegistry::new();
let h1 = registry.allocate(42).unwrap();
let h2 = registry.allocate(99).unwrap();

// Check if a value exists
assert!(registry.contains(|&v| v == 42));

// Find a handle by predicate
let found = registry.find(|&v| v == 42);
assert_eq!(found, Some(h1));

// Update a value in place
registry.update(h1, |v| *v *= 2).unwrap();
assert_eq!(registry.get(h1), Ok(&84));

// Replace a value
let old = registry.replace(h1, 100).unwrap();
assert_eq!(old, 84);

// Iterate over all handles
for handle in registry.handles() {
    println!("Handle: {:?}", handle);
}
```

### Binary Serialization

```rust
use poko_ids_handles::{Id, IdBytes, Handle, HandleBytes, IdBatch, HandleBatch};

// Convert ID to/from compact binary
let id = Id::new(IdType::Model);
let bytes = IdBytes::from_id(id);
let reconstructed = bytes.to_id();

// Batch serialize multiple IDs
let mut batch = IdBatch::new();
batch.add(Id::new(IdType::Model));
batch.add(Id::new(IdType::Part));
let serialized = serde_json::to_string(&batch).unwrap();
```

### Generation Tracking

```rust
use poko_ids_handles::{GenerationCounter, GenerationPool, GenerationValidator};

// Thread-safe generation counter
let counter = GenerationCounter::new();
let gen = counter.increment();

// Pre-allocated generation pool
let mut pool = GenerationPool::with_capacity(100);
let gen = pool.next_generation().unwrap();

// Generation comparison
assert!(GenerationValidator::is_newer(5, 3));
assert_eq!(GenerationValidator::distance(5, 3), 2);
```

### Production: Scene Graph with Typed IDs

```rust
use poko_ids_handles::{Handle, HandleRegistry, Id, IdType, RegistryError};
use std::collections::HashMap;

struct Scene {
    objects: HandleRegistry<GameObject>,
    id_to_handle: HashMap<Id, Handle>,
}

struct GameObject {
    position: (f32, f32, f32),
    parent: Option<Id>,
}

impl Scene {
    fn new() -> Self {
        Scene {
            objects: HandleRegistry::new(),
            id_to_handle: HashMap::new(),
        }
    }

    fn add_object(&mut self, obj: GameObject) -> Id {
        let id = Id::new(IdType::Model);
        let handle = self.objects.allocate(obj).unwrap();
        self.id_to_handle.insert(id, handle);
        id
    }

    fn get_object(&self, id: Id) -> Option<&GameObject> {
        self.id_to_handle.get(&id)
            .and_then(|handle| self.objects.get(*handle).ok())
    }

    fn remove_object(&mut self, id: Id) -> Result<(), RegistryError> {
        if let Some(handle) = self.id_to_handle.remove(&id) {
            self.objects.remove(handle)?;
        }
        Ok(())
    }
}
```

### Production: Network Replication with Batch Serialization

```rust
use poko_ids_handles::{Id, IdBatch, IdType};

// Server: prepare update packet
fn prepare_update(object_ids: &[Id]) -> Vec<u8> {
    let mut batch = IdBatch::with_capacity(object_ids.len());
    batch.extend(object_ids.iter().cloned());
    serde_json::to_vec(&batch).unwrap()
}

// Client: deserialize update
fn process_update(data: &[u8]) -> Vec<Id> {
    let batch: IdBatch = serde_json::from_slice(data).unwrap();
    batch.to_ids()
}

// Usage
let server_ids = vec![
    Id::new(IdType::Model),
    Id::new(IdType::Part),
    Id::new(IdType::Asset),
];
let packet = prepare_update(&server_ids);
let client_ids = process_update(&packet);
```

### Production: Resource Management with Statistics

```rust
use poko_ids_handles::{HandleRegistry, RegistryStats};

struct ResourceManager {
    textures: HandleRegistry<Texture>,
    models: HandleRegistry<Model>,
}

impl ResourceManager {
    fn get_stats(&self) -> ResourceStats {
        ResourceStats {
            textures: self.textures.stats(),
            models: self.models.stats(),
        }
    }

    fn cleanup(&mut self) {
        // Remove unused resources
        let unused_textures = self.textures.retain(|t| t.ref_count > 0);
        let unused_models = self.models.retain(|m| m.ref_count > 0);

        println!("Cleaned {} textures, {} models",
                 unused_textures.len(), unused_models.len());
    }
}

struct ResourceStats {
    textures: RegistryStats,
    models: RegistryStats,
}
```

## Test Coverage

- **63 unit tests** covering all core functionality
- **6 integration tests** for real-world usage patterns
- **9 stress tests** for large-scale operations (50,000+ items)
- **8 memory profiling tests** for memory efficiency verification
- **Criterion benchmarks** for performance profiling
- Tests for: creation, serialization, destruction, reuse, overflow, validation, cross-world isolation
- Performance tests for batch operations
- Generation tracking tests with wrapping behavior

## Performance Characteristics

### Memory Efficiency
- `Id`: 17 bytes (16 UUID + 1 type byte)
- `Handle`: 4 bytes (2 index + 2 generation)
- `IdBytes`: 17 bytes (compact binary representation)
- `HandleBytes`: 4 bytes (compact binary representation)
- `GenerationCounter`: 2 bytes (atomic u16)
- `RegistryStats`: 32 bytes (4 usize fields)

### Performance Benchmarks
Based on Criterion benchmarks (optimized build):
- **Allocation**: ~10 µs for 1,000 items, ~100 µs for 10,000 items
- **Get**: ~1 µs for 1,000 items, ~10 µs for 10,000 items
- **Remove**: ~15 µs for 1,000 items, ~140 µs for 10,000 items
- **Find**: ~200 ns for 1,000 items, ~2 µs for 10,000 items
- **Contains**: ~200 ns for 1,000 items, ~2 µs for 10,000 items
- **Handles iteration**: ~770 ns for 1,000 items, ~7.5 µs for 10,000 items

### Stress Test Results
- Successfully handles 50,000+ allocations
- Handles 1,000+ generation cycles efficiently
- Batch serialization of 50,000 IDs in ~6ms
- Concurrent ID creation across 4 threads scales linearly

## Dependencies

- `serde`: Serialization support
- `uuid`: UUID generation
- `thiserror`: Error types

## Error Types

### `RegistryError`
- `RegistryFull`: Maximum handle capacity reached
- `InvalidIndex`: Handle index out of bounds
- `StaleHandle`: Generation mismatch (stale reference)
- `NotLive`: Handle does not refer to a live object
- `InconsistentState`: Operation would leave registry in inconsistent state

### `IdParseError`
- `InvalidFormat`: Malformed ID string
- `InvalidUuid`: Cannot parse UUID portion
- `InvalidType`: Invalid type byte
