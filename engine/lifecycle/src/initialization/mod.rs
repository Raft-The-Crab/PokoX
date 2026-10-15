//! Lifecycle initialization phase management

use chrono::{DateTime, Utc};
use thiserror::Error;
use uuid::Uuid;

/// Unique component identifier
pub type ComponentId = Uuid;

/// Initialization phase
#[derive(Debug, Clone, Copy, PartialEq, Eq, serde::Serialize, serde::Deserialize)]
pub enum InitializationPhase {
    /// Component not yet initialized
    NotInitialized,
    /// Component is being initialized
    Initializing,
    /// Component initialized
    Initialized,
    /// Initialization failed
    Failed,
}

/// Component initialization state
#[derive(Debug, Clone)]
pub struct InitializationState {
    /// Component ID
    pub component_id: ComponentId,
    /// Component name
    pub component_name: String,
    /// Current phase
    pub phase: InitializationPhase,
    /// Initialization timestamp
    pub initialized_at: Option<DateTime<Utc>>,
    /// Failure reason (if failed)
    pub failure_reason: Option<String>,
}

impl InitializationState {
    /// Create a new initialization state
    pub fn new(component_id: ComponentId, component_name: String) -> Self {
        Self {
            component_id,
            component_name,
            phase: InitializationPhase::NotInitialized,
            initialized_at: None,
            failure_reason: None,
        }
    }

    /// Mark as initializing
    pub fn mark_initializing(&mut self) {
        self.phase = InitializationPhase::Initializing;
    }

    /// Mark as initialized
    pub fn mark_initialized(&mut self) {
        self.phase = InitializationPhase::Initialized;
        self.initialized_at = Some(Utc::now());
    }

    /// Mark as failed
    pub fn mark_failed(&mut self, reason: String) {
        self.phase = InitializationPhase::Failed;
        self.failure_reason = Some(reason);
    }

    /// Check if the component is initialized
    pub fn is_initialized(&self) -> bool {
        self.phase == InitializationPhase::Initialized
    }

    /// Check if the component is initializing
    pub fn is_initializing(&self) -> bool {
        self.phase == InitializationPhase::Initializing
    }

    /// Check if the component failed
    pub fn is_failed(&self) -> bool {
        self.phase == InitializationPhase::Failed
    }
}

/// Errors that can occur during initialization
#[derive(Debug, Clone, Error)]
pub enum InitializationError {
    #[error("Component already initialized: {0}")]
    AlreadyInitialized(ComponentId),
    #[error("Component initialization failed: {0}")]
    InitializationFailed(String),
    #[error("Component not found: {0}")]
    ComponentNotFound(ComponentId),
    #[error("Invalid initialization phase transition")]
    InvalidPhaseTransition,
}

/// Lifecycle initializer for managing component initialization
pub struct LifecycleInitializer {
    /// Initialization states by component ID
    states: std::collections::HashMap<ComponentId, InitializationState>,
}

impl LifecycleInitializer {
    /// Create a new lifecycle initializer
    pub fn new() -> Self {
        Self {
            states: std::collections::HashMap::new(),
        }
    }

    /// Register a component for initialization
    pub fn register(&mut self, component_id: ComponentId, component_name: String) {
        let state = InitializationState::new(component_id, component_name);
        self.states.insert(component_id, state);
    }

    /// Start initializing a component
    pub fn start_initialization(&mut self, component_id: ComponentId) -> Result<(), InitializationError> {
        let state = self
            .states
            .get_mut(&component_id)
            .ok_or(InitializationError::ComponentNotFound(component_id))?;

        if state.phase != InitializationPhase::NotInitialized {
            return Err(InitializationError::InvalidPhaseTransition);
        }

        state.mark_initializing();
        Ok(())
    }

    /// Complete initialization of a component
    pub fn complete_initialization(&mut self, component_id: ComponentId) -> Result<(), InitializationError> {
        let state = self
            .states
            .get_mut(&component_id)
            .ok_or(InitializationError::ComponentNotFound(component_id))?;

        if state.phase != InitializationPhase::Initializing {
            return Err(InitializationError::InvalidPhaseTransition);
        }

        state.mark_initialized();
        Ok(())
    }

    /// Fail initialization of a component
    pub fn fail_initialization(&mut self, component_id: ComponentId, reason: String) -> Result<(), InitializationError> {
        let state = self
            .states
            .get_mut(&component_id)
            .ok_or(InitializationError::ComponentNotFound(component_id))?;

        state.mark_failed(reason);
        Ok(())
    }

