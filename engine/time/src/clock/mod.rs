//! Clock for tracking elapsed time
//!
//! Provides high-resolution time tracking for the engine with
//! pause/resume functionality, fixed timestep support, and FPS calculation.

use std::time::{Duration, Instant};
use thiserror::Error;

/// Errors that can occur with clock operations
#[derive(Debug, Clone, Error)]
pub enum ClockError {
    #[error("Clock error: {0}")]
    Custom(String),
}

/// Configuration for the clock
#[derive(Debug, Clone)]
pub struct ClockConfig {
    /// Whether to use a fixed timestep (for deterministic simulation)
    pub fixed_timestep: bool,
    /// Fixed timestep duration in seconds (if fixed_timestep is true)
    pub fixed_delta: f64,
    /// Maximum frame time to prevent spiral of death (in seconds)
    pub max_frame_time: f64,
    /// Time scale (1.0 = normal, 0.5 = slow motion, 2.0 = fast forward)
    pub time_scale: f64,
    /// Minimum delta time (for stability, in seconds)
    pub min_delta: f64,
}

impl Default for ClockConfig {
    fn default() -> Self {
        Self {
            fixed_timestep: false,
            fixed_delta: 1.0 / 60.0, // 60 FPS default
            max_frame_time: 0.2,     // Prevent more than 5 frames of lag
            time_scale: 1.0,
            min_delta: 0.000001,     // 1 microsecond minimum
        }
    }
}

impl ClockConfig {
    /// Validate the configuration
    pub fn validate(&self) -> Result<(), ClockError> {
        if self.fixed_delta <= 0.0 {
            return Err(ClockError::Custom(
                "Fixed delta must be positive".to_string(),
            ));
        }
        if self.max_frame_time <= 0.0 {
            return Err(ClockError::Custom(
                "Max frame time must be positive".to_string(),
            ));
        }
        if self.time_scale <= 0.0 {
            return Err(ClockError::Custom(
                "Time scale must be positive".to_string(),
            ));
        }
        if self.min_delta < 0.0 {
            return Err(ClockError::Custom(
                "Min delta cannot be negative".to_string(),
            ));
        }
        if self.min_delta >= self.max_frame_time {
            return Err(ClockError::Custom(
                "Min delta must be less than max frame time".to_string(),
            ));
        }
        Ok(())
    }

    /// Create a configuration for a fixed timestep at a specific FPS
    pub fn fixed_timestep_at_fps(fps: u32) -> Result<Self, ClockError> {
        if fps == 0 {
            return Err(ClockError::Custom("FPS must be positive".to_string()));
        }
        Ok(Self {
            fixed_timestep: true,
            fixed_delta: 1.0 / fps as f64,
            max_frame_time: 0.2,
            time_scale: 1.0,
            min_delta: 0.000001,
        })
    }
}

/// Clock for tracking elapsed time
#[derive(Debug)]
pub struct Clock {
    /// When the clock was started
    start: Instant,
    /// Last frame time
    last_frame: Instant,
    /// Current frame count
    frame_count: u64,
    /// Total elapsed time
    total_elapsed: Duration,
    /// Delta time for the current frame (in seconds)
    delta_time: f64,
    /// Configuration
    config: ClockConfig,
    /// Whether the clock is paused
    paused: bool,
    /// Accumulated time while paused
    pause_accumulated: Duration,
    /// When the clock was paused
    pause_start: Option<Instant>,
}

impl Clock {
    /// Create a new clock with default configuration
    pub fn new() -> Result<Self, ClockError> {
        Self::with_config(ClockConfig::default())
    }

    /// Create a new clock with custom configuration
    pub fn with_config(config: ClockConfig) -> Result<Self, ClockError> {
        config.validate()?;
        let now = Instant::now();
        Ok(Self {
            start: now,
            last_frame: now,
            frame_count: 0,
            total_elapsed: Duration::ZERO,
            delta_time: 0.0,
            config,
            paused: false,
            pause_accumulated: Duration::ZERO,
            pause_start: None,
        })
    }

