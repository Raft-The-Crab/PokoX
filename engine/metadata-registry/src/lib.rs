//! Metadata Registry for the Poko Engine
//!
//! This module provides canonical type/property/function/event metadata that drives:
//! - Mute script bindings
//! - Studio inspector behavior
//! - Serialization/replication policies
//! - Autocomplete and validation
//!
//! # Modules
//!
//! - `types` - Type metadata and inheritance
//! - `properties` - Property metadata with validators and inspector config
//! - `functions` - Function metadata with parameters
//! - `events` - Event metadata with parameters
//! - `validation` - Validation logic for metadata integrity

use serde::{Deserialize, Serialize};
use std::collections::HashMap;

pub mod events;
pub mod functions;
pub mod properties;
pub mod types;
pub mod validation;

// Re-export main types for convenience
pub use events::EventMetadata;
pub use functions::{FunctionMetadata, ParameterMetadata};
pub use properties::{
    DataType, DefaultValue, InspectorConfig, PropertyMetadata, ReplicationPolicy, Validator,
};
pub use types::{TypeMetadata, Visibility};
pub use validation::{ValidationError, validate_metadata};

/// Central metadata registry
#[derive(Serialize, Deserialize)]
pub struct MetadataRegistry {
    /// Type metadata indexed by ID
    types: HashMap<String, TypeMetadata>,
    /// Property metadata indexed by ID
    properties: HashMap<String, PropertyMetadata>,
    /// Function metadata indexed by ID
    functions: HashMap<String, FunctionMetadata>,
    /// Event metadata indexed by ID
    events: HashMap<String, EventMetadata>,
}

impl Default for MetadataRegistry {
    fn default() -> Self {
        Self::new()
    }
}

impl MetadataRegistry {
    /// Create a new empty registry
    pub fn new() -> Self {
        Self {
            types: HashMap::new(),
            properties: HashMap::new(),
            functions: HashMap::new(),
            events: HashMap::new(),
        }
    }

    /// Register a type
    pub fn register_type(&mut self, type_meta: TypeMetadata) -> Result<(), ValidationError> {
        if self.types.contains_key(&type_meta.id) {
            return Err(ValidationError::DuplicateId(type_meta.id));
        }
        self.types.insert(type_meta.id.clone(), type_meta);
        Ok(())
    }

    /// Register multiple types
    pub fn register_types(&mut self, types: Vec<TypeMetadata>) -> Result<(), ValidationError> {
        for type_meta in types {
            self.register_type(type_meta)?;
        }
        Ok(())
    }

    /// Register a property
    pub fn register_property(&mut self, prop_meta: PropertyMetadata) -> Result<(), ValidationError> {
        if self.properties.contains_key(&prop_meta.id) {
            return Err(ValidationError::DuplicateId(prop_meta.id));
        }
        self.properties.insert(prop_meta.id.clone(), prop_meta);
        Ok(())
    }

    /// Register multiple properties
    pub fn register_properties(&mut self, props: Vec<PropertyMetadata>) -> Result<(), ValidationError> {
        for prop_meta in props {
            self.register_property(prop_meta)?;
        }
        Ok(())
    }

    /// Register a function
    pub fn register_function(&mut self, func_meta: FunctionMetadata) -> Result<(), ValidationError> {
        if self.functions.contains_key(&func_meta.id) {
            return Err(ValidationError::DuplicateId(func_meta.id));
        }
        self.functions.insert(func_meta.id.clone(), func_meta);
        Ok(())
    }

    /// Register multiple functions
    pub fn register_functions(&mut self, funcs: Vec<FunctionMetadata>) -> Result<(), ValidationError> {
        for func_meta in funcs {
            self.register_function(func_meta)?;
        }
        Ok(())
    }

    /// Register an event
    pub fn register_event(&mut self, event_meta: EventMetadata) -> Result<(), ValidationError> {
        if self.events.contains_key(&event_meta.id) {
            return Err(ValidationError::DuplicateId(event_meta.id));
        }
        self.events.insert(event_meta.id.clone(), event_meta);
        Ok(())
    }

    /// Register multiple events
    pub fn register_events(&mut self, events: Vec<EventMetadata>) -> Result<(), ValidationError> {
        for event_meta in events {
            self.register_event(event_meta)?;
        }
        Ok(())
    }

