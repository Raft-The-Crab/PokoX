//! Job system for Poko Engine
//!
//! Provides a job scheduling and execution system with priorities, dependencies, and cancellation.

pub mod cancellation;
pub mod dependencies;
pub mod priorities;
pub mod scheduler;

pub use cancellation::{CancellationError, CancellationToken, JobCancellation};
pub use dependencies::{DependencyCheckResult, DependencyResolution, JobDependency};
pub use priorities::{JobPriority, PriorityPolicy};
pub use scheduler::{
    JobId, JobScheduler, JobStatus, SchedulerConfig, SchedulerError, SchedulerStats,
};
