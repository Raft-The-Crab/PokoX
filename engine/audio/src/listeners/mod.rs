use crate::error::AudioError;
use glam::Vec3;

/// Audio listener configuration
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct AudioListenerConfig {
    /// Enable doppler effect
    pub enable_doppler: bool,
    /// Doppler scale factor
    pub doppler_scale: f32,
}

impl AudioListenerConfig {
    /// Validate the configuration
    pub fn validate(&self) -> Result<(), AudioError> {
        if self.doppler_scale < 0.0 {
            return Err(AudioError::PlaybackFailed(
                "Doppler scale must be non-negative".to_string(),
            ));
        }
        Ok(())
    }

    /// Set doppler scale
    pub fn with_doppler_scale(mut self, scale: f32) -> Self {
        self.doppler_scale = scale;
        self
    }

    /// Enable/disable doppler
    pub fn with_doppler(mut self, enabled: bool) -> Self {
        self.enable_doppler = enabled;
        self
    }
}

impl Default for AudioListenerConfig {
    fn default() -> Self {
        Self {
            enable_doppler: true,
            doppler_scale: 1.0,
        }
    }
}

/// Audio listener for spatial audio
#[derive(Debug, Clone)]
pub struct AudioListener {
    /// 3D position
    pub position: Vec3,
    /// Forward direction (where the listener is facing)
    pub forward: Vec3,
    /// Up direction
    pub up: Vec3,
    /// Velocity (for Doppler effect)
    pub velocity: Vec3,
    /// Configuration
    config: AudioListenerConfig,
}

impl Default for AudioListener {
    fn default() -> Self {
        Self {
            position: Vec3::ZERO,
            forward: Vec3::NEG_Z,
            up: Vec3::Y,
            velocity: Vec3::ZERO,
            config: AudioListenerConfig::default(),
        }
    }
}

impl AudioListener {
    /// Create a new listener
    pub fn new(position: Vec3) -> Self {
        Self {
            position,
            ..Default::default()
        }
    }

    /// Create with orientation
    pub fn with_orientation(position: Vec3, forward: Vec3, up: Vec3) -> Self {
        Self {
            position,
            forward: forward.normalize(),
            up: up.normalize(),
            velocity: Vec3::ZERO,
            config: AudioListenerConfig::default(),
        }
    }

    /// Create with configuration
    pub fn with_config(position: Vec3, config: AudioListenerConfig) -> Self {
        Self {
            position,
            config,
            ..Default::default()
        }
    }

    /// Get configuration
    pub fn config(&self) -> &AudioListenerConfig {
        &self.config
    }

    /// Set configuration
    pub fn set_config(&mut self, config: AudioListenerConfig) -> Result<(), AudioError> {
        config.validate()?;
        self.config = config;
        Ok(())
    }

    /// Set position
    pub fn set_position(&mut self, position: Vec3) {
        self.position = position;
    }

    /// Set orientation
    pub fn set_orientation(&mut self, forward: Vec3, up: Vec3) {
        self.forward = forward.normalize();
        self.up = up.normalize();
    }

    /// Set velocity
    pub fn set_velocity(&mut self, velocity: Vec3) {
        self.velocity = velocity;
    }

    /// Get right direction (cross product of forward and up)
    pub fn right(&self) -> Vec3 {
        self.up.cross(self.forward).normalize()
    }

    /// Calculate direction to a point
    pub fn direction_to(&self, point: Vec3) -> Vec3 {
        (point - self.position).normalize()
    }

    /// Calculate distance to a point
    pub fn distance_to(&self, point: Vec3) -> f32 {
        self.position.distance(point)
    }

    /// Get speed (magnitude of velocity)
    pub fn speed(&self) -> f32 {
        self.velocity.length()
    }

    /// Check if listener is moving
    pub fn is_moving(&self) -> bool {
        self.speed() > 0.0
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_listener_creation() {
        let listener = AudioListener::new(Vec3::new(1.0, 2.0, 3.0));
        assert_eq!(listener.position, Vec3::new(1.0, 2.0, 3.0));
        assert_eq!(listener.forward, Vec3::NEG_Z);
    }

    #[test]
    fn test_listener_with_orientation() {
        let listener = AudioListener::with_orientation(Vec3::ZERO, Vec3::X, Vec3::Y);
        assert_eq!(listener.forward, Vec3::X);
        assert_eq!(listener.up, Vec3::Y);
    }

    #[test]
    fn test_right_direction() {
        let listener = AudioListener::default();
        let right = listener.right();
        // Right should be cross product of Y and -Z = -X
        assert!((right - Vec3::NEG_X).length() < 0.01);
    }

    #[test]
    fn test_direction_to() {
        let listener = AudioListener::new(Vec3::ZERO);
        let direction = listener.direction_to(Vec3::X);
        assert!((direction - Vec3::X).length() < 0.01);
    }

    #[test]
    fn test_distance_to() {
        let listener = AudioListener::new(Vec3::ZERO);
        let distance = listener.distance_to(Vec3::new(3.0, 4.0, 0.0));
        assert!((distance - 5.0).abs() < 0.01);
    }

    #[test]
    fn test_speed() {
        let mut listener = AudioListener::new(Vec3::ZERO);
        assert_eq!(listener.speed(), 0.0);
        listener.set_velocity(Vec3::new(3.0, 4.0, 0.0));
        assert!((listener.speed() - 5.0).abs() < 0.01);
    }

    #[test]
    fn test_is_moving() {
        let mut listener = AudioListener::new(Vec3::ZERO);
        assert!(!listener.is_moving());
        listener.set_velocity(Vec3::X);
        assert!(listener.is_moving());
    }

    #[test]
    fn test_config_validate_valid() {
        let config = AudioListenerConfig::default();
        assert!(config.validate().is_ok());
    }

    #[test]
    fn test_config_validate_invalid_doppler() {
        let config = AudioListenerConfig {
            doppler_scale: -1.0,
            ..Default::default()
        };
        assert!(config.validate().is_err());
    }

    #[test]
    fn test_config_builder() {
        let config = AudioListenerConfig::default()
            .with_doppler_scale(2.0)
            .with_doppler(false);
        assert_eq!(config.doppler_scale, 2.0);
        assert!(!config.enable_doppler);
    }

    #[test]
    fn test_listener_with_config() {
        let config = AudioListenerConfig::default();
        let listener = AudioListener::with_config(Vec3::ZERO, config);
        assert_eq!(listener.config().doppler_scale, 1.0);
    }

    #[test]
    fn test_set_config_valid() {
        let mut listener = AudioListener::new(Vec3::ZERO);
        let config = AudioListenerConfig::default();
        assert!(listener.set_config(config).is_ok());
    }

    #[test]
    fn test_set_config_invalid() {
        let mut listener = AudioListener::new(Vec3::ZERO);
        let config = AudioListenerConfig {
            doppler_scale: -1.0,
            ..Default::default()
        };
        assert!(listener.set_config(config).is_err());
    }
}