    /// Get type metadata by ID
    pub fn get_type(&self, id: &str) -> Option<&TypeMetadata> {
        self.types.get(id)
    }

    /// Get property metadata by ID
    pub fn get_property(&self, id: &str) -> Option<&PropertyMetadata> {
        self.properties.get(id)
    }

    /// Get function metadata by ID
    pub fn get_function(&self, id: &str) -> Option<&FunctionMetadata> {
        self.functions.get(id)
    }

    /// Get event metadata by ID
    pub fn get_event(&self, id: &str) -> Option<&EventMetadata> {
        self.events.get(id)
    }

    /// Get all types
    pub fn all_types(&self) -> Vec<&TypeMetadata> {
        self.types.values().collect()
    }

    /// Get all properties
    pub fn all_properties(&self) -> Vec<&PropertyMetadata> {
        self.properties.values().collect()
    }

    /// Get all functions
    pub fn all_functions(&self) -> Vec<&FunctionMetadata> {
        self.functions.values().collect()
    }

    /// Get all events
    pub fn all_events(&self) -> Vec<&EventMetadata> {
        self.events.values().collect()
    }

    /// Get properties for a specific type
    pub fn get_type_properties(&self, type_id: &str) -> Vec<&PropertyMetadata> {
        if let Some(type_meta) = self.types.get(type_id) {
            type_meta
                .properties
                .iter()
                .filter_map(|prop_id| self.properties.get(prop_id))
                .collect()
        } else {
            Vec::new()
        }
    }

    /// Get functions for a specific type
    pub fn get_type_functions(&self, type_id: &str) -> Vec<&FunctionMetadata> {
        if let Some(type_meta) = self.types.get(type_id) {
            type_meta
                .functions
                .iter()
                .filter_map(|func_id| self.functions.get(func_id))
                .collect()
        } else {
            Vec::new()
        }
    }

    /// Get events for a specific type
    pub fn get_type_events(&self, type_id: &str) -> Vec<&EventMetadata> {
        if let Some(type_meta) = self.types.get(type_id) {
            type_meta
                .events
                .iter()
                .filter_map(|event_id| self.events.get(event_id))
                .collect()
        } else {
            Vec::new()
        }
    }

    /// Get inherited properties for a type (includes parent properties)
    pub fn get_inherited_properties(&self, type_id: &str) -> Vec<&PropertyMetadata> {
        let mut prop_ids = Vec::new();
        let mut visited = std::collections::HashSet::new();
        self.collect_inherited_property_ids(type_id, &mut prop_ids, &mut visited);

        prop_ids
            .iter()
            .filter_map(|id| self.properties.get(id))
            .collect()
    }

    fn collect_inherited_property_ids(
        &self,
        type_id: &str,
        prop_ids: &mut Vec<String>,
        visited: &mut std::collections::HashSet<String>,
    ) {
        if visited.contains(type_id) {
            return;
        }
        visited.insert(type_id.to_string());

        if let Some(type_meta) = self.types.get(type_id) {
            // Add own property IDs
            for prop_id in &type_meta.properties {
                prop_ids.push(prop_id.clone());
            }

            // Recursively add parent property IDs
            if let Some(parent) = &type_meta.base_type {
                self.collect_inherited_property_ids(parent, prop_ids, visited);
            }
        }
    }

    /// Check if a type exists
    pub fn has_type(&self, type_id: &str) -> bool {
        self.types.contains_key(type_id)
    }

    /// Check if a property exists
    pub fn has_property(&self, prop_id: &str) -> bool {
        self.properties.contains_key(prop_id)
    }

    /// Check if a function exists
    pub fn has_function(&self, func_id: &str) -> bool {
        self.functions.contains_key(func_id)
    }

    /// Check if an event exists
    pub fn has_event(&self, event_id: &str) -> bool {
        self.events.contains_key(event_id)
    }

    /// Remove a type
    pub fn remove_type(&mut self, type_id: &str) -> Option<TypeMetadata> {
        self.types.remove(type_id)
    }

    /// Remove a property
    pub fn remove_property(&mut self, prop_id: &str) -> Option<PropertyMetadata> {
        self.properties.remove(prop_id)
    }

    /// Remove a function
    pub fn remove_function(&mut self, func_id: &str) -> Option<FunctionMetadata> {
        self.functions.remove(func_id)
    }

