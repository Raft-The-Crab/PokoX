# Poko Object Model

The object model provides the core Workspace → Model → Part hierarchy for the Poko Engine, with:
- Stable object parenting with cycle prevention
- Lifecycle events (create, destroy, reparent, rename, child add/remove)
- Hierarchy traversal (depth-first, breadth-first, path)
- Integrity validation
- Serialization/deserialization
- Integration with the IDs/handles system

## Core Concepts

### Object Types

- **Workspace**: Top-level container for models. Workspaces have no parent.
- **Model**: Container for parts within a workspace. Models belong to workspaces.
- **Part**: Individual object within a model. Parts belong to models or other supported parents.

### Object Data

All objects share common data through `ObjectData`:
- `id`: Stable UUID-based identifier with type
- `object_type`: The object's type (Workspace, Model, Part)
- `parent`: Optional parent ID
- `children`: List of child IDs
- `name`: Human-readable name
- `destroyed`: Whether the object has been destroyed

### Handles and IDs

- **ID**: Stable UUID-based identifier for serialization and persistence
- **Handle**: Generation-safe runtime handle for memory access
- The registry maintains an ID-to-handle mapping for fast lookup

## Usage Examples

### Basic Object Creation

```rust
use poko_object_model::ObjectRegistry;

let mut registry = ObjectRegistry::new();

// Create a workspace
let workspace = registry.create_workspace("MyWorkspace".to_string()).unwrap();

// Create a model under the workspace
let model = registry.create_model(workspace.data.id, "MyModel".to_string()).unwrap();

// Create parts under the model
let part1 = registry.create_part(model.data.id, "Part1".to_string()).unwrap();
let part2 = registry.create_part(model.data.id, "Part2".to_string()).unwrap();
```

### Parenting and Reparenting

```rust
// Reparent a part to a different model
let model2 = registry.create_model(workspace.data.id, "Model2".to_string()).unwrap();
registry.reparent(part1.data.id, Some(model2.data.id)).unwrap();

// Cycle detection is automatic - this will return an error:
let result = registry.reparent(workspace.data.id, Some(part1.data.id));
assert!(result.is_err()); // Cycle would be created
```

### Hierarchy Traversal

```rust
// Depth-first traversal
registry.traverse_depth_first(workspace.data.id, |obj| {
    println!("Visiting: {}", obj.data().name);
    true // Return true to continue traversal
}).unwrap();

// Breadth-first traversal
registry.traverse_breadth_first(workspace.data.id, |obj| {
    println!("Visiting: {}", obj.data().name);
    true
}).unwrap();

// Get all descendants
let descendants = registry.get_descendants(model.data.id).unwrap();
println!("Model has {} descendants", descendants.len());

// Get all ancestors
let ancestors = registry.get_ancestors(part1.data.id).unwrap();
println!("Part has {} ancestors", ancestors.len());

// Get path from root to object
let path = registry.get_path(part1.data.id).unwrap();
println!("Path: {:?}", path);
```

### Lifecycle Events

```rust
use std::sync::{Arc, Mutex};

let events = Arc::new(Mutex::new(Vec::new()));
let events_clone = Arc::clone(&events);

registry.add_event_listener(move |event| {
    events_clone.lock().unwrap().push(event);
});

// Events are emitted for create, destroy, reparent, rename, and child add/remove
let workspace = registry.create_workspace("Workspace".to_string()).unwrap();
let model = registry.create_model(workspace.data.id, "Model".to_string()).unwrap();

let events = events.lock().unwrap();
assert_eq!(events.len(), 2); // Created events for workspace and model
```

### Cascade Destruction

```rust
// Destroying an object also destroys all its descendants
registry.destroy(model.data.id).unwrap();

// The model and all its parts are now destroyed
assert!(registry.get_object(model.data.id).is_err());
```

### Validation

```rust
// Validate the entire hierarchy for integrity
let errors = registry.validate_hierarchy().unwrap();
if errors.is_empty() {
    println!("Hierarchy is valid");
} else {
    for error in errors {
        println!("Validation error: {}", error);
    }
}
```

### Serialization

```rust
// Serialize the entire hierarchy to JSON
let json = registry.serialize_hierarchy().unwrap();
println!("Serialized: {}", json);

// Deserialize into a new registry
let mut registry2 = ObjectRegistry::new();
registry2.deserialize_hierarchy(&json).unwrap();

// The hierarchy is now restored
let restored_workspace = registry2.get_object(workspace.data.id).unwrap();
assert_eq!(restored_workspace.data().name, "MyWorkspace");
```

## Error Handling

The object model uses `ObjectModelError` for all operations:

```rust
#[derive(Debug, Clone, PartialEq, Eq, Error)]
pub enum ObjectModelError {
    #[error("Parent not found: {0}")]
    ParentNotFound(Id),
    #[error("Cycle detected in hierarchy: {0} would create a cycle")]
    CycleDetected(Id),
    #[error("Object not found: {0}")]
    ObjectNotFound(Id),
    #[error("Registry error: {0}")]
    RegistryError(#[from] RegistryError),
    #[error("Invalid operation: {0}")]
    InvalidOperation(String),
}
```

## Performance Characteristics

- **Object creation**: O(1) allocation in HandleRegistry
- **Parent lookup**: O(1) via ID-to-handle map
- **Child enumeration**: O(k) where k is the number of children
- **Depth-first traversal**: O(n) where n is the number of descendants
- **Breadth-first traversal**: O(n) where n is the number of descendants
- **Cycle detection**: O(d) where d is the depth of the ancestor chain
- **Cascade destruction**: O(n) where n is the number of descendants
- **Serialization**: O(n) where n is the total number of objects
- **Deserialization**: O(n) where n is the total number of objects

## Testing

Run the test suite:

```bash
cargo test --package poko-object-model
```

Run benchmarks:

```bash
cargo bench --bench hierarchy
```

## Integration with IDs/Handles

The object model integrates with `poko-ids-handles` for:
- Stable typed IDs (Workspace, Model, Part)
- Generation-safe handles for lifecycle safety
- HandleRegistry for object storage
- ID-to-handle mapping for fast lookup

## Thread Safety

The `ObjectRegistry` is not thread-safe by default. For concurrent access, wrap it in a `Mutex` or `RwLock`:

```rust
use std::sync::Mutex;

let registry = Arc::new(Mutex::new(ObjectRegistry::new()));
```

## License

See the project LICENSE file.
