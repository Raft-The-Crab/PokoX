//! Lifecycle cleanup phase management

use chrono::{DateTime, Utc};
use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use thiserror::Error;
use uuid::Uuid;

/// Unique component identifier
pub type ComponentId = Uuid;

/// Cleanup phase
#[derive(Debug, Clone, Copy, PartialEq, Eq, serde::Serialize, serde::Deserialize)]
pub enum CleanupPhase {
    /// Component not yet cleaned up
    NotCleaned,
    /// Component is being cleaned up
    Cleaning,
    /// Component cleaned up
    Cleaned,
    /// Cleanup failed
    Failed,
}

/// Component cleanup state
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct CleanupState {
    /// Component ID
    pub component_id: ComponentId,
    /// Component name
    pub component_name: String,
    /// Current phase
    pub phase: CleanupPhase,
    /// Cleanup timestamp
    pub cleaned_at: Option<DateTime<Utc>>,
    /// Failure reason (if failed)
    pub failure_reason: Option<String>,
    /// State timestamp
    pub updated_at: DateTime<Utc>,
}

impl CleanupState {
    /// Create a new cleanup state
    pub fn new(component_id: ComponentId, component_name: String) -> Self {
        Self {
            component_id,
            component_name,
            phase: CleanupPhase::NotCleaned,
            cleaned_at: None,
            failure_reason: None,
            updated_at: Utc::now(),
        }
    }

    /// Mark as cleaning
    pub fn mark_cleaning(&mut self) {
        self.phase = CleanupPhase::Cleaning;
        self.updated_at = Utc::now();
    }

    /// Mark as cleaned
    pub fn mark_cleaned(&mut self) {
        self.phase = CleanupPhase::Cleaned;
        self.cleaned_at = Some(Utc::now());
        self.updated_at = Utc::now();
    }

    /// Mark as failed
    pub fn mark_failed(&mut self, reason: String) {
        self.phase = CleanupPhase::Failed;
        self.failure_reason = Some(reason);
        self.updated_at = Utc::now();
    }

    /// Check if the component is cleaned
    pub fn is_cleaned(&self) -> bool {
        self.phase == CleanupPhase::Cleaned
    }

    /// Check if the component is cleaning
    pub fn is_cleaning(&self) -> bool {
        self.phase == CleanupPhase::Cleaning
    }

    /// Check if the component failed
    pub fn is_failed(&self) -> bool {
        self.phase == CleanupPhase::Failed
    }

    /// Get the age of the state
    pub fn age(&self) -> chrono::Duration {
        Utc::now().signed_duration_since(self.updated_at)
    }

    /// Check if the state is stale
    pub fn is_stale(&self, max_age: chrono::Duration) -> bool {
        self.age() > max_age
    }
}

/// Errors that can occur during cleanup
#[derive(Debug, Clone, Error)]
pub enum CleanupError {
    #[error("Component already cleaned: {0}")]
    AlreadyCleaned(ComponentId),
    #[error("Component cleanup failed: {0}")]
    CleanupFailed(String),
    #[error("Component not found: {0}")]
    ComponentNotFound(ComponentId),
    #[error("Invalid cleanup phase transition")]
    InvalidPhaseTransition,
}

/// Lifecycle cleaner for managing component cleanup
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct LifecycleCleaner {
    /// Cleanup states by component ID
    states: HashMap<ComponentId, CleanupState>,
}

impl LifecycleCleaner {
    /// Create a new lifecycle cleaner
    pub fn new() -> Self {
        Self {
            states: HashMap::new(),
        }
    }

    /// Register a component for cleanup
    pub fn register(&mut self, component_id: ComponentId, component_name: String) {
        let state = CleanupState::new(component_id, component_name);
        self.states.insert(component_id, state);
    }

    /// Start cleaning a component
    pub fn start_cleanup(&mut self, component_id: ComponentId) -> Result<(), CleanupError> {
        let state = self
            .states
            .get_mut(&component_id)
            .ok_or(CleanupError::ComponentNotFound(component_id))?;

        if state.phase != CleanupPhase::NotCleaned {
            return Err(CleanupError::InvalidPhaseTransition);
        }

        state.mark_cleaning();
        Ok(())
    }

