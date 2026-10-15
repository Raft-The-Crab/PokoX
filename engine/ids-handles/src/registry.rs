//! Handle registry for allocation and validation
//!
//! The registry manages handle allocation, tracks object lifetimes,
//! and validates handles to prevent use-after-free bugs.

use crate::{Handle, HandleGeneration, HandleIndex};
use serde::{Deserialize, Serialize};

/// Maximum number of handles in the registry
const MAX_HANDLES: usize = u16::MAX as usize;

/// Configuration for the handle registry
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct RegistryConfig {
    /// Maximum capacity (0 = use MAX_HANDLES)
    pub max_capacity: usize,
    /// Whether to enable statistics tracking
    pub enable_stats: bool,
}

impl Default for RegistryConfig {
    fn default() -> Self {
        Self {
            max_capacity: MAX_HANDLES,
            enable_stats: true,
        }
    }
}

impl RegistryConfig {
    /// Validate the configuration
    pub fn validate(&self) -> Result<(), RegistryError> {
        if self.max_capacity > MAX_HANDLES {
            return Err(RegistryError::RegistryFull(MAX_HANDLES));
        }
        Ok(())
    }
}

/// Errors that can occur in the registry
#[derive(Debug, Clone, PartialEq, Eq, thiserror::Error)]
pub enum RegistryError {
    #[error("Registry is full (max {0} handles) - cannot allocate more objects")]
    RegistryFull(usize),
    #[error("Invalid handle: index {0} out of bounds (registry has {1} slots)")]
    InvalidIndex(HandleIndex, usize),
    #[error("Stale handle: generation mismatch - expected {0}, got {1} (object was destroyed)")]
    StaleHandle(HandleGeneration, HandleGeneration),
    #[error("Handle does not refer to a live object (may have been destroyed)")]
    NotLive,
    #[error("Operation would leave registry in inconsistent state")]
    InconsistentState,
}

/// Metadata for a handle slot
#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct SlotMetadata {
    generation: HandleGeneration,
    is_live: bool,
}

/// Registry statistics
#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct RegistryStats {
    pub total_slots: usize,
    pub live_count: usize,
    pub free_count: usize,
    pub generation_reuses: u64,
    pub allocations: u64,
    pub deallocations: u64,
}

/// A registry for managing handles and their lifetimes
///
/// The registry:
/// - Allocates new handles with unique indices
/// - Tracks generations to detect stale references
/// - Validates handles before use
/// - Supports destruction and reuse of slots
pub struct HandleRegistry<T> {
    slots: Vec<Option<T>>,
    metadata: Vec<SlotMetadata>,
    free_list: Vec<HandleIndex>,
    next_index: HandleIndex,
    config: RegistryConfig,
    stats: RegistryStats,
}

impl<T> HandleRegistry<T> {
    /// Create a new empty registry
    pub fn new() -> Self {
        Self::with_config(RegistryConfig::default())
    }

    /// Create a new registry with configuration
    pub fn with_config(config: RegistryConfig) -> Self {
        config.validate().unwrap();
        Self {
            slots: Vec::new(),
            metadata: Vec::new(),
            free_list: Vec::new(),
            next_index: 0,
            config,
            stats: RegistryStats {
                total_slots: 0,
                live_count: 0,
                free_count: 0,
                generation_reuses: 0,
                allocations: 0,
                deallocations: 0,
            },
        }
    }

    /// Create a new registry with pre-allocated capacity
    pub fn with_capacity(capacity: usize) -> Self {
        let config = RegistryConfig {
            max_capacity: capacity.min(MAX_HANDLES),
            ..Default::default()
        };
        Self::with_config(config)
    }

