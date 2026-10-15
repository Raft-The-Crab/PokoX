//! Job priorities and priority policies

/// Job priority levels
#[derive(
    Debug,
    Clone,
    Copy,
    PartialEq,
    Eq,
    PartialOrd,
    Ord,
    serde::Serialize,
    serde::Deserialize,
    Default,
)]
pub enum JobPriority {
    /// Low priority
    Low = 0,
    /// Normal priority
    #[default]
    Normal = 1,
    /// High priority
    High = 2,
    /// Critical priority
    Critical = 3,
}

/// Priority policy for job scheduling
#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub enum PriorityPolicy {
    /// FIFO (First In, First Out)
    Fifo,
    /// LIFO (Last In, First Out)
    Lifo,
    /// Priority-based (higher priority jobs execute first)
    #[default]
    Priority,
    /// Round-robin
    RoundRobin,
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_priority_ordering() {
        assert!(JobPriority::Critical > JobPriority::High);
        assert!(JobPriority::High > JobPriority::Normal);
        assert!(JobPriority::Normal > JobPriority::Low);
    }

    #[test]
    fn test_priority_default() {
        assert_eq!(JobPriority::default(), JobPriority::Normal);
    }

    #[test]
    fn test_policy_default() {
        assert_eq!(PriorityPolicy::default(), PriorityPolicy::Priority);
    }
}
