//! Frame stepping for controlled simulation
//!
//! Provides frame-by-frame stepping control for debugging and testing.

use thiserror::Error;

/// Errors that can occur with frame stepping
#[derive(Debug, Clone, Error)]
pub enum StepError {
    #[error("Step error: {0}")]
    Custom(String),
}

/// Frame step mode
#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub enum FrameStepMode {
    /// Run normally (no stepping)
    #[default]
    Normal,
    /// Step one frame then pause
    StepOne,
    /// Step N frames then pause
    StepN(u32),
    /// Pause (stop stepping)
    Paused,
    /// Run until a breakpoint (frame number)
    Breakpoint(u64),
}

/// Frame stepper for controlled simulation
#[derive(Debug, Clone)]
pub struct FrameStepper {
    /// Current step mode
    mode: FrameStepMode,
    /// Frames remaining in current step sequence
    frames_remaining: u32,
    /// Total frames stepped
    total_stepped: u32,
    /// Whether the stepper is waiting for input
    waiting: bool,
    /// Breakpoint frame number (if set)
    breakpoint: Option<u64>,
    /// Whether breakpoints are enabled
    breakpoints_enabled: bool,
}

impl FrameStepper {
    /// Create a new frame stepper
    pub fn new() -> Self {
        Self {
            mode: FrameStepMode::Normal,
            frames_remaining: 0,
            total_stepped: 0,
            waiting: false,
            breakpoint: None,
            breakpoints_enabled: false,
        }
    }

    /// Set the step mode
    pub fn set_mode(&mut self, mode: FrameStepMode) -> Result<(), StepError> {
        match mode {
            FrameStepMode::Normal => {
                self.mode = mode;
                self.frames_remaining = 0;
                self.waiting = false;
            }
            FrameStepMode::StepOne => {
                self.mode = mode;
                self.frames_remaining = 1;
                self.waiting = false;
            }
            FrameStepMode::StepN(n) => {
                if n == 0 {
                    return Err(StepError::Custom(
                        "Step count must be greater than 0".to_string(),
                    ));
                }
                self.mode = mode;
                self.frames_remaining = n;
                self.waiting = false;
            }
            FrameStepMode::Paused => {
                self.mode = mode;
                self.frames_remaining = 0;
                self.waiting = true;
            }
            FrameStepMode::Breakpoint(frame) => {
                self.mode = mode;
                self.frames_remaining = 0;
                self.waiting = false;
                self.breakpoint = Some(frame);
            }
        }
        Ok(())
    }

    /// Get the current step mode
    pub fn mode(&self) -> FrameStepMode {
        self.mode
    }

    /// Check if a frame should be processed
    pub fn should_process(&mut self, current_frame: u64) -> bool {
        match self.mode {
            FrameStepMode::Normal => {
                if self.breakpoints_enabled {
                    if let Some(bp) = self.breakpoint {
                        if current_frame >= bp {
                            self.breakpoint = None;
                            return false; // Pause at breakpoint
                        }
                    }
                }
                true
            }
            FrameStepMode::StepOne | FrameStepMode::StepN(_) => {
                !self.waiting && self.frames_remaining > 0
            }
            FrameStepMode::Paused => false,
            FrameStepMode::Breakpoint(frame) => current_frame < frame,
        }
    }

    /// Notify that a frame has been processed
    pub fn frame_processed(&mut self) {
        if self.frames_remaining > 0 {
            self.frames_remaining -= 1;
            self.total_stepped += 1;

            if self.frames_remaining == 0 {
                self.waiting = true;
            }
        }
    }

    /// Step one frame
    pub fn step_one(&mut self) -> Result<(), StepError> {
        self.set_mode(FrameStepMode::StepOne)
    }

    /// Step N frames
    pub fn step_n(&mut self, n: u32) -> Result<(), StepError> {
        self.set_mode(FrameStepMode::StepN(n))
    }

    /// Pause the stepper
    pub fn pause(&mut self) {
        let _ = self.set_mode(FrameStepMode::Paused);
    }

    /// Resume normal execution
    pub fn resume(&mut self) {
        let _ = self.set_mode(FrameStepMode::Normal);
    }

    /// Set a breakpoint frame
    pub fn set_breakpoint(&mut self, frame: u64) {
        self.mode = FrameStepMode::Breakpoint(frame);
        self.breakpoint = Some(frame);
    }

    /// Clear the breakpoint
    pub fn clear_breakpoint(&mut self) {
        self.breakpoint = None;
        if matches!(self.mode, FrameStepMode::Breakpoint(_)) {
            self.mode = FrameStepMode::Normal;
        }
    }

    /// Enable breakpoints
    pub fn enable_breakpoints(&mut self) {
        self.breakpoints_enabled = true;
    }

    /// Disable breakpoints
    pub fn disable_breakpoints(&mut self) {
        self.breakpoints_enabled = false;
    }

    /// Check if breakpoints are enabled
    pub fn breakpoints_enabled(&self) -> bool {
        self.breakpoints_enabled
    }

    /// Check if the stepper is paused
    pub fn is_paused(&self) -> bool {
        self.mode == FrameStepMode::Paused
    }

    /// Check if the stepper is waiting for input
    pub fn is_waiting(&self) -> bool {
        self.waiting
    }

    /// Get the number of frames remaining
    pub fn frames_remaining(&self) -> u32 {
        self.frames_remaining
    }

    /// Get the total number of frames stepped
    pub fn total_stepped(&self) -> u32 {
        self.total_stepped
    }

