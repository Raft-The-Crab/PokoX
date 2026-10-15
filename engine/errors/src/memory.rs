//! Memory-related errors

use thiserror::Error;

/// Memory error kind
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum MemoryErrorKind {
    /// Allocation failed
    AllocationFailed,
    /// Deallocation failed
    DeallocationFailed,
    /// Out of memory
    OutOfMemory,
    /// Memory limit exceeded
    LimitExceeded,
    /// Invalid memory address
    InvalidAddress,
    /// Memory corruption detected
    Corruption,
    /// Access violation
    AccessViolation,
}

/// Memory-specific error
#[derive(Debug, Error)]
pub enum MemoryError {
    #[error("Memory allocation failed: {0}")]
    AllocationFailed(String),
    #[error("Memory deallocation failed: {0}")]
    DeallocationFailed(String),
    #[error("Out of memory: requested {requested} bytes, available {available} bytes")]
    OutOfMemory { requested: u64, available: u64 },
    #[error("Memory limit exceeded: {current} / {limit} bytes")]
    LimitExceeded { current: u64, limit: u64 },
    #[error("Invalid memory address: {0}")]
    InvalidAddress(String),
    #[error("Memory corruption detected at {address}")]
    Corruption { address: String },
    #[error("Access violation at {address}: {operation}")]
    AccessViolation { address: String, operation: String },
}

impl MemoryError {
    /// Get the error kind
    pub fn kind(&self) -> MemoryErrorKind {
        match self {
            MemoryError::AllocationFailed(_) => MemoryErrorKind::AllocationFailed,
            MemoryError::DeallocationFailed(_) => MemoryErrorKind::DeallocationFailed,
            MemoryError::OutOfMemory { .. } => MemoryErrorKind::OutOfMemory,
            MemoryError::LimitExceeded { .. } => MemoryErrorKind::LimitExceeded,
            MemoryError::InvalidAddress(_) => MemoryErrorKind::InvalidAddress,
            MemoryError::Corruption { .. } => MemoryErrorKind::Corruption,
            MemoryError::AccessViolation { .. } => MemoryErrorKind::AccessViolation,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_memory_error_kind() {
        let error = MemoryError::OutOfMemory {
            requested: 1024,
            available: 512,
        };
        assert_eq!(error.kind(), MemoryErrorKind::OutOfMemory);
    }

    #[test]
    fn test_out_of_memory_error() {
        let error = MemoryError::OutOfMemory {
            requested: 1024,
            available: 512,
        };
        assert!(error.to_string().contains("1024"));
        assert!(error.to_string().contains("512"));
    }

    #[test]
    fn test_limit_exceeded_error() {
        let error = MemoryError::LimitExceeded {
            current: 2048,
            limit: 1024,
        };
        assert!(error.to_string().contains("2048"));
        assert!(error.to_string().contains("1024"));
    }
}
