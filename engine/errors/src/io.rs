//! I/O-related errors

use thiserror::Error;

/// I/O error kind
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum IoErrorKind {
    /// File not found
    NotFound,
    /// Permission denied
    PermissionDenied,
    /// Read error
    ReadError,
    /// Write error
    WriteError,
    /// Directory not found
    DirectoryNotFound,
    /// Already exists
    AlreadyExists,
    /// Invalid path
    InvalidPath,
    /// End of file
    EndOfFile,
    /// Device error
    DeviceError,
}

/// I/O-specific error
#[derive(Debug, Error)]
pub enum IoError {
    #[error("File not found: {0}")]
    NotFound(String),
    #[error("Permission denied: {0}")]
    PermissionDenied(String),
    #[error("Read error: {0}")]
    ReadError(String),
    #[error("Write error: {0}")]
    WriteError(String),
    #[error("Directory not found: {0}")]
    DirectoryNotFound(String),
    #[error("Already exists: {0}")]
    AlreadyExists(String),
    #[error("Invalid path: {0}")]
    InvalidPath(String),
    #[error("End of file: {0}")]
    EndOfFile(String),
    #[error("Device error: {0}")]
    DeviceError(String),
}

impl IoError {
    /// Get the error kind
    pub fn kind(&self) -> IoErrorKind {
        match self {
            IoError::NotFound(_) => IoErrorKind::NotFound,
            IoError::PermissionDenied(_) => IoErrorKind::PermissionDenied,
            IoError::ReadError(_) => IoErrorKind::ReadError,
            IoError::WriteError(_) => IoErrorKind::WriteError,
            IoError::DirectoryNotFound(_) => IoErrorKind::DirectoryNotFound,
            IoError::AlreadyExists(_) => IoErrorKind::AlreadyExists,
            IoError::InvalidPath(_) => IoErrorKind::InvalidPath,
            IoError::EndOfFile(_) => IoErrorKind::EndOfFile,
            IoError::DeviceError(_) => IoErrorKind::DeviceError,
        }
    }

    /// Convert from std::io::Error
    pub fn from_std(error: std::io::Error, path: String) -> Self {
        match error.kind() {
            std::io::ErrorKind::NotFound => IoError::NotFound(path),
            std::io::ErrorKind::PermissionDenied => IoError::PermissionDenied(path),
            std::io::ErrorKind::InvalidInput => IoError::InvalidPath(path),
            std::io::ErrorKind::AlreadyExists => IoError::AlreadyExists(path),
            _ => IoError::ReadError(format!("{}: {}", path, error)),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_io_error_kind() {
        let error = IoError::NotFound("test.txt".to_string());
        assert_eq!(error.kind(), IoErrorKind::NotFound);
    }

    #[test]
    fn test_from_std_error() {
        let std_err = std::io::Error::new(std::io::ErrorKind::NotFound, "test");
        let error = IoError::from_std(std_err, "test.txt".to_string());
        assert!(matches!(error, IoError::NotFound(_)));
    }
}
