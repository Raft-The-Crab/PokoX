//! Log output destinations

use crate::levels::LogEntry;
use std::fmt;
use std::vec::Vec;
use thiserror::Error;

/// Errors that can occur with log output
#[derive(Debug, Clone, Error)]
pub enum OutputError {
    #[error("Output error: {0}")]
    WriteFailed(String),
    #[error("Flush error: {0}")]
    FlushFailed(String),
}

/// Configuration for log output
#[derive(Debug, Clone)]
pub struct OutputConfig {
    /// Whether to include timestamps
    pub include_timestamp: bool,
    /// Whether to include target/module
    pub include_target: bool,
    /// Whether to include log level
    pub include_level: bool,
    /// Whether to include location information
    pub include_location: bool,
    /// Whether to include context
    pub include_context: bool,
}

impl Default for OutputConfig {
    fn default() -> Self {
        Self {
            include_timestamp: true,
            include_target: true,
            include_level: true,
            include_location: false,
            include_context: true,
        }
    }
}

/// Trait for log output destinations
pub trait LogOutput: fmt::Debug {
    /// Write a log entry
    fn write(&self, entry: &LogEntry) -> Result<(), OutputError>;

    /// Flush any buffered output
    fn flush(&self) -> Result<(), OutputError>;
}

/// Console output for logs
#[derive(Debug)]
pub struct ConsoleOutput {
    config: OutputConfig,
}

impl ConsoleOutput {
    /// Create a new console output with default configuration
    pub fn new() -> Self {
        Self::with_config(OutputConfig::default())
    }

    /// Create a new console output with custom configuration
    pub fn with_config(config: OutputConfig) -> Self {
        Self { config }
    }

    /// Get the output configuration
    pub fn config(&self) -> &OutputConfig {
        &self.config
    }

    /// Set the output configuration
    pub fn set_config(&mut self, config: OutputConfig) {
        self.config = config;
    }

    /// Format a log entry according to the configuration
    fn format_entry(&self, entry: &LogEntry) -> String {
        let mut parts = Vec::new();

        if self.config.include_timestamp {
            parts.push(format!(
                "[{}]",
                entry.timestamp.format("%Y-%m-%d %H:%M:%S%.3f")
            ));
        }

        if self.config.include_level {
            parts.push(format!("[{}]", entry.level));
        }

        if self.config.include_target {
            parts.push(format!("[{}]", entry.target));
        }

        if self.config.include_location {
            if let Some(loc) = &entry.location {
                parts.push(format!("[{}]", loc));
            }
        }

        parts.push(entry.message.clone());

        if self.config.include_context {
            if let Some(ctx) = &entry.context {
                parts.push(format!("| {}", ctx));
            }
        }

        parts.join(" ")
    }
}

impl Default for ConsoleOutput {
    fn default() -> Self {
        Self::new()
    }
}

impl LogOutput for ConsoleOutput {
    fn write(&self, entry: &LogEntry) -> Result<(), OutputError> {
        let formatted = self.format_entry(entry);
        println!("{}", formatted);
        Ok(())
    }

    fn flush(&self) -> Result<(), OutputError> {
        // Console output is not buffered
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::levels::LogLevel;

    #[test]
    fn test_console_output_creation() {
        let output = ConsoleOutput::new();
        assert!(output.config().include_timestamp);
    }

    #[test]
    fn test_console_output_write() {
        let output = ConsoleOutput::new();
        let entry = LogEntry::new(
            LogLevel::Info,
            "test".to_string(),
            "test message".to_string(),
        );
        let result = output.write(&entry);
        assert!(result.is_ok());
    }

    #[test]
    fn test_console_output_flush() {
        let output = ConsoleOutput::new();
        let result = output.flush();
        assert!(result.is_ok());
    }

    #[test]
    fn test_output_config() {
        let config = OutputConfig {
            include_timestamp: false,
            include_target: false,
            include_level: true,
            include_location: true,
            include_context: false,
        };
        let output = ConsoleOutput::with_config(config);
        let entry = LogEntry::new(
            LogLevel::Info,
            "test".to_string(),
            "test message".to_string(),
        )
        .with_location("file.rs:42".to_string());
        let result = output.write(&entry);
        assert!(result.is_ok());
    }

    #[test]
    fn test_console_output_with_location() {
        let output = ConsoleOutput::new();
        let entry = LogEntry::new(
            LogLevel::Info,
            "test".to_string(),
            "test message".to_string(),
        )
        .with_location("src/lib.rs:123".to_string());
        let result = output.write(&entry);
        assert!(result.is_ok());
    }

    #[test]
    fn test_console_output_set_config() {
        let mut output = ConsoleOutput::new();
        let new_config = OutputConfig {
            include_timestamp: false,
            include_target: false,
            include_level: false,
            include_location: false,
            include_context: false,
        };
        output.set_config(new_config);
        assert!(!output.config().include_timestamp);
    }
}