    /// Complete cleanup of a component
    pub fn complete_cleanup(&mut self, component_id: ComponentId) -> Result<(), CleanupError> {
        let state = self
            .states
            .get_mut(&component_id)
            .ok_or(CleanupError::ComponentNotFound(component_id))?;

        if state.phase != CleanupPhase::Cleaning {
            return Err(CleanupError::InvalidPhaseTransition);
        }

        state.mark_cleaned();
        Ok(())
    }

    /// Fail cleanup of a component
    pub fn fail_cleanup(
        &mut self,
        component_id: ComponentId,
        reason: String,
    ) -> Result<(), CleanupError> {
        let state = self
            .states
            .get_mut(&component_id)
            .ok_or(CleanupError::ComponentNotFound(component_id))?;

        state.mark_failed(reason);
        Ok(())
    }

    /// Get the cleanup state of a component
    pub fn get_state(&self, component_id: ComponentId) -> Option<&CleanupState> {
        self.states.get(&component_id)
    }

    /// Get the cleanup phase of a component
    pub fn get_phase(&self, component_id: ComponentId) -> Option<CleanupPhase> {
        self.states.get(&component_id).map(|s| s.phase)
    }

    /// Unregister a component
    pub fn unregister(&mut self, component_id: ComponentId) {
        self.states.remove(&component_id);
    }

    /// Get the number of registered components
    pub fn count(&self) -> usize {
        self.states.len()
    }

    /// Get the number of components in each phase
    pub fn phase_counts(&self) -> (usize, usize, usize, usize) {
        let mut not_cleaned = 0;
        let mut cleaning = 0;
        let mut cleaned = 0;
        let mut failed = 0;

        for state in self.states.values() {
            match state.phase {
                CleanupPhase::NotCleaned => not_cleaned += 1,
                CleanupPhase::Cleaning => cleaning += 1,
                CleanupPhase::Cleaned => cleaned += 1,
                CleanupPhase::Failed => failed += 1,
            }
        }

        (not_cleaned, cleaning, cleaned, failed)
    }

    /// Get cleanup statistics
    pub fn stats(&self) -> CleanupStats {
        let (not_cleaned, cleaning, cleaned, failed) = self.phase_counts();
        CleanupStats {
            total: self.count(),
            not_cleaned,
            cleaning,
            cleaned,
            failed,
        }
    }

    /// Clear all registered components
    pub fn clear(&mut self) {
        self.states.clear();
    }

    /// Serialize to JSON
    pub fn to_json(&self) -> Result<String, serde_json::Error> {
        serde_json::to_string_pretty(&self.states)
    }

    /// Deserialize from JSON
    pub fn from_json(json: &str) -> Result<Self, serde_json::Error> {
        let states = serde_json::from_str(json)?;
        Ok(Self { states })
    }
}

/// Cleanup statistics
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct CleanupStats {
    pub total: usize,
    pub not_cleaned: usize,
    pub cleaning: usize,
    pub cleaned: usize,
    pub failed: usize,
}

impl Default for LifecycleCleaner {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_cleanup_state() {
        let component_id = Uuid::new_v4();
        let mut state = CleanupState::new(component_id, "TestComponent".to_string());
        assert_eq!(state.phase, CleanupPhase::NotCleaned);

        state.mark_cleaning();
        assert_eq!(state.phase, CleanupPhase::Cleaning);
        assert!(state.is_cleaning());

        state.mark_cleaned();
        assert_eq!(state.phase, CleanupPhase::Cleaned);
        assert!(state.is_cleaned());
        assert!(state.cleaned_at.is_some());
    }

    #[test]
    fn test_cleanup_state_failed() {
        let component_id = Uuid::new_v4();
        let mut state = CleanupState::new(component_id, "TestComponent".to_string());
        state.mark_failed("Test failure".to_string());
        assert_eq!(state.phase, CleanupPhase::Failed);
        assert!(state.is_failed());
        assert_eq!(state.failure_reason, Some("Test failure".to_string()));
    }

