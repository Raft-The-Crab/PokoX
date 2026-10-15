//! Poko Engine Core
//!
//! Core types and utilities for the Poko engine.

pub use poko_ids_handles::{
    GenerationCounter, GenerationPool, GenerationValidator, Handle, HandleBatch, HandleBytes,
    HandleGeneration, HandleIndex, HandleRegistry, Id, IdBatch, IdBytes, IdGenerator, IdParseError,
    IdType, RegistryError, RegistryStats,
};

pub use poko_object_model::{
    DetailedRegistryStats, LifecycleEvent, Model, Object, ObjectData, ObjectModelError,
    ObjectRegistry, ObjectType, Part, RegistryStatistics, Workspace,
};

pub use poko_transforms::{Transform, TransformError, TransformRegistry, TransformStats};

pub use poko_metadata_registry::{
    DataType, DefaultValue, EventMetadata, FunctionMetadata, InspectorConfig, MetadataRegistry,
    MetadataRegistryStats, ParameterMetadata, PropertyMetadata, ReplicationPolicy, TypeMetadata,
    ValidationError, Validator, Visibility,
};

pub use poko_time::{
    AccumulatorError, Clock, ClockConfig, ClockError, FixedStepAccumulator, FixedStepConfig,
    FrameStepMode, FrameStepper, StepError, StepperStats,
};

pub use poko_logging::{
    ConsoleOutput, FilterConfig, LogEntry, LogFilter, LogLevel, LogLevelError, LogOutput,
    OutputConfig, OutputError,
};

pub use poko_memory::{
    AllocationInfo, ManagerConfig, MemoryManager, MemoryManagerError, MemoryPool, MemoryStats,
    MemoryTracker, MemoryTrackerError, PoolConfig, PoolError, PoolStats, PoolType,
};
