//! IDs and Handles for the Poko Engine
//!
//! This module provides:
//! - Stable serialized IDs for persistence across sessions
//! - Generation-safe handles for runtime object references
//! - Handle registry for allocation and validation
//! - Serialization utilities for efficient network transmission
//! - Generation tracking utilities

pub mod handle;
pub mod id;
pub mod registry;

// Submodules in separate directories
pub mod generation;
pub mod serialization;

pub use handle::{Handle, HandleGeneration, HandleIndex};
pub use id::{Id, IdGenerator, IdParseError, IdType};
pub use registry::{
    HandleRegistry, RegistryConfig, RegistryDiagnostics, RegistryError, RegistryStats, SlotMetadata,
};

pub use generation::{GenerationCounter, GenerationPool, GenerationValidator};
pub use serialization::{HandleBatch, HandleBytes, IdBatch, IdBytes};
