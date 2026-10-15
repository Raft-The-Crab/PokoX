//! Event bus for pub/sub event handling

use chrono::{DateTime, Utc};
use crossbeam_channel::{bounded, unbounded, Sender, Receiver};
use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use thiserror::Error;
use uuid::Uuid;

/// Unique event identifier
pub type EventId = Uuid;

/// Event priority
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Serialize, Deserialize)]
pub enum EventPriority {
    Low = 0,
    Normal = 1,
    High = 2,
    Critical = 3,
}

/// Event metadata
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct EventMetadata {
    /// Unique event ID
    pub id: EventId,
    /// Event type name
    pub event_type: String,
    /// Source of the event
    pub source: String,
    /// Timestamp when the event was created
    pub timestamp: DateTime<Utc>,
    /// Event priority
    pub priority: EventPriority,
    /// Event correlation ID (for tracking related events)
    pub correlation_id: Option<Uuid>,
    /// Additional metadata
    pub properties: HashMap<String, String>,
}

impl EventMetadata {
    /// Create new event metadata
    pub fn new(event_type: String, source: String) -> Self {
        Self {
            id: Uuid::new_v4(),
            event_type,
            source,
            timestamp: Utc::now(),
            priority: EventPriority::Normal,
            correlation_id: None,
            properties: HashMap::new(),
        }
    }

    /// Set the event priority
    pub fn with_priority(mut self, priority: EventPriority) -> Self {
        self.priority = priority;
        self
    }

    /// Set the correlation ID
    pub fn with_correlation_id(mut self, correlation_id: Uuid) -> Self {
        self.correlation_id = Some(correlation_id);
        self
    }

    /// Add a property
    pub fn add_property(mut self, key: String, value: String) -> Self {
        self.properties.insert(key, value);
        self
    }
}

/// Trait for events
pub trait Event: Send + Sync {
    /// Get the event type name
    fn event_type(&self) -> &'static str;

    /// Get the event source
    fn source(&self) -> &'static str;

    /// Create event metadata
    fn metadata(&self) -> EventMetadata {
        EventMetadata::new(self.event_type().to_string(), self.source().to_string())
    }
}

/// Event payload wrapper
#[derive(Debug, Clone)]
pub struct EventPayload {
    /// Event metadata
    pub metadata: EventMetadata,
    /// Serialized event data
    pub data: Vec<u8>,
}

impl EventPayload {
    /// Create a new event payload
    pub fn new(metadata: EventMetadata, data: Vec<u8>) -> Self {
        Self { metadata, data }
    }
}

/// Errors that can occur with the event bus
#[derive(Debug, Clone, Error)]
pub enum EventBusError {
    #[error("Event bus is full")]
    BusFull,
    #[error("Event bus is closed")]
    BusClosed,
    #[error("Channel error: {0}")]
    ChannelError(String),
    #[error("Serialization error: {0}")]
    SerializationError(String),
}

/// Configuration for the event bus
#[derive(Debug, Clone)]
pub struct EventBusConfig {
    /// Maximum number of events in the bus (0 = unbounded)
    pub max_events: usize,
    /// Number of worker threads
    pub worker_threads: usize,
}

impl Default for EventBusConfig {
    fn default() -> Self {
        Self {
            max_events: 10000,
            worker_threads: 4,
        }
    }
}

/// Event bus for pub/sub event handling
pub struct EventBus {
    /// Sender for events
    sender: Sender<EventPayload>,
    /// Receiver for events
    receiver: Receiver<EventPayload>,
    /// Configuration
    config: EventBusConfig,
}

impl EventBus {
    /// Create a new event bus
    pub fn new(config: EventBusConfig) -> Self {
        let (sender, receiver) = if config.max_events == 0 {
            unbounded()
        } else {
            bounded(config.max_events)
        };

        Self {
            sender,
            receiver,
            config,
        }
    }

    /// Create an event bus with default configuration
    pub fn default() -> Self {
        Self::new(EventBusConfig::default())
    }

    /// Publish an event
    pub fn publish(&self, payload: EventPayload) -> Result<(), EventBusError> {
        self.sender
            .send(payload)
            .map_err(|_| EventBusError::BusFull)
    }

    /// Subscribe to events
    pub fn subscribe(&self) -> Receiver<EventPayload> {
        self.receiver.clone()
    }

    /// Get the sender for direct publishing
    pub fn sender(&self) -> Sender<EventPayload> {
        self.sender.clone()
    }

    /// Get the receiver for direct subscription
    pub fn receiver(&self) -> Receiver<EventPayload> {
        self.receiver.clone()
    }

    /// Get the configuration
    pub fn config(&self) -> &EventBusConfig {
        &self.config
    }

    /// Check if the bus is bounded
    pub fn is_bounded(&self) -> bool {
        self.config.max_events > 0
    }

    /// Get the maximum number of events
    pub fn max_events(&self) -> usize {
        self.config.max_events
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_event_bus_creation() {
        let bus = EventBus::default();
        assert!(bus.is_bounded());
        assert_eq!(bus.max_events(), 10000);
    }

    #[test]
    fn test_event_bus_unbounded() {
        let config = EventBusConfig {
            max_events: 0,
            worker_threads: 2,
        };
        let bus = EventBus::new(config);
        assert!(!bus.is_bounded());
    }

    #[test]
    fn test_event_publish() {
        let bus = EventBus::default();
        let metadata = EventMetadata::new("TestEvent".to_string(), "TestSource".to_string());
        let payload = EventPayload::new(metadata, vec![1, 2, 3]);
        assert!(bus.publish(payload).is_ok());
    }

    #[test]
    fn test_event_subscribe() {
        let bus = EventBus::default();
        let receiver = bus.subscribe();
        let metadata = EventMetadata::new("TestEvent".to_string(), "TestSource".to_string());
        let payload = EventPayload::new(metadata, vec![1, 2, 3]);
        bus.publish(payload).unwrap();
        assert!(receiver.recv().is_ok());
    }

    #[test]
    fn test_event_metadata_creation() {
        let metadata = EventMetadata::new("TestEvent".to_string(), "TestSource".to_string());
        assert_eq!(metadata.event_type, "TestEvent");
        assert_eq!(metadata.source, "TestSource");
        assert_eq!(metadata.priority, EventPriority::Normal);
    }

    #[test]
    fn test_event_metadata_with_priority() {
        let metadata = EventMetadata::new("TestEvent".to_string(), "TestSource".to_string())
            .with_priority(EventPriority::Critical);
        assert_eq!(metadata.priority, EventPriority::Critical);
    }

    #[test]
    fn test_event_metadata_with_correlation() {
        let correlation_id = Uuid::new_v4();
        let metadata = EventMetadata::new("TestEvent".to_string(), "TestSource".to_string())
            .with_correlation_id(correlation_id);
        assert_eq!(metadata.correlation_id, Some(correlation_id));
    }

    #[test]
    fn test_event_metadata_with_properties() {
        let metadata = EventMetadata::new("TestEvent".to_string(), "TestSource".to_string())
            .add_property("key".to_string(), "value".to_string());
        assert_eq!(metadata.properties.len(), 1);
    }
}