    /// Remove an event
    pub fn remove_event(&mut self, event_id: &str) -> Option<EventMetadata> {
        self.events.remove(event_id)
    }

    /// Clear all metadata
    pub fn clear(&mut self) {
        self.types.clear();
        self.properties.clear();
        self.functions.clear();
        self.events.clear();
    }

    /// Validate all registered metadata
    pub fn validate(&self) -> Result<(), ValidationError> {
        let types: Vec<_> = self.types.values().cloned().collect();
        let properties: Vec<_> = self.properties.values().cloned().collect();
        let functions: Vec<_> = self.functions.values().cloned().collect();
        let events: Vec<_> = self.events.values().cloned().collect();

        validate_metadata(&types, &properties, &functions, &events)
    }

    /// Get registry statistics
    pub fn stats(&self) -> MetadataRegistryStats {
        MetadataRegistryStats {
            type_count: self.types.len(),
            property_count: self.properties.len(),
            function_count: self.functions.len(),
            event_count: self.events.len(),
        }
    }

    /// Serialize the registry to JSON
    pub fn to_json(&self) -> Result<String, serde_json::Error> {
        serde_json::to_string(self)
    }

    /// Deserialize the registry from JSON
    pub fn from_json(json: &str) -> Result<Self, serde_json::Error> {
        serde_json::from_str(json)
    }

    /// Create registry with predefined core types
    pub fn with_core_types() -> Self {
        let mut registry = Self::new();
        registry.register_core_types();
        registry
    }

