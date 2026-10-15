//! Event system for Poko Engine
//!
//! Provides a comprehensive event bus for decoupled communication between engine components.

pub mod bus;
pub mod dispatching;
pub mod subscription;
pub mod system;

pub use bus::{EventBus, EventBusConfig, EventBusError, EventId, EventMetadata, EventPayload, EventPriority};
pub use dispatching::{EventDispatcher, DispatcherConfig, DispatcherError};
pub use subscription::{EventSubscription, SubscriptionFilter, SubscriptionError};
pub use system::{EventSystem, SystemError, SystemEvent, SystemEventKind};
