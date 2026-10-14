//! Serialization utilities for IDs and Handles
//!
//! This module provides efficient serialization formats for network transmission
//! and persistent storage.

use crate::{Handle, Id};
use serde::{Deserialize, Serialize};

/// Compact binary representation of an ID (17 bytes: 16 UUID + 1 type)
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct IdBytes {
    bytes: [u8; 17],
}

impl IdBytes {
    /// Create IdBytes from an Id
    pub fn from_id(id: Id) -> Self {
        Self {
            bytes: id.to_bytes(),
        }
    }

    /// Convert back to Id
    pub fn to_id(&self) -> Id {
        Id::from_bytes(self.bytes)
    }

    /// Get the raw bytes
    pub fn as_bytes(&self) -> &[u8; 17] {
        &self.bytes
    }

    /// Create from raw bytes
    pub fn from_bytes(bytes: [u8; 17]) -> Self {
        Self { bytes }
    }
}

/// Compact binary representation of a Handle (4 bytes: 2 index + 2 generation)
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub struct HandleBytes {
    bytes: [u8; 4],
}

impl HandleBytes {
    /// Create HandleBytes from a Handle
    pub fn from_handle(handle: Handle) -> Self {
        let raw = handle.to_raw();
        Self {
            bytes: raw.to_le_bytes(),
        }
    }

    /// Convert back to Handle
    pub fn to_handle(&self) -> Handle {
        let raw = u32::from_le_bytes(self.bytes);
        Handle::from_raw(raw)
    }

    /// Get the raw bytes
    pub fn as_bytes(&self) -> &[u8; 4] {
        &self.bytes
    }

    /// Create from raw bytes
    pub fn from_bytes(bytes: [u8; 4]) -> Self {
        Self { bytes }
    }
}

/// Serialized batch of IDs for efficient transmission
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct IdBatch {
    ids: Vec<Vec<u8>>,
}

impl IdBatch {
    /// Create a new empty batch
    pub fn new() -> Self {
        Self { ids: Vec::new() }
    }

    /// Create a batch with pre-allocated capacity
    pub fn with_capacity(capacity: usize) -> Self {
        Self {
            ids: Vec::with_capacity(capacity),
        }
    }

    /// Add an ID to the batch
    pub fn add(&mut self, id: Id) {
        self.ids.push(id.to_bytes().to_vec());
    }

    /// Add multiple IDs to the batch
    pub fn extend(&mut self, ids: impl IntoIterator<Item = Id>) {
        for id in ids {
            self.add(id);
        }
    }

    /// Get the number of IDs in the batch
    pub fn len(&self) -> usize {
        self.ids.len()
    }

    /// Check if the batch is empty
    pub fn is_empty(&self) -> bool {
        self.ids.is_empty()
    }

    /// Convert to a vector of Ids
    pub fn to_ids(&self) -> Vec<Id> {
        self.ids
            .iter()
            .filter_map(|bytes| {
                if bytes.len() == 17 {
                    let mut arr = [0u8; 17];
                    arr.copy_from_slice(bytes);
                    Some(Id::from_bytes(arr))
                } else {
                    None
                }
            })
            .collect()
    }

    /// Iterate over the IDs
    pub fn iter(&self) -> impl Iterator<Item = Id> + '_ {
        self.ids.iter().filter_map(|bytes| {
            if bytes.len() == 17 {
                let mut arr = [0u8; 17];
                arr.copy_from_slice(bytes);
                Some(Id::from_bytes(arr))
            } else {
                None
            }
        })
    }
}

impl Default for IdBatch {
    fn default() -> Self {
        Self::new()
    }
}

/// Serialized batch of Handles for efficient transmission
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct HandleBatch {
    handles: Vec<u32>,
}

impl HandleBatch {
    /// Create a new empty batch
    pub fn new() -> Self {
        Self {
            handles: Vec::new(),
        }
    }

    /// Create a batch with pre-allocated capacity
    pub fn with_capacity(capacity: usize) -> Self {
        Self {
            handles: Vec::with_capacity(capacity),
        }
    }

    /// Add a Handle to the batch
    pub fn add(&mut self, handle: Handle) {
        self.handles.push(handle.to_raw());
    }

    /// Add multiple Handles to the batch
    pub fn extend(&mut self, handles: impl IntoIterator<Item = Handle>) {
        for handle in handles {
            self.add(handle);
        }
    }

    /// Get the number of Handles in the batch
    pub fn len(&self) -> usize {
        self.handles.len()
    }

    /// Check if the batch is empty
    pub fn is_empty(&self) -> bool {
        self.handles.is_empty()
    }

    /// Convert to a vector of Handles
    pub fn to_handles(&self) -> Vec<Handle> {
        self.handles
            .iter()
            .map(|&raw| Handle::from_raw(raw))
            .collect()
    }

    /// Iterate over the Handles
    pub fn iter(&self) -> impl Iterator<Item = Handle> + '_ {
        self.handles.iter().map(|&raw| Handle::from_raw(raw))
    }
}

impl Default for HandleBatch {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::IdType;

    #[test]
    fn test_id_bytes_roundtrip() {
        let id = Id::new(IdType::Model);
        let bytes = IdBytes::from_id(id);
        let reconstructed = bytes.to_id();
        assert_eq!(id, reconstructed);
    }

    #[test]
    fn test_handle_bytes_roundtrip() {
        let handle = Handle::new(42, 1);
        let bytes = HandleBytes::from_handle(handle);
        let reconstructed = bytes.to_handle();
        assert_eq!(handle, reconstructed);
    }

    #[test]
    fn test_id_batch() {
        let mut batch = IdBatch::with_capacity(10);
        batch.add(Id::new(IdType::Model));
        batch.add(Id::new(IdType::Part));
        batch.add(Id::new(IdType::Asset));

        assert_eq!(batch.len(), 3);
        assert!(!batch.is_empty());

        let ids = batch.to_ids();
        assert_eq!(ids.len(), 3);
    }

    #[test]
    fn test_handle_batch() {
        let mut batch = HandleBatch::with_capacity(10);
        batch.add(Handle::new(1, 1));
        batch.add(Handle::new(2, 1));
        batch.add(Handle::new(3, 1));

        assert_eq!(batch.len(), 3);
        assert!(!batch.is_empty());

        let handles = batch.to_handles();
        assert_eq!(handles.len(), 3);
    }

    #[test]
    fn test_id_batch_serialization() {
        let mut batch = IdBatch::new();
        batch.add(Id::new(IdType::Model));
        batch.add(Id::new(IdType::Part));

        let serialized = serde_json::to_string(&batch).unwrap();
        let deserialized: IdBatch = serde_json::from_str(&serialized).unwrap();

        assert_eq!(batch.len(), deserialized.len());
    }

    #[test]
    fn test_handle_batch_serialization() {
        let mut batch = HandleBatch::new();
        batch.add(Handle::new(1, 1));
        batch.add(Handle::new(2, 1));

        let serialized = serde_json::to_string(&batch).unwrap();
        let deserialized: HandleBatch = serde_json::from_str(&serialized).unwrap();

        assert_eq!(batch.len(), deserialized.len());
    }
}
