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
pub mod serialization;
pub mod generation;

pub use handle::{Handle, HandleGeneration, HandleIndex};
pub use id::{Id, IdGenerator, IdParseError, IdType};
pub use registry::{HandleRegistry, RegistryError, RegistryStats};

pub use serialization::{HandleBatch, HandleBytes, IdBatch, IdBytes};
pub use generation::{GenerationCounter, GenerationPool, GenerationValidator};
