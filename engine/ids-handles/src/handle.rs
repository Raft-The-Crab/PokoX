//! Generation-safe handles for runtime object references
//!
//! Handles are process-local identifiers that include a generation counter
//! to detect stale references (use-after-free bugs).
//!
//! A handle consists of:
//! - Index: The slot in the registry (16 bits)
//! - Generation: Incremented on each reuse (16 bits)
//!
//! This provides 65,536 possible slots with 65,536 generations each.

use serde::{Deserialize, Serialize};
use std::fmt;

/// The index portion of a handle (slot in registry)
pub type HandleIndex = u16;

/// The generation portion of a handle (prevents stale references)
pub type HandleGeneration = u16;

/// A generation-safe handle for runtime object references
///
/// Handles are used for fast, safe object references within a single process.
/// The generation counter ensures that stale handles (from destroyed objects)
/// are detected before use.
#[derive(Clone, Copy, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub struct Handle {
    index: HandleIndex,
    generation: HandleGeneration,
}

impl Handle {
    /// Create a new handle from index and generation
    pub fn new(index: HandleIndex, generation: HandleGeneration) -> Self {
        Self { index, generation }
    }

    /// Get the handle's index
    pub fn index(&self) -> HandleIndex {
        self.index
    }

    /// Get the handle's generation
    pub fn generation(&self) -> HandleGeneration {
        self.generation
    }

    /// Create a handle from a raw u32 value
    ///
    /// Bits 0-15: index
    /// Bits 16-31: generation
    pub fn from_raw(value: u32) -> Self {
        Self {
            index: (value & 0xFFFF) as HandleIndex,
            generation: ((value >> 16) & 0xFFFF) as HandleGeneration,
        }
    }

    /// Convert handle to raw u32 value
    pub fn to_raw(&self) -> u32 {
        (self.generation as u32) << 16 | (self.index as u32)
    }

    /// Create an invalid/null handle
    pub fn null() -> Self {
        Self {
            index: u16::MAX,
            generation: 0,
        }
    }

    /// Check if this is a null handle
    pub fn is_null(&self) -> bool {
        self.index == u16::MAX
    }

    /// Check if the handle is valid (not null)
    pub fn is_valid(&self) -> bool {
        !self.is_null()
    }

    /// Increment the generation (for testing purposes)
    #[cfg(test)]
    pub fn with_incremented_generation(self) -> Self {
        Self {
            index: self.index,
            generation: self.generation.wrapping_add(1),
        }
    }

    /// Create a handle with the same index but different generation
    pub fn with_generation(self, generation: HandleGeneration) -> Self {
        Self {
            index: self.index,
            generation,
        }
    }

    /// Check if this handle is newer than another (based on generation)
    pub fn is_newer_than(&self, other: &Self) -> bool {
        if self.index != other.index {
            return false;
        }
        self.generation.wrapping_sub(other.generation) < u16::MAX / 2
    }

    /// Check if this handle is older than another (based on generation)
    pub fn is_older_than(&self, other: &Self) -> bool {
        if self.index != other.index {
            return false;
        }
        other.is_newer_than(self)
    }

    /// Get the generation distance between two handles (same index only)
    ///
    /// Returns None if handles have different indices.
    pub fn generation_distance(&self, other: &Self) -> Option<u16> {
        if self.index != other.index {
            return None;
        }
        Some(self.generation.wrapping_sub(other.generation))
    }

    /// Convert handle to bytes (4 bytes, little-endian)
    pub fn to_bytes(&self) -> [u8; 4] {
        let raw = self.to_raw();
        raw.to_le_bytes()
    }

    /// Create handle from bytes (4 bytes, little-endian)
    pub fn from_bytes(bytes: [u8; 4]) -> Self {
        let raw = u32::from_le_bytes(bytes);
        Self::from_raw(raw)
    }

    /// Create a handle with index 0 and generation 0
    pub fn first() -> Self {
        Self::new(0, 0)
    }

    /// Increment the index (for testing purposes)
    #[cfg(test)]
    pub fn with_incremented_index(self) -> Self {
        Self {
            index: self.index.wrapping_add(1),
            generation: self.generation,
        }
    }

    /// Create a handle with a specific index, null generation
    pub fn from_index(index: HandleIndex) -> Self {
        Self::new(index, 0)
    }
}

