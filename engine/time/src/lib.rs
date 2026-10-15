//! Time and Frame Management for Poko Engine
//!
//! This module provides:
//! - Clock for tracking elapsed time
//! - Fixed-step accumulator for deterministic game loops
//! - Frame stepping for controlled simulation

pub mod accumulator;
pub mod clock;
pub mod step;

pub use accumulator::{AccumulatorError, FixedStepAccumulator, FixedStepConfig};
pub use clock::{Clock, ClockConfig, ClockError};
pub use step::{FrameStepMode, FrameStepper, StepError, StepperStats};