    /// Register core engine types (Workspace, Model, Part)
    pub fn register_core_types(&mut self) {
        // Workspace type
        let workspace = TypeMetadata::new("Workspace".to_string(), "Workspace".to_string())
            .with_property("name".to_string())
            .with_property("created_at".to_string())
            .with_property("modified_at".to_string())
            .with_visibility(Visibility::Public)
            .with_docs("Root container for all objects in a Poko experience".to_string());

        // Model type
        let model = TypeMetadata::new("Model".to_string(), "Model".to_string())
            .with_property("name".to_string())
            .with_property("parent".to_string())
            .with_property("created_at".to_string())
            .with_property("modified_at".to_string())
            .with_visibility(Visibility::Public)
            .with_docs("Container for parts and child models".to_string());

        // Part type
        let part = TypeMetadata::new("Part".to_string(), "Part".to_string())
            .with_property("name".to_string())
            .with_property("parent".to_string())
            .with_property("position".to_string())
            .with_property("rotation".to_string())
            .with_property("scale".to_string())
            .with_property("size".to_string())
            .with_property("color".to_string())
            .with_property("created_at".to_string())
            .with_property("modified_at".to_string())
            .with_visibility(Visibility::Public)
            .with_docs("Basic 3D object with transform and appearance".to_string());

        let _ = self.register_type(workspace);
        let _ = self.register_type(model);
        let _ = self.register_type(part);

        // Core properties
        let name_prop = PropertyMetadata::new("name".to_string(), DataType::String)
            .with_default(DefaultValue::String("".to_string()))
            .with_validator(Validator::Length { min: 1, max: 256 })
            .with_visibility(Visibility::Public)
            .with_replication(ReplicationPolicy::All)
            .with_inspector(InspectorConfig::new("text".to_string()).with_widget("text".to_string()).with_category("General".to_string()));

        let parent_prop = PropertyMetadata::new("parent".to_string(), DataType::Id)
            .with_default(DefaultValue::Null)
            .with_visibility(Visibility::Public)
            .with_replication(ReplicationPolicy::All)
            .with_inspector(InspectorConfig::new("object_picker".to_string()).with_widget("object_picker".to_string()).with_category("General".to_string()));

        let position_prop = PropertyMetadata::new("position".to_string(), DataType::Vec3)
            .with_default(DefaultValue::Vec3([0.0, 0.0, 0.0]))
            .with_visibility(Visibility::Public)
            .with_replication(ReplicationPolicy::All)
            .with_inspector(InspectorConfig::new("vector3".to_string()).with_widget("vector3".to_string()).with_category("Transform".to_string()));

        let rotation_prop = PropertyMetadata::new("rotation".to_string(), DataType::Quat)
            .with_default(DefaultValue::Quat([0.0, 0.0, 0.0, 1.0]))
            .with_visibility(Visibility::Public)
            .with_replication(ReplicationPolicy::All)
            .with_inspector(InspectorConfig::new("quaternion".to_string()).with_widget("quaternion".to_string()).with_category("Transform".to_string()));

        let scale_prop = PropertyMetadata::new("scale".to_string(), DataType::Vec3)
            .with_default(DefaultValue::Vec3([1.0, 1.0, 1.0]))
            .with_visibility(Visibility::Public)
            .with_replication(ReplicationPolicy::All)
            .with_inspector(InspectorConfig::new("vector3".to_string()).with_widget("vector3".to_string()).with_category("Transform".to_string()));

        let size_prop = PropertyMetadata::new("size".to_string(), DataType::Vec3)
            .with_default(DefaultValue::Vec3([1.0, 1.0, 1.0]))
            .with_validator(Validator::Range { min: 0.01, max: 1000.0 })
            .with_visibility(Visibility::Public)
            .with_replication(ReplicationPolicy::All)
            .with_inspector(InspectorConfig::new("vector3".to_string()).with_widget("vector3".to_string()).with_category("Appearance".to_string()));

        let color_prop = PropertyMetadata::new("color".to_string(), DataType::Color)
            .with_default(DefaultValue::Vec4([1.0, 1.0, 1.0, 1.0]))
            .with_validator(Validator::ColorRange)
            .with_visibility(Visibility::Public)
            .with_replication(ReplicationPolicy::All)
            .with_inspector(InspectorConfig::new("color".to_string()).with_widget("color".to_string()).with_category("Appearance".to_string()));

        let created_at_prop = PropertyMetadata::new("created_at".to_string(), DataType::Timestamp)
            .with_default(DefaultValue::Timestamp(0))
            .with_visibility(Visibility::Internal)
            .with_replication(ReplicationPolicy::All)
            .with_inspector(InspectorConfig::new("timestamp".to_string()).with_widget("timestamp".to_string()).with_category("Metadata".to_string()));

        let modified_at_prop = PropertyMetadata::new("modified_at".to_string(), DataType::Timestamp)
            .with_default(DefaultValue::Timestamp(0))
            .with_visibility(Visibility::Internal)
            .with_replication(ReplicationPolicy::All)
            .with_inspector(InspectorConfig::new("timestamp".to_string()).with_widget("timestamp".to_string()).with_category("Metadata".to_string()));

        let _ = self.register_property(name_prop);
        let _ = self.register_property(parent_prop);
        let _ = self.register_property(position_prop);
        let _ = self.register_property(rotation_prop);
        let _ = self.register_property(scale_prop);
        let _ = self.register_property(size_prop);
        let _ = self.register_property(color_prop);
        let _ = self.register_property(created_at_prop);
        let _ = self.register_property(modified_at_prop);
    }
}

