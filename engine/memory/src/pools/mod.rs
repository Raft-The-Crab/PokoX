//! Memory pools for efficient allocation

use std::collections::VecDeque;
use thiserror::Error;

/// Errors that can occur with memory pools
#[derive(Debug, Clone, Error, PartialEq)]
pub enum PoolError {
    #[error("Pool capacity exceeded: {0}")]
    CapacityExceeded(usize),
    #[error("Invalid pool configuration: {0}")]
    InvalidConfig(String),
    #[error("Pool is empty")]
    EmptyPool,
}

/// Pool type
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum PoolType {
    /// Fixed-size pool for objects of constant size
    FixedSize { size: usize, capacity: usize },
    /// Growing pool for variable-sized allocations
    Growing {
        initial_size: usize,
        max_size: usize,
    },
    /// Ring buffer pool
    RingBuffer { size: usize },
}

impl PoolType {
    /// Validate the pool type configuration
    pub fn validate(&self) -> Result<(), PoolError> {
        match self {
            PoolType::FixedSize { size, capacity } => {
                if *size == 0 {
                    return Err(PoolError::InvalidConfig("Size cannot be zero".to_string()));
                }
                if *capacity == 0 {
                    return Err(PoolError::InvalidConfig(
                        "Capacity cannot be zero".to_string(),
                    ));
                }
                if *size > *capacity {
                    return Err(PoolError::InvalidConfig(
                        "Size cannot exceed capacity".to_string(),
                    ));
                }
            }
            PoolType::Growing {
                initial_size,
                max_size,
            } => {
                if *max_size == 0 {
                    return Err(PoolError::InvalidConfig(
                        "Max size cannot be zero".to_string(),
                    ));
                }
                if *initial_size > *max_size {
                    return Err(PoolError::InvalidConfig(
                        "Initial size cannot exceed max size".to_string(),
                    ));
                }
            }
            PoolType::RingBuffer { size } => {
                if *size == 0 {
                    return Err(PoolError::InvalidConfig(
                        "Ring buffer size cannot be zero".to_string(),
                    ));
                }
            }
        }
        Ok(())
    }

    /// Get the maximum capacity of the pool type
    pub fn max_capacity(&self) -> usize {
        match self {
            PoolType::FixedSize { capacity, .. } => *capacity,
            PoolType::Growing { max_size, .. } => *max_size,
            PoolType::RingBuffer { size } => *size,
        }
    }
}

/// Configuration for a memory pool
#[derive(Debug, Clone)]
pub struct PoolConfig {
    /// Pool type
    pub pool_type: PoolType,
    /// Pool name/identifier
    pub name: String,
}

impl PoolConfig {
    /// Create a new pool configuration
    pub fn new(pool_type: PoolType, name: String) -> Result<Self, PoolError> {
        pool_type.validate()?;
        Ok(Self { pool_type, name })
    }

    /// Validate the configuration
    pub fn validate(&self) -> Result<(), PoolError> {
        self.pool_type.validate()
    }
}

/// Statistics for a memory pool
#[derive(Debug, Clone, PartialEq)]
pub struct PoolStats {
    /// Total items created
    pub total_created: usize,
    /// Total items reclaimed (reused)
    pub total_reclaimed: usize,
    /// Currently available items
    pub available_count: usize,
    /// Pool efficiency percentage
    pub efficiency: f64,
}

/// Generic memory pool
#[derive(Debug)]
pub struct MemoryPool<T> {
    /// Pool configuration
    config: PoolConfig,
    /// Available items in the pool
    available: VecDeque<T>,
    /// Total items created
    total_created: usize,
    /// Total items reclaimed
    total_reclaimed: usize,
}

impl<T> MemoryPool<T> {
    /// Create a new memory pool
    pub fn new(config: PoolConfig) -> Result<Self, PoolError> {
        config.validate()?;
        Ok(Self {
            config,
            available: VecDeque::new(),
            total_created: 0,
            total_reclaimed: 0,
        })
    }

    /// Allocate an item from the pool
    pub fn allocate<F>(&mut self, factory: F) -> T
    where
        F: FnOnce() -> T,
    {
        if let Some(item) = self.available.pop_front() {
            self.total_reclaimed += 1;
            item
        } else {
            self.total_created += 1;
            factory()
        }
    }

