//! Job dependencies and dependency resolution

use uuid::Uuid;

/// Unique job identifier
pub type JobId = Uuid;

/// Job dependency
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum JobDependency {
    /// No dependencies
    None,
    /// Depends on a single job
    Single(JobId),
    /// Depends on multiple jobs (all must complete)
    All(Vec<JobId>),
    /// Depends on multiple jobs (any one must complete)
    Any(Vec<JobId>),
}

/// Dependency resolution strategy
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum DependencyResolution {
    /// Fail if dependencies fail
    FailOnDependencyFailure,
    /// Skip if dependencies fail
    SkipOnDependencyFailure,
    /// Continue regardless of dependency status
    ContinueRegardless,
}

/// Dependency check result
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum DependencyCheckResult {
    /// All dependencies satisfied
    Satisfied,
    /// Dependencies not yet met
    NotSatisfied,
    /// Dependencies failed
    Failed,
    /// Dependency not found
    NotFound(JobId),
}

impl JobDependency {
    /// Check if the dependency is satisfied
    pub fn is_satisfied(
        &self,
        completed_jobs: &[JobId],
        failed_jobs: &[JobId],
    ) -> DependencyCheckResult {
        match self {
            JobDependency::None => DependencyCheckResult::Satisfied,
            JobDependency::Single(job_id) => {
                if completed_jobs.contains(job_id) {
                    DependencyCheckResult::Satisfied
                } else if failed_jobs.contains(job_id) {
                    DependencyCheckResult::Failed
                } else {
                    DependencyCheckResult::NotSatisfied
                }
            }
            JobDependency::All(job_ids) => {
                for job_id in job_ids {
                    if !completed_jobs.contains(job_id) {
                        if failed_jobs.contains(job_id) {
                            return DependencyCheckResult::Failed;
                        }
                        return DependencyCheckResult::NotSatisfied;
                    }
                }
                DependencyCheckResult::Satisfied
            }
            JobDependency::Any(job_ids) => {
                for job_id in job_ids {
                    if completed_jobs.contains(job_id) {
                        return DependencyCheckResult::Satisfied;
                    }
                }
                // Check if all failed
                let all_failed = job_ids.iter().all(|id| failed_jobs.contains(id));
                if all_failed {
                    DependencyCheckResult::Failed
                } else {
                    DependencyCheckResult::NotSatisfied
                }
            }
        }
    }

    /// Get the number of dependencies
    pub fn count(&self) -> usize {
        match self {
            JobDependency::None => 0,
            JobDependency::Single(_) => 1,
            JobDependency::All(ids) => ids.len(),
            JobDependency::Any(ids) => ids.len(),
        }
    }

    /// Get the dependency job IDs
    pub fn job_ids(&self) -> Vec<JobId> {
        match self {
            JobDependency::None => Vec::new(),
            JobDependency::Single(id) => vec![*id],
            JobDependency::All(ids) => ids.clone(),
            JobDependency::Any(ids) => ids.clone(),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_dependency_none() {
        let dep = JobDependency::None;
        let result = dep.is_satisfied(&[], &[]);
        assert_eq!(result, DependencyCheckResult::Satisfied);
    }

    #[test]
    fn test_dependency_single_satisfied() {
        let job_id = Uuid::new_v4();
        let dep = JobDependency::Single(job_id);
        let result = dep.is_satisfied(&[job_id], &[]);
        assert_eq!(result, DependencyCheckResult::Satisfied);
    }

    #[test]
    fn test_dependency_single_not_satisfied() {
        let job_id = Uuid::new_v4();
        let dep = JobDependency::Single(job_id);
        let result = dep.is_satisfied(&[], &[]);
        assert_eq!(result, DependencyCheckResult::NotSatisfied);
    }

    #[test]
    fn test_dependency_single_failed() {
        let job_id = Uuid::new_v4();
        let dep = JobDependency::Single(job_id);
        let result = dep.is_satisfied(&[], &[job_id]);
        assert_eq!(result, DependencyCheckResult::Failed);
    }

    #[test]
    fn test_dependency_all_satisfied() {
        let job_id1 = Uuid::new_v4();
        let job_id2 = Uuid::new_v4();
        let dep = JobDependency::All(vec![job_id1, job_id2]);
        let result = dep.is_satisfied(&[job_id1, job_id2], &[]);
        assert_eq!(result, DependencyCheckResult::Satisfied);
    }

    #[test]
    fn test_dependency_all_not_satisfied() {
        let job_id1 = Uuid::new_v4();
        let job_id2 = Uuid::new_v4();
        let dep = JobDependency::All(vec![job_id1, job_id2]);
        let result = dep.is_satisfied(&[job_id1], &[]);
        assert_eq!(result, DependencyCheckResult::NotSatisfied);
    }

    #[test]
    fn test_dependency_any_satisfied() {
        let job_id1 = Uuid::new_v4();
        let job_id2 = Uuid::new_v4();
        let dep = JobDependency::Any(vec![job_id1, job_id2]);
        let result = dep.is_satisfied(&[job_id1], &[]);
        assert_eq!(result, DependencyCheckResult::Satisfied);
    }

    #[test]
    fn test_dependency_any_not_satisfied() {
        let job_id1 = Uuid::new_v4();
        let job_id2 = Uuid::new_v4();
        let dep = JobDependency::Any(vec![job_id1, job_id2]);
        let result = dep.is_satisfied(&[], &[]);
        assert_eq!(result, DependencyCheckResult::NotSatisfied);
    }

    #[test]
    fn test_dependency_count() {
        assert_eq!(JobDependency::None.count(), 0);
        assert_eq!(JobDependency::Single(Uuid::new_v4()).count(), 1);
        assert_eq!(
            JobDependency::All(vec![Uuid::new_v4(), Uuid::new_v4()]).count(),
            2
        );
    }

    #[test]
    fn test_dependency_job_ids() {
        let job_id1 = Uuid::new_v4();
        let job_id2 = Uuid::new_v4();
        let dep = JobDependency::All(vec![job_id1, job_id2]);
        let ids = dep.job_ids();
        assert_eq!(ids.len(), 2);
        assert!(ids.contains(&job_id1));
        assert!(ids.contains(&job_id2));
    }
}
