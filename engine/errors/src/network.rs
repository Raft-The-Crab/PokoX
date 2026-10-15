//! Network-related errors

use thiserror::Error;

/// Network error kind
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum NetworkErrorKind {
    /// Connection failed
    ConnectionFailed,
    /// Connection timeout
    ConnectionTimeout,
    /// Disconnected
    Disconnected,
    /// Send failed
    SendFailed,
    /// Receive failed
    ReceiveFailed,
    /// Invalid packet
    InvalidPacket,
    /// Packet too large
    PacketTooLarge,
    /// Version mismatch
    VersionMismatch,
    /// Authentication failed
    AuthenticationFailed,
    /// Rate limited
    RateLimited,
}

/// Network-specific error
#[derive(Debug, Error)]
pub enum NetworkError {
    #[error("Connection failed: {0}")]
    ConnectionFailed(String),
    #[error("Connection timeout: {0}")]
    ConnectionTimeout(String),
    #[error("Disconnected: {0}")]
    Disconnected(String),
    #[error("Send failed: {0}")]
    SendFailed(String),
    #[error("Receive failed: {0}")]
    ReceiveFailed(String),
    #[error("Invalid packet: {0}")]
    InvalidPacket(String),
    #[error("Packet too large: {size} bytes, max {max} bytes")]
    PacketTooLarge { size: usize, max: usize },
    #[error("Version mismatch: expected {expected}, got {actual}")]
    VersionMismatch { expected: u32, actual: u32 },
    #[error("Authentication failed: {0}")]
    AuthenticationFailed(String),
    #[error("Rate limited: retry after {retry_after} seconds")]
    RateLimited { retry_after: u64 },
}

impl NetworkError {
    /// Get the error kind
    pub fn kind(&self) -> NetworkErrorKind {
        match self {
            NetworkError::ConnectionFailed(_) => NetworkErrorKind::ConnectionFailed,
            NetworkError::ConnectionTimeout(_) => NetworkErrorKind::ConnectionTimeout,
            NetworkError::Disconnected(_) => NetworkErrorKind::Disconnected,
            NetworkError::SendFailed(_) => NetworkErrorKind::SendFailed,
            NetworkError::ReceiveFailed(_) => NetworkErrorKind::ReceiveFailed,
            NetworkError::InvalidPacket(_) => NetworkErrorKind::InvalidPacket,
            NetworkError::PacketTooLarge { .. } => NetworkErrorKind::PacketTooLarge,
            NetworkError::VersionMismatch { .. } => NetworkErrorKind::VersionMismatch,
            NetworkError::AuthenticationFailed(_) => NetworkErrorKind::AuthenticationFailed,
            NetworkError::RateLimited { .. } => NetworkErrorKind::RateLimited,
        }
    }

    /// Check if the error is recoverable
    pub fn is_recoverable(&self) -> bool {
        matches!(
            self.kind(),
            NetworkErrorKind::ConnectionTimeout
                | NetworkErrorKind::RateLimited
                | NetworkErrorKind::ConnectionFailed
        )
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_network_error_kind() {
        let error = NetworkError::ConnectionFailed("test".to_string());
        assert_eq!(error.kind(), NetworkErrorKind::ConnectionFailed);
    }

    #[test]
    fn test_recoverable_errors() {
        assert!(NetworkError::ConnectionTimeout("test".to_string()).is_recoverable());
        assert!(NetworkError::RateLimited { retry_after: 5 }.is_recoverable());
        assert!(!NetworkError::Disconnected("test".to_string()).is_recoverable());
    }

    #[test]
    fn test_version_mismatch() {
        let error = NetworkError::VersionMismatch {
            expected: 1,
            actual: 2,
        };
        assert!(error.to_string().contains("expected 1"));
        assert!(error.to_string().contains("got 2"));
    }
}
