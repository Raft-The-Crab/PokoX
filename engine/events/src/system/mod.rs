//! System-level event management

use crate::bus::{EventBus, EventMetadata, EventPayload};
use crate::dispatching::{DispatcherConfig, EventDispatcher};
use crate::subscription::{EventSubscription, SubscriptionFilter};
use serde_json;
use std::collections::HashMap;
use std::sync::Arc;
use thiserror::Error;
use uuid::Uuid;

/// System event kinds
#[derive(Debug, Clone, PartialEq, Eq, Hash, serde::Serialize, serde::Deserialize)]
pub enum SystemEventKind {
    /// Engine started
    EngineStarted,
    /// Engine stopped
    EngineStopped,
    /// Engine paused
    EnginePaused,
    /// Engine resumed
    EngineResumed,
    /// Scene loaded
    SceneLoaded,
    /// Scene unloaded
    SceneUnloaded,
    /// Component added
    ComponentAdded,
    /// Component removed
    ComponentRemoved,
    /// Entity created
    EntityCreated,
    /// Entity destroyed
    EntityDestroyed,
    /// Resource loaded
    ResourceLoaded,
    /// Resource unloaded
    ResourceUnloaded,
    /// Error occurred
    ErrorOccurred,
    /// Warning occurred
    WarningOccurred,
}

/// System event
#[derive(Debug, Clone, serde::Serialize, serde::Deserialize)]
pub struct SystemEvent {
    /// Event kind
    pub kind: SystemEventKind,
    /// Event data
    pub data: HashMap<String, String>,
    /// Event source
    pub source: String,
}

impl SystemEvent {
    /// Create a new system event
    pub fn new(kind: SystemEventKind, source: String) -> Self {
        Self {
            kind,
            data: HashMap::new(),
            source,
        }
    }

    /// Add event data
    pub fn add_data(mut self, key: String, value: String) -> Self {
        self.data.insert(key, value);
        self
    }

    /// Convert to event payload
    pub fn to_payload(&self) -> Result<EventPayload, Box<dyn std::error::Error>> {
        let metadata = EventMetadata::new(format!("{:?}", self.kind), self.source.clone());
        let data = serde_json::to_vec(self)?;
        Ok(EventPayload::new(metadata, data))
    }
}

/// Errors that can occur with the event system
#[derive(Debug, Clone, Error)]
pub enum SystemError {
    #[error("Event system is not initialized")]
    NotInitialized,
    #[error("Event system is already initialized")]
    AlreadyInitialized,
    #[error("Event serialization failed: {0}")]
    SerializationError(String),
    #[error("Event deserialization failed: {0}")]
    DeserializationError(String),
    #[error("Invalid configuration: {0}")]
    InvalidConfig(String),
}

/// High-level event system coordinator
pub struct EventSystem {
    /// Event bus
    bus: Arc<EventBus>,
    /// Event dispatcher
    dispatcher: Arc<EventDispatcher>,
    /// Event subscriptions
    subscriptions: Arc<std::sync::Mutex<HashMap<Uuid, EventSubscription>>>,
    /// Whether the system is initialized
    initialized: bool,
}

impl EventSystem {
    /// Create a new event system
    pub fn new() -> Self {
        let bus = Arc::new(EventBus::default());
        let dispatcher = Arc::new(EventDispatcher::new(Arc::clone(&bus)));

        Self {
            bus,
            dispatcher,
            subscriptions: Arc::new(std::sync::Mutex::new(HashMap::new())),
            initialized: true,
        }
    }

    /// Create a new event system with custom configuration
    pub fn with_config(
        bus_config: crate::bus::EventBusConfig,
        dispatcher_config: DispatcherConfig,
    ) -> Result<Self, SystemError> {
        let bus = Arc::new(EventBus::new(bus_config).map_err(SystemError::InvalidConfig)?);
        let dispatcher = Arc::new(EventDispatcher::with_config(
            Arc::clone(&bus),
            dispatcher_config,
        ));

        Ok(Self {
            bus,
            dispatcher,
            subscriptions: Arc::new(std::sync::Mutex::new(HashMap::new())),
            initialized: true,
        })
    }

    /// Check if the system is initialized
    pub fn is_initialized(&self) -> bool {
        self.initialized
    }