impl PartialOrd for Handle {
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
        Some(self.cmp(other))
    }
}

impl Ord for Handle {
    fn cmp(&self, other: &Self) -> std::cmp::Ordering {
        if self.index != other.index {
            return self.index.cmp(&other.index);
        }
        self.generation.cmp(&other.generation)
    }
}

impl Default for Handle {
    fn default() -> Self {
        Self::null()
    }
}

impl fmt::Display for Handle {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        if self.is_null() {
            write!(f, "Handle(null)")
        } else {
            write!(f, "Handle(idx={}, gen={})", self.index, self.generation)
        }
    }
}

impl fmt::Debug for Handle {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}", self)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_handle_creation() {
        let handle = Handle::new(42, 1);
        assert_eq!(handle.index(), 42);
        assert_eq!(handle.generation(), 1);
    }

    #[test]
    fn test_handle_roundtrip() {
        let handle = Handle::new(123, 456);
        let raw = handle.to_raw();
        let reconstructed = Handle::from_raw(raw);
        assert_eq!(handle, reconstructed);
    }

    #[test]
    fn test_handle_null() {
        let null = Handle::null();
        assert!(null.is_null());
        assert_eq!(null, Handle::null());
    }

    #[test]
    fn test_handle_serialization() {
        let handle = Handle::new(100, 200);
        let serialized = serde_json::to_string(&handle).unwrap();
        let deserialized: Handle = serde_json::from_str(&serialized).unwrap();
        assert_eq!(handle, deserialized);
    }

    #[test]
    fn test_handle_equality() {
        let h1 = Handle::new(1, 1);
        let h2 = Handle::new(1, 1);
        let h3 = Handle::new(1, 2);
        assert_eq!(h1, h2);
        assert_ne!(h1, h3);
    }

    #[test]
    fn test_handle_with_generation() {
        let h1 = Handle::new(1, 1);
        let h2 = h1.with_generation(5);
        assert_eq!(h1.index(), h2.index());
        assert_eq!(h2.generation(), 5);
    }

    #[test]
    fn test_handle_is_newer_than() {
        let h1 = Handle::new(1, 1);
        let h2 = Handle::new(1, 2);
        let h3 = Handle::new(2, 1);

        assert!(h2.is_newer_than(&h1));
        assert!(!h1.is_newer_than(&h2));
        // Different indices should return false
        assert!(!h3.is_newer_than(&h1));
    }

    #[test]
    fn test_handle_is_older_than() {
        let h1 = Handle::new(1, 1);
        let h2 = Handle::new(1, 2);

        assert!(h1.is_older_than(&h2));
        assert!(!h2.is_older_than(&h1));
    }

    #[test]
    fn test_generation_distance() {
        let h1 = Handle::new(1, 1);
        let h2 = Handle::new(1, 5);
        let h3 = Handle::new(2, 1);

        let dist = h1.generation_distance(&h2);
        // wrapping_sub will give the wrapping distance
        assert!(dist.is_some());

        // Different indices
        assert_eq!(h1.generation_distance(&h3), None);
    }

    #[test]
    fn test_to_from_bytes() {
        let handle = Handle::new(123, 456);
        let bytes = handle.to_bytes();
        let reconstructed = Handle::from_bytes(bytes);
        assert_eq!(handle, reconstructed);
    }

    #[test]
    fn test_handle_first() {
        let first = Handle::first();
        assert_eq!(first.index(), 0);
        assert_eq!(first.generation(), 0);
    }

    #[test]
    fn test_from_index() {
        let handle = Handle::from_index(42);
        assert_eq!(handle.index(), 42);
        assert_eq!(handle.generation(), 0);
    }

    #[test]
    fn test_handle_ordering() {
        let h1 = Handle::new(1, 1);
        let h2 = Handle::new(1, 2);
        let h3 = Handle::new(2, 1);

        assert!(h1 < h2);
        assert!(h2 > h1);
        assert!(h1 < h3); // Different index, compare by index
    }

    #[test]
    fn test_handle_bytes_null() {
        let null = Handle::null();
        let bytes = null.to_bytes();
        let reconstructed = Handle::from_bytes(bytes);
        assert_eq!(null, reconstructed);
    }
}
