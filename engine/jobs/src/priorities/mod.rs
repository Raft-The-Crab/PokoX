//! Job priorities and priority policies

/// Job priority levels
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, serde::Serialize, serde::Deserialize)]
pub enum JobPriority {
    /// Low priority
    Low = 0,
    /// Normal priority
    Normal = 1,
    /// High priority
    High = 2,
    /// Critical priority
    Critical = 3,
}

impl Default for JobPriority {
    fn default() -> Self {
        JobPriority::Normal
    }
}

/// Priority policy for job scheduling
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum PriorityPolicy {
    /// FIFO (First In, First Out)
    Fifo,
    /// LIFO (Last In, First Out)
    Lifo,
    /// Priority-based (higher priority jobs execute first)
    Priority,
    /// Round-robin
    RoundRobin,
}

impl Default for PriorityPolicy {
    fn default() -> Self {
        PriorityPolicy::Priority
    }
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
