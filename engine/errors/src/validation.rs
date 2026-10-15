//! Validation-related errors

use thiserror::Error;

/// Validation error kind
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum ValidationKind {
    /// Required field missing
    RequiredFieldMissing,
    /// Invalid format
    InvalidFormat,
    /// Out of range
    OutOfRange,
    /// Invalid value
    InvalidValue,
    /// Constraint violation
    ConstraintViolation,
    /// Type mismatch
    TypeMismatch,
    /// Failed custom validation
    Custom(String),
}

/// Validation-specific error
#[derive(Debug, Error)]
pub enum ValidationError {
    #[error("Required field missing: {0}")]
    RequiredFieldMissing(String),
    #[error("Invalid format for {field}: {value}")]
    InvalidFormat { field: String, value: String },
    #[error("Value out of range for {field}: {value} (min: {min}, max: {max})")]
    OutOfRange {
        field: String,
        value: String,
        min: String,
        max: String,
    },
    #[error("Invalid value for {field}: {value}")]
    InvalidValue { field: String, value: String },
    #[error("Constraint violation: {0}")]
    ConstraintViolation(String),
    #[error("Type mismatch for {field}: expected {expected}, got {actual}")]
    TypeMismatch {
        field: String,
        expected: String,
        actual: String,
    },
    #[error("Custom validation failed: {0}")]
    Custom(String),
}

impl ValidationError {
    /// Get the error kind
    pub fn kind(&self) -> ValidationKind {
        match self {
            ValidationError::RequiredFieldMissing(_) => ValidationKind::RequiredFieldMissing,
            ValidationError::InvalidFormat { .. } => ValidationKind::InvalidFormat,
            ValidationError::OutOfRange { .. } => ValidationKind::OutOfRange,
            ValidationError::InvalidValue { .. } => ValidationKind::InvalidValue,
            ValidationError::ConstraintViolation(_) => ValidationKind::ConstraintViolation,
            ValidationError::TypeMismatch { .. } => ValidationKind::TypeMismatch,
            ValidationError::Custom(s) => ValidationKind::Custom(s.clone()),
        }
    }

    /// Get the field name if applicable
    pub fn field(&self) -> Option<&str> {
        match self {
            ValidationError::InvalidFormat { field, .. } => Some(field),
            ValidationError::OutOfRange { field, .. } => Some(field),
            ValidationError::InvalidValue { field, .. } => Some(field),
            ValidationError::TypeMismatch { field, .. } => Some(field),
            _ => None,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_validation_error_kind() {
        let error = ValidationError::RequiredFieldMissing("name".to_string());
        assert_eq!(error.kind(), ValidationKind::RequiredFieldMissing);
    }

    #[test]
    fn test_field_extraction() {
        let error = ValidationError::InvalidFormat {
            field: "email".to_string(),
            value: "invalid".to_string(),
        };
        assert_eq!(error.field(), Some("email"));
    }

    #[test]
    fn test_out_of_range() {
        let error = ValidationError::OutOfRange {
            field: "age".to_string(),
            value: "150".to_string(),
            min: "0".to_string(),
            max: "120".to_string(),
        };
        assert!(error.to_string().contains("age"));
        assert!(error.to_string().contains("150"));
    }

    #[test]
    fn test_type_mismatch() {
        let error = ValidationError::TypeMismatch {
            field: "count".to_string(),
            expected: "number".to_string(),
            actual: "string".to_string(),
        };
        assert!(error.to_string().contains("count"));
        assert!(error.to_string().contains("number"));
        assert!(error.to_string().contains("string"));
    }
}
