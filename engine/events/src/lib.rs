//! Event system for Poko Engine
//!
//! Provides a comprehensive event bus for decoupled communication between engine components.

pub mod bus;
pub mod dispatching;
pub mod subscription;
pub mod system;

pub use bus::{
    EventBus, EventBusConfig, EventBusError, EventId, EventMetadata, EventPayload, EventPriority,
};
pub use dispatching::{DispatcherConfig, DispatcherError, EventDispatcher};
pub use subscription::{EventSubscription, SubscriptionError, SubscriptionFilter};
pub use system::{EventSystem, SystemError, SystemEvent, SystemEventKind};
