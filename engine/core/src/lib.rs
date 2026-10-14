//! Poko Engine Core
//!
//! Core types and utilities for the Poko engine.

pub use poko_ids_handles::{
    GenerationCounter, GenerationPool, GenerationValidator, Handle, HandleBatch, HandleBytes,
    HandleGeneration, HandleIndex, HandleRegistry, Id, IdBatch, IdBytes, IdGenerator, IdParseError,
    IdType, RegistryError, RegistryStats,
};

pub use poko_object_model::{
    LifecycleEvent, Model, Object, ObjectData, ObjectModelError, ObjectRegistry, ObjectType, Part,
    Workspace,
};
