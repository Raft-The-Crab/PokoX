//! Generation tracking utilities
//!
//! This module provides utilities for tracking and managing handle generations,
//! including generation counters, pools, and validation.

use crate::HandleGeneration;
use std::sync::atomic::{AtomicU16, Ordering};

/// A thread-safe generation counter
///
/// Uses atomic operations for safe concurrent access across threads.
#[derive(Debug)]
pub struct GenerationCounter {
    counter: AtomicU16,
}

impl GenerationCounter {
    /// Create a new generation counter starting at 0
    pub fn new() -> Self {
        Self {
            counter: AtomicU16::new(0),
        }
    }

    /// Create a new generation counter with a specific starting value
    pub fn with_start(start: HandleGeneration) -> Self {
        Self {
            counter: AtomicU16::new(start),
        }
    }

    /// Get the current generation value
    pub fn get(&self) -> HandleGeneration {
        self.counter.load(Ordering::Acquire)
    }

    /// Increment the generation and return the new value
    ///
    /// Wraps around to 0 after reaching u16::MAX.
    pub fn increment(&self) -> HandleGeneration {
        let old = self.counter.fetch_add(1, Ordering::AcqRel);
        old.wrapping_add(1)
    }

    /// Set the generation to a specific value
    pub fn set(&self, value: HandleGeneration) {
        self.counter.store(value, Ordering::Release);
    }

    /// Reset the counter to 0
    pub fn reset(&self) {
        self.set(0);
    }
}

impl Default for GenerationCounter {
    fn default() -> Self {
        Self::new()
    }
}

/// A pool of pre-allocated generation values
///
/// Useful for scenarios where many generations need to be allocated
/// in advance, such as during initialization.
#[derive(Debug, Clone)]
pub struct GenerationPool {
    pool: Vec<HandleGeneration>,
    next: usize,
}

impl GenerationPool {
    /// Create a new generation pool with a specific capacity
    pub fn with_capacity(capacity: usize) -> Self {
        Self {
            pool: (0..capacity as HandleGeneration).collect(),
            next: 0,
        }
    }

    /// Get the next generation from the pool
    ///
    /// Returns None if the pool is exhausted.
    pub fn next(&mut self) -> Option<HandleGeneration> {
        if self.next < self.pool.len() {
            let gen = self.pool[self.next];
            self.next += 1;
            Some(gen)
        } else {
            None
        }
    }

    /// Get the next generation, wrapping around if exhausted
    pub fn next_wrapping(&mut self) -> HandleGeneration {
        if self.next >= self.pool.len() {
            self.next = 0;
        }
        let gen = self.pool[self.next];
        self.next += 1;
        gen
    }

    /// Reset the pool to the beginning
    pub fn reset(&mut self) {
        self.next = 0;
    }

    /// Get the number of remaining generations in the pool
    pub fn remaining(&self) -> usize {
        self.pool.len().saturating_sub(self.next)
    }

    /// Check if the pool is exhausted
    pub fn is_exhausted(&self) -> bool {
        self.next >= self.pool.len()
    }

    /// Get the total capacity of the pool
    pub fn capacity(&self) -> usize {
        self.pool.len()
    }
}

/// Validation utilities for generation tracking
pub struct GenerationValidator;

impl GenerationValidator {
    /// Check if a generation is valid (not the maximum value)
    ///
    /// u16::MAX is reserved for special cases (e.g., null handles).
    pub fn is_valid(generation: HandleGeneration) -> bool {
        generation != u16::MAX
    }

    /// Check if generation A is newer than generation B
    ///
    /// Handles generation wrapping by considering the "distance"
    /// between generations.
    pub fn is_newer(a: HandleGeneration, b: HandleGeneration) -> bool {
        // If the difference is less than or equal to half the max value, A is newer
        let diff = a.wrapping_sub(b);
        diff <= u16::MAX / 2
    }