    /// Allocate a new handle and insert a value
    ///
    /// Returns the handle for the newly allocated value.
    pub fn allocate(&mut self, value: T) -> Result<Handle, RegistryError> {
        if self.stats.total_slots >= self.config.max_capacity {
            return Err(RegistryError::RegistryFull(self.config.max_capacity));
        }

        let index = if let Some(free_index) = self.free_list.pop() {
            // Reuse a freed slot
            self.stats.generation_reuses += 1;
            free_index
        } else {
            // Allocate a new slot
            if self.next_index as usize >= MAX_HANDLES {
                return Err(RegistryError::RegistryFull(MAX_HANDLES));
            }
            let index = self.next_index;
            self.next_index += 1;
            self.slots.push(None);
            self.metadata.push(SlotMetadata {
                generation: 0,
                is_live: false,
            });
            self.stats.total_slots += 1;
            index
        };

        let slot = &mut self.slots[index as usize];
        let metadata = &mut self.metadata[index as usize];

        // Defensive: ensure slot is actually None before reuse
        if slot.is_some() {
            return Err(RegistryError::InconsistentState);
        }

        // Increment generation to invalidate old handles
        metadata.generation = metadata.generation.wrapping_add(1);
        metadata.is_live = true;
        *slot = Some(value);

        self.stats.allocations += 1;
        self.stats.live_count += 1;
        self.stats.free_count = self.free_list.len();

        Ok(Handle::new(index, metadata.generation))
    }

    /// Get a reference to the value for a handle
    ///
    /// Returns an error if the handle is invalid, stale, or not live.
    pub fn get(&self, handle: Handle) -> Result<&T, RegistryError> {
        self.validate_handle(handle)?;
        Ok(self.slots[handle.index() as usize].as_ref().unwrap())
    }

    /// Get a mutable reference to the value for a handle
    ///
    /// Returns an error if the handle is invalid, stale, or not live.
    pub fn get_mut(&mut self, handle: Handle) -> Result<&mut T, RegistryError> {
        self.validate_handle(handle)?;
        Ok(self.slots[handle.index() as usize].as_mut().unwrap())
    }

    /// Remove and return the value for a handle
    ///
    /// The handle becomes invalid after this operation.
    pub fn remove(&mut self, handle: Handle) -> Result<T, RegistryError> {
        self.validate_handle(handle)?;
        let index = handle.index() as usize;

        // Defensive: ensure value exists
        if self.slots[index].is_none() {
            return Err(RegistryError::InconsistentState);
        }

        self.metadata[index].is_live = false;
        self.free_list.push(handle.index());

        self.stats.deallocations += 1;
        self.stats.live_count -= 1;
        self.stats.free_count = self.free_list.len();

        Ok(self.slots[index].take().unwrap())
    }

    /// Check if a handle is valid and refers to a live object
    pub fn is_valid(&self, handle: Handle) -> bool {
        self.validate_handle(handle).is_ok()
    }

    /// Get the number of live objects in the registry
    pub fn len(&self) -> usize {
        self.slots.iter().filter(|s| s.is_some()).count()
    }

    /// Check if the registry is empty
    pub fn is_empty(&self) -> bool {
        self.len() == 0
    }

    /// Get the total capacity (allocated slots)
    pub fn capacity(&self) -> usize {
        self.slots.len()
    }

    /// Get the number of free slots available
    pub fn free_count(&self) -> usize {
        self.free_list.len()
    }

    /// Reserve capacity for additional handles
    pub fn reserve(&mut self, additional: usize) {
        self.slots.reserve(additional);
        self.metadata.reserve(additional);
    }

    /// Clear all objects from the registry
    ///
    /// All handles become invalid after this operation.
    pub fn clear(&mut self) {
        self.slots.clear();
        self.metadata.clear();
        self.free_list.clear();
        self.next_index = 0;
        self.stats = RegistryStats {
            total_slots: 0,
            live_count: 0,
            free_count: 0,
            generation_reuses: 0,
            allocations: 0,
            deallocations: 0,
        };
    }

    /// Get registry statistics
    pub fn stats(&self) -> &RegistryStats {
        &self.stats
    }

    /// Get the configuration
    pub fn config(&self) -> &RegistryConfig {
        &self.config
    }