    /// Get the breakpoint frame (if set)
    pub fn breakpoint(&self) -> Option<u64> {
        self.breakpoint
    }

    /// Reset the stepper
    pub fn reset(&mut self) {
        self.mode = FrameStepMode::Normal;
        self.frames_remaining = 0;
        self.total_stepped = 0;
        self.waiting = false;
        self.breakpoint = None;
    }

    /// Get statistics about the stepper
    pub fn stats(&self) -> StepperStats {
        StepperStats {
            total_stepped: self.total_stepped,
            frames_remaining: self.frames_remaining,
            is_paused: self.is_paused(),
            is_waiting: self.is_waiting(),
            has_breakpoint: self.breakpoint.is_some(),
        }
    }
}

/// Statistics about the frame stepper
#[derive(Debug, Clone)]
pub struct StepperStats {
    pub total_stepped: u32,
    pub frames_remaining: u32,
    pub is_paused: bool,
    pub is_waiting: bool,
    pub has_breakpoint: bool,
}

impl Default for FrameStepper {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_stepper_creation() {
        let mut stepper = FrameStepper::new();
        assert_eq!(stepper.mode(), FrameStepMode::Normal);
        assert!(stepper.should_process(0));
    }

    #[test]
    fn test_step_one() {
        let mut stepper = FrameStepper::new();
        stepper.step_one().unwrap();
        assert!(stepper.should_process(0));
        stepper.frame_processed();
        assert!(!stepper.should_process(0));
        assert!(stepper.is_waiting());
    }

    #[test]
    fn test_step_n() {
        let mut stepper = FrameStepper::new();
        stepper.step_n(3).unwrap();
        assert_eq!(stepper.frames_remaining(), 3);
        assert!(stepper.should_process(0));
        stepper.frame_processed();
        assert_eq!(stepper.frames_remaining(), 2);
        stepper.frame_processed();
        assert_eq!(stepper.frames_remaining(), 1);
        stepper.frame_processed();
        assert_eq!(stepper.frames_remaining(), 0);
        assert!(!stepper.should_process(0));
    }

    #[test]
    fn test_step_n_zero() {
        let mut stepper = FrameStepper::new();
        assert!(stepper.step_n(0).is_err());
    }

    #[test]
    fn test_pause_resume() {
        let mut stepper = FrameStepper::new();
        stepper.pause();
        assert!(stepper.is_paused());
        assert!(!stepper.should_process(0));
        stepper.resume();
        assert!(!stepper.is_paused());
        assert!(stepper.should_process(0));
    }

    #[test]
    fn test_reset() {
        let mut stepper = FrameStepper::new();
        stepper.step_n(5).unwrap();
        stepper.frame_processed();
        stepper.frame_processed();
        assert_eq!(stepper.total_stepped(), 2);
        stepper.reset();
        assert_eq!(stepper.total_stepped(), 0);
        assert_eq!(stepper.mode(), FrameStepMode::Normal);
    }

    #[test]
    fn test_normal_mode() {
        let mut stepper = FrameStepper::new();
        stepper.set_mode(FrameStepMode::Normal).unwrap();
        for _ in 0..10 {
            assert!(stepper.should_process(0));
            stepper.frame_processed();
        }
        assert!(stepper.should_process(0));
    }

    #[test]
    fn test_breakpoint() {
        let mut stepper = FrameStepper::new();
        stepper.set_breakpoint(10);
        assert_eq!(stepper.breakpoint(), Some(10));
        for i in 0..10 {
            assert!(stepper.should_process(i));
        }
        assert!(!stepper.should_process(10));
    }

    #[test]
    fn test_clear_breakpoint() {
        let mut stepper = FrameStepper::new();
        stepper.set_breakpoint(10);
        stepper.clear_breakpoint();
        assert!(stepper.breakpoint().is_none());
        assert_eq!(stepper.mode(), FrameStepMode::Normal);
    }

    #[test]
    fn test_breakpoints_enabled() {
        let mut stepper = FrameStepper::new();
        assert!(!stepper.breakpoints_enabled());
        stepper.enable_breakpoints();
        assert!(stepper.breakpoints_enabled());
        stepper.disable_breakpoints();
        assert!(!stepper.breakpoints_enabled());
    }

    #[test]
    fn test_stats() {
        let mut stepper = FrameStepper::new();
        stepper.step_n(5).unwrap();
        stepper.frame_processed();
        stepper.frame_processed();
        let stats = stepper.stats();
        assert_eq!(stats.total_stepped, 2);
        assert_eq!(stats.frames_remaining, 3);
    }

    #[test]
    fn test_should_process_normal() {
        let mut stepper = FrameStepper::new();
        assert!(stepper.should_process(0));
        assert!(stepper.should_process(100));
    }

    #[test]
    fn test_should_process_paused() {
        let mut stepper = FrameStepper::new();
        stepper.pause();
        assert!(!stepper.should_process(0));
    }

    #[test]
    fn test_should_process_step_one() {
        let mut stepper = FrameStepper::new();
        stepper.step_one().unwrap();
        assert!(stepper.should_process(0));
        stepper.frame_processed();
        assert!(!stepper.should_process(0));
    }

    #[test]
    fn test_should_process_breakpoint_mode() {
        let mut stepper = FrameStepper::new();
        stepper.set_mode(FrameStepMode::Breakpoint(5)).unwrap();
        assert!(stepper.should_process(0));
        assert!(stepper.should_process(4));
        assert!(!stepper.should_process(5));
        assert!(!stepper.should_process(10));
    }
}
