//! Validation logic for metadata
//!
//! This module provides validation functions for checking metadata integrity,
//! including reference validation, circular inheritance detection, and
//! data type consistency checks.

use thiserror::Error;

use super::events::EventMetadata;
use super::functions::FunctionMetadata;
use super::properties::{DataType, DefaultValue, PropertyMetadata};
use super::types::TypeMetadata;

/// Errors that can occur during metadata validation
#[derive(Debug, Clone, PartialEq, Eq, Error)]
pub enum ValidationError {
    #[error("Duplicate ID: {0}")]
    DuplicateId(String),
    #[error("Reference not found: {0}")]
    ReferenceNotFound(String),
    #[error("Circular inheritance detected: {0}")]
    CircularInheritance(String),
    #[error("Invalid data type: {0}")]
    InvalidDataType(String),
    #[error("Validation failed: {0}")]
    ValidationFailed(String),
    #[error("Invalid default value for type {data_type}: {reason}")]
    InvalidDefaultValue { data_type: String, reason: String },
    #[error("Inconsistent metadata: {0}")]
    InconsistentMetadata(String),
}

/// Validate a complete set of metadata
pub fn validate_metadata(
    types: &[TypeMetadata],
    properties: &[PropertyMetadata],
    functions: &[FunctionMetadata],
    events: &[EventMetadata],
) -> Result<(), ValidationError> {
    // Check for duplicate IDs
    validate_no_duplicates(types, properties, functions, events)?;

    // Check for broken references
    validate_references(types, properties, functions, events)?;

    // Check for circular inheritance
    validate_inheritance(types)?;

    // Validate property default values match their data types
    validate_property_defaults(properties)?;

    // Validate property data types
    validate_property_data_types(properties)?;

    // Validate function parameter types
    validate_function_parameters(functions)?;

    // Validate event parameter types
    validate_event_parameters(events)?;

    Ok(())
}

/// Check for duplicate IDs across all metadata
fn validate_no_duplicates(
    types: &[TypeMetadata],
    properties: &[PropertyMetadata],
    functions: &[FunctionMetadata],
    events: &[EventMetadata],
) -> Result<(), ValidationError> {
    let mut seen = std::collections::HashSet::new();

    for type_meta in types {
        if !seen.insert(type_meta.id.clone()) {
            return Err(ValidationError::DuplicateId(format!(
                "Type: {}",
                type_meta.id
            )));
        }
    }

    for prop_meta in properties {
        if !seen.insert(prop_meta.id.clone()) {
            return Err(ValidationError::DuplicateId(format!(
                "Property: {}",
                prop_meta.id
            )));
        }
    }

    for func_meta in functions {
        if !seen.insert(func_meta.id.clone()) {
            return Err(ValidationError::DuplicateId(format!(
                "Function: {}",
                func_meta.id
            )));
        }
    }

    for event_meta in events {
        if !seen.insert(event_meta.id.clone()) {
            return Err(ValidationError::DuplicateId(format!(
                "Event: {}",
                event_meta.id
            )));
        }
    }

    Ok(())
}

/// Validate that all references point to existing metadata
fn validate_references(
    types: &[TypeMetadata],
    properties: &[PropertyMetadata],
    functions: &[FunctionMetadata],
    events: &[EventMetadata],
) -> Result<(), ValidationError> {
    let type_ids: std::collections::HashSet<&str> = types.iter().map(|t| t.id.as_str()).collect();
    let prop_ids: std::collections::HashSet<&str> =
        properties.iter().map(|p| p.id.as_str()).collect();
    let function_ids: std::collections::HashSet<&str> =
        functions.iter().map(|f| f.id.as_str()).collect();
    let event_ids: std::collections::HashSet<&str> = events.iter().map(|e| e.id.as_str()).collect();

    // Check type references
    for type_meta in types {
        if let Some(base_type) = &type_meta.base_type {
            if !type_ids.contains(base_type.as_str()) {
                return Err(ValidationError::ReferenceNotFound(format!(
                    "Base type '{}' not found for '{}'",
                    base_type, type_meta.id
                )));
            }
        }

        for prop_id in &type_meta.properties {
            if !prop_ids.contains(prop_id.as_str()) {
                return Err(ValidationError::ReferenceNotFound(format!(
                    "Property '{}' not found for type '{}'",
                    prop_id, type_meta.id
                )));
            }
        }

        for func_id in &type_meta.functions {
            if !function_ids.contains(func_id.as_str()) {
                return Err(ValidationError::ReferenceNotFound(format!(
                    "Function '{}' not found for type '{}'",
                    func_id, type_meta.id
                )));
            }
        }

        for event_id in &type_meta.events {
            if !event_ids.contains(event_id.as_str()) {
                return Err(ValidationError::ReferenceNotFound(format!(
                    "Event '{}' not found for type '{}'",
                    event_id, type_meta.id
                )));
            }
        }
    }

    // Check function target types
    for func_meta in functions {
        if let Some(target_type) = &func_meta.target_type {
            if !type_ids.contains(target_type.as_str()) {
                return Err(ValidationError::ReferenceNotFound(format!(
                    "Target type '{}' not found for function '{}'",
                    target_type, func_meta.id
                )));
            }
        }
    }

    // Check event target types
    for event_meta in events {
        if let Some(target_type) = &event_meta.target_type {
            if !type_ids.contains(target_type.as_str()) {
                return Err(ValidationError::ReferenceNotFound(format!(
                    "Target type '{}' not found for event '{}'",
                    target_type, event_meta.id
                )));
            }
        }
    }

    Ok(())
}

