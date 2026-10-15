//! Memory tracking and statistics

use std::collections::HashMap;
use std::sync::atomic::{AtomicU64, Ordering};
use thiserror::Error;

/// Errors that can occur with memory tracking
#[derive(Debug, Clone, Error, PartialEq)]
pub enum MemoryTrackerError {
    #[error("Deallocation size exceeds current allocation: dealloc={0}, current={1}")]
    DeallocationExceedsAllocation(u64, u64),
    #[error("Invalid allocation size: {0}")]
    InvalidSize(u64),
    #[error("Category not found: {0}")]
    CategoryNotFound(String),
}

/// Information about a single allocation
#[derive(Debug, Clone)]
pub struct AllocationInfo {
    /// Size of the allocation in bytes
    pub size: u64,
    /// Timestamp when the allocation was made
    pub timestamp: u64,
    /// Stack trace or source location (optional)
    pub location: Option<String>,
}

/// Memory statistics snapshot
#[derive(Debug, Clone, PartialEq, Default)]
pub struct MemoryStats {
    /// Total allocated bytes
    pub total_allocated: u64,
    /// Total deallocated bytes
    pub total_deallocated: u64,
    /// Current allocated bytes (live)
    pub current_allocated: u64,
    /// Number of allocations
    pub allocation_count: u64,
    /// Number of deallocations
    pub deallocation_count: u64,
    /// Peak allocated bytes
    pub peak_allocated: u64,
}

impl MemoryStats {
    /// Calculate the efficiency (ratio of peak to total)
    pub fn efficiency(&self) -> f64 {
        if self.total_allocated == 0 {
            return 0.0;
        }
        (self.peak_allocated as f64 / self.total_allocated as f64) * 100.0
    }

    /// Calculate fragmentation (unused vs peak)
    pub fn fragmentation(&self) -> f64 {
        if self.peak_allocated == 0 {
            return 0.0;
        }
        let unused = self.peak_allocated - self.current_allocated;
        (unused as f64 / self.peak_allocated as f64) * 100.0
    }
}

/// Memory tracker for monitoring allocations
#[derive(Debug)]
pub struct MemoryTracker {
    /// Total allocated bytes
    total_allocated: AtomicU64,
    /// Total deallocated bytes
    total_deallocated: AtomicU64,
    /// Current allocated bytes
    current_allocated: AtomicU64,
    /// Allocation count
    allocation_count: AtomicU64,
    /// Deallocation count
    deallocation_count: AtomicU64,
    /// Peak allocated bytes
    peak_allocated: AtomicU64,
    /// Per-category tracking
    category_stats: HashMap<String, AtomicU64>,
}

impl MemoryTracker {
    /// Create a new memory tracker
    pub fn new() -> Self {
        Self {
            total_allocated: AtomicU64::new(0),
            total_deallocated: AtomicU64::new(0),
            current_allocated: AtomicU64::new(0),
            allocation_count: AtomicU64::new(0),
            deallocation_count: AtomicU64::new(0),
            peak_allocated: AtomicU64::new(0),
            category_stats: HashMap::new(),
        }
    }

    /// Record an allocation
    pub fn record_allocation(
        &mut self,
        size: u64,
        category: Option<&str>,
    ) -> Result<(), MemoryTrackerError> {
        if size == 0 {
            return Err(MemoryTrackerError::InvalidSize(size));
        }

        self.total_allocated.fetch_add(size, Ordering::Relaxed);
        let current = self.current_allocated.fetch_add(size, Ordering::Relaxed) + size;
        self.allocation_count.fetch_add(1, Ordering::Relaxed);

        // Update peak
        let mut peak = self.peak_allocated.load(Ordering::Relaxed);
        while current > peak {
            match self.peak_allocated.compare_exchange_weak(
                peak,
                current,
                Ordering::Relaxed,
                Ordering::Relaxed,
            ) {
                Ok(_) => break,
                Err(new_peak) => peak = new_peak,
            }
        }

        // Update category stats
        if let Some(cat) = category {
            if let Some(counter) = self.category_stats.get(cat) {
                counter.fetch_add(size, Ordering::Relaxed);
            } else {
                let counter = AtomicU64::new(size);
                self.category_stats.insert(cat.to_string(), counter);
            }
        }

        Ok(())
    }

