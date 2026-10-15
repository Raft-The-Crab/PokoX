use crate::error::AudioError;

/// Volume control utilities
pub struct VolumeControl;

impl VolumeControl {
    /// Convert linear volume to decibels
    pub fn linear_to_db(linear: f32) -> f32 {
        if linear <= 0.0 {
            return -f32::INFINITY;
        }
        20.0 * linear.log10()
    }

    /// Convert decibels to linear volume
    pub fn db_to_linear(db: f32) -> f32 {
        10.0_f32.powf(db / 20.0)
    }

    /// Clamp volume to valid range
    pub fn clamp_volume(volume: f32) -> f32 {
        volume.clamp(0.0, 1.0)
    }

    /// Smooth volume transition (linear interpolation)
    pub fn smooth_volume(current: f32, target: f32, factor: f32) -> f32 {
        current + (target - current) * factor.clamp(0.0, 1.0)
    }

    /// Validate volume is in valid range
    pub fn validate_volume(volume: f32) -> Result<(), AudioError> {
        if !(0.0..=1.0).contains(&volume) {
            return Err(AudioError::VolumeOutOfRange(volume));
        }
        Ok(())
    }

    /// Calculate fade in volume based on progress (0.0 to 1.0)
    pub fn fade_in(progress: f32) -> f32 {
        progress.clamp(0.0, 1.0)
    }

    /// Calculate fade out volume based on progress (0.0 to 1.0)
    pub fn fade_out(progress: f32) -> f32 {
        (1.0 - progress).clamp(0.0, 1.0)
    }

    /// Calculate exponential fade in for smoother transitions
    pub fn fade_in_exponential(progress: f32, power: f32) -> f32 {
        let p = progress.clamp(0.0, 1.0);
        p.powf(power)
    }

    /// Calculate exponential fade out for smoother transitions
    pub fn fade_out_exponential(progress: f32, power: f32) -> f32 {
        let p = progress.clamp(0.0, 1.0);
        (1.0 - p).powf(power)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_linear_to_db() {
        assert_eq!(VolumeControl::linear_to_db(1.0), 0.0);
        assert!((VolumeControl::linear_to_db(0.5) - (-6.0206)).abs() < 0.01);
        assert!(VolumeControl::linear_to_db(0.0).is_infinite());
    }

    #[test]
    fn test_db_to_linear() {
        assert!((VolumeControl::db_to_linear(0.0) - 1.0).abs() < 0.01);
        assert!((VolumeControl::db_to_linear(-6.0) - 0.5).abs() < 0.01);
    }

    #[test]
    fn test_clamp_volume() {
        assert_eq!(VolumeControl::clamp_volume(0.5), 0.5);
        assert_eq!(VolumeControl::clamp_volume(1.5), 1.0);
        assert_eq!(VolumeControl::clamp_volume(-0.1), 0.0);
    }

    #[test]
    fn test_smooth_volume() {
        assert_eq!(VolumeControl::smooth_volume(0.0, 1.0, 0.5), 0.5);
        assert_eq!(VolumeControl::smooth_volume(0.0, 1.0, 0.0), 0.0);
        assert_eq!(VolumeControl::smooth_volume(0.0, 1.0, 1.0), 1.0);
    }

    #[test]
    fn test_validate_volume_valid() {
        assert!(VolumeControl::validate_volume(0.5).is_ok());
        assert!(VolumeControl::validate_volume(0.0).is_ok());
        assert!(VolumeControl::validate_volume(1.0).is_ok());
    }

    #[test]
    fn test_validate_volume_invalid() {
        assert!(VolumeControl::validate_volume(1.5).is_err());
        assert!(VolumeControl::validate_volume(-0.1).is_err());
    }

    #[test]
    fn test_fade_in() {
        assert_eq!(VolumeControl::fade_in(0.0), 0.0);
        assert_eq!(VolumeControl::fade_in(0.5), 0.5);
        assert_eq!(VolumeControl::fade_in(1.0), 1.0);
        assert_eq!(VolumeControl::fade_in(1.5), 1.0); // Clamped
        assert_eq!(VolumeControl::fade_in(-0.5), 0.0); // Clamped
    }

    #[test]
    fn test_fade_out() {
        assert_eq!(VolumeControl::fade_out(0.0), 1.0);
        assert_eq!(VolumeControl::fade_out(0.5), 0.5);
        assert_eq!(VolumeControl::fade_out(1.0), 0.0);
        assert_eq!(VolumeControl::fade_out(1.5), 0.0); // Clamped
        assert_eq!(VolumeControl::fade_out(-0.5), 1.0); // Clamped
    }

    #[test]
    fn test_fade_in_exponential() {
        assert_eq!(VolumeControl::fade_in_exponential(0.0, 2.0), 0.0);
        assert_eq!(VolumeControl::fade_in_exponential(1.0, 2.0), 1.0);
        // At 0.5 with power 2, should be 0.25
        assert!((VolumeControl::fade_in_exponential(0.5, 2.0) - 0.25).abs() < 0.01);
    }

    #[test]
    fn test_fade_out_exponential() {
        assert_eq!(VolumeControl::fade_out_exponential(0.0, 2.0), 1.0);
        assert_eq!(VolumeControl::fade_out_exponential(1.0, 2.0), 0.0);
        // At 0.5 with power 2, should be 0.25
        assert!((VolumeControl::fade_out_exponential(0.5, 2.0) - 0.25).abs() < 0.01);
    }
}