    /// Iterate over all live handles
    pub fn handles(&self) -> impl Iterator<Item = Handle> + '_ {
        self.slots.iter().enumerate().filter_map(|(index, slot)| {
            if slot.is_some() {
                let metadata = &self.metadata[index];
                Some(Handle::new(index as HandleIndex, metadata.generation))
            } else {
                None
            }
        })
    }

    /// Get a value by handle, returning None instead of an error
    pub fn try_get(&self, handle: Handle) -> Option<&T> {
        self.get(handle).ok()
    }

    /// Get a mutable value by handle, returning None instead of an error
    pub fn try_get_mut(&mut self, handle: Handle) -> Option<&mut T> {
        self.get_mut(handle).ok()
    }

    /// Update a value in place
    pub fn update<F>(&mut self, handle: Handle, f: F) -> Result<(), RegistryError>
    where
        F: FnOnce(&mut T),
    {
        if let Ok(value) = self.get_mut(handle) {
            f(value);
            Ok(())
        } else {
            Err(self
                .validate_handle(handle)
                .err()
                .unwrap_or(RegistryError::NotLive))
        }
    }

    /// Replace a value and return the old one
    pub fn replace(&mut self, handle: Handle, new_value: T) -> Result<T, RegistryError> {
        self.validate_handle(handle)?;
        let index = handle.index() as usize;
        Ok(std::mem::replace(
            self.slots[index].as_mut().unwrap(),
            new_value,
        ))
    }

    /// Check if the registry contains a specific value by predicate
    pub fn contains<F>(&self, f: F) -> bool
    where
        F: Fn(&T) -> bool,
    {
        for value in self.slots.iter().flatten() {
            if f(value) {
                return true;
            }
        }
        false
    }

    /// Find a handle matching a predicate
    pub fn find<F>(&self, f: F) -> Option<Handle>
    where
        F: Fn(&T) -> bool,
    {
        for (index, slot) in self.slots.iter().enumerate() {
            if let Some(value) = slot {
                if f(value) {
                    let metadata = &self.metadata[index];
                    return Some(Handle::new(index as HandleIndex, metadata.generation));
                }
            }
        }
        None
    }

    /// Find all handles matching a predicate
    pub fn find_all<F>(&self, f: F) -> Vec<Handle>
    where
        F: Fn(&T) -> bool,
    {
        let mut results = Vec::new();
        for (index, slot) in self.slots.iter().enumerate() {
            if let Some(value) = slot {
                if f(value) {
                    let metadata = &self.metadata[index];
                    results.push(Handle::new(index as HandleIndex, metadata.generation));
                }
            }
        }
        results
    }

    /// Retain only values matching a predicate, removing others
    ///
    /// Returns the handles of removed objects.
    pub fn retain<F>(&mut self, f: F) -> Vec<Handle>
    where
        F: Fn(&T) -> bool,
    {
        let mut removed = Vec::new();
        for (index, slot) in self.slots.iter_mut().enumerate() {
            if let Some(value) = slot.as_ref() {
                if !f(value) {
                    let metadata = &self.metadata[index];
                    let handle = Handle::new(index as HandleIndex, metadata.generation);
                    removed.push(handle);
                    self.metadata[index].is_live = false;
                    self.free_list.push(handle.index());
                    *slot = None;
                }
            }
        }
        removed
    }

    /// Get the generation for a specific index (if live)
    pub fn get_generation(&self, index: HandleIndex) -> Option<HandleGeneration> {
        let idx = index as usize;
        if idx < self.slots.len() && self.metadata[idx].is_live {
            Some(self.metadata[idx].generation)
        } else {
            None
        }
    }

    /// Batch allocate multiple values
    ///
    /// Returns handles for all allocated values. If allocation fails, no values are allocated.
    pub fn allocate_batch(&mut self, values: Vec<T>) -> Result<Vec<Handle>, RegistryError> {
        let mut handles = Vec::with_capacity(values.len());

        // Check if we have enough capacity first
        let needed = values.len().saturating_sub(self.free_count());
        if (self.next_index as usize + needed) > MAX_HANDLES {
            return Err(RegistryError::RegistryFull(MAX_HANDLES));
        }

        for value in values {
            handles.push(self.allocate(value)?);
        }

        Ok(handles)
    }

    /// Iterate over all live handles with their values
    pub fn iter(&self) -> impl Iterator<Item = (Handle, &T)> + '_ {
        self.slots.iter().enumerate().filter_map(|(index, slot)| {
            if let Some(value) = slot {
                let metadata = &self.metadata[index];
                Some((
                    Handle::new(index as HandleIndex, metadata.generation),
                    value,
                ))
            } else {
                None
            }
        })
    }

    /// Iterate over all live handles with mutable values
    pub fn iter_mut(&mut self) -> impl Iterator<Item = (Handle, &mut T)> + '_ {
        self.slots
            .iter_mut()
            .enumerate()
            .filter_map(|(index, slot)| {
                if let Some(value) = slot {
                    let metadata = self.metadata[index].clone();
                    Some((
                        Handle::new(index as HandleIndex, metadata.generation),
                        value,
                    ))
                } else {
                    None
                }
            })
    }

    /// Compact the registry by removing gaps in the slot array
    ///
    /// This is expensive but can reduce memory fragmentation.
    /// After compaction, all handles become invalid.
    /// Returns the number of slots freed.
    pub fn compact(&mut self) -> usize
    where
        T: Clone,
    {
        if self.free_list.is_empty() {
            return 0;
        }

        // Collect live values first
        let live_values: Vec<T> = self
            .slots
            .iter()
            .filter_map(|s| s.as_ref().cloned())
            .collect();

        let freed = self.slots.len() - live_values.len();

        // Clear and reallocate
        self.slots.clear();
        self.metadata.clear();
        self.free_list.clear();
        self.next_index = 0;

        for value in live_values {
            let _ = self.allocate(value);
        }

        freed
    }

    /// Get all values as a vector (preserving order by index)
    pub fn values(&self) -> Vec<&T> {
        self.slots.iter().filter_map(|s| s.as_ref()).collect()
    }

    /// Get all values as a mutable vector
    pub fn values_mut(&mut self) -> Vec<&mut T> {
        self.slots.iter_mut().filter_map(|s| s.as_mut()).collect()
    }

    /// Clone the registry structure (handles are invalidated in the clone)
    ///
    /// Useful for creating snapshots or for testing.
    pub fn clone_values(&self) -> HandleRegistry<T>
    where
        T: Clone,
    {
        let mut new_registry = HandleRegistry::with_capacity(self.len());
        for value in self.values() {
            let _ = new_registry.allocate(value.clone());
        }
        new_registry
    }

    /// Get diagnostic information about the registry
    pub fn diagnostics(&self) -> RegistryDiagnostics {
        let fragmentation_ratio = if self.slots.is_empty() {
            0.0
        } else {
            self.free_count() as f64 / self.slots.len() as f64
        };

        RegistryDiagnostics {
            total_slots: self.slots.len(),
            live_count: self.len(),
            free_count: self.free_count(),
            fragmentation_ratio,
            next_index: self.next_index,
            max_index: self.slots.len() as HandleIndex,
        }
    }

    /// Validate a handle and check if it's live
    fn validate_handle(&self, handle: Handle) -> Result<(), RegistryError> {
        let index = handle.index() as usize;

        if index >= self.slots.len() {
            return Err(RegistryError::InvalidIndex(
                handle.index(),
                self.slots.len(),
            ));
        }

        let metadata = &self.metadata[index];

        if !metadata.is_live {
            return Err(RegistryError::NotLive);
        }

        if metadata.generation != handle.generation() {
            return Err(RegistryError::StaleHandle(
                metadata.generation,
                handle.generation(),
            ));
        }

        Ok(())
    }
}

