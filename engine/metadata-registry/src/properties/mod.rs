//! Property metadata definitions
//!
//! This module contains the PropertyMetadata struct and related functionality
//! for describing object properties in the Poko engine.

use poko_ids_handles::Id;
use serde::{Deserialize, Serialize};

/// Data type for properties and function parameters
#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub enum DataType {
    /// Signed integer
    I32,
    /// Unsigned integer
    U32,
    /// 64-bit signed integer
    I64,
    /// 64-bit unsigned integer
    U64,
    /// 32-bit floating point
    F32,
    /// 64-bit floating point
    F64,
    /// Boolean
    Bool,
    /// String
    String,
    /// Vector of 3 floats (position, direction)
    Vec3,
    /// Quaternion (rotation)
    Quat,
    /// Matrix 4x4
    Mat4,
    /// Stable object ID
    Id,
    /// Reference to another object by type
    ObjectType(String),
    /// Enum value
    Enum(String),
    /// Optional value
    Option(Box<DataType>),
    /// Array of values
    Array(Box<DataType>),
    /// Color (RGBA, 4 floats)
    Color,
    /// Axis-aligned bounding box (min and max Vec3)
    AABB { min: [f32; 3], max: [f32; 3] },
    /// 2D vector (X, Y)
    Vec2,
    /// 4D vector (X, Y, Z, W)
    Vec4,
    /// Timestamp (Unix milliseconds)
    Timestamp,
    /// Duration (milliseconds)
    Duration,
}

/// Default value for a property
#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum DefaultValue {
    I32(i32),
    U32(u32),
    I64(i64),
    U64(u64),
    F32(f32),
    F64(f64),
    Bool(bool),
    String(String),
    Vec3([f32; 3]),
    Quat([f32; 4]),
    Mat4([f32; 16]),
    Id(Id),
    Vec2([f32; 2]),
    Vec4([f32; 4]),
    Color([f32; 4]),
    AABB { min: [f32; 3], max: [f32; 3] },
    Timestamp(u64),
    Duration(u64),
    Null,
}

/// Validator for property values
#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum Validator {
    /// Range validation for numeric types
    Range { min: f64, max: f64 },
    /// String length validation
    Length { min: usize, max: usize },
    /// Regex pattern validation
    Pattern(String),
    /// Custom validation (by name)
    Custom(String),
    /// Enum of allowed values
    EnumValues(Vec<String>),
    /// Vector length validation
    VectorLength { min: usize, max: usize },
    /// Color validation (RGBA components must be 0-1)
    ColorRange,
    /// UUID format validation
    UuidFormat,
    /// Not null validation
    NotNull,
    /// Minimum timestamp (for datetime fields)
    MinTimestamp(u64),
    /// Maximum timestamp (for datetime fields)
    MaxTimestamp(u64),
}

/// Replication policy for networked properties
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum ReplicationPolicy {
    /// Replicate to all clients
    All,
    /// Replicate only to owner
    Owner,
    /// Never replicate
    None,
}

/// Inspector display configuration
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct InspectorConfig {
    /// Display name in Studio UI
    pub display_name: String,
    /// UI widget type (slider, dropdown, text input, etc.)
    pub widget: String,
    /// Order in inspector list
    pub order: i32,
    /// Whether to show in inspector by default
    pub visible: bool,
    /// Category/group in inspector
    pub category: Option<String>,
    /// Tooltip text
    pub tooltip: Option<String>,
}

impl Default for InspectorConfig {
    fn default() -> Self {
        Self {
            display_name: String::new(),
            widget: "default".to_string(),
            order: 0,
            visible: true,
            category: None,
            tooltip: None,
        }
    }
}

impl InspectorConfig {
    /// Create a new InspectorConfig
    pub fn new(display_name: String) -> Self {
        Self {
            display_name,
            widget: "default".to_string(),
            order: 0,
            visible: true,
            category: None,
            tooltip: None,
        }
    }

    /// Set the widget type
    pub fn with_widget(mut self, widget: String) -> Self {
        self.widget = widget;
        self
    }

    /// Set the order
    pub fn with_order(mut self, order: i32) -> Self {
        self.order = order;
        self
    }

    /// Set visibility
    pub fn with_visible(mut self, visible: bool) -> Self {
        self.visible = visible;
        self
    }

    /// Set category
    pub fn with_category(mut self, category: String) -> Self {
        self.category = Some(category);
        self
    }

    /// Set tooltip
    pub fn with_tooltip(mut self, tooltip: String) -> Self {
        self.tooltip = Some(tooltip);
        self
    }
}

use super::types::Visibility;

