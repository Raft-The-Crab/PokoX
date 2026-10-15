//! Fixed-step accumulator for deterministic game loops
//!
//! Implements a fixed timestep accumulator pattern for consistent
//! simulation regardless of frame rate variations.

use thiserror::Error;

/// Errors that can occur with accumulator operations
#[derive(Debug, Clone, Error)]
pub enum AccumulatorError {
    #[error("Accumulator error: {0}")]
    Custom(String),
}

/// Configuration for the fixed-step accumulator
#[derive(Debug, Clone)]
pub struct FixedStepConfig {
    /// Fixed timestep duration in seconds
    pub step_duration: f64,
    /// Maximum accumulator value to prevent spiral of death
    pub max_accumulator: f64,
    /// Minimum accumulator value (prevents negative values)
    pub min_accumulator: f64,
}

impl Default for FixedStepConfig {
    fn default() -> Self {
        Self {
            step_duration: 1.0 / 60.0, // 60 Hz
            max_accumulator: 0.2,      // Prevent more than 5 frames of lag
            min_accumulator: 0.0,
        }
    }
}

/// Fixed-step accumulator for deterministic game loops
#[derive(Debug, Clone)]
pub struct FixedStepAccumulator {
    /// Accumulated time (in seconds)
    accumulator: f64,
    /// Configuration
    config: FixedStepConfig,
    /// Number of steps performed
    steps_performed: u64,
    /// Time remaining after steps (for interpolation)
    alpha: f64,
    /// Total time added to accumulator
    total_added: f64,
    /// Total time consumed by steps
    total_consumed: f64,
}

impl FixedStepAccumulator {
    /// Create a new accumulator with default configuration
    pub fn new() -> Self {
        Self::with_config(FixedStepConfig::default())
    }

    /// Create a new accumulator with custom configuration
    pub fn with_config(config: FixedStepConfig) -> Self {
        Self {
            accumulator: 0.0,
            config,
            steps_performed: 0,
            alpha: 0.0,
            total_added: 0.0,
            total_consumed: 0.0,
        }
    }

    /// Add delta time to the accumulator
    pub fn add_delta(&mut self, delta: f64) -> Result<(), AccumulatorError> {
        if delta < 0.0 {
            return Err(AccumulatorError::Custom(
                "Delta time cannot be negative".to_string(),
            ));
        }

        self.accumulator += delta;
        self.total_added += delta;

        // Clamp to prevent spiral of death
        if self.accumulator > self.config.max_accumulator {
            self.accumulator = self.config.max_accumulator;
        }

        // Clamp to minimum
        if self.accumulator < self.config.min_accumulator {
            self.accumulator = self.config.min_accumulator;
        }

        Ok(())
    }

    /// Check if a step should be performed
    pub fn has_step(&self) -> bool {
        self.accumulator >= self.config.step_duration
    }

    /// Perform a step and consume the fixed timestep
    pub fn step(&mut self) -> Result<f64, AccumulatorError> {
        if !self.has_step() {
            return Ok(0.0);
        }

        let step = self.config.step_duration;
        self.accumulator -= step;
        self.steps_performed += 1;
        self.total_consumed += step;
        Ok(step)
    }

    /// Get the number of steps performed
    pub fn steps_performed(&self) -> u64 {
        self.steps_performed
    }

    /// Reset the step counter
    pub fn reset_step_counter(&mut self) {
        self.steps_performed = 0;
    }

    /// Get the alpha value for interpolation (0.0 to 1.0)
    ///
    /// This represents how far we are between the last physics step
    /// and the next, useful for smooth rendering.
    pub fn alpha(&self) -> f64 {
        if self.config.step_duration > 0.0 {
            (self.accumulator / self.config.step_duration).clamp(0.0, 1.0)
        } else {
            0.0
        }
    }

    /// Get the current accumulator value
    pub fn accumulator(&self) -> f64 {
        self.accumulator
    }

    /// Reset the accumulator
    pub fn reset(&mut self) {
        self.accumulator = 0.0;
        self.steps_performed = 0;
        self.alpha = 0.0;
        self.total_added = 0.0;
        self.total_consumed = 0.0;
    }