/// Validate no circular inheritance in type hierarchy
fn validate_inheritance(types: &[TypeMetadata]) -> Result<(), ValidationError> {
    let type_map: std::collections::HashMap<&str, &TypeMetadata> =
        types.iter().map(|t| (t.id.as_str(), t)).collect();

    for type_id in type_map.keys() {
        check_circular_inheritance(type_id, &type_map, &mut Vec::new())?;
    }

    Ok(())
}

/// Recursive check for circular inheritance
fn check_circular_inheritance(
    type_id: &str,
    type_map: &std::collections::HashMap<&str, &TypeMetadata>,
    visited: &mut Vec<String>,
) -> Result<(), ValidationError> {
    if visited.contains(&type_id.to_string()) {
        return Err(ValidationError::CircularInheritance(format!(
            "Circular inheritance detected: {}",
            visited.join(" -> ")
        )));
    }

    visited.push(type_id.to_string());

    if let Some(type_meta) = type_map.get(type_id) {
        if let Some(base_type) = &type_meta.base_type {
            check_circular_inheritance(base_type, type_map, visited)?;
        }
    }

    visited.pop();
    Ok(())
}

/// Validate that property default values match their data types
fn validate_property_defaults(properties: &[PropertyMetadata]) -> Result<(), ValidationError> {
    for prop in properties {
        if let Some(default) = &prop.default_value {
            validate_default_matches_type(&prop.data_type, default, &prop.id)?;
        }
    }
    Ok(())
}

/// Validate property data types
fn validate_property_data_types(properties: &[PropertyMetadata]) -> Result<(), ValidationError> {
    for prop in properties {
        validate_data_type_is_valid(&prop.data_type, &prop.id)?;
    }
    Ok(())
}

/// Validate that a default value matches the expected data type
fn validate_default_matches_type(
    data_type: &DataType,
    default: &DefaultValue,
    prop_id: &str,
) -> Result<(), ValidationError> {
    let is_valid = matches!(
        (data_type, default),
        (DataType::I32, DefaultValue::I32(_))
            | (DataType::U32, DefaultValue::U32(_))
            | (DataType::I64, DefaultValue::I64(_))
            | (DataType::U64, DefaultValue::U64(_))
            | (DataType::F32, DefaultValue::F32(_))
            | (DataType::F64, DefaultValue::F64(_))
            | (DataType::Bool, DefaultValue::Bool(_))
            | (DataType::String, DefaultValue::String(_))
            | (DataType::Vec3, DefaultValue::Vec3(_))
            | (DataType::Quat, DefaultValue::Quat(_))
            | (DataType::Mat4, DefaultValue::Mat4(_))
            | (DataType::Id, DefaultValue::Id(_))
            | (DataType::Option(_), DefaultValue::Null)
            | (DataType::Color, DefaultValue::Vec4(_))
            | (DataType::Vec2, DefaultValue::Vec2(_))
            | (DataType::Vec4, DefaultValue::Vec4(_))
            | (DataType::AABB { min: _, max: _ }, DefaultValue::AABB { .. })
            | (DataType::Timestamp, DefaultValue::Timestamp(_))
            | (DataType::Duration, DefaultValue::Duration(_))
    );

    if !is_valid {
        return Err(ValidationError::InvalidDefaultValue {
            data_type: format!("{:?}", data_type),
            reason: format!(
                "Default value does not match type for property '{}'",
                prop_id
            ),
        });
    }

    Ok(())
}

/// Validate function parameter types
fn validate_function_parameters(functions: &[FunctionMetadata]) -> Result<(), ValidationError> {
    for func in functions {
        for param in &func.parameters {
            validate_data_type_is_valid(&param.data_type, &func.id)?;
        }
        if let Some(return_type) = &func.return_type {
            validate_data_type_is_valid(return_type, &func.id)?;
        }
    }
    Ok(())
}

