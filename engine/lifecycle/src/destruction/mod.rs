//! Lifecycle destruction phase management

use chrono::{DateTime, Utc};
use thiserror::Error;
use uuid::Uuid;

/// Unique component identifier
pub type ComponentId = Uuid;

/// Destruction phase
#[derive(Debug, Clone, Copy, PartialEq, Eq, serde::Serialize, serde::Deserialize)]
pub enum DestructionPhase {
    /// Component is active
    Active,
    /// Component is being destroyed
    Destroying,
    /// Component destroyed
    Destroyed,
    /// Destruction failed
    Failed,
}

/// Component destruction state
#[derive(Debug, Clone)]
pub struct DestructionState {
    /// Component ID
    pub component_id: ComponentId,
    /// Component name
    pub component_name: String,
    /// Current phase
    pub phase: DestructionPhase,
    /// Destruction timestamp
    pub destroyed_at: Option<DateTime<Utc>>,
    /// Failure reason (if failed)
    pub failure_reason: Option<String>,
}

impl DestructionState {
    /// Create a new destruction state
    pub fn new(component_id: ComponentId, component_name: String) -> Self {
        Self {
            component_id,
            component_name,
            phase: DestructionPhase::Active,
            destroyed_at: None,
            failure_reason: None,
        }
    }

    /// Mark as destroying
    pub fn mark_destroying(&mut self) {
        self.phase = DestructionPhase::Destroying;
    }

    /// Mark as destroyed
    pub fn mark_destroyed(&mut self) {
        self.phase = DestructionPhase::Destroyed;
        self.destroyed_at = Some(Utc::now());
    }

    /// Mark as failed
    pub fn mark_failed(&mut self, reason: String) {
        self.phase = DestructionPhase::Failed;
        self.failure_reason = Some(reason);
    }

    /// Check if the component is destroyed
    pub fn is_destroyed(&self) -> bool {
        self.phase == DestructionPhase::Destroyed
    }

    /// Check if the component is destroying
    pub fn is_destroying(&self) -> bool {
        self.phase == DestructionPhase::Destroying
    }

    /// Check if the component is active
    pub fn is_active(&self) -> bool {
        self.phase == DestructionPhase::Active
    }

    /// Check if the component failed
    pub fn is_failed(&self) -> bool {
        self.phase == DestructionPhase::Failed
    }
}

/// Errors that can occur during destruction
#[derive(Debug, Clone, Error)]
pub enum DestructionError {
    #[error("Component already destroyed: {0}")]
    AlreadyDestroyed(ComponentId),
    #[error("Component destruction failed: {0}")]
    DestructionFailed(String),
    #[error("Component not found: {0}")]
    ComponentNotFound(ComponentId),
    #[error("Invalid destruction phase transition")]
    InvalidPhaseTransition,
}

/// Lifecycle destroyer for managing component destruction
pub struct LifecycleDestroyer {
    /// Destruction states by component ID
    states: std::collections::HashMap<ComponentId, DestructionState>,
}

impl LifecycleDestroyer {
    /// Create a new lifecycle destroyer
    pub fn new() -> Self {
        Self {
            states: std::collections::HashMap::new(),
        }
    }

    /// Register a component for destruction tracking
    pub fn register(&mut self, component_id: ComponentId, component_name: String) {
        let state = DestructionState::new(component_id, component_name);
        self.states.insert(component_id, state);
    }

    /// Start destroying a component
    pub fn start_destruction(&mut self, component_id: ComponentId) -> Result<(), DestructionError> {
        let state = self
            .states
            .get_mut(&component_id)
            .ok_or(DestructionError::ComponentNotFound(component_id))?;

        if state.phase != DestructionPhase::Active {
            return Err(DestructionError::InvalidPhaseTransition);
        }

        state.mark_destroying();
        Ok(())
    }

    /// Complete destruction of a component
    pub fn complete_destruction(&mut self, component_id: ComponentId) -> Result<(), DestructionError> {
        let state = self
            .states
            .get_mut(&component_id)
            .ok_or(DestructionError::ComponentNotFound(component_id))?;

        if state.phase != DestructionPhase::Destroying {
            return Err(DestructionError::InvalidPhaseTransition);
        }

        state.mark_destroyed();
        Ok(())
    }

    /// Fail destruction of a component
    pub fn fail_destruction(&mut self, component_id: ComponentId, reason: String) -> Result<(), DestructionError> {
        let state = self
            .states
            .get_mut(&component_id)
            .ok_or(DestructionError::ComponentNotFound(component_id))?;

        state.mark_failed(reason);
        Ok(())
    }