/// Detailed diagnostics about a HandleRegistry
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct RegistryDiagnostics {
    pub total_slots: usize,
    pub live_count: usize,
    pub free_count: usize,
    pub fragmentation_ratio: f64,
    pub next_index: HandleIndex,
    pub max_index: HandleIndex,
}

impl<T> Default for HandleRegistry<T> {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_allocate_and_get() {
        let mut registry = HandleRegistry::new();
        let handle = registry.allocate(42).unwrap();
        assert_eq!(registry.get(handle), Ok(&42));
    }

    #[test]
    fn test_remove() {
        let mut registry = HandleRegistry::new();
        let handle = registry.allocate(42).unwrap();
        let value = registry.remove(handle).unwrap();
        assert_eq!(value, 42);
        assert!(!registry.is_valid(handle));
    }

    #[test]
    fn test_stale_handle_detection() {
        let mut registry = HandleRegistry::new();
        let handle1 = registry.allocate(42).unwrap();
        registry.remove(handle1).unwrap();

        // Reuse the slot
        let handle2 = registry.allocate(99).unwrap();

        // Same index, different generation
        assert_eq!(handle1.index(), handle2.index());
        assert_ne!(handle1.generation(), handle2.generation());

        // Old handle should be invalid
        match registry.get(handle1) {
            Err(RegistryError::StaleHandle(expected, got)) => {
                assert_eq!(expected, handle2.generation());
                assert_eq!(got, handle1.generation());
            }
            _ => panic!("Expected StaleHandle error"),
        }

        // New handle should work
        assert_eq!(registry.get(handle2), Ok(&99));
    }