    /// Return an item to the pool
    pub fn deallocate(&mut self, item: T) -> Result<(), PoolError> {
        let max_cap = self.config.pool_type.max_capacity();
        if self.available.len() >= max_cap {
            return Err(PoolError::CapacityExceeded(max_cap));
        }
        self.available.push_back(item);
        Ok(())
    }

    /// Get the number of available items
    pub fn available_count(&self) -> usize {
        self.available.len()
    }

    /// Get the total number of items created
    pub fn total_created(&self) -> usize {
        self.total_created
    }

    /// Get the total number of items reclaimed
    pub fn total_reclaimed(&self) -> usize {
        self.total_reclaimed
    }

    /// Clear the pool
    pub fn clear(&mut self) {
        self.available.clear();
    }

    /// Get the pool configuration
    pub fn config(&self) -> &PoolConfig {
        &self.config
    }

    /// Calculate pool efficiency (reclaimed / total created)
    pub fn efficiency(&self) -> f64 {
        if self.total_created == 0 {
            0.0
        } else {
            (self.total_reclaimed as f64 / self.total_created as f64) * 100.0
        }
    }

    /// Get pool statistics
    pub fn stats(&self) -> PoolStats {
        PoolStats {
            total_created: self.total_created,
            total_reclaimed: self.total_reclaimed,
            available_count: self.available.len(),
            efficiency: self.efficiency(),
        }
    }

    /// Pre-populate the pool with items
    pub fn prepopulate<F>(&mut self, count: usize, factory: F) -> Result<(), PoolError>
    where
        F: Fn() -> T,
    {
        let max_cap = self.config.pool_type.max_capacity();
        if self.available.len() + count > max_cap {
            return Err(PoolError::CapacityExceeded(max_cap));
        }

        for _ in 0..count {
            self.available.push_back(factory());
            self.total_created += 1;
        }

        Ok(())
    }

    /// Reserve space in the pool (for performance)
    pub fn reserve(&mut self, additional: usize) {
        self.available.reserve(additional);
    }
}

