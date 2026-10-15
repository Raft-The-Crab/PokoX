//! Job scheduler for scheduling and executing jobs

use chrono::{DateTime, Utc};
use std::collections::{HashMap, VecDeque};
use std::sync::Arc;
use thiserror::Error;
use tokio::sync::{Mutex, Semaphore};
use uuid::Uuid;

/// Unique job identifier
pub type JobId = Uuid;

/// Job status
#[derive(Debug, Clone, Copy, PartialEq, Eq, serde::Serialize, serde::Deserialize)]
pub enum JobStatus {
    /// Job is pending
    Pending,
    /// Job is running
    Running,
    /// Job completed successfully
    Completed,
    /// Job failed
    Failed,
    /// Job was cancelled
    Cancelled,
}

/// Job result
pub type JobResult<T> = Result<T, Box<dyn std::error::Error + Send + Sync>>;

/// Job function type
pub type JobFunction<T> = Arc<dyn Fn() -> JobResult<T> + Send + Sync>;

/// Job definition
pub struct Job<T> {
    /// Job ID
    pub id: JobId,
    /// Job name
    pub name: String,
    /// Job function
    pub function: JobFunction<T>,
    /// Job priority
    pub priority: crate::priorities::JobPriority,
    /// Job status
    pub status: JobStatus,
    /// Job creation time
    pub created_at: DateTime<Utc>,
    /// Job start time
    pub started_at: Option<DateTime<Utc>>,
    /// Job completion time
    pub completed_at: Option<DateTime<Utc>>,
    /// Job error message (if failed)
    pub error: Option<String>,
}

impl<T> std::fmt::Debug for Job<T> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("Job")
            .field("id", &self.id)
            .field("name", &self.name)
            .field("priority", &self.priority)
            .field("status", &self.status)
            .field("created_at", &self.created_at)
            .field("started_at", &self.started_at)
            .field("completed_at", &self.completed_at)
            .field("error", &self.error)
            .finish()
    }
}

impl<T> Clone for Job<T> {
    fn clone(&self) -> Self {
        Self {
            id: self.id,
            name: self.name.clone(),
            function: Arc::clone(&self.function),
            priority: self.priority,
            status: self.status,
            created_at: self.created_at,
            started_at: self.started_at,
            completed_at: self.completed_at,
            error: self.error.clone(),
        }
    }
}

impl<T> Job<T> {
    /// Create a new job
    pub fn new(name: String, function: JobFunction<T>, priority: crate::priorities::JobPriority) -> Self {
        Self {
            id: Uuid::new_v4(),
            name,
            function,
            priority,
            status: JobStatus::Pending,
            created_at: Utc::now(),
            started_at: None,
            completed_at: None,
            error: None,
        }
    }

    /// Get the job duration
    pub fn duration(&self) -> Option<chrono::Duration> {
        match (self.started_at, self.completed_at) {
            (Some(start), Some(end)) => Some(end.signed_duration_since(start)),
            (Some(start), None) => Some(Utc::now().signed_duration_since(start)),
            _ => None,
        }
    }
}

/// Errors that can occur with the job scheduler
#[derive(Debug, Clone, Error)]
pub enum SchedulerError {
    #[error("Job not found: {0}")]
    JobNotFound(JobId),
    #[error("Job already exists: {0}")]
    JobAlreadyExists(JobId),
    #[error("Scheduler is full")]
    SchedulerFull,
    #[error("Scheduler is stopped")]
    SchedulerStopped,
    #[error("Job execution failed: {0}")]
    ExecutionFailed(String),
}

/// Configuration for the job scheduler
#[derive(Debug, Clone)]
pub struct SchedulerConfig {
    /// Maximum number of concurrent jobs
    pub max_concurrent_jobs: usize,
    /// Maximum number of pending jobs
    pub max_pending_jobs: usize,
    /// Maximum job duration (0 = no limit)
    pub max_job_duration_secs: u64,
}

impl Default for SchedulerConfig {
    fn default() -> Self {
        Self {
            max_concurrent_jobs: 4,
            max_pending_jobs: 1000,
            max_job_duration_secs: 0,
        }
    }
}

/// Job scheduler for scheduling and executing jobs
pub struct JobScheduler {
    /// Configuration
    config: SchedulerConfig,
    /// Pending jobs (priority queue)
    pending_jobs: Arc<Mutex<VecDeque<JobId>>>,
    /// Running jobs
    running_jobs: Arc<Mutex<HashMap<JobId, Job<()>>>>,
    /// Completed jobs
    completed_jobs: Arc<Mutex<HashMap<JobId, Job<()>>>>,
    /// Failed jobs
    failed_jobs: Arc<Mutex<HashMap<JobId, Job<()>>>>,
    /// Semaphore for concurrent job execution
    #[allow(dead_code)]
    semaphore: Arc<Semaphore>,
    /// Whether the scheduler is running
    running: Arc<Mutex<bool>>,
}

impl JobScheduler {
    /// Create a new job scheduler
    pub fn new(config: SchedulerConfig) -> Self {
        let semaphore = Arc::new(Semaphore::new(config.max_concurrent_jobs));

        Self {
            config,
            pending_jobs: Arc::new(Mutex::new(VecDeque::new())),
            running_jobs: Arc::new(Mutex::new(HashMap::new())),
            completed_jobs: Arc::new(Mutex::new(HashMap::new())),
            failed_jobs: Arc::new(Mutex::new(HashMap::new())),
            semaphore,
            running: Arc::new(Mutex::new(true)),
        }
    }

    /// Create a job scheduler with default configuration
    pub fn default() -> Self {
        Self::new(SchedulerConfig::default())
    }