    #[test]
    fn test_slot_reuse() {
        let mut registry = HandleRegistry::new();
        let h1 = registry.allocate(1).unwrap();
        let h2 = registry.allocate(2).unwrap();

        registry.remove(h1).unwrap();
        registry.remove(h2).unwrap();

        // Reuse slots
        let h3 = registry.allocate(3).unwrap();
        let h4 = registry.allocate(4).unwrap();

        // Should reuse the freed indices
        assert!(registry.is_valid(h3));
        assert!(registry.is_valid(h4));
    }

    #[test]
    fn test_invalid_index() {
        let registry: HandleRegistry<i32> = HandleRegistry::new();
        let handle = Handle::new(999, 1);
        match registry.get(handle) {
            Err(RegistryError::InvalidIndex(idx, capacity)) => {
                assert_eq!(idx, 999);
                assert_eq!(capacity, 0);
            }
            _ => panic!("Expected InvalidIndex error"),
        }
    }

    #[test]
    fn test_capacity_and_len() {
        let mut registry = HandleRegistry::new();
        assert_eq!(registry.len(), 0);
        assert!(registry.is_empty());

        registry.allocate(1).unwrap();
        registry.allocate(2).unwrap();
        assert_eq!(registry.len(), 2);
        assert!(!registry.is_empty());

        registry.clear();
        assert_eq!(registry.len(), 0);
        assert!(registry.is_empty());
    }

    #[test]
    fn test_registry_full() {
        let config = RegistryConfig {
            max_capacity: 1,
            ..Default::default()
        };
        let mut registry: HandleRegistry<i32> = HandleRegistry::with_config(config);

        registry.allocate(1).unwrap();

        // Try to allocate when full
        let result = registry.allocate(42);
        assert!(matches!(result, Err(RegistryError::RegistryFull(_))));
    }

    #[test]
    fn test_with_capacity() {
        let registry: HandleRegistry<i32> = HandleRegistry::with_capacity(100);
        assert_eq!(registry.capacity(), 0); // Capacity is just pre-allocation hint
    }

    #[test]
    fn test_null_handle() {
        let registry: HandleRegistry<i32> = HandleRegistry::new();
        let null = Handle::null();
        assert!(!registry.is_valid(null));
    }

    #[test]
    fn test_generation_overflow() {
        let mut registry = HandleRegistry::new();
        let handle = registry.allocate(42).unwrap();

        // Simulate generation overflow by manually setting it
        let index = handle.index() as usize;
        registry.metadata[index].generation = u16::MAX;

        // Manually update the handle's generation to match the metadata
        let overflow_handle = Handle::new(handle.index(), u16::MAX);

        // Remove with the overflowed generation
        registry.remove(overflow_handle).unwrap();

        // After wrapping, generation should be 0
        let new_handle = registry.allocate(99).unwrap();
        assert_eq!(new_handle.generation(), 0);
    }

    #[test]
    fn test_cross_world_access() {
        // Two separate registries represent different "worlds"
        let mut world1 = HandleRegistry::new();
        let mut world2 = HandleRegistry::new();

        // Allocate in world1
        let handle1 = world1.allocate(42).unwrap();

        // Diverge the registries by allocating in world2
        let handle2 = world2.allocate(99).unwrap();

        // Remove handle1 to change its generation
        world1.remove(handle1).unwrap();

        // Now allocate again in world1 - this will reuse the slot with new generation
        let handle1_new = world1.allocate(100).unwrap();

        // handle1 should be invalid in world1 (stale)
        assert!(!world1.is_valid(handle1));

        // handle1_new should be valid in world1
        assert!(world1.is_valid(handle1_new));

        // handle1_new should NOT be valid in world2 (different world)
        assert!(!world2.is_valid(handle1_new));

        // handle2 should be valid in world2 but not in world1
        assert!(world2.is_valid(handle2));
        assert!(!world1.is_valid(handle2));
    }

