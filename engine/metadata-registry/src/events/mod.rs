//! Event metadata definitions
//!
//! This module contains the EventMetadata struct for describing
//! lifecycle and custom events in the Poko engine.

use serde::{Deserialize, Serialize};

use super::functions::ParameterMetadata;
use super::types::Visibility;

/// Metadata for an event
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct EventMetadata {
    /// Unique identifier (e.g., "Lifecycle.Created")
    pub id: String,
    /// Display name
    pub display_name: String,
    /// Event parameters
    pub parameters: Vec<ParameterMetadata>,
    /// Visibility level
    pub visibility: Visibility,
    /// Target type (if specific to a type)
    pub target_type: Option<String>,
    /// Documentation
    pub docs: Option<String>,
}

impl EventMetadata {
    /// Create a new EventMetadata
    pub fn new(id: String, display_name: String) -> Self {
        Self {
            id,
            display_name,
            parameters: Vec::new(),
            visibility: Visibility::Public,
            target_type: None,
            docs: None,
        }
    }

    /// Add a parameter
    pub fn with_parameter(mut self, param: ParameterMetadata) -> Self {
        self.parameters.push(param);
        self
    }

    /// Set visibility
    pub fn with_visibility(mut self, visibility: Visibility) -> Self {
        self.visibility = visibility;
        self
    }

    /// Set target type
    pub fn with_target_type(mut self, target_type: String) -> Self {
        self.target_type = Some(target_type);
        self
    }

    /// Set documentation
    pub fn with_docs(mut self, docs: String) -> Self {
        self.docs = Some(docs);
        self
    }

    /// Check if event is visible in scripts
    pub fn is_script_visible(&self) -> bool {
        self.visibility == Visibility::Public
    }

    /// Check if event is visible in Studio
    pub fn is_studio_visible(&self) -> bool {
        self.visibility != Visibility::Private
    }

    /// Check if event is global (not tied to a specific type)
    pub fn is_global(&self) -> bool {
        self.target_type.is_none()
    }

    /// Get the number of parameters
    pub fn param_count(&self) -> usize {
        self.parameters.len()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_event_creation() {
        let event = EventMetadata::new("TestEvent".to_string(), "Test Event".to_string());
        assert_eq!(event.id, "TestEvent");
        assert!(event.parameters.is_empty());
        assert!(event.is_global());
    }

    #[test]
    fn test_event_builder() {
        let event = EventMetadata::new("Created".to_string(), "Object Created".to_string())
            .with_parameter(ParameterMetadata::new(
                "id".to_string(),
                crate::DataType::Id,
            ))
            .with_parameter(ParameterMetadata::new(
                "timestamp".to_string(),
                crate::DataType::F64,
            ))
            .with_visibility(Visibility::Public)
            .with_target_type("Object".to_string())
            .with_docs("Emitted when an object is created".to_string());

        assert_eq!(event.parameters.len(), 2);
        assert_eq!(event.target_type, Some("Object".to_string()));
        assert!(!event.is_global());
        assert_eq!(event.param_count(), 2);
    }

    #[test]
    fn test_event_visibility() {
        let event_public = EventMetadata::new("Test".to_string(), "Test".to_string())
            .with_visibility(Visibility::Public);
        assert!(event_public.is_script_visible());
        assert!(event_public.is_studio_visible());

        let event_internal = EventMetadata::new("Test".to_string(), "Test".to_string())
            .with_visibility(Visibility::Internal);
        assert!(!event_internal.is_script_visible());
        assert!(event_internal.is_studio_visible());

        let event_private = EventMetadata::new("Test".to_string(), "Test".to_string())
            .with_visibility(Visibility::Private);
        assert!(!event_private.is_script_visible());
        assert!(!event_private.is_studio_visible());
    }
}
