//! Lifecycle creation phase management

use chrono::{DateTime, Utc};
use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use thiserror::Error;
use uuid::Uuid;

/// Unique component identifier
pub type ComponentId = Uuid;

/// Creation phase
#[derive(Debug, Clone, Copy, PartialEq, Eq, serde::Serialize, serde::Deserialize)]
pub enum CreationPhase {
    /// Component not yet created
    NotCreated,
    /// Component is being created
    Creating,
    /// Component created
    Created,
    /// Creation failed
    Failed,
}

/// Component creation state
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct CreationState {
    /// Component ID
    pub component_id: ComponentId,
    /// Component name
    pub component_name: String,
    /// Current phase
    pub phase: CreationPhase,
    /// Creation timestamp
    pub created_at: Option<DateTime<Utc>>,
    /// Failure reason (if failed)
    pub failure_reason: Option<String>,
    /// State timestamp
    pub updated_at: DateTime<Utc>,
}

impl CreationState {
    /// Create a new creation state
    pub fn new(component_id: ComponentId, component_name: String) -> Self {
        Self {
            component_id,
            component_name,
            phase: CreationPhase::NotCreated,
            created_at: None,
            failure_reason: None,
            updated_at: Utc::now(),
        }
    }

    /// Mark as creating
    pub fn mark_creating(&mut self) {
        self.phase = CreationPhase::Creating;
        self.updated_at = Utc::now();
    }

    /// Mark as created
    pub fn mark_created(&mut self) {
        self.phase = CreationPhase::Created;
        self.created_at = Some(Utc::now());
        self.updated_at = Utc::now();
    }

    /// Mark as failed
    pub fn mark_failed(&mut self, reason: String) {
        self.phase = CreationPhase::Failed;
        self.failure_reason = Some(reason);
        self.updated_at = Utc::now();
    }

    /// Check if the component is created
    pub fn is_created(&self) -> bool {
        self.phase == CreationPhase::Created
    }

    /// Check if the component is creating
    pub fn is_creating(&self) -> bool {
        self.phase == CreationPhase::Creating
    }

    /// Check if the component failed
    pub fn is_failed(&self) -> bool {
        self.phase == CreationPhase::Failed
    }

    /// Get the age of the state
    pub fn age(&self) -> chrono::Duration {
        Utc::now().signed_duration_since(self.updated_at)
    }

    /// Check if the state is stale (older than given duration)
    pub fn is_stale(&self, max_age: chrono::Duration) -> bool {
        self.age() > max_age
    }
}

/// Errors that can occur during creation
#[derive(Debug, Clone, Error)]
pub enum CreationError {
    #[error("Component already created: {0}")]
    AlreadyCreated(ComponentId),
    #[error("Component creation failed: {0}")]
    CreationFailed(String),
    #[error("Component not found: {0}")]
    ComponentNotFound(ComponentId),
    #[error("Invalid creation phase transition")]
    InvalidPhaseTransition,
}

/// Lifecycle creator for managing component creation
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct LifecycleCreator {
    /// Creation states by component ID
    states: HashMap<ComponentId, CreationState>,
}

impl LifecycleCreator {
    /// Create a new lifecycle creator
    pub fn new() -> Self {
        Self {
            states: HashMap::new(),
        }
    }

    /// Register a component for creation
    pub fn register(&mut self, component_id: ComponentId, component_name: String) {
        let state = CreationState::new(component_id, component_name);
        self.states.insert(component_id, state);
    }

    /// Start creating a component
    pub fn start_creation(&mut self, component_id: ComponentId) -> Result<(), CreationError> {
        let state = self
            .states
            .get_mut(&component_id)
            .ok_or(CreationError::ComponentNotFound(component_id))?;

        if state.phase != CreationPhase::NotCreated {
            return Err(CreationError::InvalidPhaseTransition);
        }

        state.mark_creating();
        Ok(())
    }

    /// Complete creation of a component
    pub fn complete_creation(&mut self, component_id: ComponentId) -> Result<(), CreationError> {
        let state = self
            .states
            .get_mut(&component_id)
            .ok_or(CreationError::ComponentNotFound(component_id))?;

        if state.phase != CreationPhase::Creating {
            return Err(CreationError::InvalidPhaseTransition);
        }

        state.mark_created();
        Ok(())
    }

    /// Fail creation of a component
    pub fn fail_creation(
        &mut self,
        component_id: ComponentId,
        reason: String,
    ) -> Result<(), CreationError> {
        let state = self
            .states
            .get_mut(&component_id)
            .ok_or(CreationError::ComponentNotFound(component_id))?;

        state.mark_failed(reason);
        Ok(())
    }

    /// Get the creation state of a component
    pub fn get_state(&self, component_id: ComponentId) -> Option<&CreationState> {
        self.states.get(&component_id)
    }

    /// Get the creation phase of a component
    pub fn get_phase(&self, component_id: ComponentId) -> Option<CreationPhase> {
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
        let mut not_created = 0;
        let mut creating = 0;
        let mut created = 0;
        let mut failed = 0;

        for state in self.states.values() {
            match state.phase {
                CreationPhase::NotCreated => not_created += 1,
                CreationPhase::Creating => creating += 1,
                CreationPhase::Created => created += 1,
                CreationPhase::Failed => failed += 1,
            }
        }

        (not_created, creating, created, failed)
    }