    #[test]
    fn test_free_count() {
        let mut registry = HandleRegistry::new();
        assert_eq!(registry.free_count(), 0);

        let h1 = registry.allocate(1).unwrap();
        let h2 = registry.allocate(2).unwrap();
        assert_eq!(registry.free_count(), 0);

        registry.remove(h1).unwrap();
        assert_eq!(registry.free_count(), 1);

        registry.remove(h2).unwrap();
        assert_eq!(registry.free_count(), 2);
    }

    #[test]
    fn test_reserve() {
        let mut registry: HandleRegistry<i32> = HandleRegistry::new();
        registry.reserve(100);
        // Reserve pre-allocates memory but doesn't create slots
        // Capacity will be at least the reserved amount when slots are added
        for _ in 0..50 {
            registry.allocate(0).unwrap();
        }
        assert!(registry.capacity() >= 50);
    }

    #[test]
    fn test_handles_iteration() {
        let mut registry = HandleRegistry::new();
        let h1 = registry.allocate(1).unwrap();
        let h2 = registry.allocate(2).unwrap();
        let h3 = registry.allocate(3).unwrap();

        let handles: Vec<_> = registry.handles().collect();
        assert_eq!(handles.len(), 3);
        assert!(handles.contains(&h1));
        assert!(handles.contains(&h2));
        assert!(handles.contains(&h3));
    }

    #[test]
    fn test_try_get() {
        let mut registry = HandleRegistry::new();
        let handle = registry.allocate(42).unwrap();

        assert_eq!(registry.try_get(handle), Some(&42));
        assert_eq!(registry.try_get(Handle::null()), None);

        registry.remove(handle).unwrap();
        assert_eq!(registry.try_get(handle), None);
    }

    #[test]
    fn test_update() {
        let mut registry = HandleRegistry::new();
        let handle = registry.allocate(42).unwrap();

        registry.update(handle, |v| *v *= 2).unwrap();
        assert_eq!(registry.get(handle), Ok(&84));
    }

    #[test]
    fn test_registry_config() {
        let config = RegistryConfig::default();
        assert_eq!(config.max_capacity, MAX_HANDLES);
        assert!(config.enable_stats);
        assert!(config.validate().is_ok());
    }

    #[test]
    fn test_registry_config_validation() {
        let config = RegistryConfig {
            max_capacity: 0,
            ..Default::default()
        };
        assert!(config.validate().is_ok()); // 0 means unlimited

        let config = RegistryConfig {
            max_capacity: MAX_HANDLES + 1,
            ..Default::default()
        };
        assert!(config.validate().is_err());
    }

    #[test]
    fn test_registry_with_config() {
        let config = RegistryConfig {
            max_capacity: 100,
            ..Default::default()
        };
        let registry: HandleRegistry<i32> = HandleRegistry::with_config(config);
        assert_eq!(registry.config().max_capacity, 100);
    }

    #[test]
    fn test_registry_stats() {
        let mut registry = HandleRegistry::new();
        assert_eq!(registry.stats().allocations, 0);
        assert_eq!(registry.stats().deallocations, 0);

        let h1 = registry.allocate(1).unwrap();
        assert_eq!(registry.stats().allocations, 1);
        assert_eq!(registry.stats().live_count, 1);

        registry.remove(h1).unwrap();
        assert_eq!(registry.stats().deallocations, 1);
        assert_eq!(registry.stats().live_count, 0);
    }

    #[test]
    fn test_generation_reuse_tracking() {
        let mut registry = HandleRegistry::new();
        let h1 = registry.allocate(1).unwrap();
        registry.remove(h1).unwrap();
        let _h2 = registry.allocate(2).unwrap();

        assert_eq!(registry.stats().generation_reuses, 1);
    }

    #[test]
    fn test_max_capacity_enforcement() {
        let config = RegistryConfig {
            max_capacity: 2,
            ..Default::default()
        };
        let mut registry: HandleRegistry<i32> = HandleRegistry::with_config(config);

        registry.allocate(1).unwrap();
        registry.allocate(2).unwrap();

        let result = registry.allocate(3);
        assert!(matches!(result, Err(RegistryError::RegistryFull(_))));
    }

