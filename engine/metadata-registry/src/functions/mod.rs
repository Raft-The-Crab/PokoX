//! Function metadata definitions
//!
//! This module contains the FunctionMetadata and ParameterMetadata structs
//! for describing callable functions in the Poko engine.

use serde::{Deserialize, Serialize};

use super::properties::DataType;
use super::properties::DefaultValue;
use super::types::Visibility;

/// Metadata for a function parameter
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ParameterMetadata {
    /// Parameter name
    pub name: String,
    /// Data type
    pub data_type: DataType,
    /// Default value
    pub default_value: Option<DefaultValue>,
    /// Whether parameter is optional
    pub optional: bool,
    /// Documentation
    pub docs: Option<String>,
}

impl ParameterMetadata {
    /// Create a new ParameterMetadata
    pub fn new(name: String, data_type: DataType) -> Self {
        Self {
            name,
            data_type,
            default_value: None,
            optional: false,
            docs: None,
        }
    }

    /// Set default value
    pub fn with_default(mut self, default: DefaultValue) -> Self {
        self.default_value = Some(default);
        self
    }

    /// Set optional flag
    pub fn with_optional(mut self, optional: bool) -> Self {
        self.optional = optional;
        self
    }

    /// Set documentation
    pub fn with_docs(mut self, docs: String) -> Self {
        self.docs = Some(docs);
        self
    }
}

/// Metadata for a function
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FunctionMetadata {
    /// Unique identifier (e.g., "Part.SetPosition")
    pub id: String,
    /// Display name
    pub display_name: String,
    /// Return type
    pub return_type: Option<DataType>,
    /// Parameters
    pub parameters: Vec<ParameterMetadata>,
    /// Visibility level
    pub visibility: Visibility,
    /// Whether function is async
    pub is_async: bool,
    /// Whether function is a method (requires object instance)
    pub is_method: bool,
    /// Target type (if method)
    pub target_type: Option<String>,
    /// Documentation
    pub docs: Option<String>,
}

impl FunctionMetadata {
    /// Create a new FunctionMetadata
    pub fn new(id: String, display_name: String) -> Self {
        Self {
            id,
            display_name,
            return_type: None,
            parameters: Vec::new(),
            visibility: Visibility::Public,
            is_async: false,
            is_method: false,
            target_type: None,
            docs: None,
        }
    }

    /// Set return type
    pub fn with_return_type(mut self, return_type: DataType) -> Self {
        self.return_type = Some(return_type);
        self
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

    /// Set async flag
    pub fn with_async(mut self, is_async: bool) -> Self {
        self.is_async = is_async;
        self
    }

    /// Set method flag
    pub fn with_method(mut self, is_method: bool) -> Self {
        self.is_method = is_method;
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

    /// Check if function is visible in scripts
    pub fn is_script_visible(&self) -> bool {
        self.visibility == Visibility::Public
    }

    /// Check if function is visible in Studio
    pub fn is_studio_visible(&self) -> bool {
        self.visibility != Visibility::Private
    }

    /// Get the number of required parameters
    pub fn required_param_count(&self) -> usize {
        self.parameters.iter().filter(|p| !p.optional).count()
    }

    /// Get the number of optional parameters
    pub fn optional_param_count(&self) -> usize {
        self.parameters.iter().filter(|p| p.optional).count()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_function_creation() {
        let func = FunctionMetadata::new("TestFunc".to_string(), "Test Function".to_string());
        assert_eq!(func.id, "TestFunc");
        assert!(func.parameters.is_empty());
        assert!(!func.is_async);
        assert!(!func.is_method);
    }

    #[test]
    fn test_function_builder() {
        let func = FunctionMetadata::new("SetPosition".to_string(), "Set Position".to_string())
            .with_return_type(DataType::Bool)
            .with_parameter(ParameterMetadata::new(
                "position".to_string(),
                DataType::Vec3,
            ))
            .with_parameter(
                ParameterMetadata::new("smooth".to_string(), DataType::Bool)
                    .with_optional(true)
                    .with_default(DefaultValue::Bool(false)),
            )
            .with_visibility(Visibility::Public)
            .with_async(false)
            .with_method(true)
            .with_target_type("Part".to_string())
            .with_docs("Sets the object position".to_string());

        assert!(func.return_type.is_some());
        assert_eq!(func.parameters.len(), 2);
        assert!(func.is_method);
        assert_eq!(func.target_type, Some("Part".to_string()));
        assert_eq!(func.required_param_count(), 1);
        assert_eq!(func.optional_param_count(), 1);
    }

    #[test]
    fn test_parameter_creation() {
        let param = ParameterMetadata::new("test".to_string(), DataType::F32);
        assert_eq!(param.name, "test");
        assert!(!param.optional);
    }

    #[test]
    fn test_parameter_builder() {
        let param = ParameterMetadata::new("count".to_string(), DataType::I32)
            .with_default(DefaultValue::I32(10))
            .with_optional(true)
            .with_docs("The count parameter".to_string());

        assert!(param.default_value.is_some());
        assert!(param.optional);
        assert!(param.docs.is_some());
    }

    #[test]
    fn test_function_visibility() {
        let func_public = FunctionMetadata::new("Test".to_string(), "Test".to_string())
            .with_visibility(Visibility::Public);
        assert!(func_public.is_script_visible());
        assert!(func_public.is_studio_visible());

        let func_internal = FunctionMetadata::new("Test".to_string(), "Test".to_string())
            .with_visibility(Visibility::Internal);
        assert!(!func_internal.is_script_visible());
        assert!(func_internal.is_studio_visible());

        let func_private = FunctionMetadata::new("Test".to_string(), "Test".to_string())
            .with_visibility(Visibility::Private);
        assert!(!func_private.is_script_visible());
        assert!(!func_private.is_studio_visible());
    }
}