    /// Get the configuration
    pub fn config(&self) -> &FixedStepConfig {
        &self.config
    }

    /// Set the configuration
    pub fn set_config(&mut self, config: FixedStepConfig) {
        self.config = config;
    }

    /// Estimate how many steps can be performed with current accumulator
    pub fn estimate_steps(&self) -> u64 {
        if self.config.step_duration > 0.0 {
            (self.accumulator / self.config.step_duration) as u64
        } else {
            0
        }
    }

    /// Get the total time added to the accumulator
    pub fn total_added(&self) -> f64 {
        self.total_added
    }

    /// Get the total time consumed by steps
    pub fn total_consumed(&self) -> f64 {
        self.total_consumed
    }

    /// Get the lost time (added but not consumed due to clamping)
    pub fn lost_time(&self) -> f64 {
        self.total_added - self.total_consumed - self.accumulator
    }

    /// Get the efficiency ratio (consumed / added)
    pub fn efficiency(&self) -> f64 {
        if self.total_added > 0.0 {
            (self.total_consumed / self.total_added) * 100.0
        } else {
            100.0
        }
    }
}

impl Default for FixedStepAccumulator {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_accumulator_creation() {
        let acc = FixedStepAccumulator::new();
        assert_eq!(acc.accumulator(), 0.0);
        assert_eq!(acc.steps_performed(), 0);
    }

    #[test]
    fn test_add_delta() {
        let mut acc = FixedStepAccumulator::new();
        acc.add_delta(0.02).unwrap();
        assert_eq!(acc.accumulator(), 0.02);
    }

    #[test]
    fn test_add_delta_negative() {
        let mut acc = FixedStepAccumulator::new();
        assert!(acc.add_delta(-0.01).is_err());
    }

    #[test]
    fn test_has_step() {
        let mut acc = FixedStepAccumulator::new();
        assert!(!acc.has_step());
        acc.add_delta(0.02).unwrap(); // More than 1/60 (0.0167)
        assert!(acc.has_step());
    }

    #[test]
    fn test_step() {
        let mut acc = FixedStepAccumulator::new();
        acc.add_delta(0.02).unwrap();
        let step = acc.step().unwrap();
        assert!(step > 0.0);
        assert_eq!(acc.steps_performed(), 1);
    }

    #[test]
    fn test_max_accumulator_clamp() {
        let config = FixedStepConfig {
            step_duration: 1.0 / 60.0,
            max_accumulator: 0.1,
            min_accumulator: 0.0,
        };
        let mut acc = FixedStepAccumulator::with_config(config);
        acc.add_delta(1.0).unwrap(); // Way over max
        assert_eq!(acc.accumulator(), 0.1);
    }

    #[test]
    fn test_alpha() {
        let mut acc = FixedStepAccumulator::new();
        acc.add_delta(0.01).unwrap(); // Half of 1/60
        let alpha = acc.alpha();
        assert!(alpha > 0.0 && alpha < 1.0);
    }

    #[test]
    fn test_estimate_steps() {
        let mut acc = FixedStepAccumulator::new();
        acc.add_delta(0.05).unwrap(); // About 3 steps at 60 Hz
        let steps = acc.estimate_steps();
        assert!(steps >= 2);
    }

    #[test]
    fn test_reset() {
        let mut acc = FixedStepAccumulator::new();
        acc.add_delta(0.02).unwrap();
        acc.step().unwrap();
        acc.reset();
        assert_eq!(acc.accumulator(), 0.0);
        assert_eq!(acc.steps_performed(), 0);
    }

    #[test]
    fn test_efficiency() {
        let mut acc = FixedStepAccumulator::new();
        acc.add_delta(0.03).unwrap();
        acc.step().unwrap();
        let efficiency = acc.efficiency();
        assert!(efficiency > 0.0 && efficiency <= 100.0);
    }

    #[test]
    fn test_lost_time() {
        let config = FixedStepConfig {
            step_duration: 1.0 / 60.0,
            max_accumulator: 0.05,
            min_accumulator: 0.0,
        };
        let mut acc = FixedStepAccumulator::with_config(config);
        acc.add_delta(0.1).unwrap(); // Over max
        let lost = acc.lost_time();
        assert!(lost > 0.0);
    }
}