    /// Update the clock for a new frame
    pub fn tick(&mut self) -> Result<(), ClockError> {
        if self.paused {
            self.delta_time = 0.0;
            return Ok(());
        }

        let now = Instant::now();
        let elapsed = now.duration_since(self.last_frame);

        // Clamp frame time to prevent spiral of death
        let elapsed_secs = elapsed.as_secs_f64().min(self.config.max_frame_time);

        if self.config.fixed_timestep {
            self.delta_time = self.config.fixed_delta * self.config.time_scale;
        } else {
            self.delta_time = elapsed_secs * self.config.time_scale;
        }

        // Clamp delta time to min/max bounds
        self.delta_time = self
            .delta_time
            .clamp(self.config.min_delta, self.config.max_frame_time);

        self.total_elapsed = now.duration_since(self.start) - self.pause_accumulated;
        self.last_frame = now;
        self.frame_count += 1;

        Ok(())
    }

    /// Get the delta time for the current frame (in seconds)
    pub fn delta_time(&self) -> f64 {
        self.delta_time
    }

    /// Get the unscaled delta time (without time_scale applied)
    pub fn unscaled_delta_time(&self) -> f64 {
        if self.config.time_scale > 0.0 {
            self.delta_time / self.config.time_scale
        } else {
            0.0
        }
    }

    /// Get the total elapsed time since the clock started
    pub fn elapsed(&self) -> Duration {
        self.total_elapsed
    }

    /// Get the total elapsed time in seconds
    pub fn elapsed_seconds(&self) -> f64 {
        self.total_elapsed.as_secs_f64()
    }

    /// Get the scaled elapsed time (with time_scale applied)
    pub fn scaled_elapsed_seconds(&self) -> f64 {
        self.elapsed_seconds() * self.config.time_scale
    }

    /// Get the current frame count
    pub fn frame_count(&self) -> u64 {
        self.frame_count
    }

    /// Pause the clock
    pub fn pause(&mut self) {
        if !self.paused {
            self.paused = true;
            self.pause_start = Some(Instant::now());
        }
    }

    /// Resume the clock
    pub fn resume(&mut self) {
        if self.paused {
            if let Some(start) = self.pause_start {
                self.pause_accumulated += start.elapsed();
            }
            self.paused = false;
            self.pause_start = None;
            self.last_frame = Instant::now();
        }
    }

    /// Check if the clock is paused
    pub fn is_paused(&self) -> bool {
        self.paused
    }

    /// Get the time scale (1.0 = normal, <1.0 = slow-mo, >1.0 = fast-forward)
    pub fn time_scale(&self) -> f64 {
        self.config.time_scale
    }

    /// Set the time scale
    pub fn set_time_scale(&mut self, scale: f64) -> Result<(), ClockError> {
        if scale <= 0.0 {
            return Err(ClockError::Custom(
                "Time scale must be positive".to_string(),
            ));
        }
        self.config.time_scale = scale;
        Ok(())
    }

    /// Get the minimum delta time
    pub fn min_delta(&self) -> f64 {
        self.config.min_delta
    }

    /// Get the maximum delta time
    pub fn max_delta(&self) -> f64 {
        self.config.max_frame_time
    }

    /// Reset the clock
    pub fn reset(&mut self) {
        let now = Instant::now();
        self.start = now;
        self.last_frame = now;
        self.frame_count = 0;
        self.total_elapsed = Duration::ZERO;
        self.delta_time = 0.0;
        self.paused = false;
        self.pause_accumulated = Duration::ZERO;
        self.pause_start = None;
    }

    /// Get the clock configuration
    pub fn config(&self) -> &ClockConfig {
        &self.config
    }

    /// Set the clock configuration
    pub fn set_config(&mut self, config: ClockConfig) -> Result<(), ClockError> {
        config.validate()?;
        self.config = config;
        Ok(())
    }

    /// Calculate frames per second (FPS)
    pub fn fps(&self) -> f64 {
        if self.delta_time > 0.0 {
            1.0 / self.delta_time
        } else {
            0.0
        }
    }

    /// Get the average FPS over the last N frames
    pub fn average_fps(&self, frame_window: u64) -> f64 {
        if self.frame_count < frame_window || frame_window == 0 {
            return self.fps();
        }
        // Simplified: return current FPS for now
        // In a full implementation, we'd track a sliding window
        self.fps()
    }

