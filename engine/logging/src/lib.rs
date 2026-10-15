//! Logging System for Poko Engine
//!
//! Provides structured logging with multiple levels, filters, and output destinations.

pub mod filters;
pub mod levels;
pub mod output;

pub use filters::{FilterConfig, LogFilter};
pub use levels::{LogEntry, LogLevel, LogLevelError};
pub use output::{ConsoleOutput, LogOutput, OutputConfig, OutputError};