impl<T> Default for MemoryPool<T> {
    fn default() -> Self {
        Self::new(PoolConfig {
            pool_type: PoolType::Growing {
                initial_size: 0,
                max_size: 1000,
            },
            name: "default".to_string(),
        })
        .expect("Default pool config should be valid")
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_pool_creation() {
        let pool: MemoryPool<i32> = MemoryPool::default();
        assert_eq!(pool.available_count(), 0);
        assert_eq!(pool.total_created(), 0);
    }

    #[test]
    fn test_pool_allocate() {
        let mut pool: MemoryPool<i32> = MemoryPool::default();
        let item = pool.allocate(|| 42);
        assert_eq!(item, 42);
        assert_eq!(pool.total_created(), 1);
    }

    #[test]
    fn test_pool_deallocate() {
        let mut pool: MemoryPool<i32> = MemoryPool::default();
        let item = pool.allocate(|| 42);
        pool.deallocate(item).unwrap();
        assert_eq!(pool.available_count(), 1);
    }

    #[test]
    fn test_pool_reclaim() {
        let mut pool: MemoryPool<i32> = MemoryPool::default();
        let item = pool.allocate(|| 42);
        pool.deallocate(item).unwrap();
        let reclaimed = pool.allocate(|| 0);
        assert_eq!(reclaimed, 42);
        assert_eq!(pool.total_reclaimed(), 1);
    }

    #[test]
    fn test_pool_efficiency() {
        let mut pool: MemoryPool<i32> = MemoryPool::default();
        let item1 = pool.allocate(|| 1);
        pool.deallocate(item1).unwrap();
        let _item2 = pool.allocate(|| 2); // This reuses item1
        let efficiency = pool.efficiency();
        assert!((efficiency - 100.0).abs() < 0.01);
    }

    #[test]
    fn test_pool_clear() {
        let mut pool: MemoryPool<i32> = MemoryPool::default();
        pool.allocate(|| 1);
        pool.deallocate(1).unwrap();
        pool.clear();
        assert_eq!(pool.available_count(), 0);
    }

    #[test]
    fn test_pool_stats() {
        let mut pool: MemoryPool<i32> = MemoryPool::default();
        let item1 = pool.allocate(|| 1);
        let _item2 = pool.allocate(|| 2);
        pool.deallocate(item1).unwrap();
        let stats = pool.stats();
        assert_eq!(stats.total_created, 2);
        assert_eq!(stats.total_reclaimed, 0); // Not reclaimed yet
        assert_eq!(stats.available_count, 1);
    }

    #[test]
    fn test_pool_deallocate_capacity_exceeded() {
        let config = PoolConfig::new(
            PoolType::FixedSize {
                size: 1,
                capacity: 2,
            },
            "test".to_string(),
        )
        .unwrap();
        let mut pool: MemoryPool<i32> = MemoryPool::new(config).unwrap();
        // Fill the pool to capacity
        pool.deallocate(1).unwrap();
        pool.deallocate(2).unwrap();
        assert_eq!(pool.available_count(), 2);
        // Try to deallocate one more - should fail
        let result = pool.deallocate(3);
        assert!(result.is_err());
    }

    #[test]
    fn test_prepopulate() {
        let config = PoolConfig::new(
            PoolType::FixedSize {
                size: 2,
                capacity: 10,
            },
            "test".to_string(),
        )
        .unwrap();
        let mut pool: MemoryPool<i32> = MemoryPool::new(config).unwrap();
        pool.prepopulate(5, || 42).unwrap();
        assert_eq!(pool.available_count(), 5);
        assert_eq!(pool.total_created(), 5);
    }

    #[test]
    fn test_prepopulate_exceeds_capacity() {
        let config = PoolConfig::new(
            PoolType::FixedSize {
                size: 1,
                capacity: 2,
            },
            "test".to_string(),
        )
        .unwrap();
        let mut pool: MemoryPool<i32> = MemoryPool::new(config).unwrap();
        let result = pool.prepopulate(5, || 42);
        assert!(result.is_err());
    }

    #[test]
    fn test_pool_type_validate_fixed_size() {
        let valid = PoolType::FixedSize {
            size: 1,
            capacity: 10,
        };
        assert!(valid.validate().is_ok());

        let invalid_size = PoolType::FixedSize {
            size: 0,
            capacity: 10,
        };
        assert!(invalid_size.validate().is_err());

        let invalid_capacity = PoolType::FixedSize {
            size: 10,
            capacity: 0,
        };
        assert!(invalid_capacity.validate().is_err());

        let invalid_exceeds = PoolType::FixedSize {
            size: 10,
            capacity: 5,
        };
        assert!(invalid_exceeds.validate().is_err());
    }

    #[test]
    fn test_pool_type_validate_growing() {
        let valid = PoolType::Growing {
            initial_size: 0,
            max_size: 100,
        };
        assert!(valid.validate().is_ok());

        let invalid_max = PoolType::Growing {
            initial_size: 0,
            max_size: 0,
        };
        assert!(invalid_max.validate().is_err());

        let invalid_exceeds = PoolType::Growing {
            initial_size: 100,
            max_size: 50,
        };
        assert!(invalid_exceeds.validate().is_err());
    }

    #[test]
    fn test_pool_type_validate_ring_buffer() {
        let valid = PoolType::RingBuffer { size: 10 };
        assert!(valid.validate().is_ok());

        let invalid = PoolType::RingBuffer { size: 0 };
        assert!(invalid.validate().is_err());
    }

    #[test]
    fn test_pool_type_max_capacity() {
        assert_eq!(
            PoolType::FixedSize {
                size: 1,
                capacity: 100
            }
            .max_capacity(),
            100
        );
        assert_eq!(
            PoolType::Growing {
                initial_size: 0,
                max_size: 500
            }
            .max_capacity(),
            500
        );
        assert_eq!(PoolType::RingBuffer { size: 10 }.max_capacity(), 10);
    }

    #[test]
    fn test_reserve() {
        let mut pool: MemoryPool<i32> = MemoryPool::default();
        pool.reserve(10);
        assert_eq!(pool.available_count(), 0);
    }
}