    /// Emit a system event
    pub fn emit(&self, event: SystemEvent) -> Result<(), SystemError> {
        if !self.initialized {
            return Err(SystemError::NotInitialized);
        }

        let payload = event
            .to_payload()
            .map_err(|e| SystemError::SerializationError(e.to_string()))?;

        self.dispatcher
            .dispatch(payload)
            .map_err(|e| SystemError::SerializationError(e.to_string()))?;

        Ok(())
    }

    /// Subscribe to events
    pub fn subscribe(&self, filter: SubscriptionFilter) -> Result<Uuid, SystemError> {
        if !self.initialized {
            return Err(SystemError::NotInitialized);
        }

        let subscription = EventSubscription::new(filter);
        let id = subscription.id;

        let mut subscriptions = self.subscriptions.lock().unwrap();
        subscriptions.insert(id, subscription);

        Ok(id)
    }

    /// Unsubscribe from events
    pub fn unsubscribe(&self, id: Uuid) -> Result<(), SystemError> {
        let mut subscriptions = self.subscriptions.lock().unwrap();
        subscriptions
            .remove(&id)
            .ok_or(SystemError::NotInitialized)?;

        Ok(())
    }

    /// Get the event bus
    pub fn bus(&self) -> Arc<EventBus> {
        Arc::clone(&self.bus)
    }

    /// Get the event dispatcher
    pub fn dispatcher(&self) -> Arc<EventDispatcher> {
        Arc::clone(&self.dispatcher)
    }

    /// Get the number of active subscriptions
    pub fn subscription_count(&self) -> usize {
        let subscriptions = self.subscriptions.lock().unwrap();
        subscriptions.len()
    }

    /// Clear all subscriptions
    pub fn clear_subscriptions(&self) {
        let mut subscriptions = self.subscriptions.lock().unwrap();
        subscriptions.clear();
    }
}

impl Default for EventSystem {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_event_system_creation() {
        let system = EventSystem::new();
        assert!(system.is_initialized());
    }

    #[test]
    fn test_system_event_creation() {
        let event = SystemEvent::new(SystemEventKind::EngineStarted, "TestSource".to_string());
        assert_eq!(event.kind, SystemEventKind::EngineStarted);
    }

    #[test]
    fn test_system_event_with_data() {
        let event = SystemEvent::new(SystemEventKind::SceneLoaded, "TestSource".to_string())
            .add_data("scene".to_string(), "test_scene".to_string());
        assert_eq!(event.data.len(), 1);
    }

    #[test]
    fn test_emit_system_event() {
        let system = EventSystem::new();
        let event = SystemEvent::new(SystemEventKind::EngineStarted, "TestSource".to_string());
        assert!(system.emit(event).is_ok());
    }

    #[test]
    fn test_subscribe() {
        let system = EventSystem::new();
        let filter = SubscriptionFilter::None;
        let _id = system.subscribe(filter).unwrap();
        assert_eq!(system.subscription_count(), 1);
    }

    #[test]
    fn test_unsubscribe() {
        let system = EventSystem::new();
        let filter = SubscriptionFilter::None;
        let id = system.subscribe(filter).unwrap();
        system.unsubscribe(id).unwrap();
        assert_eq!(system.subscription_count(), 0);
    }

    #[test]
    fn test_clear_subscriptions() {
        let system = EventSystem::new();
        let filter = SubscriptionFilter::None;
        system.subscribe(filter).unwrap();
        system.clear_subscriptions();
        assert_eq!(system.subscription_count(), 0);
    }

    #[test]
    fn test_system_with_config() {
        let bus_config = crate::bus::EventBusConfig {
            max_events: 100,
            worker_threads: 2,
            max_event_size: 1024,
            enable_batching: false,
            batch_size: 50,
        };
        let dispatcher_config = DispatcherConfig {
            max_handlers_per_type: 5,
            continue_on_error: true,
        };
        let system = EventSystem::with_config(bus_config, dispatcher_config).unwrap();
        assert!(system.is_initialized());
    }

    #[test]
    fn test_system_with_invalid_config() {
        let bus_config = crate::bus::EventBusConfig {
            max_events: 100,
            worker_threads: 0,
            max_event_size: 1024,
            enable_batching: false,
            batch_size: 50,
        };
        let dispatcher_config = DispatcherConfig {
            max_handlers_per_type: 5,
            continue_on_error: true,
        };
        let result = EventSystem::with_config(bus_config, dispatcher_config);
        assert!(result.is_err());
    }
}
