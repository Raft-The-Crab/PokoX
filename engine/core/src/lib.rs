//! Poko Engine Core
//!
//! Core types and utilities for the Poko engine.

pub use poko_ids_handles::{
    GenerationCounter, GenerationPool, GenerationValidator, Handle, HandleBatch, HandleBytes,
    HandleGeneration, HandleIndex, HandleRegistry, Id, IdBatch, IdBytes, IdGenerator, IdParseError,
    IdType, RegistryError, RegistryStats,
};

pub use poko_object_model::{
    DetailedRegistryStats, LifecycleEvent, Model, Object, ObjectData, ObjectModelError, ObjectRegistry,
    ObjectType, Part, RegistryStatistics, Workspace,
};

pub use poko_transforms::{
    Transform, TransformError, TransformRegistry, TransformStats,
};

pub use poko_metadata_registry::{
    DataType, DefaultValue, EventMetadata, FunctionMetadata, InspectorConfig, MetadataRegistry,
    MetadataRegistryStats, ParameterMetadata, PropertyMetadata, ReplicationPolicy, TypeMetadata,
    Validator, ValidationError, Visibility,
};