    /// Record a deallocation
    pub fn record_deallocation(
        &mut self,
        size: u64,
        category: Option<&str>,
    ) -> Result<(), MemoryTrackerError> {
        let current = self.current_allocated.load(Ordering::Relaxed);
        if size > current {
            return Err(MemoryTrackerError::DeallocationExceedsAllocation(
                size, current,
            ));
        }

        self.total_deallocated.fetch_add(size, Ordering::Relaxed);
        self.current_allocated.fetch_sub(size, Ordering::Relaxed);
        self.deallocation_count.fetch_add(1, Ordering::Relaxed);

        // Update category stats
        if let Some(cat) = category {
            if let Some(counter) = self.category_stats.get(cat) {
                counter.fetch_sub(size, Ordering::Relaxed);
            }
        }

        Ok(())
    }

    /// Get current memory statistics
    pub fn stats(&self) -> MemoryStats {
        MemoryStats {
            total_allocated: self.total_allocated.load(Ordering::Relaxed),
            total_deallocated: self.total_deallocated.load(Ordering::Relaxed),
            current_allocated: self.current_allocated.load(Ordering::Relaxed),
            allocation_count: self.allocation_count.load(Ordering::Relaxed),
            deallocation_count: self.deallocation_count.load(Ordering::Relaxed),
            peak_allocated: self.peak_allocated.load(Ordering::Relaxed),
        }
    }

    /// Get memory usage for a specific category
    pub fn category_usage(&self, category: &str) -> u64 {
        self.category_stats
            .get(category)
            .map(|c| c.load(Ordering::Relaxed))
            .unwrap_or(0)
    }

    /// Get all category statistics
    pub fn all_category_stats(&self) -> HashMap<String, u64> {
        self.category_stats
            .iter()
            .map(|(k, v)| (k.clone(), v.load(Ordering::Relaxed)))
            .collect()
    }

    /// Get current allocated bytes
    pub fn current_allocated(&self) -> u64 {
        self.current_allocated.load(Ordering::Relaxed)
    }

    /// Get peak allocated bytes
    pub fn peak_allocated(&self) -> u64 {
        self.peak_allocated.load(Ordering::Relaxed)
    }

    /// Get total allocation count
    pub fn allocation_count(&self) -> u64 {
        self.allocation_count.load(Ordering::Relaxed)
    }

    /// Get total deallocation count
    pub fn deallocation_count(&self) -> u64 {
        self.deallocation_count.load(Ordering::Relaxed)
    }

    /// Reset all statistics
    pub fn reset(&mut self) {
        self.total_allocated.store(0, Ordering::Relaxed);
        self.total_deallocated.store(0, Ordering::Relaxed);
        self.current_allocated.store(0, Ordering::Relaxed);
        self.allocation_count.store(0, Ordering::Relaxed);
        self.deallocation_count.store(0, Ordering::Relaxed);
        self.peak_allocated.store(0, Ordering::Relaxed);

        for counter in self.category_stats.values() {
            counter.store(0, Ordering::Relaxed);
        }
    }

    /// Check if memory usage is within limits
    pub fn is_within_limit(&self, limit_bytes: u64) -> bool {
        self.current_allocated.load(Ordering::Relaxed) <= limit_bytes
    }

    /// Get memory usage as a percentage of limit
    pub fn usage_percentage(&self, limit_bytes: u64) -> f64 {
        if limit_bytes == 0 {
            return 0.0;
        }
        let current = self.current_allocated.load(Ordering::Relaxed);
        (current as f64 / limit_bytes as f64) * 100.0
    }

    /// Get the number of tracked categories
    pub fn category_count(&self) -> usize {
        self.category_stats.len()
    }
}

impl Default for MemoryTracker {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_tracker_creation() {
        let tracker = MemoryTracker::new();
        let stats = tracker.stats();
        assert_eq!(stats.total_allocated, 0);
        assert_eq!(stats.current_allocated, 0);
    }

    #[test]
    fn test_record_allocation() {
        let mut tracker = MemoryTracker::new();
        tracker.record_allocation(1024, Some("test")).unwrap();
        let stats = tracker.stats();
        assert_eq!(stats.total_allocated, 1024);
        assert_eq!(stats.current_allocated, 1024);
        assert_eq!(stats.allocation_count, 1);
    }

