//! Lifecycle management for Poko Engine
//!
//! Provides comprehensive lifecycle management for engine components including creation, initialization, cleanup, and destruction.

pub mod creation;
pub mod destruction;
pub mod initialization;
pub mod cleanup;

pub use creation::{LifecycleCreator, CreationPhase, CreationError, CreationState};
pub use destruction::{LifecycleDestroyer, DestructionPhase, DestructionError, DestructionState};
pub use initialization::{LifecycleInitializer, InitializationPhase, InitializationError, InitializationState};
pub use cleanup::{LifecycleCleaner, CleanupPhase, CleanupError, CleanupState};
