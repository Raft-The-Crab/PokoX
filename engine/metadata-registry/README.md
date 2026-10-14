# Metadata Registry

Canonical type/property/function/event metadata for the Poko Engine.

## Purpose

The Metadata Registry provides a centralized, validated source of truth for:
- **Mute script bindings** - What functions and properties are accessible from scripts
- **Studio inspector** - How properties are displayed and edited in the UI
- **Serialization/replication** - Which properties are networked and how
- **Autocomplete** - IDE and editor suggestions
- **Validation** - Runtime and compile-time checking of object properties

## Module Structure

- **types** - TypeMetadata for object types with inheritance
- **properties** - PropertyMetadata with validators, defaults, inspector config
- **functions** - FunctionMetadata and ParameterMetadata for callable functions
- **events** - EventMetadata for lifecycle and custom events
- **validation** - Validation logic for reference checking, circular inheritance, data type consistency

## Core Concepts

### TypeMetadata
Describes object types (Workspace, Model, Part, etc.) with:
- Inheritance hierarchy (base_type)
- Properties (list of property IDs)
- Supported functions (list of function IDs)
- Emitted events (list of event IDs)
- Visibility controls (Public/Internal/Private)
- Instantiability flag

### PropertyMetadata
Describes individual properties with:
- Data type (Vec3, Quat, Id, F32, String, etc.)
- Default values
- Validators (Range, Length, Pattern, Custom, EnumValues)
- Replication policy (All/Owner/None)
- Inspector display configuration (widget, order, category, tooltip)
- Read-only flag

### FunctionMetadata
Describes callable functions with:
- Parameters with data types and defaults
- Return type
- Async/method flags
- Target type (for methods)
- Visibility controls

### EventMetadata
Describes lifecycle events with:
- Event parameters
- Target type (for type-specific events)
- Visibility controls

## Usage

```rust
use poko_metadata_registry::{
    MetadataRegistry, TypeMetadata, PropertyMetadata, DataType, Visibility, ReplicationPolicy,
    InspectorConfig, Validator, DefaultValue
};

// Create registry
let mut registry = MetadataRegistry::new();

// Register a type
let workspace_type = TypeMetadata::new("Workspace".to_string(), "Workspace".to_string())
    .with_instantiable(true)
    .with_docs("Top-level container for models".to_string());
registry.register_type(workspace_type)?;

// Register a property
let position_prop = PropertyMetadata::new("Transform.Position".to_string(), DataType::Vec3)
    .with_default(DefaultValue::Vec3([0.0, 0.0, 0.0]))
    .with_validator(Validator::Range { min: -1000.0, max: 1000.0 })
    .with_visibility(Visibility::Public)
    .with_replication(ReplicationPolicy::All)
    .with_read_only(false)
    .with_inspector(
        InspectorConfig::new("Position".to_string())
            .with_widget("vector3")
            .with_category("Transform")
            .with_tooltip("Object position in world space")
    )
    .with_docs("Object position in world space");
registry.register_property(position_prop)?;

// Validate all metadata
registry.validate()?;

// Get type properties
let props = registry.get_type_properties("Workspace");
```

## Features

- **Modular design** - Organized into separate modules for types, properties, functions, events, validation
- **Duplicate detection** - Prevents registering same ID twice
- **Reference validation** - Ensures all type/function/event references exist
- **Circular inheritance detection** - Prevents infinite inheritance chains
- **Data type validation** - Checks default values match their declared types
- **Visibility control** - Public/Internal/Private for Mute and Studio
- **Replication policies** - Fine-grained network synchronization control
- **Inspector configuration** - Studio UI customization per property
- **JSON serialization** - Save/load registry state
- **Type-specific queries** - Get properties/functions/events for a specific type

## Integration

The registry will be used by:
- **Mute compiler** - Generate bindings from function/type metadata
- **Studio** - Build inspector UI from property metadata
- **Networking** - Use replication policies for sync
- **Serialization** - Serialize based on type metadata