    #[test]
    fn test_lifecycle_cleaner() {
        let mut cleaner = LifecycleCleaner::new();
        let component_id = Uuid::new_v4();
        cleaner.register(component_id, "TestComponent".to_string());
        assert_eq!(cleaner.count(), 1);
    }

    #[test]
    fn test_start_cleanup() {
        let mut cleaner = LifecycleCleaner::new();
        let component_id = Uuid::new_v4();
        cleaner.register(component_id, "TestComponent".to_string());
        cleaner.start_cleanup(component_id).unwrap();
        assert_eq!(
            cleaner.get_phase(component_id),
            Some(CleanupPhase::Cleaning)
        );
    }

    #[test]
    fn test_complete_cleanup() {
        let mut cleaner = LifecycleCleaner::new();
        let component_id = Uuid::new_v4();
        cleaner.register(component_id, "TestComponent".to_string());
        cleaner.start_cleanup(component_id).unwrap();
        cleaner.complete_cleanup(component_id).unwrap();
        assert_eq!(cleaner.get_phase(component_id), Some(CleanupPhase::Cleaned));
    }

    #[test]
    fn test_fail_cleanup() {
        let mut cleaner = LifecycleCleaner::new();
        let component_id = Uuid::new_v4();
        cleaner.register(component_id, "TestComponent".to_string());
        cleaner
            .fail_cleanup(component_id, "Test failure".to_string())
            .unwrap();
        assert_eq!(cleaner.get_phase(component_id), Some(CleanupPhase::Failed));
    }

    #[test]
    fn test_invalid_phase_transition() {
        let mut cleaner = LifecycleCleaner::new();
        let component_id = Uuid::new_v4();
        cleaner.register(component_id, "TestComponent".to_string());
        cleaner.complete_cleanup(component_id).unwrap_err();
    }

    #[test]
    fn test_phase_counts() {
        let mut cleaner = LifecycleCleaner::new();
        let id1 = Uuid::new_v4();
        let id2 = Uuid::new_v4();
        let id3 = Uuid::new_v4();
        let id4 = Uuid::new_v4();

        cleaner.register(id1, "Component1".to_string());
        cleaner.register(id2, "Component2".to_string());
        cleaner.register(id3, "Component3".to_string());
        cleaner.register(id4, "Component4".to_string());

        cleaner.start_cleanup(id2).unwrap();
        cleaner.start_cleanup(id3).unwrap();
        cleaner.complete_cleanup(id3).unwrap();
        cleaner.start_cleanup(id4).unwrap();
        cleaner.fail_cleanup(id4, "Test".to_string()).unwrap();

        let (not_cleaned, cleaning, cleaned, failed) = cleaner.phase_counts();
        assert_eq!(not_cleaned, 1);
        assert_eq!(cleaning, 1);
        assert_eq!(cleaned, 1);
        assert_eq!(failed, 1);
    }

    #[test]
    fn test_cleanup_stats() {
        let mut cleaner = LifecycleCleaner::new();
        let id = Uuid::new_v4();
        cleaner.register(id, "TestComponent".to_string());
        cleaner.start_cleanup(id).unwrap();
        let stats = cleaner.stats();
        assert_eq!(stats.total, 1);
        assert_eq!(stats.cleaning, 1);
    }

    #[test]
    fn test_state_age() {
        let mut state = CleanupState::new(Uuid::new_v4(), "Test".to_string());
        state.mark_cleaning();
        let age = state.age();
        assert!(age.num_seconds() >= 0);
    }

    #[test]
    fn test_state_serialization() {
        let mut cleaner = LifecycleCleaner::new();
        let id = Uuid::new_v4();
        cleaner.register(id, "TestComponent".to_string());
        let json = cleaner.to_json().unwrap();
        let restored = LifecycleCleaner::from_json(&json).unwrap();
        assert_eq!(restored.count(), 1);
    }
}
