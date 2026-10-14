//! Stable serialized IDs for persistence across sessions
//!
//! IDs are UUID-based and remain stable across serialization/deserialization.
//! They are used for:
//! - Persistent storage (database, files)
//! - Network replication
//! - Cross-session references

use serde::{Deserialize, Serialize};
use std::fmt;
use thiserror::Error;
use uuid::Uuid;

/// Errors that can occur when parsing an ID
#[derive(Debug, Clone, PartialEq, Eq, Error)]
pub enum IdParseError {
    #[error("Invalid ID format")]
    InvalidFormat,
    #[error("Invalid UUID")]
    InvalidUuid,
    #[error("Invalid ID type")]
    InvalidType,
}

/// A generator for creating IDs in bulk
///
/// Useful for scenarios where many IDs need to be generated at once,
/// such as during initial scene loading or batch operations.
pub struct IdGenerator {
    id_type: IdType,
}

impl IdGenerator {
    /// Create a new ID generator for a specific type
    pub fn new(id_type: IdType) -> Self {
        Self { id_type }
    }

    /// Generate a single ID
    pub fn generate(&self) -> Id {
        Id::new(self.id_type)
    }

    /// Generate multiple IDs at once
    pub fn generate_batch(&self, count: usize) -> Vec<Id> {
        (0..count).map(|_| Id::new(self.id_type)).collect()
    }

    /// Generate IDs with a custom function
    pub fn generate_with<F>(&self, count: usize, mut f: F) -> Vec<Id>
    where
        F: FnMut() -> Id,
    {
        (0..count).map(|_| f()).collect()
    }
}

impl Default for IdGenerator {
    fn default() -> Self {
        Self::new(IdType::Custom)
    }
}

/// The type/category of an object
#[derive(Clone, Copy, PartialEq, Eq, Hash, Debug, Serialize, Deserialize)]
#[repr(u8)]
pub enum IdType {
    /// Workspace container
    Workspace = 0,
    /// Model object
    Model = 1,
    /// Part object
    Part = 2,
    /// Asset
    Asset = 3,
    /// Package
    Package = 4,
    /// User-defined or custom type
    Custom = 255,
}

impl IdType {
    /// Get the numeric value of the type
    pub fn as_u8(self) -> u8 {
        self as u8
    }

    /// Create an IdType from a numeric value
    pub fn from_u8(value: u8) -> Option<Self> {
        match value {
            0 => Some(IdType::Workspace),
            1 => Some(IdType::Model),
            2 => Some(IdType::Part),
            3 => Some(IdType::Asset),
            4 => Some(IdType::Package),
            255 => Some(IdType::Custom),
            _ => None,
        }
    }
}

/// A stable, unique identifier for objects
///
/// IDs are UUIDs that remain stable across serialization/deserialization
/// and can be used for persistent storage and network replication.
#[derive(Clone, Copy, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub struct Id {
    uuid: Uuid,
    id_type: IdType,
}

impl Id {
    /// Create a new random ID with a specific type
    pub fn new(id_type: IdType) -> Self {
        Self {
            uuid: Uuid::new_v4(),
            id_type,
        }
    }

    /// Create a new random ID (defaults to Custom type)
    pub fn new_custom() -> Self {
        Self::new(IdType::Custom)
    }

    /// Create an ID from a UUID and type
    pub fn from_uuid(uuid: Uuid, id_type: IdType) -> Self {
        Self { uuid, id_type }
    }

    /// Get the underlying UUID
    pub fn as_uuid(&self) -> Uuid {
        self.uuid
    }

    /// Get the ID type
    pub fn id_type(&self) -> IdType {
        self.id_type
    }

    /// Set the ID type
    pub fn with_type(mut self, id_type: IdType) -> Self {
        self.id_type = id_type;
        self
    }

    /// Create an ID from bytes (16 bytes UUID + 1 byte type)
    pub fn from_bytes(bytes: [u8; 17]) -> Self {
        let uuid_bytes: [u8; 16] = bytes[0..16].try_into().unwrap();
        let id_type = IdType::from_u8(bytes[16]).unwrap_or(IdType::Custom);
        Self {
            uuid: Uuid::from_bytes(uuid_bytes),
            id_type,
        }
    }

    /// Get the ID as bytes (16 bytes UUID + 1 byte type)
    pub fn to_bytes(&self) -> [u8; 17] {
        let mut bytes = [0u8; 17];
        bytes[0..16].copy_from_slice(self.uuid.as_bytes());
        bytes[16] = self.id_type.as_u8();
        bytes
    }

    /// Parse an ID from a string (format: "uuid-type")
    pub fn parse_str(s: &str) -> Result<Self, IdParseError> {
        let parts: Vec<&str> = s.split('-').collect();
        if parts.len() < 2 {
            return Err(IdParseError::InvalidFormat);
        }

        // The last part should be the type
        let type_str = parts.last().ok_or(IdParseError::InvalidFormat)?;
        let id_type = type_str.parse::<u8>()
            .ok()
            .and_then(IdType::from_u8)
            .ok_or(IdParseError::InvalidType)?;

        // Reconstruct the UUID part (everything except the last type segment)
        let uuid_str = parts[..parts.len() - 1].join("-");
        let uuid = Uuid::parse_str(&uuid_str).map_err(|_| IdParseError::InvalidUuid)?;

        Ok(Self { uuid, id_type })
    }

    /// Create a nil ID (all zeros) with a specific type
    pub fn nil(id_type: IdType) -> Self {
        Self {
            uuid: Uuid::nil(),
            id_type,
        }
    }

    /// Check if this is a nil ID
    pub fn is_nil(&self) -> bool {
        self.uuid.is_nil()
    }

