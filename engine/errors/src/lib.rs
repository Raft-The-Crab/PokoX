//! Error handling for Poko Engine
//!
//! Provides unified error types and error handling utilities across the engine.

pub mod core;
pub mod memory;
pub mod io;
pub mod network;
pub mod validation;

pub use core::{EngineError, ErrorKind, ErrorSeverity, ErrorContext};
pub use memory::{MemoryError, MemoryErrorKind};
pub use io::{IoError, IoErrorKind};
pub use network::{NetworkError, NetworkErrorKind};
pub use validation::{ValidationError, ValidationKind};
