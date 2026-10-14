//! Poko Engine Core
//!
//! Core types and utilities for the Poko engine.

pub use poko_ids_handles::{
    Handle, HandleBatch, HandleBytes, HandleGeneration, HandleIndex, HandleRegistry, Id,
    IdBatch, IdBytes, IdGenerator, IdParseError, IdType, RegistryError, RegistryStats,
    GenerationCounter, GenerationPool, GenerationValidator,
};

pub use poko_object_model::{
    Object, ObjectData, ObjectModelError, ObjectRegistry, ObjectType,
    Workspace, Model, Part, LifecycleEvent,
};