/// Validate event parameter types
fn validate_event_parameters(events: &[EventMetadata]) -> Result<(), ValidationError> {
    for event in events {
        for param in &event.parameters {
            validate_data_type_is_valid(&param.data_type, &event.id)?;
        }
    }
    Ok(())
}

/// Validate that a data type is well-formed
fn validate_data_type_is_valid(
    data_type: &crate::DataType,
    context: &str,
) -> Result<(), ValidationError> {
    match data_type {
        crate::DataType::Option(inner) => validate_data_type_is_valid(inner, context)?,
        crate::DataType::Array(inner) => validate_data_type_is_valid(inner, context)?,
        crate::DataType::ObjectType(type_name) if type_name.is_empty() => {
            return Err(ValidationError::InvalidDataType(format!(
                "Empty object type name in '{}'",
                context
            )));
        }
        crate::DataType::Enum(enum_name) if enum_name.is_empty() => {
            return Err(ValidationError::InvalidDataType(format!(
                "Empty enum name in '{}'",
                context
            )));
        }
        crate::DataType::Color
        | crate::DataType::Vec2
        | crate::DataType::Vec4
        | crate::DataType::AABB { min: _, max: _ }
        | crate::DataType::Timestamp
        | crate::DataType::Duration => {}
        _ => {}
    }
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::DataType;

    #[test]
    fn test_validate_no_duplicates() {
        let types = vec![TypeMetadata::new("Type1".to_string(), "Type 1".to_string())];
        let properties = vec![PropertyMetadata::new("Prop1".to_string(), DataType::F32)];
        let functions = vec![FunctionMetadata::new(
            "Func1".to_string(),
            "Func 1".to_string(),
        )];
        let events = vec![EventMetadata::new(
            "Event1".to_string(),
            "Event 1".to_string(),
        )];

        assert!(validate_metadata(&types, &properties, &functions, &events).is_ok());
    }

    #[test]
    fn test_validate_duplicate_type() {
        let types = vec![
            TypeMetadata::new("Type1".to_string(), "Type 1".to_string()),
            TypeMetadata::new("Type1".to_string(), "Type 1 Duplicate".to_string()),
        ];

        assert!(matches!(
            validate_metadata(&types, &[], &[], &[]),
            Err(ValidationError::DuplicateId(_))
        ));
    }

    #[test]
    fn test_validate_circular_inheritance() {
        let types = vec![
            TypeMetadata::new("TypeA".to_string(), "Type A".to_string())
                .with_base_type("TypeB".to_string()),
            TypeMetadata::new("TypeB".to_string(), "Type B".to_string())
                .with_base_type("TypeA".to_string()),
        ];

        assert!(matches!(
            validate_metadata(&types, &[], &[], &[]),
            Err(ValidationError::CircularInheritance(_))
        ));
    }

    #[test]
    fn test_validate_reference_not_found() {
        let types = vec![TypeMetadata::new("Type1".to_string(), "Type 1".to_string())
            .with_base_type("NonExistent".to_string())];

        assert!(matches!(
            validate_metadata(&types, &[], &[], &[]),
            Err(ValidationError::ReferenceNotFound(_))
        ));
    }

    #[test]
    fn test_validate_invalid_default_value() {
        let properties = vec![PropertyMetadata::new("Prop1".to_string(), DataType::F32)
            .with_default(DefaultValue::I32(42))];

        assert!(matches!(
            validate_metadata(&[], &properties, &[], &[]),
            Err(ValidationError::InvalidDefaultValue { .. })
        ));
    }

    #[test]
    fn test_validate_valid_default_value() {
        let properties = vec![PropertyMetadata::new("Prop1".to_string(), DataType::F32)
            .with_default(DefaultValue::F32(1.0))];

        assert!(validate_metadata(&[], &properties, &[], &[]).is_ok());
    }

    #[test]
    fn test_validate_property_reference_not_found() {
        let types = vec![TypeMetadata::new("Type1".to_string(), "Type 1".to_string())
            .with_property("NonExistentProp".to_string())];

        assert!(matches!(
            validate_metadata(&types, &[], &[], &[]),
            Err(ValidationError::ReferenceNotFound(_))
        ));
    }

    #[test]
    fn test_validate_extended_data_types() {
        let properties = vec![
            PropertyMetadata::new("ColorProp".to_string(), DataType::Color)
                .with_default(DefaultValue::Vec4([1.0, 1.0, 1.0, 1.0])),
            PropertyMetadata::new("TimestampProp".to_string(), DataType::Timestamp)
                .with_default(DefaultValue::Timestamp(0)),
            PropertyMetadata::new("DurationProp".to_string(), DataType::Duration)
                .with_default(DefaultValue::Duration(1000)),
        ];

        assert!(validate_metadata(&[], &properties, &[], &[]).is_ok());
    }
}