/// Metadata for a property on a type
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PropertyMetadata {
    /// Unique identifier (e.g., "Transform.Position")
    pub id: String,
    /// Data type
    pub data_type: DataType,
    /// Default value
    pub default_value: Option<DefaultValue>,
    /// Validators
    pub validators: Vec<Validator>,
    /// Visibility level
    pub visibility: Visibility,
    /// Replication policy
    pub replication: ReplicationPolicy,
    /// Whether property is read-only
    pub read_only: bool,
    /// Inspector display configuration
    pub inspector: InspectorConfig,
    /// Documentation
    pub docs: Option<String>,
}

impl PropertyMetadata {
    /// Create a new PropertyMetadata
    pub fn new(id: String, data_type: DataType) -> Self {
        Self {
            id,
            data_type,
            default_value: None,
            validators: Vec::new(),
            visibility: Visibility::Public,
            replication: ReplicationPolicy::All,
            read_only: false,
            inspector: InspectorConfig::default(),
            docs: None,
        }
    }

    /// Set default value
    pub fn with_default(mut self, default: DefaultValue) -> Self {
        self.default_value = Some(default);
        self
    }

    /// Add a validator
    pub fn with_validator(mut self, validator: Validator) -> Self {
        self.validators.push(validator);
        self
    }

    /// Set visibility
    pub fn with_visibility(mut self, visibility: Visibility) -> Self {
        self.visibility = visibility;
        self
    }

    /// Set replication policy
    pub fn with_replication(mut self, replication: ReplicationPolicy) -> Self {
        self.replication = replication;
        self
    }

    /// Set read-only flag
    pub fn with_read_only(mut self, read_only: bool) -> Self {
        self.read_only = read_only;
        self
    }

    /// Set inspector config
    pub fn with_inspector(mut self, inspector: InspectorConfig) -> Self {
        self.inspector = inspector;
        self
    }

    /// Set documentation
    pub fn with_docs(mut self, docs: String) -> Self {
        self.docs = Some(docs);
        self
    }

    /// Check if property is networked
    pub fn is_networked(&self) -> bool {
        self.replication != ReplicationPolicy::None
    }

    /// Check if property is visible in scripts
    pub fn is_script_visible(&self) -> bool {
        self.visibility == Visibility::Public
    }

    /// Check if property is visible in Studio
    pub fn is_studio_visible(&self) -> bool {
        self.visibility != Visibility::Private
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_property_creation() {
        let prop = PropertyMetadata::new("TestProp".to_string(), DataType::F32);
        assert_eq!(prop.id, "TestProp");
        assert!(matches!(prop.data_type, DataType::F32));
        assert!(prop.validators.is_empty());
    }

    #[test]
    fn test_property_builder() {
        let prop = PropertyMetadata::new("Position".to_string(), DataType::Vec3)
            .with_default(DefaultValue::Vec3([0.0, 0.0, 0.0]))
            .with_validator(Validator::Range {
                min: -1000.0,
                max: 1000.0,
            })
            .with_visibility(Visibility::Public)
            .with_replication(ReplicationPolicy::All)
            .with_read_only(false)
            .with_inspector(
                InspectorConfig::new("Position".to_string()).with_widget("vector3".to_string()),
            )
            .with_docs("Object position".to_string());

        assert!(prop.default_value.is_some());
        assert_eq!(prop.validators.len(), 1);
        assert!(prop.is_networked());
        assert!(prop.is_script_visible());
        assert!(prop.is_studio_visible());
    }

    #[test]
    fn test_inspector_config() {
        let config = InspectorConfig::new("Test".to_string())
            .with_widget("slider".to_string())
            .with_order(10)
            .with_visible(true)
            .with_category("Transform".to_string())
            .with_tooltip("A test property".to_string());

        assert_eq!(config.widget, "slider");
        assert_eq!(config.order, 10);
        assert_eq!(config.category, Some("Transform".to_string()));
    }

    #[test]
    fn test_replication_policy() {
        let prop_all = PropertyMetadata::new("Test".to_string(), DataType::F32)
            .with_replication(ReplicationPolicy::All);
        assert!(prop_all.is_networked());

        let prop_none = PropertyMetadata::new("Test".to_string(), DataType::F32)
            .with_replication(ReplicationPolicy::None);
        assert!(!prop_none.is_networked());
    }

    #[test]
    fn test_visibility() {
        let prop_public = PropertyMetadata::new("Test".to_string(), DataType::F32)
            .with_visibility(Visibility::Public);
        assert!(prop_public.is_script_visible());
        assert!(prop_public.is_studio_visible());

        let prop_internal = PropertyMetadata::new("Test".to_string(), DataType::F32)
            .with_visibility(Visibility::Internal);
        assert!(!prop_internal.is_script_visible());
        assert!(prop_internal.is_studio_visible());

        let prop_private = PropertyMetadata::new("Test".to_string(), DataType::F32)
            .with_visibility(Visibility::Private);
        assert!(!prop_private.is_script_visible());
        assert!(!prop_private.is_studio_visible());
    }
}