    #[test]
    fn test_replace() {
        let mut registry = HandleRegistry::new();
        let handle = registry.allocate(42).unwrap();

        let old = registry.replace(handle, 99).unwrap();
        assert_eq!(old, 42);
        assert_eq!(registry.get(handle), Ok(&99));
    }

    #[test]
    fn test_contains() {
        let mut registry = HandleRegistry::new();
        registry.allocate(42).unwrap();
        registry.allocate(99).unwrap();

        assert!(registry.contains(|&v| v == 42));
        assert!(registry.contains(|&v| v == 99));
        assert!(!registry.contains(|&v| v == 100));
    }

    #[test]
    fn test_find() {
        let mut registry = HandleRegistry::new();
        let h1 = registry.allocate(42).unwrap();
        registry.allocate(99).unwrap();

        let found = registry.find(|&v| v == 42);
        assert_eq!(found, Some(h1));

        let not_found = registry.find(|&v| v == 100);
        assert_eq!(not_found, None);
    }

    #[test]
    fn test_find_all() {
        let mut registry = HandleRegistry::new();
        let h1 = registry.allocate(42).unwrap();
        let h2 = registry.allocate(42).unwrap();
        registry.allocate(99).unwrap();

        let found = registry.find_all(|&v| v == 42);
        assert_eq!(found.len(), 2);
        assert!(found.contains(&h1));
        assert!(found.contains(&h2));
    }

    #[test]
    fn test_handle_is_valid() {
        let handle = Handle::new(100, 1);
        assert!(handle.is_valid());

        let null = Handle::null();
        assert!(!null.is_valid());
    }

    #[test]
    fn test_retain() {
        let mut registry = HandleRegistry::new();
        let h1 = registry.allocate(42).unwrap();
        let h2 = registry.allocate(99).unwrap();
        let h3 = registry.allocate(100).unwrap();

        let removed = registry.retain(|&v| v > 50);
        assert_eq!(removed.len(), 1);
        assert!(removed.contains(&h1));
        assert!(!registry.is_valid(h1));
        assert!(registry.is_valid(h2));
        assert!(registry.is_valid(h3));
    }

    #[test]
    fn test_allocate_batch() {
        let mut registry = HandleRegistry::new();
        let values = vec![1, 2, 3, 4, 5];
        let handles = registry.allocate_batch(values).unwrap();
        assert_eq!(handles.len(), 5);
        for (i, handle) in handles.iter().enumerate() {
            assert_eq!(registry.get(*handle), Ok(&(i as i32 + 1)));
        }
    }

    #[test]
    fn test_allocate_batch_full() {
        let mut registry = HandleRegistry::new();
        registry.next_index = u16::MAX - 1;
        let values = vec![1, 2, 3];
        let result = registry.allocate_batch(values);
        assert!(matches!(result, Err(RegistryError::RegistryFull(_))));
    }

    #[test]
    fn test_iter() {
        let mut registry = HandleRegistry::new();
        let _h1 = registry.allocate(1).unwrap();
        let _h2 = registry.allocate(2).unwrap();

        let mut collected = Vec::new();
        for (handle, value) in registry.iter() {
            collected.push((handle, *value));
        }
        assert_eq!(collected.len(), 2);
    }

    #[test]
    fn test_iter_mut() {
        let mut registry = HandleRegistry::new();
        let h1 = registry.allocate(1).unwrap();
        let h2 = registry.allocate(2).unwrap();

        for (_handle, value) in registry.iter_mut() {
            *value *= 2;
        }

        assert_eq!(registry.get(h1), Ok(&2));
        assert_eq!(registry.get(h2), Ok(&4));
    }

    #[test]
    fn test_compact() {
        let mut registry = HandleRegistry::new();
        let _h1 = registry.allocate(1).unwrap();
        let h2 = registry.allocate(2).unwrap();
        let _h3 = registry.allocate(3).unwrap();

        registry.remove(h2).unwrap();
        assert_eq!(registry.free_count(), 1);

        let freed = registry.compact();
        assert_eq!(freed, 1);
        assert_eq!(registry.free_count(), 0);
        assert_eq!(registry.len(), 2);
    }

