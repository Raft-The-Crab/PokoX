//! Event bus for pub/sub event handling

use chrono::{DateTime, Utc};
use crossbeam_channel::{bounded, unbounded, Receiver, Sender};
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
    /// Event size in bytes
    pub size_bytes: usize,
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
            size_bytes: 0,
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

    /// Set the event size
    pub fn with_size(mut self, size: usize) -> Self {
        self.size_bytes = size;
        self
    }

    /// Get the age of the event
    pub fn age(&self) -> chrono::Duration {
        Utc::now().signed_duration_since(self.timestamp)
    }

    /// Check if the event is older than a given duration
    pub fn is_older_than(&self, duration: chrono::Duration) -> bool {
        self.age() > duration
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
        let size = data.len();
        Self {
            metadata: metadata.with_size(size),
            data,
        }
    }

    /// Get the total size of the payload
    pub fn total_size(&self) -> usize {
        self.metadata.size_bytes
    }

    /// Check if the payload is empty
    pub fn is_empty(&self) -> bool {
        self.data.is_empty()
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
    /// Maximum event size in bytes (0 = no limit)
    pub max_event_size: usize,
    /// Enable event batching
    pub enable_batching: bool,
    /// Batch size
    pub batch_size: usize,
}

impl Default for EventBusConfig {
    fn default() -> Self {
        Self {
            max_events: 10000,
            worker_threads: 4,
            max_event_size: 10 * 1024 * 1024, // 10 MB
            enable_batching: false,
            batch_size: 100,
        }
    }
}

impl EventBusConfig {
    /// Validate the configuration
    pub fn validate(&self) -> Result<(), String> {
        if self.worker_threads == 0 {
            return Err("worker_threads must be at least 1".to_string());
        }
        if self.max_event_size > 100 * 1024 * 1024 {
            return Err("max_event_size cannot exceed 100 MB".to_string());
        }
        if self.enable_batching && self.batch_size == 0 {
            return Err("batch_size must be at least 1 when batching is enabled".to_string());
        }
        Ok(())
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
    pub fn new(config: EventBusConfig) -> Result<Self, String> {
        config.validate()?;
        let (sender, receiver) = if config.max_events == 0 {
            unbounded()
        } else {
            bounded(config.max_events)
        };

        Ok(Self {
            sender,
            receiver,
            config,
        })
    }

    /// Publish an event
    pub fn publish(&self, payload: EventPayload) -> Result<(), EventBusError> {
        if self.config.max_event_size > 0 && payload.total_size() > self.config.max_event_size {
            return Err(EventBusError::BusFull);
        }
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

    /// Get the number of pending events
    pub fn pending_count(&self) -> usize {
        self.receiver.len()
    }

    /// Check if the bus is empty
    pub fn is_empty(&self) -> bool {
        self.receiver.is_empty()
    }

    /// Get bus statistics
    pub fn stats(&self) -> EventBusStats {
        EventBusStats {
            pending_count: self.pending_count(),
            is_bounded: self.is_bounded(),
            max_events: self.max_events(),
            worker_threads: self.config.worker_threads,
        }
    }
}

/// Event bus statistics
#[derive(Debug, Clone)]
pub struct EventBusStats {
    pub pending_count: usize,
    pub is_bounded: bool,
    pub max_events: usize,
    pub worker_threads: usize,
}

impl Default for EventBus {
    fn default() -> Self {
        Self::new(EventBusConfig::default()).unwrap()
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
            max_event_size: 1024,
            enable_batching: false,
            batch_size: 100,
        };
        let bus = EventBus::new(config).unwrap();
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

    #[test]
    fn test_event_metadata_age() {
        let metadata = EventMetadata::new("TestEvent".to_string(), "TestSource".to_string());
        let age = metadata.age();
        assert!(age.num_seconds() >= 0);
    }

    #[test]
    fn test_event_payload_size() {
        let metadata = EventMetadata::new("TestEvent".to_string(), "TestSource".to_string());
        let payload = EventPayload::new(metadata, vec![1, 2, 3, 4, 5]);
        assert_eq!(payload.total_size(), 5);
        assert!(!payload.is_empty());
    }

    #[test]
    fn test_event_bus_config_validation() {
        let config = EventBusConfig {
            max_events: 100,
            worker_threads: 0,
            max_event_size: 1024,
            enable_batching: false,
            batch_size: 100,
        };
        assert!(config.validate().is_err());

        let config = EventBusConfig {
            max_events: 100,
            worker_threads: 2,
            max_event_size: 200 * 1024 * 1024,
            enable_batching: false,
            batch_size: 100,
        };
        assert!(config.validate().is_err());
    }

    #[test]
    fn test_event_bus_stats() {
        let bus = EventBus::default();
        let stats = bus.stats();
        assert_eq!(stats.pending_count, 0);
        assert!(stats.is_bounded);
    }

    #[test]
    fn test_event_bus_max_size() {
        let config = EventBusConfig {
            max_events: 1000,
            worker_threads: 2,
            max_event_size: 10,
            enable_batching: false,
            batch_size: 100,
        };
        let bus = EventBus::new(config).unwrap();
        let metadata = EventMetadata::new("TestEvent".to_string(), "TestSource".to_string());
        let payload = EventPayload::new(metadata, vec![1u8; 20]); // 20 bytes > 10
        assert!(bus.publish(payload).is_err());
    }
}
