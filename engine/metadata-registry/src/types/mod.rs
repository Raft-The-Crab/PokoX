//! Type metadata definitions
//!
//! This module contains the TypeMetadata struct and related functionality
//! for describing object types in the Poko engine.

use serde::{Deserialize, Serialize};

/// Visibility level for metadata (controls Mute/Studio access)
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum Visibility {
    /// Fully visible in scripts and Studio
    Public,
    /// Visible only in Studio, not in scripts
    Internal,
    /// Hidden from both scripts and Studio
    Private,
}

/// Metadata for a type (Workspace, Model, Part, etc.)
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TypeMetadata {
    /// Unique identifier (e.g., "Workspace", "Model", "Part")
    pub id: String,
    /// Display name
    pub display_name: String,
    /// Base type (if inheriting)
    pub base_type: Option<String>,
    /// Property IDs this type has
    pub properties: Vec<String>,
    /// Function IDs this type supports
    pub functions: Vec<String>,
    /// Event IDs this type emits
    pub events: Vec<String>,
    /// Visibility level
    pub visibility: Visibility,
    /// Whether this type can be instantiated
    pub instantiable: bool,
    /// Documentation
    pub docs: Option<String>,
}

impl TypeMetadata {
    /// Create a new TypeMetadata
    pub fn new(id: String, display_name: String) -> Self {
        Self {
            id,
            display_name,
            base_type: None,
            properties: Vec::new(),
            functions: Vec::new(),
            events: Vec::new(),
            visibility: Visibility::Public,
            instantiable: true,
            docs: None,
        }
    }

    /// Set the base type
    pub fn with_base_type(mut self, base_type: String) -> Self {
        self.base_type = Some(base_type);
        self
    }

    /// Add a property
    pub fn with_property(mut self, property_id: String) -> Self {
        self.properties.push(property_id);
        self
    }

    /// Add a function
    pub fn with_function(mut self, function_id: String) -> Self {
        self.functions.push(function_id);
        self
    }

    /// Add an event
    pub fn with_event(mut self, event_id: String) -> Self {
        self.events.push(event_id);
        self
    }

    /// Set visibility
    pub fn with_visibility(mut self, visibility: Visibility) -> Self {
        self.visibility = visibility;
        self
    }

    /// Set whether instantiable
    pub fn with_instantiable(mut self, instantiable: bool) -> Self {
        self.instantiable = instantiable;
        self
    }

    /// Set documentation
    pub fn with_docs(mut self, docs: String) -> Self {
        self.docs = Some(docs);
        self
    }

    /// Check if this type inherits from another
    pub fn inherits_from(&self, type_id: &str) -> bool {
        match &self.base_type {
            Some(base) => base == type_id,
            None => false,
        }
    }

    /// Get all inherited types (direct and indirect)
    pub fn get_inheritance_chain(&self) -> Vec<String> {
        let mut chain = Vec::new();
        if let Some(base) = &self.base_type {
            chain.push(base.clone());
        }
        chain
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_type_creation() {
        let type_meta = TypeMetadata::new("TestType".to_string(), "Test Type".to_string());
        assert_eq!(type_meta.id, "TestType");
        assert_eq!(type_meta.display_name, "Test Type");
        assert!(type_meta.base_type.is_none());
        assert!(type_meta.properties.is_empty());
    }

    #[test]
    fn test_type_builder() {
        let type_meta = TypeMetadata::new("Model".to_string(), "Model".to_string())
            .with_base_type("BaseObject".to_string())
            .with_property("Transform.Position".to_string())
            .with_function("SetPosition".to_string())
            .with_event("Modified".to_string())
            .with_visibility(Visibility::Public)
            .with_instantiable(true)
            .with_docs("A 3D model".to_string());

        assert_eq!(type_meta.base_type, Some("BaseObject".to_string()));
        assert_eq!(type_meta.properties.len(), 1);
        assert_eq!(type_meta.functions.len(), 1);
        assert_eq!(type_meta.events.len(), 1);
    }

    #[test]
    fn test_inherits_from() {
        let type_meta = TypeMetadata::new("Child".to_string(), "Child".to_string())
            .with_base_type("Parent".to_string());

        assert!(type_meta.inherits_from("Parent"));
        assert!(!type_meta.inherits_from("Other"));
    }
}