    /// Check if the ID matches a specific type
    pub fn is_type(&self, id_type: IdType) -> bool {
        self.id_type == id_type
    }

    /// Create a new random ID (convenience method for default Custom type)
    pub fn random() -> Self {
        Self::new(IdType::Custom)
    }

    /// Clone the ID with a different type
    pub fn clone_with_type(&self, id_type: IdType) -> Self {
        Self {
            uuid: self.uuid,
            id_type,
        }
    }

    /// Get the ID as a hex string (UUID portion only)
    pub fn to_hex(&self) -> String {
        format!("{}", self.uuid)
    }

    /// Check if two IDs have the same UUID (ignoring type)
    pub fn same_uuid(&self, other: &Self) -> bool {
        self.uuid == other.uuid
    }
}

impl Default for Id {
    fn default() -> Self {
        Self::new_custom()
    }
}

impl fmt::Display for Id {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}-{}", self.uuid, self.id_type.as_u8())
    }
}

impl fmt::Debug for Id {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "Id(uuid={}, type={:?})", self.uuid, self.id_type)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_id_creation() {
        let id1 = Id::new(IdType::Model);
        let id2 = Id::new(IdType::Model);
        assert_ne!(id1, id2, "IDs should be unique");
        assert_eq!(id1.id_type(), IdType::Model);
    }

    #[test]
    fn test_id_with_type() {
        let workspace_id = Id::new(IdType::Workspace);
        let part_id = Id::new(IdType::Part);
        assert_eq!(workspace_id.id_type(), IdType::Workspace);
        assert_eq!(part_id.id_type(), IdType::Part);
        assert_ne!(workspace_id.id_type(), part_id.id_type());
    }

    #[test]
    fn test_id_serialization() {
        let id = Id::new(IdType::Model);
        let serialized = serde_json::to_string(&id).unwrap();
        let deserialized: Id = serde_json::from_str(&serialized).unwrap();
        assert_eq!(id, deserialized);
        assert_eq!(id.id_type(), deserialized.id_type());
    }

    #[test]
    fn test_id_nil() {
        let nil = Id::nil(IdType::Custom);
        assert!(nil.is_nil());
        assert_eq!(nil, Id::nil(IdType::Custom));
    }

    #[test]
    fn test_id_from_bytes() {
        let id = Id::new(IdType::Asset);
        let bytes = id.to_bytes();
        assert_eq!(bytes.len(), 17);
        let reconstructed = Id::from_bytes(bytes);
        assert_eq!(id, reconstructed);
        assert_eq!(id.id_type(), reconstructed.id_type());
    }

    #[test]
    fn test_id_parse_str() {
        let id = Id::new(IdType::Model);
        let s = id.to_string();
        let parsed = Id::parse_str(&s).unwrap();
        assert_eq!(id, parsed);
        assert_eq!(id.id_type(), parsed.id_type());
    }

    #[test]
    fn test_id_is_type() {
        let model_id = Id::new(IdType::Model);
        assert!(model_id.is_type(IdType::Model));
        assert!(!model_id.is_type(IdType::Part));
    }

    #[test]
    fn test_id_with_type_method() {
        let id = Id::new(IdType::Custom).with_type(IdType::Part);
        assert_eq!(id.id_type(), IdType::Part);
    }

    #[test]
    fn test_id_type_from_u8() {
        assert_eq!(IdType::from_u8(0), Some(IdType::Workspace));
        assert_eq!(IdType::from_u8(1), Some(IdType::Model));
        assert_eq!(IdType::from_u8(255), Some(IdType::Custom));
        assert_eq!(IdType::from_u8(99), None);
    }

    #[test]
    fn test_id_equality_with_different_types() {
        let uuid = Uuid::new_v4();
        let id1 = Id::from_uuid(uuid, IdType::Model);
        let id2 = Id::from_uuid(uuid, IdType::Part);
        assert_ne!(id1, id2, "IDs with same UUID but different types should not be equal");
    }

    #[test]
    fn test_id_generator() {
        let generator = IdGenerator::new(IdType::Model);
        let id = generator.generate();
        assert_eq!(id.id_type(), IdType::Model);
    }

    #[test]
    fn test_id_generator_batch() {
        let generator = IdGenerator::new(IdType::Part);
        let ids = generator.generate_batch(10);
        assert_eq!(ids.len(), 10);
        for id in &ids {
            assert_eq!(id.id_type(), IdType::Part);
        }
        // All IDs should be unique
        let unique_count = ids.iter().collect::<std::collections::HashSet<_>>().len();
        assert_eq!(unique_count, 10);
    }

    #[test]
    fn test_id_random() {
        let id = Id::random();
        assert_eq!(id.id_type(), IdType::Custom);
        assert!(!id.is_nil());
    }

    #[test]
    fn test_id_clone_with_type() {
        let id = Id::new(IdType::Model);
        let cloned = id.clone_with_type(IdType::Part);
        assert_eq!(id.id_type(), IdType::Model);
        assert_eq!(cloned.id_type(), IdType::Part);
        assert!(id.same_uuid(&cloned));
    }

    #[test]
    fn test_id_to_hex() {
        let id = Id::new(IdType::Model);
        let hex = id.to_hex();
        assert!(hex.len() == 36); // UUID hex format
    }

    #[test]
    fn test_id_same_uuid() {
        let uuid = Uuid::new_v4();
        let id1 = Id::from_uuid(uuid, IdType::Model);
        let id2 = Id::from_uuid(uuid, IdType::Part);
        assert!(id1.same_uuid(&id2));
        assert_ne!(id1, id2);
    }
}
