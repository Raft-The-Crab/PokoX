//! Log levels and log entry structure

use chrono::{DateTime, Utc};
use std::fmt;
use thiserror::Error;

/// Errors that can occur with log level operations
#[derive(Debug, Clone, Error, PartialEq)]
pub enum LogLevelError {
    #[error("Invalid log level: {0}")]
    InvalidLevel(String),
}

/// Log level severity
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub enum LogLevel {
    /// Trace level - very detailed diagnostic information
    Trace,
    /// Debug level - diagnostic information for debugging
    Debug,
    /// Info level - general informational messages
    Info,
    /// Warn level - warning messages for potentially harmful situations
    Warn,
    /// Error level - error messages for error events
    Error,
}

impl LogLevel {
    /// Convert log level to string
    pub fn as_str(&self) -> &'static str {
        match self {
            LogLevel::Trace => "TRACE",
            LogLevel::Debug => "DEBUG",
            LogLevel::Info => "INFO",
            LogLevel::Warn => "WARN",
            LogLevel::Error => "ERROR",
        }
    }

    /// Get the numeric value of the log level
    pub fn as_u8(&self) -> u8 {
        match self {
            LogLevel::Trace => 0,
            LogLevel::Debug => 1,
            LogLevel::Info => 2,
            LogLevel::Warn => 3,
            LogLevel::Error => 4,
        }
    }

    /// Create log level from numeric value
    pub fn from_u8(value: u8) -> Result<Self, LogLevelError> {
        match value {
            0 => Ok(LogLevel::Trace),
            1 => Ok(LogLevel::Debug),
            2 => Ok(LogLevel::Info),
            3 => Ok(LogLevel::Warn),
            4 => Ok(LogLevel::Error),
            _ => Err(LogLevelError::InvalidLevel(format!("{}", value))),
        }
    }

    /// Check if this level is more severe than another
    pub fn is_more_severe_than(&self, other: LogLevel) -> bool {
        *self > other
    }

    /// Check if this level is less severe than another
    pub fn is_less_severe_than(&self, other: LogLevel) -> bool {
        *self < other
    }
}

impl fmt::Display for LogLevel {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}", self.as_str())
    }
}

impl std::str::FromStr for LogLevel {
    type Err = LogLevelError;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        match s.to_uppercase().as_str() {
            "TRACE" => Ok(LogLevel::Trace),
            "DEBUG" => Ok(LogLevel::Debug),
            "INFO" => Ok(LogLevel::Info),
            "WARN" => Ok(LogLevel::Warn),
            "ERROR" => Ok(LogLevel::Error),
            _ => Err(LogLevelError::InvalidLevel(s.to_string())),
        }
    }
}

/// A single log entry
#[derive(Debug, Clone)]
pub struct LogEntry {
    /// Timestamp when the log was created
    pub timestamp: DateTime<Utc>,
    /// Log level
    pub level: LogLevel,
    /// Target/module that produced the log
    pub target: String,
    /// Log message
    pub message: String,
    /// Optional context/metadata
    pub context: Option<String>,
    /// Optional file/line information
    pub location: Option<String>,
}

impl LogEntry {
    /// Create a new log entry
    pub fn new(level: LogLevel, target: String, message: String) -> Self {
        Self {
            timestamp: Utc::now(),
            level,
            target,
            message,
            context: None,
            location: None,
        }
    }

    /// Add context to the log entry
    pub fn with_context(mut self, context: String) -> Self {
        self.context = Some(context);
        self
    }

    /// Add location information to the log entry
    pub fn with_location(mut self, location: String) -> Self {
        self.location = Some(location);
        self
    }

    /// Format the log entry as a string
    pub fn format(&self) -> String {
        let base = format!(
            "[{}] [{}] [{}] {}",
            self.timestamp.format("%Y-%m-%d %H:%M:%S%.3f"),
            self.level,
            self.target,
            self.message
        );

        let mut parts = vec![base];

        if let Some(loc) = &self.location {
            parts.push(format!(" [{}]", loc));
        }

        if let Some(ctx) = &self.context {
            parts.push(format!(" | {}", ctx));
        }

        parts.join("")
    }

    /// Get the timestamp as a Unix timestamp
    pub fn timestamp_unix(&self) -> i64 {
        self.timestamp.timestamp()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::str::FromStr;

    #[test]
    fn test_log_level_ordering() {
        assert!(LogLevel::Error > LogLevel::Warn);
        assert!(LogLevel::Warn > LogLevel::Info);
        assert!(LogLevel::Info > LogLevel::Debug);
        assert!(LogLevel::Debug > LogLevel::Trace);
    }

    #[test]
    fn test_log_level_severity() {
        assert!(LogLevel::Error.is_more_severe_than(LogLevel::Warn));
        assert!(LogLevel::Info.is_less_severe_than(LogLevel::Warn));
    }

    #[test]
    fn test_log_level_from_str() {
        assert_eq!(LogLevel::from_str("INFO"), Ok(LogLevel::Info));
        assert_eq!(LogLevel::from_str("info"), Ok(LogLevel::Info));
        assert!(LogLevel::from_str("invalid").is_err());
    }

    #[test]
    fn test_log_level_from_u8() {
        assert_eq!(LogLevel::from_u8(2), Ok(LogLevel::Info));
        assert!(LogLevel::from_u8(99).is_err());
    }

    #[test]
    fn test_log_entry_creation() {
        let entry = LogEntry::new(
            LogLevel::Info,
            "test".to_string(),
            "test message".to_string(),
        );
        assert_eq!(entry.level, LogLevel::Info);
        assert_eq!(entry.target, "test");
        assert_eq!(entry.message, "test message");
    }

    #[test]
    fn test_log_entry_with_context() {
        let entry = LogEntry::new(
            LogLevel::Info,
            "test".to_string(),
            "test message".to_string(),
        )
        .with_context("context data".to_string());
        assert_eq!(entry.context, Some("context data".to_string()));
    }

    #[test]
    fn test_log_entry_with_location() {
        let entry = LogEntry::new(
            LogLevel::Info,
            "test".to_string(),
            "test message".to_string(),
        )
        .with_location("file.rs:42".to_string());
        assert_eq!(entry.location, Some("file.rs:42".to_string()));
    }

    #[test]
    fn test_log_entry_format() {
        let entry = LogEntry::new(
            LogLevel::Info,
            "test".to_string(),
            "test message".to_string(),
        );
        let formatted = entry.format();
        assert!(formatted.contains("INFO"));
        assert!(formatted.contains("test"));
        assert!(formatted.contains("test message"));
    }

    #[test]
    fn test_log_entry_timestamp_unix() {
        let entry = LogEntry::new(
            LogLevel::Info,
            "test".to_string(),
            "test message".to_string(),
        );
        let unix = entry.timestamp_unix();
        assert!(unix > 0);
    }
}