    /// Get the initialization state of a component
    pub fn get_state(&self, component_id: ComponentId) -> Option<&InitializationState> {
        self.states.get(&component_id)
    }

    /// Get the initialization phase of a component
    pub fn get_phase(&self, component_id: ComponentId) -> Option<InitializationPhase> {
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
        let mut not_initialized = 0;
        let mut initializing = 0;
        let mut initialized = 0;
        let mut failed = 0;

        for state in self.states.values() {
            match state.phase {
                InitializationPhase::NotInitialized => not_initialized += 1,
                InitializationPhase::Initializing => initializing += 1,
                InitializationPhase::Initialized => initialized += 1,
                InitializationPhase::Failed => failed += 1,
            }
        }

        (not_initialized, initializing, initialized, failed)
    }

    /// Clear all registered components
    pub fn clear(&mut self) {
        self.states.clear();
    }
}

impl Default for LifecycleInitializer {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_initialization_state() {
        let component_id = Uuid::new_v4();
        let mut state = InitializationState::new(component_id, "TestComponent".to_string());
        assert_eq!(state.phase, InitializationPhase::NotInitialized);

        state.mark_initializing();
        assert_eq!(state.phase, InitializationPhase::Initializing);
        assert!(state.is_initializing());

        state.mark_initialized();
        assert_eq!(state.phase, InitializationPhase::Initialized);
        assert!(state.is_initialized());
        assert!(state.initialized_at.is_some());
    }

    #[test]
    fn test_initialization_state_failed() {
        let component_id = Uuid::new_v4();
        let mut state = InitializationState::new(component_id, "TestComponent".to_string());
        state.mark_failed("Test failure".to_string());
        assert_eq!(state.phase, InitializationPhase::Failed);
        assert!(state.is_failed());
        assert_eq!(state.failure_reason, Some("Test failure".to_string()));
    }

    #[test]
    fn test_lifecycle_initializer() {
        let mut initializer = LifecycleInitializer::new();
        let component_id = Uuid::new_v4();
        initializer.register(component_id, "TestComponent".to_string());
        assert_eq!(initializer.count(), 1);
    }

    #[test]
    fn test_start_initialization() {
        let mut initializer = LifecycleInitializer::new();
        let component_id = Uuid::new_v4();
        initializer.register(component_id, "TestComponent".to_string());
        initializer.start_initialization(component_id).unwrap();
        assert_eq!(initializer.get_phase(component_id), Some(InitializationPhase::Initializing));
    }

    #[test]
    fn test_complete_initialization() {
        let mut initializer = LifecycleInitializer::new();
        let component_id = Uuid::new_v4();
        initializer.register(component_id, "TestComponent".to_string());
        initializer.start_initialization(component_id).unwrap();
        initializer.complete_initialization(component_id).unwrap();
        assert_eq!(initializer.get_phase(component_id), Some(InitializationPhase::Initialized));
    }

    #[test]
    fn test_fail_initialization() {
        let mut initializer = LifecycleInitializer::new();
        let component_id = Uuid::new_v4();
        initializer.register(component_id, "TestComponent".to_string());
        initializer.fail_initialization(component_id, "Test failure".to_string()).unwrap();
        assert_eq!(initializer.get_phase(component_id), Some(InitializationPhase::Failed));
    }

    #[test]
    fn test_invalid_phase_transition() {
        let mut initializer = LifecycleInitializer::new();
        let component_id = Uuid::new_v4();
        initializer.register(component_id, "TestComponent".to_string());
        initializer.complete_initialization(component_id).unwrap_err();
    }

    #[test]
    fn test_phase_counts() {
        let mut initializer = LifecycleInitializer::new();
        let id1 = Uuid::new_v4();
        let id2 = Uuid::new_v4();
        let id3 = Uuid::new_v4();
        let id4 = Uuid::new_v4();

        initializer.register(id1, "Component1".to_string());
        initializer.register(id2, "Component2".to_string());
        initializer.register(id3, "Component3".to_string());
        initializer.register(id4, "Component4".to_string());

        initializer.start_initialization(id2).unwrap();
        initializer.start_initialization(id3).unwrap();
        initializer.complete_initialization(id3).unwrap();
        initializer.start_initialization(id4).unwrap();
        initializer.fail_initialization(id4, "Test".to_string()).unwrap();

        let (not_initialized, initializing, initialized, failed) = initializer.phase_counts();
        assert_eq!(not_initialized, 1);
        assert_eq!(initializing, 1);
        assert_eq!(initialized, 1);
        assert_eq!(failed, 1);
    }
}
