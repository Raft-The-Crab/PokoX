//! Lifecycle management for Poko Engine
//!
//! Provides comprehensive lifecycle management for engine components including creation, initialization, cleanup, and destruction.

pub mod cleanup;
pub mod creation;
pub mod destruction;
pub mod initialization;

pub use cleanup::{CleanupError, CleanupPhase, CleanupState, CleanupStats, LifecycleCleaner};
pub use creation::{CreationError, CreationPhase, CreationState, CreationStats, LifecycleCreator};
pub use destruction::{
    DestructionError, DestructionPhase, DestructionState, DestructionStats, LifecycleDestroyer,
};
pub use initialization::{
    InitializationError, InitializationPhase, InitializationState, InitializationStats,
    LifecycleInitializer,
};
