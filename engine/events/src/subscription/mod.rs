//! Event subscription management

use crate::bus::{EventPayload, EventPriority};
use thiserror::Error;

/// Errors that can occur with event subscriptions
#[derive(Debug, Clone, Error)]
pub enum SubscriptionError {
    #[error("Invalid subscription filter: {0}")]
    InvalidFilter(String),
    #[error("Subscription already exists: {0}")]
    AlreadyExists(String),
    #[error("Subscription not found: {0}")]
    NotFound(String),
}

/// Filter for event subscriptions
pub enum SubscriptionFilter {
    /// No filtering - receive all events
    None,
    /// Filter by event type
    ByEventType(String),
    /// Filter by event source
    BySource(String),
    /// Filter by event priority (only receive events >= this priority)
    ByMinPriority(EventPriority),
    /// Filter by correlation ID
    ByCorrelationId(uuid::Uuid),
    /// Custom filter function
    Custom(Box<dyn Fn(&EventPayload) -> bool + Send + Sync>),
}

impl std::fmt::Debug for SubscriptionFilter {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            SubscriptionFilter::None => write!(f, "None"),
            SubscriptionFilter::ByEventType(s) => write!(f, "ByEventType({})", s),
            SubscriptionFilter::BySource(s) => write!(f, "BySource({})", s),
            SubscriptionFilter::ByMinPriority(p) => write!(f, "ByMinPriority({:?})", p),
            SubscriptionFilter::ByCorrelationId(id) => write!(f, "ByCorrelationId({})", id),
            SubscriptionFilter::Custom(_) => write!(f, "Custom(<function>)"),
        }
    }
}

impl Clone for SubscriptionFilter {
    fn clone(&self) -> Self {
        match self {
            SubscriptionFilter::None => SubscriptionFilter::None,
            SubscriptionFilter::ByEventType(s) => SubscriptionFilter::ByEventType(s.clone()),
            SubscriptionFilter::BySource(s) => SubscriptionFilter::BySource(s.clone()),
            SubscriptionFilter::ByMinPriority(p) => SubscriptionFilter::ByMinPriority(*p),
            SubscriptionFilter::ByCorrelationId(id) => SubscriptionFilter::ByCorrelationId(*id),
            SubscriptionFilter::Custom(_) => {
                // Cannot clone function pointers, return None filter instead
                SubscriptionFilter::None
            }
        }
    }
}

impl SubscriptionFilter {
    /// Check if an event passes the filter
    pub fn matches(&self, payload: &EventPayload) -> bool {
        match self {
            SubscriptionFilter::None => true,
            SubscriptionFilter::ByEventType(event_type) => {
                &payload.metadata.event_type == event_type
            }
            SubscriptionFilter::BySource(source) => &payload.metadata.source == source,
            SubscriptionFilter::ByMinPriority(min_priority) => {
                payload.metadata.priority >= *min_priority
            }
            SubscriptionFilter::ByCorrelationId(correlation_id) => {
                payload.metadata.correlation_id.as_ref() == Some(correlation_id)
            }
            SubscriptionFilter::Custom(filter) => filter(payload),
        }
    }
}

/// Event subscription
#[derive(Debug)]
pub struct EventSubscription {
    /// Subscription ID
    pub id: uuid::Uuid,
    /// Subscription filter
    pub filter: SubscriptionFilter,
    /// Whether the subscription is active
    pub active: bool,
    /// Subscription creation time
    pub created_at: chrono::DateTime<chrono::Utc>,
}

impl Clone for EventSubscription {
    fn clone(&self) -> Self {
        Self {
            id: self.id,
            filter: self.filter.clone(),
            active: self.active,
            created_at: self.created_at,
        }
    }
}

impl EventSubscription {
    /// Create a new event subscription
    pub fn new(filter: SubscriptionFilter) -> Self {
        Self {
            id: uuid::Uuid::new_v4(),
            filter,
            active: true,
            created_at: chrono::Utc::now(),
        }
    }

    /// Activate the subscription
    pub fn activate(&mut self) {
        self.active = true;
    }

    /// Deactivate the subscription
    pub fn deactivate(&mut self) {
        self.active = false;
    }

    /// Check if the subscription is active
    pub fn is_active(&self) -> bool {
        self.active
    }

    /// Check if an event passes the subscription filter
    pub fn matches(&self, payload: &EventPayload) -> bool {
        if !self.active {
            return false;
        }
        self.filter.matches(payload)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_subscription_creation() {
        let filter = SubscriptionFilter::ByEventType("TestEvent".to_string());
        let subscription = EventSubscription::new(filter);
        assert!(subscription.is_active());
    }

    #[test]
    fn test_subscription_activate_deactivate() {
        let filter = SubscriptionFilter::None;
        let mut subscription = EventSubscription::new(filter);
        subscription.deactivate();
        assert!(!subscription.is_active());
        subscription.activate();
        assert!(subscription.is_active());
    }

    #[test]
    fn test_filter_by_event_type() {
        let filter = SubscriptionFilter::ByEventType("TestEvent".to_string());
        let subscription = EventSubscription::new(filter);

        let mut metadata = crate::bus::EventMetadata::new("TestEvent".to_string(), "TestSource".to_string());
        let payload = EventPayload::new(metadata, vec![]);
        assert!(subscription.matches(&payload));

        metadata = crate::bus::EventMetadata::new("OtherEvent".to_string(), "TestSource".to_string());
        let payload = EventPayload::new(metadata, vec![]);
        assert!(!subscription.matches(&payload));
    }

    #[test]
    fn test_filter_by_priority() {
        let filter = SubscriptionFilter::ByMinPriority(EventPriority::High);
        let subscription = EventSubscription::new(filter);

        let mut metadata = crate::bus::EventMetadata::new("TestEvent".to_string(), "TestSource".to_string())
            .with_priority(EventPriority::Critical);
        let payload = EventPayload::new(metadata, vec![]);
        assert!(subscription.matches(&payload));

        metadata = crate::bus::EventMetadata::new("TestEvent".to_string(), "TestSource".to_string())
            .with_priority(EventPriority::Low);
        let payload = EventPayload::new(metadata, vec![]);
        assert!(!subscription.matches(&payload));
    }

    #[test]
    fn test_deactivated_subscription() {
        let filter = SubscriptionFilter::None;
        let mut subscription = EventSubscription::new(filter);
        subscription.deactivate();

        let metadata = crate::bus::EventMetadata::new("TestEvent".to_string(), "TestSource".to_string());
        let payload = EventPayload::new(metadata, vec![]);
        assert!(!subscription.matches(&payload));
    }
}
