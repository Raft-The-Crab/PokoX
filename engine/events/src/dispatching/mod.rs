//! Event dispatcher for routing events to handlers

use crate::bus::{EventBus, EventPayload};
use std::collections::HashMap;
use std::sync::Arc;
use thiserror::Error;

/// Errors that can occur with event dispatching
#[derive(Debug, Clone, Error)]
pub enum DispatcherError {
    #[error("No handler registered for event type: {0}")]
    NoHandler(String),
    #[error("Handler failed: {0}")]
    HandlerFailed(String),
    #[error("Event bus error: {0}")]
    BusError(String),
}

/// Event handler function type
pub type EventHandler = Arc<dyn Fn(&EventPayload) -> Result<(), Box<dyn std::error::Error + Send + Sync>> + Send + Sync>;

/// Configuration for the event dispatcher
#[derive(Debug, Clone)]
pub struct DispatcherConfig {
    /// Maximum number of handlers per event type
    pub max_handlers_per_type: usize,
    /// Whether to continue dispatching on handler errors
    pub continue_on_error: bool,
}

impl Default for DispatcherConfig {
    fn default() -> Self {
        Self {
            max_handlers_per_type: 10,
            continue_on_error: true,
        }
    }
}

/// Event dispatcher for routing events to handlers
pub struct EventDispatcher {
    /// Event bus
    bus: Arc<EventBus>,
    /// Event handlers by event type
    handlers: Arc<std::sync::Mutex<HashMap<String, Vec<EventHandler>>>>,
    /// Configuration
    config: DispatcherConfig,
}

impl EventDispatcher {
    /// Create a new event dispatcher
    pub fn new(bus: Arc<EventBus>) -> Self {
        Self {
            bus,
            handlers: Arc::new(std::sync::Mutex::new(HashMap::new())),
            config: DispatcherConfig::default(),
        }
    }

    /// Create a new event dispatcher with custom configuration
    pub fn with_config(bus: Arc<EventBus>, config: DispatcherConfig) -> Self {
        Self {
            bus,
            handlers: Arc::new(std::sync::Mutex::new(HashMap::new())),
            config,
        }
    }

    /// Register a handler for an event type
    pub fn register_handler(
        &self,
        event_type: String,
        handler: EventHandler,
    ) -> Result<(), DispatcherError> {
        let mut handlers = self.handlers.lock().unwrap();
        let handlers_vec = handlers.entry(event_type).or_insert_with(Vec::new);

        if handlers_vec.len() >= self.config.max_handlers_per_type {
            return Err(DispatcherError::NoHandler(
                "Maximum handlers reached".to_string(),
            ));
        }

        handlers_vec.push(handler);
        Ok(())
    }

    /// Unregister all handlers for an event type
    pub fn unregister_handlers(&self, event_type: &str) {
        let mut handlers = self.handlers.lock().unwrap();
        handlers.remove(event_type);
    }

    /// Dispatch an event to all registered handlers
    pub fn dispatch(&self, payload: EventPayload) -> Result<(), DispatcherError> {
        let event_type = payload.metadata.event_type.clone();
        let handlers = {
            let handlers_map = self.handlers.lock().unwrap();
            handlers_map.get(&event_type).cloned()
        };

        // Publish to event bus
        self.bus
            .publish(payload.clone())
            .map_err(|e| DispatcherError::BusError(e.to_string()))?;

        if let Some(handlers_vec) = handlers {
            for handler in handlers_vec {
                if let Err(e) = handler(&payload) {
                    if self.config.continue_on_error {
                        eprintln!("Handler error: {}", e);
                    } else {
                        return Err(DispatcherError::HandlerFailed(e.to_string()));
                    }
                }
            }
        }

        Ok(())
    }

    /// Get the event bus
    pub fn bus(&self) -> Arc<EventBus> {
        Arc::clone(&self.bus)
    }

    /// Get the number of handlers for an event type
    pub fn handler_count(&self, event_type: &str) -> usize {
        let handlers = self.handlers.lock().unwrap();
        handlers.get(event_type).map(|h| h.len()).unwrap_or(0)
    }

    /// Get the total number of registered handlers
    pub fn total_handlers(&self) -> usize {
        let handlers = self.handlers.lock().unwrap();
        handlers.values().map(|h| h.len()).sum()
    }

    /// Clear all handlers
    pub fn clear_handlers(&self) {
        let mut handlers = self.handlers.lock().unwrap();
        handlers.clear();
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::sync::Arc;

    #[test]
    fn test_dispatcher_creation() {
        let bus = Arc::new(EventBus::default());
        let dispatcher = EventDispatcher::new(bus);
        assert_eq!(dispatcher.total_handlers(), 0);
    }

    #[test]
    fn test_register_handler() {
        let bus = Arc::new(EventBus::default());
        let dispatcher = EventDispatcher::new(bus);
        let handler: EventHandler = Arc::new(|_payload| Ok(()));
        assert!(dispatcher
            .register_handler("TestEvent".to_string(), handler)
            .is_ok());
        assert_eq!(dispatcher.handler_count("TestEvent"), 1);
    }

    #[test]
    fn test_unregister_handlers() {
        let bus = Arc::new(EventBus::default());
        let dispatcher = EventDispatcher::new(bus);
        let handler: EventHandler = Arc::new(|_payload| Ok(()));
        dispatcher
            .register_handler("TestEvent".to_string(), handler)
            .unwrap();
        dispatcher.unregister_handlers("TestEvent");
        assert_eq!(dispatcher.handler_count("TestEvent"), 0);
    }

    #[test]
    fn test_dispatch() {
        let bus = Arc::new(EventBus::default());
        let dispatcher = EventDispatcher::new(bus);
        let handler: EventHandler = Arc::new(|_payload| Ok(()));
        dispatcher
            .register_handler("TestEvent".to_string(), handler)
            .unwrap();

        let metadata = crate::bus::EventMetadata::new("TestEvent".to_string(), "TestSource".to_string());
        let payload = EventPayload::new(metadata, vec![]);
        assert!(dispatcher.dispatch(payload).is_ok());
    }

    #[test]
    fn test_max_handlers() {
        let config = DispatcherConfig {
            max_handlers_per_type: 2,
            continue_on_error: true,
        };
        let bus = Arc::new(EventBus::default());
        let dispatcher = EventDispatcher::with_config(bus, config);

        let handler: EventHandler = Arc::new(|_payload| Ok(()));
        dispatcher
            .register_handler("TestEvent".to_string(), handler.clone())
            .unwrap();
        dispatcher
            .register_handler("TestEvent".to_string(), handler.clone())
            .unwrap();

        let result = dispatcher.register_handler("TestEvent".to_string(), handler);
        assert!(result.is_err());
    }

    #[test]
    fn test_clear_handlers() {
        let bus = Arc::new(EventBus::default());
        let dispatcher = EventDispatcher::new(bus);
        let handler: EventHandler = Arc::new(|_payload| Ok(()));
        dispatcher
            .register_handler("TestEvent".to_string(), handler)
            .unwrap();
        dispatcher.clear_handlers();
        assert_eq!(dispatcher.total_handlers(), 0);
    }
}
