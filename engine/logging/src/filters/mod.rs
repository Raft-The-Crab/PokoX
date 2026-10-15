//! Log filtering functionality

use crate::levels::{LogEntry, LogLevel, LogLevelError};
use std::collections::HashSet;

/// Configuration for log filtering
#[derive(Debug, Clone)]
pub struct FilterConfig {
    /// Minimum log level to allow
    pub min_level: LogLevel,
    /// Specific targets to include (empty means all)
    pub include_targets: HashSet<String>,
    /// Specific targets to exclude
    pub exclude_targets: HashSet<String>,
}

impl Default for FilterConfig {
    fn default() -> Self {
        Self {
            min_level: LogLevel::Info,
            include_targets: HashSet::new(),
            exclude_targets: HashSet::new(),
        }
    }
}

impl FilterConfig {
    /// Validate the filter configuration
    pub fn validate(&self) -> Result<(), LogLevelError> {
        // Check for conflicts between include and exclude
        for target in &self.include_targets {
            if self.exclude_targets.contains(target) {
                return Err(LogLevelError::InvalidLevel(format!(
                    "Target '{}' is in both include and exclude lists",
                    target
                )));
            }
        }
        Ok(())
    }

    /// Create a filter config with a minimum level
    pub fn with_min_level(min_level: LogLevel) -> Self {
        Self {
            min_level,
            include_targets: HashSet::new(),
            exclude_targets: HashSet::new(),
        }
    }

    /// Add an include target
    pub fn include_target(mut self, target: String) -> Self {
        self.include_targets.insert(target);
        self
    }

    /// Add an exclude target
    pub fn exclude_target(mut self, target: String) -> Self {
        self.exclude_targets.insert(target);
        self
    }
}

/// Log filter for filtering log entries
#[derive(Debug, Clone)]
pub struct LogFilter {
    config: FilterConfig,
}

impl LogFilter {
    /// Create a new log filter with default configuration
    pub fn new() -> Result<Self, LogLevelError> {
        Self::with_config(FilterConfig::default())
    }

    /// Create a new log filter with custom configuration
    pub fn with_config(config: FilterConfig) -> Result<Self, LogLevelError> {
        config.validate()?;
        Ok(Self { config })
    }

    /// Check if a log entry should be allowed
    pub fn should_log(&self, entry: &LogEntry) -> bool {
        // Check minimum level
        if entry.level < self.config.min_level {
            return false;
        }

        // Check excluded targets
        if self.config.exclude_targets.contains(&entry.target) {
            return false;
        }

        // Check included targets (if specified)
        if !self.config.include_targets.is_empty()
            && !self.config.include_targets.contains(&entry.target)
        {
            return false;
        }

        true
    }

    /// Set the minimum log level
    pub fn set_min_level(&mut self, level: LogLevel) {
        self.config.min_level = level;
    }

    /// Add a target to include
    pub fn include_target(&mut self, target: String) {
        self.config.include_targets.insert(target);
    }

    /// Add a target to exclude
    pub fn exclude_target(&mut self, target: String) {
        self.config.exclude_targets.insert(target);
    }

    /// Remove a target from include
    pub fn remove_include_target(&mut self, target: &str) {
        self.config.include_targets.remove(target);
    }

    /// Remove a target from exclude
    pub fn remove_exclude_target(&mut self, target: &str) {
        self.config.exclude_targets.remove(target);
    }

    /// Get the filter configuration
    pub fn config(&self) -> &FilterConfig {
        &self.config
    }

    /// Set the filter configuration
    pub fn set_config(&mut self, config: FilterConfig) -> Result<(), LogLevelError> {
        config.validate()?;
        self.config = config;
        Ok(())
    }

    /// Clear all include targets
    pub fn clear_includes(&mut self) {
        self.config.include_targets.clear();
    }

    /// Clear all exclude targets
    pub fn clear_excludes(&mut self) {
        self.config.exclude_targets.clear();
    }
}