    #[test]
    fn test_record_allocation_zero_size() {
        let mut tracker = MemoryTracker::new();
        let result = tracker.record_allocation(0, Some("test"));
        assert!(result.is_err());
    }

    #[test]
    fn test_record_deallocation() {
        let mut tracker = MemoryTracker::new();
        tracker.record_allocation(1024, Some("test")).unwrap();
        tracker.record_deallocation(512, Some("test")).unwrap();
        let stats = tracker.stats();
        assert_eq!(stats.total_deallocated, 512);
        assert_eq!(stats.current_allocated, 512);
        assert_eq!(stats.deallocation_count, 1);
    }

    #[test]
    fn test_record_deallocation_exceeds() {
        let mut tracker = MemoryTracker::new();
        tracker.record_allocation(512, Some("test")).unwrap();
        let result = tracker.record_deallocation(1024, Some("test"));
        assert!(result.is_err());
    }

    #[test]
    fn test_peak_tracking() {
        let mut tracker = MemoryTracker::new();
        tracker.record_allocation(1024, None).unwrap();
        tracker.record_allocation(2048, None).unwrap();
        let stats = tracker.stats();
        assert_eq!(stats.peak_allocated, 3072);
    }

    #[test]
    fn test_category_tracking() {
        let mut tracker = MemoryTracker::new();
        tracker.record_allocation(1024, Some("test")).unwrap();
        tracker.record_allocation(512, Some("other")).unwrap();
        assert_eq!(tracker.category_usage("test"), 1024);
        assert_eq!(tracker.category_usage("other"), 512);
    }

    #[test]
    fn test_reset() {
        let mut tracker = MemoryTracker::new();
        tracker.record_allocation(1024, Some("test")).unwrap();
        tracker.reset();
        let stats = tracker.stats();
        assert_eq!(stats.total_allocated, 0);
        assert_eq!(stats.current_allocated, 0);
    }

    #[test]
    fn test_within_limit() {
        let mut tracker = MemoryTracker::new();
        tracker.record_allocation(1024, None).unwrap();
        assert!(tracker.is_within_limit(2048));
        assert!(!tracker.is_within_limit(512));
    }

    #[test]
    fn test_usage_percentage() {
        let mut tracker = MemoryTracker::new();
        tracker.record_allocation(512, None).unwrap();
        let percentage = tracker.usage_percentage(1024);
        assert!((percentage - 50.0).abs() < 0.01);
    }

    #[test]
    fn test_stats_efficiency() {
        let mut tracker = MemoryTracker::new();
        tracker.record_allocation(1024, None).unwrap();
        tracker.record_allocation(512, None).unwrap();
        tracker.record_deallocation(512, None).unwrap();
        let stats = tracker.stats();
        let efficiency = stats.efficiency();
        assert!(efficiency > 0.0 && efficiency <= 100.0);
    }

    #[test]
    fn test_stats_fragmentation() {
        let mut tracker = MemoryTracker::new();
        tracker.record_allocation(1024, None).unwrap();
        tracker.record_deallocation(512, None).unwrap();
        let stats = tracker.stats();
        let fragmentation = stats.fragmentation();
        assert!(fragmentation > 0.0);
    }

    #[test]
    fn test_peak_allocated_accessor() {
        let mut tracker = MemoryTracker::new();
        tracker.record_allocation(1024, None).unwrap();
        assert_eq!(tracker.peak_allocated(), 1024);
    }

    #[test]
    fn test_allocation_count_accessor() {
        let mut tracker = MemoryTracker::new();
        tracker.record_allocation(1024, None).unwrap();
        tracker.record_allocation(512, None).unwrap();
        assert_eq!(tracker.allocation_count(), 2);
    }

    #[test]
    fn test_deallocation_count_accessor() {
        let mut tracker = MemoryTracker::new();
        tracker.record_allocation(1024, None).unwrap();
        tracker.record_deallocation(512, None).unwrap();
        assert_eq!(tracker.deallocation_count(), 1);
    }

    #[test]
    fn test_category_count() {
        let mut tracker = MemoryTracker::new();
        tracker.record_allocation(1024, Some("cat1")).unwrap();
        tracker.record_allocation(512, Some("cat2")).unwrap();
        assert_eq!(tracker.category_count(), 2);
    }
}
