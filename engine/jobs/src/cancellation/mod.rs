//! Job cancellation support

use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Arc;
use thiserror::Error;
use uuid::Uuid;

/// Unique job identifier
pub type JobId = Uuid;

/// Cancellation token for job cancellation
#[derive(Debug, Clone)]
pub struct CancellationToken {
    /// Cancellation flag
    cancelled: Arc<AtomicBool>,
    /// Job ID
    job_id: JobId,
}

impl CancellationToken {
    /// Create a new cancellation token
    pub fn new(job_id: JobId) -> Self {
        Self {
            cancelled: Arc::new(AtomicBool::new(false)),
            job_id,
        }
    }

    /// Check if the token is cancelled
    pub fn is_cancelled(&self) -> bool {
        self.cancelled.load(Ordering::SeqCst)
    }

    /// Cancel the token
    pub fn cancel(&self) {
        self.cancelled.store(true, Ordering::SeqCst);
    }

    /// Get the job ID
    pub fn job_id(&self) -> JobId {
        self.job_id
    }
}

/// Job cancellation manager
#[derive(Debug, Clone)]
pub struct JobCancellation {
    /// Cancellation tokens by job ID
    tokens: Arc<std::sync::Mutex<std::collections::HashMap<JobId, CancellationToken>>>,
}

impl JobCancellation {
    /// Create a new job cancellation manager
    pub fn new() -> Self {
        Self {
            tokens: Arc::new(std::sync::Mutex::new(std::collections::HashMap::new())),
        }
    }

    /// Register a job for cancellation
    pub fn register(&self, job_id: JobId) -> CancellationToken {
        let token = CancellationToken::new(job_id);
        let mut tokens = self.tokens.lock().unwrap();
        tokens.insert(job_id, token.clone());
        token
    }

    /// Cancel a job
    pub fn cancel(&self, job_id: JobId) -> Result<(), CancellationError> {
        let tokens = self.tokens.lock().unwrap();
        if let Some(token) = tokens.get(&job_id) {
            token.cancel();
            Ok(())
        } else {
            Err(CancellationError::JobNotFound(job_id))
        }
    }

    /// Check if a job is cancelled
    pub fn is_cancelled(&self, job_id: JobId) -> Result<bool, CancellationError> {
        let tokens = self.tokens.lock().unwrap();
        if let Some(token) = tokens.get(&job_id) {
            Ok(token.is_cancelled())
        } else {
            Err(CancellationError::JobNotFound(job_id))
        }
    }

    /// Unregister a job
    pub fn unregister(&self, job_id: JobId) {
        let mut tokens = self.tokens.lock().unwrap();
        tokens.remove(&job_id);
    }

    /// Cancel all jobs
    pub fn cancel_all(&self) {
        let tokens = self.tokens.lock().unwrap();
        for token in tokens.values() {
            token.cancel();
        }
    }

    /// Get the number of registered jobs
    pub fn count(&self) -> usize {
        self.tokens.lock().unwrap().len()
    }

    /// Clear all registered jobs
    pub fn clear(&self) {
        let mut tokens = self.tokens.lock().unwrap();
        tokens.clear();
    }
}

impl Default for JobCancellation {
    fn default() -> Self {
        Self::new()
    }
}

/// Errors that can occur with job cancellation
#[derive(Debug, Clone, Error)]
pub enum CancellationError {
    #[error("Job not found: {0}")]
    JobNotFound(JobId),
    #[error("Job already cancelled")]
    AlreadyCancelled,
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_cancellation_token() {
        let job_id = Uuid::new_v4();
        let token = CancellationToken::new(job_id);
        assert!(!token.is_cancelled());
        token.cancel();
        assert!(token.is_cancelled());
    }

    #[test]
    fn test_cancellation_manager() {
        let manager = JobCancellation::new();
        let job_id = Uuid::new_v4();
        let token = manager.register(job_id);
        assert_eq!(manager.count(), 1);
        manager.cancel(job_id).unwrap();
        assert!(token.is_cancelled());
    }

    #[test]
    fn test_cancellation_manager_not_found() {
        let manager = JobCancellation::new();
        let job_id = Uuid::new_v4();
        let result = manager.cancel(job_id);
        assert!(result.is_err());
    }

    #[test]
    fn test_cancel_all() {
        let manager = JobCancellation::new();
        let job_id1 = Uuid::new_v4();
        let job_id2 = Uuid::new_v4();
        let token1 = manager.register(job_id1);
        let token2 = manager.register(job_id2);
        manager.cancel_all();
        assert!(token1.is_cancelled());
        assert!(token2.is_cancelled());
    }

    #[test]
    fn test_unregister() {
        let manager = JobCancellation::new();
        let job_id = Uuid::new_v4();
        manager.register(job_id);
        manager.unregister(job_id);
        assert_eq!(manager.count(), 0);
    }

    #[test]
    fn test_clear() {
        let manager = JobCancellation::new();
        manager.register(Uuid::new_v4());
        manager.register(Uuid::new_v4());
        manager.clear();
        assert_eq!(manager.count(), 0);
    }
}