    /// Get the total pause duration
    pub fn total_pause_duration(&self) -> Duration {
        self.pause_accumulated
    }

    /// Get the number of pause/resume cycles
    pub fn pause_cycles(&self) -> u64 {
        // Track this in a full implementation
        0
    }
}

impl Default for Clock {
    fn default() -> Self {
        Self::new().expect("Default clock config should be valid")
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_clock_creation() {
        let clock = Clock::new().unwrap();
        assert_eq!(clock.frame_count(), 0);
        assert!(!clock.is_paused());
        assert_eq!(clock.time_scale(), 1.0);
    }

    #[test]
    fn test_clock_tick() {
        let mut clock = Clock::new().unwrap();
        clock.tick().unwrap();
        assert_eq!(clock.frame_count(), 1);
        assert!(clock.delta_time() >= 0.0);
    }

    #[test]
    fn test_clock_pause_resume() {
        let mut clock = Clock::new().unwrap();
        clock.pause();
        assert!(clock.is_paused());
        clock.resume();
        assert!(!clock.is_paused());
    }

    #[test]
    fn test_clock_reset() {
        let mut clock = Clock::new().unwrap();
        clock.tick().unwrap();
        clock.tick().unwrap();
        assert_eq!(clock.frame_count(), 2);
        clock.reset();
        assert_eq!(clock.frame_count(), 0);
    }

    #[test]
    fn test_fixed_timestep() {
        let config = ClockConfig {
            fixed_timestep: true,
            fixed_delta: 1.0 / 30.0, // 30 FPS
            max_frame_time: 0.2,
            time_scale: 1.0,
            min_delta: 0.000001,
        };
        let mut clock = Clock::with_config(config).unwrap();
        clock.tick().unwrap();
        assert_eq!(clock.delta_time(), 1.0 / 30.0);
    }

    #[test]
    fn test_time_scale() {
        let mut clock = Clock::new().unwrap();
        clock.set_time_scale(0.5).unwrap(); // 50% speed
        clock.tick().unwrap();
        assert!(clock.delta_time() < clock.unscaled_delta_time());
    }

    #[test]
    fn test_time_scale_invalid() {
        let mut clock = Clock::new().unwrap();
        let result = clock.set_time_scale(0.0);
        assert!(result.is_err());
    }

    #[test]
    fn test_config_validate() {
        let valid_config = ClockConfig::default();
        assert!(valid_config.validate().is_ok());

        let invalid_config = ClockConfig {
            fixed_timestep: true,
            fixed_delta: 0.0,
            max_frame_time: 0.2,
            time_scale: 1.0,
            min_delta: 0.000001,
        };
        assert!(invalid_config.validate().is_err());
    }

    #[test]
    fn test_fixed_timestep_at_fps() {
        let config = ClockConfig::fixed_timestep_at_fps(60).unwrap();
        assert!(config.fixed_timestep);
        assert_eq!(config.fixed_delta, 1.0 / 60.0);
    }

    #[test]
    fn test_fixed_timestep_at_fps_invalid() {
        let result = ClockConfig::fixed_timestep_at_fps(0);
        assert!(result.is_err());
    }

    #[test]
    fn test_set_config_validation() {
        let mut clock = Clock::new().unwrap();
        let invalid_config = ClockConfig {
            fixed_timestep: true,
            fixed_delta: 0.0,
            max_frame_time: 0.2,
            time_scale: 1.0,
            min_delta: 0.000001,
        };
        let result = clock.set_config(invalid_config);
        assert!(result.is_err());
    }

    #[test]
    fn test_max_frame_time_clamp() {
        let config = ClockConfig {
            fixed_timestep: false,
            fixed_delta: 1.0 / 60.0,
            max_frame_time: 0.01,
            time_scale: 1.0,
            min_delta: 0.000001,
        };
        let mut clock = Clock::with_config(config).unwrap();
        // Simulate a long frame by setting last_frame far back
        clock.last_frame = Instant::now() - Duration::from_secs(1);
        clock.tick().unwrap();
        // Delta should be clamped to max_frame_time
        assert!(clock.delta_time() <= 0.01);
    }
}