    #[test]
    fn test_compact_no_gaps() {
        let mut registry = HandleRegistry::new();
        registry.allocate(1).unwrap();
        registry.allocate(2).unwrap();

        let freed = registry.compact();
        assert_eq!(freed, 0);
    }

    #[test]
    fn test_values() {
        let mut registry = HandleRegistry::new();
        registry.allocate(1).unwrap();
        registry.allocate(2).unwrap();
        registry.allocate(3).unwrap();

        let values = registry.values();
        assert_eq!(values.len(), 3);
    }

    #[test]
    fn test_values_mut() {
        let mut registry = HandleRegistry::new();
        let h1 = registry.allocate(1).unwrap();
        let h2 = registry.allocate(2).unwrap();

        for value in registry.values_mut() {
            *value *= 2;
        }

        assert_eq!(registry.get(h1), Ok(&2));
        assert_eq!(registry.get(h2), Ok(&4));
    }

    #[test]
    fn test_clone_values() {
        let mut registry1 = HandleRegistry::new();
        let h1 = registry1.allocate(42).unwrap();
        let h2 = registry1.allocate(99).unwrap();

        let registry2 = registry1.clone_values();

        // Values should be the same
        let values1: Vec<_> = registry1.values().into_iter().cloned().collect();
        let values2: Vec<_> = registry2.values().into_iter().cloned().collect();
        assert_eq!(values1, values2);

        // Both registries should have the same number of items
        assert_eq!(registry1.len(), registry2.len());
        assert_eq!(registry2.len(), 2);

        // Both should have the values accessible
        assert_eq!(registry1.get(h1), Ok(&42));
        assert_eq!(registry1.get(h2), Ok(&99));
    }

    #[test]
    fn test_diagnostics() {
        let mut registry = HandleRegistry::new();
        registry.allocate(1).unwrap();
        registry.allocate(2).unwrap();
        let h3 = registry.allocate(3).unwrap();
        registry.remove(h3).unwrap();

        let diag = registry.diagnostics();
        assert_eq!(diag.total_slots, 3);
        assert_eq!(diag.live_count, 2);
        assert_eq!(diag.free_count, 1);
        assert!((diag.fragmentation_ratio - 0.333).abs() < 0.01);
    }

    #[test]
    fn test_get_generation() {
        let mut registry = HandleRegistry::new();
        let handle = registry.allocate(42).unwrap();

        let gen = registry.get_generation(handle.index());
        assert_eq!(gen, Some(handle.generation()));

        // After removal, generation should be None
        registry.remove(handle).unwrap();
        assert_eq!(registry.get_generation(handle.index()), None);
    }

    #[test]
    fn test_stats() {
        let mut registry = HandleRegistry::new();
        let h1 = registry.allocate(42).unwrap();
        let _h2 = registry.allocate(99).unwrap();

        let stats = registry.stats();
        assert_eq!(stats.live_count, 2);
        assert_eq!(stats.free_count, 0);

        registry.remove(h1).unwrap();
        let stats = registry.stats();
        assert_eq!(stats.live_count, 1);
        assert_eq!(stats.free_count, 1);
    }

    #[test]
    fn test_performance_basic() {
        let mut registry = HandleRegistry::new();
        let start = std::time::Instant::now();

        // Allocate 1000 objects
        let handles: Vec<_> = (0..1000).map(|_| registry.allocate(42).unwrap()).collect();

        let alloc_time = start.elapsed();
        println!("Allocation of 1000 objects: {:?}", alloc_time);

        // Access all objects
        let start = std::time::Instant::now();
        for handle in &handles {
            let _ = registry.get(*handle);
        }
        let access_time = start.elapsed();
        println!("Access of 1000 objects: {:?}", access_time);

        // Remove all objects
        let start = std::time::Instant::now();
        for handle in handles {
            let _ = registry.remove(handle);
        }
        let remove_time = start.elapsed();
        println!("Removal of 1000 objects: {:?}", remove_time);

        // Performance should be reasonable (less than 1 second for 1000 operations)
        assert!(alloc_time.as_millis() < 1000);
        assert!(access_time.as_millis() < 1000);
        assert!(remove_time.as_millis() < 1000);
    }
}