    /// Get the destruction state of a component
    pub fn get_state(&self, component_id: ComponentId) -> Option<&DestructionState> {
        self.states.get(&component_id)
    }

    /// Get the destruction phase of a component
    pub fn get_phase(&self, component_id: ComponentId) -> Option<DestructionPhase> {
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
        let mut active = 0;
        let mut destroying = 0;
        let mut destroyed = 0;
        let mut failed = 0;

        for state in self.states.values() {
            match state.phase {
                DestructionPhase::Active => active += 1,
                DestructionPhase::Destroying => destroying += 1,
                DestructionPhase::Destroyed => destroyed += 1,
                DestructionPhase::Failed => failed += 1,
            }
        }

        (active, destroying, destroyed, failed)
    }

    /// Clear all registered components
    pub fn clear(&mut self) {
        self.states.clear();
    }
}

impl Default for LifecycleDestroyer {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_destruction_state() {
        let component_id = Uuid::new_v4();
        let mut state = DestructionState::new(component_id, "TestComponent".to_string());
        assert_eq!(state.phase, DestructionPhase::Active);
        assert!(state.is_active());

        state.mark_destroying();
        assert_eq!(state.phase, DestructionPhase::Destroying);
        assert!(state.is_destroying());

        state.mark_destroyed();
        assert_eq!(state.phase, DestructionPhase::Destroyed);
        assert!(state.is_destroyed());
        assert!(state.destroyed_at.is_some());
    }

    #[test]
    fn test_destruction_state_failed() {
        let component_id = Uuid::new_v4();
        let mut state = DestructionState::new(component_id, "TestComponent".to_string());
        state.mark_failed("Test failure".to_string());
        assert_eq!(state.phase, DestructionPhase::Failed);
        assert!(state.is_failed());
        assert_eq!(state.failure_reason, Some("Test failure".to_string()));
    }

    #[test]
    fn test_lifecycle_destroyer() {
        let mut destroyer = LifecycleDestroyer::new();
        let component_id = Uuid::new_v4();
        destroyer.register(component_id, "TestComponent".to_string());
        assert_eq!(destroyer.count(), 1);
    }

    #[test]
    fn test_start_destruction() {
        let mut destroyer = LifecycleDestroyer::new();
        let component_id = Uuid::new_v4();
        destroyer.register(component_id, "TestComponent".to_string());
        destroyer.start_destruction(component_id).unwrap();
        assert_eq!(destroyer.get_phase(component_id), Some(DestructionPhase::Destroying));
    }

    #[test]
    fn test_complete_destruction() {
        let mut destroyer = LifecycleDestroyer::new();
        let component_id = Uuid::new_v4();
        destroyer.register(component_id, "TestComponent".to_string());
        destroyer.start_destruction(component_id).unwrap();
        destroyer.complete_destruction(component_id).unwrap();
        assert_eq!(destroyer.get_phase(component_id), Some(DestructionPhase::Destroyed));
    }

    #[test]
    fn test_fail_destruction() {
        let mut destroyer = LifecycleDestroyer::new();
        let component_id = Uuid::new_v4();
        destroyer.register(component_id, "TestComponent".to_string());
        destroyer.fail_destruction(component_id, "Test failure".to_string()).unwrap();
        assert_eq!(destroyer.get_phase(component_id), Some(DestructionPhase::Failed));
    }

    #[test]
    fn test_invalid_phase_transition() {
        let mut destroyer = LifecycleDestroyer::new();
        let component_id = Uuid::new_v4();
        destroyer.register(component_id, "TestComponent".to_string());
        destroyer.complete_destruction(component_id).unwrap_err();
    }

    #[test]
    fn test_phase_counts() {
        let mut destroyer = LifecycleDestroyer::new();
        let id1 = Uuid::new_v4();
        let id2 = Uuid::new_v4();
        let id3 = Uuid::new_v4();
        let id4 = Uuid::new_v4();

        destroyer.register(id1, "Component1".to_string());
        destroyer.register(id2, "Component2".to_string());
        destroyer.register(id3, "Component3".to_string());
        destroyer.register(id4, "Component4".to_string());

        destroyer.start_destruction(id2).unwrap();
        destroyer.start_destruction(id3).unwrap();
        destroyer.complete_destruction(id3).unwrap();
        destroyer.start_destruction(id4).unwrap();
        destroyer.fail_destruction(id4, "Test".to_string()).unwrap();

        let (active, destroying, destroyed, failed) = destroyer.phase_counts();
        assert_eq!(active, 1);
        assert_eq!(destroying, 1);
        assert_eq!(destroyed, 1);
        assert_eq!(failed, 1);
    }
}
