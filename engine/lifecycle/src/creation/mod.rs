//! Lifecycle creation phase management

use chrono::{DateTime, Utc};
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
#[derive(Debug, Clone)]
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
        }
    }

    /// Mark as creating
    pub fn mark_creating(&mut self) {
        self.phase = CreationPhase::Creating;
    }

    /// Mark as created
    pub fn mark_created(&mut self) {
        self.phase = CreationPhase::Created;
        self.created_at = Some(Utc::now());
    }

    /// Mark as failed
    pub fn mark_failed(&mut self, reason: String) {
        self.phase = CreationPhase::Failed;
        self.failure_reason = Some(reason);
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
pub struct LifecycleCreator {
    /// Creation states by component ID
    states: std::collections::HashMap<ComponentId, CreationState>,
}

impl LifecycleCreator {
    /// Create a new lifecycle creator
    pub fn new() -> Self {
        Self {
            states: std::collections::HashMap::new(),
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
    pub fn fail_creation(&mut self, component_id: ComponentId, reason: String) -> Result<(), CreationError> {
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

    /// Clear all registered components
    pub fn clear(&mut self) {
        self.states.clear();
    }
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
        assert_eq!(creator.get_phase(component_id), Some(CreationPhase::Creating));
    }

    #[test]
    fn test_complete_creation() {
        let mut creator = LifecycleCreator::new();
        let component_id = Uuid::new_v4();
        creator.register(component_id, "TestComponent".to_string());
        creator.start_creation(component_id).unwrap();
        creator.complete_creation(component_id).unwrap();
        assert_eq!(creator.get_phase(component_id), Some(CreationPhase::Created));
    }

    #[test]
    fn test_fail_creation() {
        let mut creator = LifecycleCreator::new();
        let component_id = Uuid::new_v4();
        creator.register(component_id, "TestComponent".to_string());
        creator.fail_creation(component_id, "Test failure".to_string()).unwrap();
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
}