/// Registry statistics
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct MetadataRegistryStats {
    pub type_count: usize,
    pub property_count: usize,
    pub function_count: usize,
    pub event_count: usize,
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_registry_creation() {
        let registry = MetadataRegistry::new();
        let stats = registry.stats();
        assert_eq!(stats.type_count, 0);
        assert_eq!(stats.property_count, 0);
    }

    #[test]
    fn test_register_type() {
        let mut registry = MetadataRegistry::new();
        let type_meta = TypeMetadata::new("TestType".to_string(), "Test Type".to_string());

        assert!(registry.register_type(type_meta).is_ok());
        assert!(registry.get_type("TestType").is_some());
    }

    #[test]
    fn test_duplicate_type() {
        let mut registry = MetadataRegistry::new();
        let type_meta = TypeMetadata::new("TestType".to_string(), "Test Type".to_string());

        assert!(registry.register_type(type_meta.clone()).is_ok());
        assert!(matches!(
            registry.register_type(type_meta),
            Err(ValidationError::DuplicateId(_))
        ));
    }

    #[test]
    fn test_get_type_properties() {
        let mut registry = MetadataRegistry::new();

        let type_meta = TypeMetadata::new("TestType".to_string(), "Test Type".to_string())
            .with_property("Prop1".to_string());
        registry.register_type(type_meta).unwrap();

        let prop_meta = PropertyMetadata::new("Prop1".to_string(), DataType::F32);
        registry.register_property(prop_meta).unwrap();

        let props = registry.get_type_properties("TestType");
        assert_eq!(props.len(), 1);
    }

    #[test]
    fn test_registry_validation() {
        let mut registry = MetadataRegistry::new();

        let type_a = TypeMetadata::new("TypeA".to_string(), "Type A".to_string())
            .with_base_type("TypeB".to_string());
        let type_b = TypeMetadata::new("TypeB".to_string(), "Type B".to_string())
            .with_base_type("TypeA".to_string());

        registry.register_type(type_a).unwrap();
        registry.register_type(type_b).unwrap();

        assert!(matches!(
            registry.validate(),
            Err(ValidationError::CircularInheritance(_))
        ));
    }

    #[test]
    fn test_serialization() {
        let mut registry = MetadataRegistry::new();

        let type_meta = TypeMetadata::new("TestType".to_string(), "Test Type".to_string());
        registry.register_type(type_meta).unwrap();

        let json = registry.to_json().unwrap();
        let restored = MetadataRegistry::from_json(&json).unwrap();

        assert_eq!(restored.stats().type_count, 1);
        assert!(restored.get_type("TestType").is_some());
    }

    #[test]
    fn test_batch_registration() {
        let mut registry = MetadataRegistry::new();

        let types = vec![
            TypeMetadata::new("TypeA".to_string(), "Type A".to_string()),
            TypeMetadata::new("TypeB".to_string(), "Type B".to_string()),
        ];

        assert!(registry.register_types(types).is_ok());
        assert_eq!(registry.stats().type_count, 2);
    }

    #[test]
    fn test_has_methods() {
        let mut registry = MetadataRegistry::new();

        let type_meta = TypeMetadata::new("TestType".to_string(), "Test Type".to_string());
        registry.register_type(type_meta).unwrap();

        assert!(registry.has_type("TestType"));
        assert!(!registry.has_type("NonExistent"));
    }

    #[test]
    fn test_remove_methods() {
        let mut registry = MetadataRegistry::new();

        let type_meta = TypeMetadata::new("TestType".to_string(), "Test Type".to_string());
        registry.register_type(type_meta).unwrap();

        assert!(registry.remove_type("TestType").is_some());
        assert!(registry.remove_type("TestType").is_none());
        assert_eq!(registry.stats().type_count, 0);
    }

    #[test]
    fn test_clear() {
        let mut registry = MetadataRegistry::new();

        let type_meta = TypeMetadata::new("TestType".to_string(), "Test Type".to_string());
        registry.register_type(type_meta).unwrap();

        registry.clear();
        assert_eq!(registry.stats().type_count, 0);
    }

    #[test]
    fn test_inherited_properties() {
        let mut registry = MetadataRegistry::new();

        let parent_type = TypeMetadata::new("Parent".to_string(), "Parent".to_string())
            .with_property("ParentProp".to_string());
        let child_type = TypeMetadata::new("Child".to_string(), "Child".to_string())
            .with_base_type("Parent".to_string())
            .with_property("ChildProp".to_string());

        registry.register_type(parent_type).unwrap();
        registry.register_type(child_type).unwrap();

        let parent_prop = PropertyMetadata::new("ParentProp".to_string(), DataType::F32);
        let child_prop = PropertyMetadata::new("ChildProp".to_string(), DataType::F32);

        registry.register_property(parent_prop).unwrap();
        registry.register_property(child_prop).unwrap();

        let inherited = registry.get_inherited_properties("Child");
        assert_eq!(inherited.len(), 2);
    }

    #[test]
    fn test_core_types_registration() {
        let registry = MetadataRegistry::with_core_types();

        assert!(registry.has_type("Workspace"));
        assert!(registry.has_type("Model"));
        assert!(registry.has_type("Part"));

        assert!(registry.has_property("name"));
        assert!(registry.has_property("parent"));
        assert!(registry.has_property("position"));
        assert!(registry.has_property("rotation"));
        assert!(registry.has_property("scale"));
        assert!(registry.has_property("size"));
        assert!(registry.has_property("color"));
        assert!(registry.has_property("created_at"));
        assert!(registry.has_property("modified_at"));

        assert_eq!(registry.stats().type_count, 3);
        assert_eq!(registry.stats().property_count, 9);
    }
}