    /// Schedule a job
    pub async fn schedule(&self, job: Job<()>) -> Result<JobId, SchedulerError> {
        let running = self.running.lock().await;
        if !*running {
            return Err(SchedulerError::SchedulerStopped);
        }
        drop(running);

        let mut pending = self.pending_jobs.lock().await;
        if pending.len() >= self.config.max_pending_jobs {
            return Err(SchedulerError::SchedulerFull);
        }

        let job_id = job.id;
        pending.push_back(job_id);

        // Store job in appropriate queue based on status
        let mut completed = self.completed_jobs.lock().await;
        completed.insert(job_id, job);

        Ok(job_id)
    }

    /// Get job status
    pub async fn job_status(&self, job_id: JobId) -> Result<JobStatus, SchedulerError> {
        let completed = self.completed_jobs.lock().await;
        if let Some(job) = completed.get(&job_id) {
            return Ok(job.status);
        }

        let failed = self.failed_jobs.lock().await;
        if let Some(job) = failed.get(&job_id) {
            return Ok(job.status);
        }

        let running = self.running_jobs.lock().await;
        if let Some(job) = running.get(&job_id) {
            return Ok(job.status);
        }

        let pending = self.pending_jobs.lock().await;
        if pending.contains(&job_id) {
            return Ok(JobStatus::Pending);
        }

        Err(SchedulerError::JobNotFound(job_id))
    }

    /// Cancel a job
    pub async fn cancel(&self, job_id: JobId) -> Result<(), SchedulerError> {
        let mut pending = self.pending_jobs.lock().await;
        if let Some(pos) = pending.iter().position(|id| *id == job_id) {
            pending.remove(pos);

            let mut completed = self.completed_jobs.lock().await;
            if let Some(mut job) = completed.remove(&job_id) {
                job.status = JobStatus::Cancelled;
                job.completed_at = Some(Utc::now());
                return Ok(());
            }
        }

        Err(SchedulerError::JobNotFound(job_id))
    }

    /// Get the number of pending jobs
    pub async fn pending_count(&self) -> usize {
        self.pending_jobs.lock().await.len()
    }

    /// Get the number of running jobs
    pub async fn running_count(&self) -> usize {
        self.running_jobs.lock().await.len()
    }

    /// Get the number of completed jobs
    pub async fn completed_count(&self) -> usize {
        self.completed_jobs.lock().await.len()
    }

    /// Get the number of failed jobs
    pub async fn failed_count(&self) -> usize {
        self.failed_jobs.lock().await.len()
    }

    /// Check if the scheduler is running
    pub async fn is_running(&self) -> bool {
        *self.running.lock().await
    }

    /// Stop the scheduler
    pub async fn stop(&self) {
        let mut running = self.running.lock().await;
        *running = false;
    }

    /// Start the scheduler
    pub async fn start(&self) {
        let mut running = self.running.lock().await;
        *running = true;
    }

    /// Clear all jobs
    pub async fn clear(&self) {
        self.pending_jobs.lock().await.clear();
        self.running_jobs.lock().await.clear();
        self.completed_jobs.lock().await.clear();
        self.failed_jobs.lock().await.clear();
    }
}

impl Default for JobScheduler {
    fn default() -> Self {
        Self::new(SchedulerConfig::default())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_scheduler_creation() {
        let scheduler = JobScheduler::default();
        // Note: is_running is async, but this test just checks creation
        assert_eq!(scheduler.config.max_concurrent_jobs, 4);
    }

    #[test]
    fn test_scheduler_config() {
        let config = SchedulerConfig {
            max_concurrent_jobs: 8,
            max_pending_jobs: 2000,
            max_job_duration_secs: 300,
        };
        let scheduler = JobScheduler::new(config);
        assert_eq!(scheduler.config.max_concurrent_jobs, 8);
    }

    #[test]
    fn test_job_creation() {
        let function: JobFunction<()> = Arc::new(|| Ok(()));
        let job = Job::new("TestJob".to_string(), function, crate::priorities::JobPriority::Normal);
        assert_eq!(job.status, JobStatus::Pending);
        assert!(job.duration().is_none());
    }

    #[tokio::test]
    async fn test_schedule_job() {
        let scheduler = JobScheduler::default();
        let function: JobFunction<()> = Arc::new(|| Ok(()));
        let job = Job::new("TestJob".to_string(), function, crate::priorities::JobPriority::Normal);
        let _job_id = scheduler.schedule(job).await.unwrap();
        assert_eq!(scheduler.pending_count().await, 1);
    }

    #[tokio::test]
    async fn test_cancel_job() {
        let scheduler = JobScheduler::default();
        let function: JobFunction<()> = Arc::new(|| Ok(()));
        let job = Job::new("TestJob".to_string(), function, crate::priorities::JobPriority::Normal);
        let job_id = scheduler.schedule(job).await.unwrap();
        scheduler.cancel(job_id).await.unwrap();
        assert_eq!(scheduler.pending_count().await, 0);
    }

    #[tokio::test]
    async fn test_scheduler_stop_start() {
        let scheduler = JobScheduler::default();
        scheduler.stop().await;
        assert!(!scheduler.is_running().await);
        scheduler.start().await;
        assert!(scheduler.is_running().await);
    }

    #[tokio::test]
    async fn test_clear_jobs() {
        let scheduler = JobScheduler::default();
        let function: JobFunction<()> = Arc::new(|| Ok(()));
        let job = Job::new("TestJob".to_string(), function, crate::priorities::JobPriority::Normal);
        scheduler.schedule(job).await.unwrap();
        scheduler.clear().await;
        assert_eq!(scheduler.pending_count().await, 0);
    }
}