impl Default for LogFilter {
    fn default() -> Self {
        Self::new().expect("Default filter config should be valid")
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_filter_creation() {
        let filter = LogFilter::new().unwrap();
        assert_eq!(filter.config().min_level, LogLevel::Info);
    }

    #[test]
    fn test_min_level_filter() {
        let mut filter = LogFilter::new().unwrap();
        filter.set_min_level(LogLevel::Warn);

        let info_entry = LogEntry::new(LogLevel::Info, "test".to_string(), "test".to_string());
        let warn_entry = LogEntry::new(LogLevel::Warn, "test".to_string(), "test".to_string());

        assert!(!filter.should_log(&info_entry));
        assert!(filter.should_log(&warn_entry));
    }

    #[test]
    fn test_exclude_target() {
        let mut filter = LogFilter::new().unwrap();
        filter.exclude_target("spam".to_string());

        let normal_entry = LogEntry::new(LogLevel::Info, "normal".to_string(), "test".to_string());
        let spam_entry = LogEntry::new(LogLevel::Info, "spam".to_string(), "test".to_string());

        assert!(filter.should_log(&normal_entry));
        assert!(!filter.should_log(&spam_entry));
    }

    #[test]
    fn test_include_target() {
        let mut filter = LogFilter::new().unwrap();
        filter.include_target("important".to_string());

        let important_entry =
            LogEntry::new(LogLevel::Info, "important".to_string(), "test".to_string());
        let other_entry = LogEntry::new(LogLevel::Info, "other".to_string(), "test".to_string());

        assert!(filter.should_log(&important_entry));
        assert!(!filter.should_log(&other_entry));
    }

    #[test]
    fn test_combined_filters() {
        let mut filter = LogFilter::new().unwrap();
        filter.set_min_level(LogLevel::Debug);
        filter.exclude_target("spam".to_string());

        let debug_entry = LogEntry::new(LogLevel::Debug, "normal".to_string(), "test".to_string());
        let spam_entry = LogEntry::new(LogLevel::Debug, "spam".to_string(), "test".to_string());
        let trace_entry = LogEntry::new(LogLevel::Trace, "normal".to_string(), "test".to_string());

        assert!(filter.should_log(&debug_entry));
        assert!(!filter.should_log(&spam_entry));
        assert!(!filter.should_log(&trace_entry));
    }

    #[test]
    fn test_filter_config_validate_conflict() {
        let config = FilterConfig {
            min_level: LogLevel::Info,
            include_targets: {
                let mut set = HashSet::new();
                set.insert("test".to_string());
                set
            },
            exclude_targets: {
                let mut set = HashSet::new();
                set.insert("test".to_string());
                set
            },
        };
        assert!(config.validate().is_err());
    }

    #[test]
    fn test_filter_config_builder() {
        let config = FilterConfig::with_min_level(LogLevel::Debug)
            .include_target("important".to_string())
            .exclude_target("spam".to_string());
        assert_eq!(config.min_level, LogLevel::Debug);
        assert!(config.include_targets.contains("important"));
        assert!(config.exclude_targets.contains("spam"));
    }

    #[test]
    fn test_remove_targets() {
        let mut filter = LogFilter::new().unwrap();
        filter.include_target("test".to_string());
        filter.exclude_target("spam".to_string());

        assert!(filter.config().include_targets.contains("test"));
        filter.remove_include_target("test");
        assert!(!filter.config().include_targets.contains("test"));

        assert!(filter.config().exclude_targets.contains("spam"));
        filter.remove_exclude_target("spam");
        assert!(!filter.config().exclude_targets.contains("spam"));
    }

    #[test]
    fn test_clear_targets() {
        let mut filter = LogFilter::new().unwrap();
        filter.include_target("test1".to_string());
        filter.include_target("test2".to_string());
        filter.exclude_target("spam1".to_string());
        filter.exclude_target("spam2".to_string());

        filter.clear_includes();
        assert!(filter.config().include_targets.is_empty());

        filter.clear_excludes();
        assert!(filter.config().exclude_targets.is_empty());
    }

    #[test]
    fn test_set_config_validation() {
        let mut filter = LogFilter::new().unwrap();
        let invalid_config = FilterConfig {
            min_level: LogLevel::Info,
            include_targets: {
                let mut set = HashSet::new();
                set.insert("test".to_string());
                set
            },
            exclude_targets: {
                let mut set = HashSet::new();
                set.insert("test".to_string());
                set
            },
        };
        let result = filter.set_config(invalid_config);
        assert!(result.is_err());
    }
}