    /// Calculate the "distance" between two generations
    ///
    /// Returns the minimum distance considering wrapping.
    pub fn distance(a: HandleGeneration, b: HandleGeneration) -> u16 {
        let diff = a.wrapping_sub(b);
        if diff <= u16::MAX / 2 {
            diff
        } else {
            u16::MAX - diff + 1
        }
    }

    /// Check if two generations are effectively the same
    ///
    /// Considers them the same if they're within a small threshold.
    pub fn is_equivalent(a: HandleGeneration, b: HandleGeneration, threshold: u16) -> bool {
        Self::distance(a, b) <= threshold
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_generation_counter() {
        let counter = GenerationCounter::new();
        assert_eq!(counter.get(), 0);

        assert_eq!(counter.increment(), 1);
        assert_eq!(counter.get(), 1);

        assert_eq!(counter.increment(), 2);
        assert_eq!(counter.increment(), 3);

        counter.set(100);
        assert_eq!(counter.get(), 100);

        counter.reset();
        assert_eq!(counter.get(), 0);
    }

    #[test]
    fn test_generation_counter_overflow() {
        let counter = GenerationCounter::with_start(u16::MAX - 1);
        assert_eq!(counter.get(), u16::MAX - 1);

        // Should wrap to 0 after increment
        let new_gen = counter.increment();
        assert_eq!(new_gen, u16::MAX);
        assert_eq!(counter.get(), u16::MAX);

        // Next increment wraps to 0
        let new_gen = counter.increment();
        assert_eq!(new_gen, 0);
        assert_eq!(counter.get(), 0);
    }

    #[test]
    fn test_generation_pool() {
        let mut pool = GenerationPool::with_capacity(10);
        assert_eq!(pool.capacity(), 10);
        assert_eq!(pool.remaining(), 10);

        assert_eq!(pool.next(), Some(0));
        assert_eq!(pool.next(), Some(1));
        assert_eq!(pool.remaining(), 8);

        pool.reset();
        assert_eq!(pool.next(), Some(0));
    }

    #[test]
    fn test_generation_pool_exhausted() {
        let mut pool = GenerationPool::with_capacity(2);
        assert_eq!(pool.next(), Some(0));
        assert_eq!(pool.next(), Some(1));
        assert_eq!(pool.next(), None);
        assert!(pool.is_exhausted());
    }

    #[test]
    fn test_generation_pool_wrapping() {
        let mut pool = GenerationPool::with_capacity(3);
        assert_eq!(pool.next_wrapping(), 0);
        assert_eq!(pool.next_wrapping(), 1);
        assert_eq!(pool.next_wrapping(), 2);
        assert_eq!(pool.next_wrapping(), 0); // Wraps around
    }

    #[test]
    fn test_generation_validator_is_valid() {
        assert!(GenerationValidator::is_valid(0));
        assert!(GenerationValidator::is_valid(100));
        assert!(GenerationValidator::is_valid(u16::MAX - 1));
        assert!(!GenerationValidator::is_valid(u16::MAX));
    }

    #[test]
    fn test_generation_validator_is_newer() {
        assert!(GenerationValidator::is_newer(5, 3));
        assert!(!GenerationValidator::is_newer(3, 5));

        // With wrapping - 1 is newer than 65534
        assert!(GenerationValidator::is_newer(1, u16::MAX - 1));
    }

    #[test]
    fn test_generation_validator_distance() {
        assert_eq!(GenerationValidator::distance(5, 3), 2);
        assert_eq!(GenerationValidator::distance(3, 5), 2);

        // With wrapping - distance between 1 and 65534 is 3 (through 65535, 0)
        assert_eq!(GenerationValidator::distance(1, u16::MAX - 1), 3);
    }

    #[test]
    fn test_generation_validator_is_equivalent() {
        assert!(GenerationValidator::is_equivalent(5, 5, 0));
        assert!(GenerationValidator::is_equivalent(5, 6, 1));
        assert!(!GenerationValidator::is_equivalent(5, 10, 1));
    }
}
