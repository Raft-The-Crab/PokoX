//! Error handling for Poko Engine
//!
//! Provides unified error types and error handling utilities across the engine.

pub mod core;
pub mod io;
pub mod memory;
pub mod network;
pub mod validation;

pub use core::{EngineError, ErrorContext, ErrorKind, ErrorSeverity};
pub use io::{IoError, IoErrorKind};
pub use memory::{MemoryError, MemoryErrorKind};
pub use network::{NetworkError, NetworkErrorKind};
pub use validation::{ValidationError, ValidationKind};
