//! Memory manager for coordinating tracking and pools

use crate::pools::{MemoryPool, PoolConfig, PoolError};
use crate::tracking::{MemoryTracker, MemoryTrackerError};
use std::sync::Arc;
use thiserror::Error;

/// Errors that can occur with memory management
#[derive(Debug, Clone, Error, PartialEq)]
pub enum MemoryManagerError {
    #[error("Memory limit exceeded: {0} bytes")]
    LimitExceeded(u64),
    #[error("Tracker error: {0}")]
    TrackerError(String),
    #[error("Pool error: {0}")]
    PoolError(String),
    #[error("Tracking is disabled")]
    TrackingDisabled,
}

impl From<MemoryTrackerError> for MemoryManagerError {
    fn from(err: MemoryTrackerError) -> Self {
        MemoryManagerError::TrackerError(err.to_string())
    }
}

impl From<PoolError> for MemoryManagerError {
    fn from(err: PoolError) -> Self {
        MemoryManagerError::PoolError(err.to_string())
    }
}

/// Configuration for the memory manager
#[derive(Debug, Clone)]
pub struct ManagerConfig {
    /// Memory limit in bytes (0 = no limit)
    pub memory_limit: u64,
    /// Whether to enable tracking
    pub enable_tracking: bool,
    /// Whether to panic on limit exceeded
    pub panic_on_limit: bool,
}

impl Default for ManagerConfig {
    fn default() -> Self {
        Self {
            memory_limit: 0,
            enable_tracking: true,
            panic_on_limit: false,
        }
    }
}

impl ManagerConfig {
    /// Validate the configuration
    pub fn validate(&self) -> Result<(), MemoryManagerError> {
        if self.memory_limit > 0 && self.memory_limit < 1024 {
            return Err(MemoryManagerError::LimitExceeded(self.memory_limit));
        }
        Ok(())
    }

    /// Create a new configuration with a memory limit
    pub fn with_limit(limit_bytes: u64) -> Self {
        Self {
            memory_limit: limit_bytes,
            enable_tracking: true,
            panic_on_limit: false,
        }
    }

    /// Enable tracking
    pub fn with_tracking(mut self) -> Self {
        self.enable_tracking = true;
        self
    }

    /// Disable tracking
    pub fn without_tracking(mut self) -> Self {
        self.enable_tracking = false;
        self
    }

    /// Enable panic on limit exceeded
    pub fn with_panic_on_limit(mut self) -> Self {
        self.panic_on_limit = true;
        self
    }

    /// Disable panic on limit exceeded
    pub fn without_panic_on_limit(mut self) -> Self {
        self.panic_on_limit = false;
        self
    }
}

/// Memory manager for coordinating memory operations
#[derive(Debug)]
pub struct MemoryManager {
    /// Configuration
    config: ManagerConfig,
    /// Memory tracker
    tracker: Arc<std::sync::Mutex<MemoryTracker>>,
}

impl MemoryManager {
    /// Create a new memory manager
    pub fn new() -> Result<Self, MemoryManagerError> {
        Self::with_config(ManagerConfig::default())
    }

    /// Create a new memory manager with custom configuration
    pub fn with_config(config: ManagerConfig) -> Result<Self, MemoryManagerError> {
        config.validate()?;
        Ok(Self {
            config,
            tracker: Arc::new(std::sync::Mutex::new(MemoryTracker::new())),
        })
    }

    /// Get the memory tracker
    pub fn tracker(&self) -> Arc<std::sync::Mutex<MemoryTracker>> {
        Arc::clone(&self.tracker)
    }

    /// Get the manager configuration
    pub fn config(&self) -> &ManagerConfig {
        &self.config
    }

    /// Check if tracking is enabled
    pub fn is_tracking_enabled(&self) -> bool {
        self.config.enable_tracking
    }

    /// Check if a memory limit is set
    pub fn has_limit(&self) -> bool {
        self.config.memory_limit > 0
    }

    /// Get the memory limit
    pub fn memory_limit(&self) -> u64 {
        self.config.memory_limit
    }

    /// Allocate memory with tracking
    pub fn allocate(&self, size: u64, category: Option<&str>) -> Result<(), MemoryManagerError> {
        if !self.config.enable_tracking {
            return Err(MemoryManagerError::TrackingDisabled);
        }

        // Check memory limit
        if self.config.memory_limit > 0 {
            let tracker = self.tracker.lock().unwrap();
            let current = tracker.current_allocated();
            if current + size > self.config.memory_limit {
                if self.config.panic_on_limit {
                    panic!("Memory limit exceeded: {} bytes", self.config.memory_limit);
                }
                return Err(MemoryManagerError::LimitExceeded(self.config.memory_limit));
            }
        }

        let mut tracker = self.tracker.lock().unwrap();
        tracker.record_allocation(size, category)?;
        Ok(())
    }

    /// Deallocate memory with tracking
    pub fn deallocate(&self, size: u64, category: Option<&str>) -> Result<(), MemoryManagerError> {
        if !self.config.enable_tracking {
            return Err(MemoryManagerError::TrackingDisabled);
        }

        let mut tracker = self.tracker.lock().unwrap();
        tracker.record_deallocation(size, category)?;
        Ok(())
    }

    /// Create a new memory pool
    pub fn create_pool<T>(&self, config: PoolConfig) -> Result<MemoryPool<T>, MemoryManagerError> {
        MemoryPool::new(config).map_err(Into::into)
    }

    /// Get memory statistics
    pub fn stats(&self) -> crate::tracking::MemoryStats {
        let tracker = self.tracker.lock().unwrap();
        tracker.stats()
    }

