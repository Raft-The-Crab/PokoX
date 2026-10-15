//! Core error types for the engine

use std::fmt;
use thiserror::Error;

/// Severity level of an error
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub enum ErrorSeverity {
    /// Informational - can be ignored
    Info,
    /// Warning - should be noted but not fatal
    Warning,
    /// Error - operation failed but system can continue
    Error,
    /// Critical - system cannot continue
    Critical,
}

impl fmt::Display for ErrorSeverity {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            ErrorSeverity::Info => write!(f, "Info"),
            ErrorSeverity::Warning => write!(f, "Warning"),
            ErrorSeverity::Error => write!(f, "Error"),
            ErrorSeverity::Critical => write!(f, "Critical"),
        }
    }
}

/// Category of engine error
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum ErrorKind {
    /// Memory-related error
    Memory,
    /// I/O error
    Io,
    /// Network error
    Network,
    /// Validation error
    Validation,
    /// Serialization error
    Serialization,
    /// Configuration error
    Configuration,
    /// Resource not found
    NotFound,
    /// Permission denied
    PermissionDenied,
    /// Timeout error
    Timeout,
    /// Cancellation error
    Cancellation,
    /// Generic/internal error
    Internal(String),
}

impl ErrorKind {
    fn kind_to_string(&self) -> &str {
        match self {
            ErrorKind::Memory => "Memory",
            ErrorKind::Io => "I/O",
            ErrorKind::Network => "Network",
            ErrorKind::Validation => "Validation",
            ErrorKind::Serialization => "Serialization",
            ErrorKind::Configuration => "Configuration",
            ErrorKind::NotFound => "NotFound",
            ErrorKind::PermissionDenied => "PermissionDenied",
            ErrorKind::Timeout => "Timeout",
            ErrorKind::Cancellation => "Cancellation",
            ErrorKind::Internal(s) => s,
        }
    }
}

/// Context information for an error
#[derive(Debug, Clone)]
pub struct ErrorContext {
    /// Error severity
    pub severity: ErrorSeverity,
    /// Error kind
    pub kind: ErrorKind,
    /// Component/module that generated the error
    pub component: String,
    /// Additional context
    pub context: Vec<(String, String)>,
    /// Error code
    pub code: Option<String>,
    /// Stack trace (optional)
    pub stack_trace: Option<String>,
}

impl ErrorContext {
    /// Create a new error context
    pub fn new(severity: ErrorSeverity, kind: ErrorKind, component: String) -> Self {
        Self {
            severity,
            kind,
            component,
            context: Vec::new(),
            code: None,
            stack_trace: None,
        }
    }

    /// Add a context key-value pair
    pub fn add_context(mut self, key: String, value: String) -> Self {
        self.context.push((key, value));
        self
    }

    /// Set the error code
    pub fn with_code(mut self, code: String) -> Self {
        self.code = Some(code);
        self
    }

    /// Set the stack trace
    pub fn with_stack_trace(mut self, trace: String) -> Self {
        self.stack_trace = Some(trace);
        self
    }
}

/// Unified engine error
#[derive(Debug, Error)]
pub struct EngineError {
    /// Error message
    pub message: String,
    /// Error context
    pub context: ErrorContext,
    /// Source error (if any)
    #[source]
    pub source: Option<Box<dyn std::error::Error + Send + Sync>>,
}

impl EngineError {
    /// Create a new engine error
    pub fn new(message: String, context: ErrorContext) -> Self {
        Self {
            message,
            context,
            source: None,
        }
    }

    /// Create an error with a source
    pub fn with_source(
        message: String,
        context: ErrorContext,
        source: Box<dyn std::error::Error + Send + Sync>,
    ) -> Self {
        Self {
            message,
            context,
            source: Some(source),
        }
    }

    /// Get the error severity
    pub fn severity(&self) -> ErrorSeverity {
        self.context.severity
    }

    /// Get the error kind
    pub fn kind(&self) -> &ErrorKind {
        &self.context.kind
    }

    /// Get the component that generated the error
    pub fn component(&self) -> &str {
        &self.context.component
    }

    /// Check if the error is critical
    pub fn is_critical(&self) -> bool {
        self.context.severity == ErrorSeverity::Critical
    }

    /// Check if the error is recoverable
    pub fn is_recoverable(&self) -> bool {
        self.context.severity != ErrorSeverity::Critical
    }
}

impl fmt::Display for EngineError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "[{}][{}] {}: {}",
            self.context.severity,
            self.context.component,
            self.context.kind.kind_to_string(),
            self.message
        )
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_error_context_creation() {
        let context = ErrorContext::new(
            ErrorSeverity::Error,
            ErrorKind::Memory,
            "TestComponent".to_string(),
        );
        assert_eq!(context.severity, ErrorSeverity::Error);
        assert_eq!(context.component, "TestComponent");
    }

    #[test]
    fn test_error_context_with_additional_context() {
        let context = ErrorContext::new(
            ErrorSeverity::Warning,
            ErrorKind::Io,
            "TestComponent".to_string(),
        )
        .add_context("file".to_string(), "test.txt".to_string())
        .add_context("line".to_string(), "42".to_string());
        assert_eq!(context.context.len(), 2);
    }

    #[test]
    fn test_engine_error_creation() {
        let context = ErrorContext::new(
            ErrorSeverity::Critical,
            ErrorKind::Network,
            "NetworkManager".to_string(),
        );
        let error = EngineError::new("Connection failed".to_string(), context);
        assert!(error.is_critical());
        assert!(!error.is_recoverable());
    }

    #[test]
    fn test_engine_error_with_source() {
        let context = ErrorContext::new(
            ErrorSeverity::Error,
            ErrorKind::Io,
            "FileManager".to_string(),
        );
        let source: Box<dyn std::error::Error + Send + Sync> =
            Box::from(std::io::Error::new(std::io::ErrorKind::NotFound, "test"));
        let error = EngineError::with_source("File not found".to_string(), context, source);
        assert!(error.source.is_some());
    }

    #[test]
    fn test_error_severity_ordering() {
        assert!(ErrorSeverity::Critical > ErrorSeverity::Error);
        assert!(ErrorSeverity::Error > ErrorSeverity::Warning);
        assert!(ErrorSeverity::Warning > ErrorSeverity::Info);
    }
}
