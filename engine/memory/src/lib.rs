//! Memory Management for Poko Engine
//!
//! Provides memory tracking, allocation monitoring, and pool management.

pub mod management;
pub mod pools;
pub mod tracking;

pub use management::{ManagerConfig, MemoryManager, MemoryManagerError};
pub use pools::{MemoryPool, PoolConfig, PoolError, PoolStats, PoolType};
pub use tracking::{AllocationInfo, MemoryStats, MemoryTracker, MemoryTrackerError};
