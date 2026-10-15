//! Job system for Poko Engine
//!
//! Provides a job scheduling and execution system with priorities, dependencies, and cancellation.

pub mod scheduler;
pub mod priorities;
pub mod dependencies;
pub mod cancellation;

pub use scheduler::{JobScheduler, SchedulerConfig, SchedulerError, JobStatus, JobId};
pub use priorities::{JobPriority, PriorityPolicy};
pub use dependencies::{JobDependency, DependencyResolution, DependencyCheckResult};
pub use cancellation::{JobCancellation, CancellationToken, CancellationError};