    /// Reset all tracking
    pub fn reset(&self) {
        let mut tracker = self.tracker.lock().unwrap();
        tracker.reset();
    }

    /// Get current memory usage
    pub fn current_usage(&self) -> u64 {
        let tracker = self.tracker.lock().unwrap();
        tracker.current_allocated()
    }

    /// Get memory usage as a percentage of limit
    pub fn usage_percentage(&self) -> f64 {
        if self.config.memory_limit == 0 {
            return 0.0;
        }
        let tracker = self.tracker.lock().unwrap();
        tracker.usage_percentage(self.config.memory_limit)
    }

    /// Check if memory usage is within limits
    pub fn is_within_limit(&self) -> bool {
        if self.config.memory_limit == 0 {
            return true;
        }
        let tracker = self.tracker.lock().unwrap();
        tracker.is_within_limit(self.config.memory_limit)
    }
}

impl Default for MemoryManager {
    fn default() -> Self {
        Self::new().expect("Default manager config should be valid")
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_manager_creation() {
        let manager = MemoryManager::new().unwrap();
        assert!(manager.is_tracking_enabled());
    }

    #[test]
    fn test_manager_allocate() {
        let manager = MemoryManager::new().unwrap();
        let result = manager.allocate(1024, Some("test"));
        assert!(result.is_ok());
        let tracker = manager.tracker.lock().unwrap();
        let stats = tracker.stats();
        assert_eq!(stats.total_allocated, 1024);
    }

    #[test]
    fn test_manager_allocate_disabled_tracking() {
        let config = ManagerConfig {
            memory_limit: 0,
            enable_tracking: false,
            panic_on_limit: false,
        };
        let manager = MemoryManager::with_config(config).unwrap();
        let result = manager.allocate(1024, Some("test"));
        assert!(result.is_err());
    }

    #[test]
    fn test_manager_deallocate() {
        let manager = MemoryManager::new().unwrap();
        manager.allocate(1024, Some("test")).unwrap();
        manager.deallocate(512, Some("test")).unwrap();
        let tracker = manager.tracker.lock().unwrap();
        let stats = tracker.stats();
        assert_eq!(stats.current_allocated, 512);
    }

    #[test]
    fn test_memory_limit() {
        let config = ManagerConfig {
            memory_limit: 1024,
            enable_tracking: true,
            panic_on_limit: false,
        };
        let manager = MemoryManager::with_config(config).unwrap();
        assert!(manager.allocate(512, None).is_ok());
        assert!(manager.allocate(1024, None).is_err());
    }

    #[test]
    fn test_create_pool() {
        let manager = MemoryManager::new().unwrap();
        let pool_config = PoolConfig::new(
            crate::pools::PoolType::Growing {
                initial_size: 0,
                max_size: 100,
            },
            "test".to_string(),
        )
        .unwrap();
        let pool: MemoryPool<i32> = manager.create_pool(pool_config).unwrap();
        assert_eq!(pool.available_count(), 0);
    }

    #[test]
    fn test_manager_reset() {
        let manager = MemoryManager::new().unwrap();
        manager.allocate(1024, Some("test")).unwrap();
        manager.reset();
        let tracker = manager.tracker.lock().unwrap();
        let stats = tracker.stats();
        assert_eq!(stats.total_allocated, 0);
    }

    #[test]
    fn test_current_usage() {
        let manager = MemoryManager::new().unwrap();
        manager.allocate(1024, Some("test")).unwrap();
        assert_eq!(manager.current_usage(), 1024);
    }

    #[test]
    fn test_usage_percentage() {
        let config = ManagerConfig {
            memory_limit: 2048,
            enable_tracking: true,
            panic_on_limit: false,
        };
        let manager = MemoryManager::with_config(config).unwrap();
        manager.allocate(1024, Some("test")).unwrap();
        let percentage = manager.usage_percentage();
        assert!((percentage - 50.0).abs() < 0.01);
    }

    #[test]
    fn test_is_within_limit() {
        let config = ManagerConfig {
            memory_limit: 2048,
            enable_tracking: true,
            panic_on_limit: false,
        };
        let manager = MemoryManager::with_config(config).unwrap();
        manager.allocate(1024, Some("test")).unwrap();
        assert!(manager.is_within_limit());
    }

    #[test]
    fn test_has_limit() {
        let config = ManagerConfig {
            memory_limit: 1024,
            enable_tracking: true,
            panic_on_limit: false,
        };
        let manager = MemoryManager::with_config(config).unwrap();
        assert!(manager.has_limit());
    }

    #[test]
    fn test_memory_limit_accessor() {
        let config = ManagerConfig {
            memory_limit: 2048,
            enable_tracking: true,
            panic_on_limit: false,
        };
        let manager = MemoryManager::with_config(config).unwrap();
        assert_eq!(manager.memory_limit(), 2048);
    }

    #[test]
    fn test_config_validate_invalid_limit() {
        let config = ManagerConfig {
            memory_limit: 512,
            enable_tracking: true,
            panic_on_limit: false,
        };
        let result = MemoryManager::with_config(config);
        assert!(result.is_err());
    }

    #[test]
    fn test_manager_config_builder() {
        let config = ManagerConfig::with_limit(1024)
            .with_tracking()
            .without_panic_on_limit();
        assert_eq!(config.memory_limit, 1024);
        assert!(config.enable_tracking);
        assert!(!config.panic_on_limit);
    }

    #[test]
    fn test_manager_config_without_tracking() {
        let config = ManagerConfig::with_limit(1024).without_tracking();
        assert!(!config.enable_tracking);
    }

    #[test]
    fn test_manager_config_with_panic() {
        let config = ManagerConfig::with_limit(1024).with_panic_on_limit();
        assert!(config.panic_on_limit);
    }
}