    /// Get creation statistics
    pub fn stats(&self) -> CreationStats {
        let (not_created, creating, created, failed) = self.phase_counts();
        CreationStats {
            total: self.count(),
            not_created,
            creating,
            created,
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

/// Creation statistics
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct CreationStats {
    pub total: usize,
    pub not_created: usize,
    pub creating: usize,
    pub created: usize,
    pub failed: usize,
}

impl Default for LifecycleCreator {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_creation_state() {
        let component_id = Uuid::new_v4();
        let mut state = CreationState::new(component_id, "TestComponent".to_string());
        assert_eq!(state.phase, CreationPhase::NotCreated);

        state.mark_creating();
        assert_eq!(state.phase, CreationPhase::Creating);
        assert!(state.is_creating());

        state.mark_created();
        assert_eq!(state.phase, CreationPhase::Created);
        assert!(state.is_created());
        assert!(state.created_at.is_some());
    }

    #[test]
    fn test_creation_state_failed() {
        let component_id = Uuid::new_v4();
        let mut state = CreationState::new(component_id, "TestComponent".to_string());
        state.mark_failed("Test failure".to_string());
        assert_eq!(state.phase, CreationPhase::Failed);
        assert!(state.is_failed());
        assert_eq!(state.failure_reason, Some("Test failure".to_string()));
    }

    #[test]
    fn test_lifecycle_creator() {
        let mut creator = LifecycleCreator::new();
        let component_id = Uuid::new_v4();
        creator.register(component_id, "TestComponent".to_string());
        assert_eq!(creator.count(), 1);
    }

    #[test]
    fn test_start_creation() {
        let mut creator = LifecycleCreator::new();
        let component_id = Uuid::new_v4();
        creator.register(component_id, "TestComponent".to_string());
        creator.start_creation(component_id).unwrap();
        assert_eq!(
            creator.get_phase(component_id),
            Some(CreationPhase::Creating)
        );
    }

    #[test]
    fn test_complete_creation() {
        let mut creator = LifecycleCreator::new();
        let component_id = Uuid::new_v4();
        creator.register(component_id, "TestComponent".to_string());
        creator.start_creation(component_id).unwrap();
        creator.complete_creation(component_id).unwrap();
        assert_eq!(
            creator.get_phase(component_id),
            Some(CreationPhase::Created)
        );
    }

    #[test]
    fn test_fail_creation() {
        let mut creator = LifecycleCreator::new();
        let component_id = Uuid::new_v4();
        creator.register(component_id, "TestComponent".to_string());
        creator
            .fail_creation(component_id, "Test failure".to_string())
            .unwrap();
        assert_eq!(creator.get_phase(component_id), Some(CreationPhase::Failed));
    }

    #[test]
    fn test_invalid_phase_transition() {
        let mut creator = LifecycleCreator::new();
        let component_id = Uuid::new_v4();
        creator.register(component_id, "TestComponent".to_string());
        creator.complete_creation(component_id).unwrap_err();
    }

    #[test]
    fn test_phase_counts() {
        let mut creator = LifecycleCreator::new();
        let id1 = Uuid::new_v4();
        let id2 = Uuid::new_v4();
        let id3 = Uuid::new_v4();
        let id4 = Uuid::new_v4();

        creator.register(id1, "Component1".to_string());
        creator.register(id2, "Component2".to_string());
        creator.register(id3, "Component3".to_string());
        creator.register(id4, "Component4".to_string());

        creator.start_creation(id2).unwrap();
        creator.start_creation(id3).unwrap();
        creator.complete_creation(id3).unwrap();
        creator.start_creation(id4).unwrap();
        creator.fail_creation(id4, "Test".to_string()).unwrap();

        let (not_created, creating, created, failed) = creator.phase_counts();
        assert_eq!(not_created, 1);
        assert_eq!(creating, 1);
        assert_eq!(created, 1);
        assert_eq!(failed, 1);
    }

    #[test]
    fn test_creation_stats() {
        let mut creator = LifecycleCreator::new();
        let id = Uuid::new_v4();
        creator.register(id, "TestComponent".to_string());
        creator.start_creation(id).unwrap();
        let stats = creator.stats();
        assert_eq!(stats.total, 1);
        assert_eq!(stats.creating, 1);
    }

    #[test]
    fn test_state_age() {
        let mut state = CreationState::new(Uuid::new_v4(), "Test".to_string());
        state.mark_creating();
        let age = state.age();
        assert!(age.num_seconds() >= 0);
    }

    #[test]
    fn test_state_serialization() {
        let mut creator = LifecycleCreator::new();
        let id = Uuid::new_v4();
        creator.register(id, "TestComponent".to_string());
        let json = creator.to_json().unwrap();
        let restored = LifecycleCreator::from_json(&json).unwrap();
        assert_eq!(restored.count(), 1);
    }
}
