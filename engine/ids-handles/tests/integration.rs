//! Integration tests for IDs and Handles

use poko_ids_handles::{HandleRegistry, Id, IdGenerator, IdType};

#[test]
fn test_id_and_handle_combination() {
    // Simulate an object that has both a stable ID and a runtime handle
    struct GameObject {
        id: Id,
        data: String,
    }

    let mut registry = HandleRegistry::new();

    // Create objects with stable IDs
    let obj1 = GameObject {
        id: Id::new(IdType::Model),
        data: "Object 1".to_string(),
    };
    let obj2 = GameObject {
        id: Id::new(IdType::Model),
        data: "Object 2".to_string(),
    };

    // Allocate handles for the objects
    let handle1 = registry.allocate(obj1).unwrap();
    let handle2 = registry.allocate(obj2).unwrap();

    // Verify we can access objects by handle
    assert_eq!(registry.get(handle1).unwrap().data, "Object 1");
    assert_eq!(registry.get(handle2).unwrap().data, "Object 2");

    // Verify IDs are preserved
    assert_ne!(
        registry.get(handle1).unwrap().id,
        registry.get(handle2).unwrap().id
    );
}

#[test]
fn test_id_type_categorization() {
    let mut registry = HandleRegistry::new();

    struct TypedObject {
        id: Id,
        _name: String,
    }

    // Create objects of different types
    let workspace = TypedObject {
        id: Id::new(IdType::Workspace),
        _name: "Main Workspace".to_string(),
    };
    let model = TypedObject {
        id: Id::new(IdType::Model),
        _name: "Car Model".to_string(),
    };
    let part = TypedObject {
        id: Id::new(IdType::Part),
        _name: "Wheel Part".to_string(),
    };

    let h_workspace = registry.allocate(workspace).unwrap();
    let h_model = registry.allocate(model).unwrap();
    let h_part = registry.allocate(part).unwrap();

    // Verify types are preserved
    assert!(registry
        .get(h_workspace)
        .unwrap()
        .id
        .is_type(IdType::Workspace));
    assert!(registry.get(h_model).unwrap().id.is_type(IdType::Model));
    assert!(registry.get(h_part).unwrap().id.is_type(IdType::Part));
}

#[test]
fn test_batch_operations() {
    let mut registry = HandleRegistry::new();
    let generator = IdGenerator::new(IdType::Part);

    // Generate a batch of IDs
    let ids = generator.generate_batch(100);
    assert_eq!(ids.len(), 100);

    // Create objects with those IDs
    for id in ids {
        let obj = (id, "test".to_string());
        registry.allocate(obj).unwrap();
    }

    // Verify all objects are in the registry
    assert_eq!(registry.len(), 100);
}

#[test]
fn test_serialization_roundtrip() {
    let mut registry = HandleRegistry::new();

    struct SerializableObject {
        id: Id,
        _value: i32,
    }

    let obj = SerializableObject {
        id: Id::new(IdType::Model),
        _value: 42,
    };

    let handle = registry.allocate(obj).unwrap();

    // Serialize the object's ID
    let id = registry.get(handle).unwrap().id;
    let serialized = serde_json::to_string(&id).unwrap();

    // Deserialize and verify
    let deserialized: Id = serde_json::from_str(&serialized).unwrap();
    assert_eq!(id, deserialized);
    assert_eq!(id.id_type(), deserialized.id_type());
}

#[test]
fn test_handle_stability_after_removal() {
    let mut registry = HandleRegistry::new();

    let h1 = registry.allocate(1).unwrap();
    let h2 = registry.allocate(2).unwrap();
    let h3 = registry.allocate(3).unwrap();

    // Remove middle object
    registry.remove(h2).unwrap();

    // Other handles should still be valid
    assert!(registry.is_valid(h1));
    assert!(!registry.is_valid(h2));
    assert!(registry.is_valid(h3));

    // Reuse the slot
    let h4 = registry.allocate(4).unwrap();

    // New handle should be valid, old h2 should still be invalid
    assert!(registry.is_valid(h4));
    assert!(!registry.is_valid(h2));

    // h4 should have different generation than h2
    assert_ne!(h4.generation(), h2.generation());
}

#[test]
fn test_find_by_id() {
    struct ObjectWithId {
        id: Id,
        name: String,
    }

    let mut registry = HandleRegistry::new();

    let target_id = Id::new(IdType::Model);
    let obj1 = ObjectWithId {
        id: target_id,
        name: "Target".to_string(),
    };
    let obj2 = ObjectWithId {
        id: Id::new(IdType::Model),
        name: "Other".to_string(),
    };

    let h1 = registry.allocate(obj1).unwrap();
    let _h2 = registry.allocate(obj2).unwrap();

    // Find object by ID
    let found = registry.find(|obj| obj.id == target_id);
    assert_eq!(found, Some(h1));

    // Find with closure
    let found_name = registry.find(|obj| obj.name == "Target");
    assert_eq!(found_name, Some(h1));
}
