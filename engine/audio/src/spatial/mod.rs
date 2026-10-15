use crate::error::AudioError;
use glam::Vec3;

/// Configuration for spatial audio
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct SpatialAudioConfig {
    /// Enable spatial audio processing
    pub enabled: bool,
    /// Distance attenuation model
    pub attenuation_model: AttenuationModel,
    /// Reference distance (distance at which attenuation begins)
    pub reference_distance: f32,
    /// Maximum distance (beyond which sound is inaudible)
    pub max_distance: f32,
    /// Doppler effect scale
    pub doppler_scale: f32,
}

impl Default for SpatialAudioConfig {
    fn default() -> Self {
        Self {
            enabled: true,
            attenuation_model: AttenuationModel::Inverse,
            reference_distance: 1.0,
            max_distance: 100.0,
            doppler_scale: 1.0,
        }
    }
}

impl SpatialAudioConfig {
    /// Validate the configuration
    pub fn validate(&self) -> Result<(), AudioError> {
        if self.reference_distance <= 0.0 {
            return Err(AudioError::PlaybackFailed(
                "Reference distance must be positive".to_string(),
            ));
        }
        if self.max_distance <= self.reference_distance {
            return Err(AudioError::PlaybackFailed(
                "Max distance must be greater than reference distance".to_string(),
            ));
        }
        if self.doppler_scale < 0.0 {
            return Err(AudioError::PlaybackFailed(
                "Doppler scale must be non-negative".to_string(),
            ));
        }
        Ok(())
    }

    /// Set reference distance
    pub fn with_reference_distance(mut self, distance: f32) -> Self {
        self.reference_distance = distance;
        self
    }

    /// Set maximum distance
    pub fn with_max_distance(mut self, distance: f32) -> Self {
        self.max_distance = distance;
        self
    }

    /// Set attenuation model
    pub fn with_attenuation_model(mut self, model: AttenuationModel) -> Self {
        self.attenuation_model = model;
        self
    }

    /// Set doppler scale
    pub fn with_doppler_scale(mut self, scale: f32) -> Self {
        self.doppler_scale = scale;
        self
    }
}

/// Distance attenuation models
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum AttenuationModel {
    /// No attenuation
    None,
    /// Linear attenuation
    Linear,
    /// Inverse distance attenuation
    Inverse,
    /// Inverse square attenuation (realistic)
    InverseSquare,
}

/// Spatial audio state for a sound source
#[derive(Debug, Clone)]
pub struct SpatialAudio {
    /// 3D position of the sound
    pub position: Vec3,
    /// Velocity of the sound (for Doppler effect)
    pub velocity: Vec3,
    /// Configuration
    pub config: SpatialAudioConfig,
    /// Statistics
    stats: SpatialAudioStats,
}

/// Spatial audio statistics
#[derive(Debug, Clone, Copy, Default)]
pub struct SpatialAudioStats {
    /// Total attenuation calculations
    pub total_calculations: usize,
    /// Total doppler calculations
    pub total_doppler_calculations: usize,
    /// Average distance calculated
    pub average_distance: f32,
    /// Maximum distance calculated
    pub max_distance: f32,
}

impl SpatialAudio {
    /// Create new spatial audio state
    pub fn new(position: Vec3) -> Self {
        Self {
            position,
            velocity: Vec3::ZERO,
            config: SpatialAudioConfig::default(),
            stats: SpatialAudioStats::default(),
        }
    }

    /// Create with custom configuration
    pub fn with_config(position: Vec3, config: SpatialAudioConfig) -> Self {
        Self {
            position,
            velocity: Vec3::ZERO,
            config,
            stats: SpatialAudioStats::default(),
        }
    }

    /// Get statistics
    pub fn stats(&self) -> SpatialAudioStats {
        self.stats
    }

    /// Reset statistics
    pub fn reset_stats(&mut self) {
        self.stats = SpatialAudioStats::default();
    }

    /// Update statistics based on distance calculation
    fn update_distance_stats(&mut self, distance: f32) {
        self.stats.total_calculations += 1;
        self.stats.max_distance = self.stats.max_distance.max(distance);

        // Update average distance
        if self.stats.total_calculations == 1 {
            self.stats.average_distance = distance;
        } else {
            self.stats.average_distance = (self.stats.average_distance
                * (self.stats.total_calculations - 1) as f32
                + distance)
                / self.stats.total_calculations as f32;
        }
    }

    /// Calculate distance attenuation factor (immutable, no stats update)
    pub fn calculate_attenuation(&self, listener_position: Vec3) -> f32 {
        if !self.config.enabled {
            return 1.0;
        }

        let distance = self.position.distance(listener_position);

        if distance <= self.config.reference_distance {
            return 1.0;
        }

        if distance >= self.config.max_distance {
            return 0.0;
        }

        match self.config.attenuation_model {
            AttenuationModel::None => 1.0,
            AttenuationModel::Linear => {
                let range = self.config.max_distance - self.config.reference_distance;
                let normalized = (distance - self.config.reference_distance) / range;
                1.0 - normalized
            }
            AttenuationModel::Inverse => self.config.reference_distance / distance,
            AttenuationModel::InverseSquare => {
                let ratio = self.config.reference_distance / distance;
                ratio * ratio
            }
        }
    }

    /// Calculate distance attenuation factor with statistics tracking
    pub fn calculate_attenuation_with_stats(&mut self, listener_position: Vec3) -> f32 {
        if !self.config.enabled {
            return 1.0;
        }

        let distance = self.position.distance(listener_position);
        self.update_distance_stats(distance);

        if distance <= self.config.reference_distance {
            return 1.0;
        }

        if distance >= self.config.max_distance {
            return 0.0;
        }

        match self.config.attenuation_model {
            AttenuationModel::None => 1.0,
            AttenuationModel::Linear => {
                let range = self.config.max_distance - self.config.reference_distance;
                let normalized = (distance - self.config.reference_distance) / range;
                1.0 - normalized
            }
            AttenuationModel::Inverse => self.config.reference_distance / distance,
            AttenuationModel::InverseSquare => {
                let ratio = self.config.reference_distance / distance;
                ratio * ratio
            }
        }
    }

    /// Calculate doppler shift factor (immutable, no stats update)
    pub fn calculate_doppler(&self, listener_position: Vec3, listener_velocity: Vec3) -> f32 {
        if !self.config.enabled || self.config.doppler_scale == 0.0 {
            return 1.0;
        }

        // Direction from source to listener
        let to_listener = (listener_position - self.position).normalize();

        // Project velocities onto the direction vector
        let source_velocity_towards = self.velocity.dot(to_listener);
        let listener_velocity_towards = listener_velocity.dot(to_listener);

        // Speed of sound in m/s (approximate)
        const SPEED_OF_SOUND: f32 = 343.0;

        // Doppler formula: f' = f * (c + vr) / (c + vs)
        // where vr is receiver (listener) velocity towards source
        // and vs is source velocity towards receiver
        // Negative because our vectors point towards listener
        let doppler_factor = (SPEED_OF_SOUND
            - listener_velocity_towards * self.config.doppler_scale)
            / (SPEED_OF_SOUND - source_velocity_towards * self.config.doppler_scale);

        doppler_factor.clamp(0.5, 2.0)
    }

    /// Calculate doppler shift factor with statistics tracking
    pub fn calculate_doppler_with_stats(
        &mut self,
        listener_position: Vec3,
        listener_velocity: Vec3,
    ) -> f32 {
        if !self.config.enabled || self.config.doppler_scale == 0.0 {
            return 1.0;
        }

        self.stats.total_doppler_calculations += 1;

        // Direction from source to listener
        let to_listener = (listener_position - self.position).normalize();

        // Project velocities onto the direction vector
        let source_velocity_towards = self.velocity.dot(to_listener);
        let listener_velocity_towards = listener_velocity.dot(to_listener);

        // Speed of sound in m/s (approximate)
        const SPEED_OF_SOUND: f32 = 343.0;

        // Doppler formula: f' = f * (c + vr) / (c + vs)
        // where vr is receiver (listener) velocity towards source
        // and vs is source velocity towards receiver
        // Negative because our vectors point towards listener
        let doppler_factor = (SPEED_OF_SOUND
            - listener_velocity_towards * self.config.doppler_scale)
            / (SPEED_OF_SOUND - source_velocity_towards * self.config.doppler_scale);

        doppler_factor.clamp(0.5, 2.0)
    }

    /// Update position
    pub fn set_position(&mut self, position: Vec3) {
        self.position = position;
    }

    /// Update velocity
    pub fn set_velocity(&mut self, velocity: Vec3) {
        self.velocity = velocity;
    }

    /// Calculate distance to listener
    pub fn distance_to(&self, listener_position: Vec3) -> f32 {
        self.position.distance(listener_position)
    }

    /// Check if within audible range
    pub fn is_within_range(&self, listener_position: Vec3) -> bool {
        if !self.config.enabled {
            return true;
        }
        let distance = self.distance_to(listener_position);
        distance < self.config.max_distance
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_spatial_audio_creation() {
        let spatial = SpatialAudio::new(Vec3::new(1.0, 2.0, 3.0));
        assert_eq!(spatial.position, Vec3::new(1.0, 2.0, 3.0));
        assert_eq!(spatial.velocity, Vec3::ZERO);
        assert_eq!(spatial.stats().total_calculations, 0);
    }

    #[test]
    fn test_attenuation_none() {
        let spatial = SpatialAudio::with_config(
            Vec3::ZERO,
            SpatialAudioConfig {
                enabled: true,
                attenuation_model: AttenuationModel::None,
                ..Default::default()
            },
        );
        assert_eq!(spatial.calculate_attenuation(Vec3::ZERO), 1.0);
    }

    #[test]
    fn test_attenuation_within_reference() {
        let spatial = SpatialAudio::with_config(
            Vec3::ZERO,
            SpatialAudioConfig {
                enabled: true,
                reference_distance: 10.0,
                max_distance: 100.0,
                ..Default::default()
            },
        );
        assert_eq!(spatial.calculate_attenuation(Vec3::new(5.0, 0.0, 0.0)), 1.0);
    }

    #[test]
    fn test_attenuation_beyond_max() {
        let spatial = SpatialAudio::with_config(
            Vec3::ZERO,
            SpatialAudioConfig {
                enabled: true,
                reference_distance: 1.0,
                max_distance: 10.0,
                ..Default::default()
            },
        );
        assert_eq!(
            spatial.calculate_attenuation(Vec3::new(15.0, 0.0, 0.0)),
            0.0
        );
    }

    #[test]
    fn test_attenuation_linear() {
        let spatial = SpatialAudio::with_config(
            Vec3::ZERO,
            SpatialAudioConfig {
                enabled: true,
                attenuation_model: AttenuationModel::Linear,
                reference_distance: 1.0,
                max_distance: 11.0,
                ..Default::default()
            },
        );
        // At 6.0 distance (halfway between 1.0 and 11.0), attenuation should be 0.5
        let attenuation = spatial.calculate_attenuation(Vec3::new(6.0, 0.0, 0.0));
        assert!((attenuation - 0.5).abs() < 0.01);
    }

    #[test]
    fn test_set_position() {
        let mut spatial = SpatialAudio::new(Vec3::ZERO);
        spatial.set_position(Vec3::new(10.0, 20.0, 30.0));
        assert_eq!(spatial.position, Vec3::new(10.0, 20.0, 30.0));
    }

    #[test]
    fn test_set_velocity() {
        let mut spatial = SpatialAudio::new(Vec3::ZERO);
        spatial.set_velocity(Vec3::new(1.0, 2.0, 3.0));
        assert_eq!(spatial.velocity, Vec3::new(1.0, 2.0, 3.0));
    }

    #[test]
    fn test_distance_to() {
        let spatial = SpatialAudio::new(Vec3::new(0.0, 0.0, 0.0));
        let distance = spatial.distance_to(Vec3::new(3.0, 4.0, 0.0));
        assert!((distance - 5.0).abs() < 0.01);
    }

    #[test]
    fn test_is_within_range() {
        let spatial = SpatialAudio::with_config(
            Vec3::ZERO,
            SpatialAudioConfig {
                max_distance: 10.0,
                ..Default::default()
            },
        );
        assert!(spatial.is_within_range(Vec3::new(5.0, 0.0, 0.0)));
        assert!(!spatial.is_within_range(Vec3::new(15.0, 0.0, 0.0)));
    }

    #[test]
    fn test_doppler_calculation() {
        let mut spatial = SpatialAudio::with_config(
            Vec3::ZERO,
            SpatialAudioConfig {
                enabled: true,
                doppler_scale: 1.0,
                ..Default::default()
            },
        );
        spatial.set_velocity(Vec3::new(10.0, 0.0, 0.0));

        // Sound moving towards listener at 10 m/s should increase pitch
        let doppler = spatial.calculate_doppler(Vec3::new(10.0, 0.0, 0.0), Vec3::ZERO);
        assert!(doppler > 1.0);
        assert!(doppler < 1.1); // Should be reasonable
    }

    #[test]
    fn test_doppler_listener_moving() {
        let spatial = SpatialAudio::with_config(
            Vec3::ZERO,
            SpatialAudioConfig {
                enabled: true,
                doppler_scale: 1.0,
                ..Default::default()
            },
        );
        // Listener moving towards stationary source should decrease pitch
        let doppler =
            spatial.calculate_doppler(Vec3::new(10.0, 0.0, 0.0), Vec3::new(10.0, 0.0, 0.0));
        assert!(doppler < 1.0);
        assert!(doppler > 0.9); // Should be reasonable
    }

    #[test]
    fn test_doppler_with_stats() {
        let mut spatial = SpatialAudio::with_config(
            Vec3::ZERO,
            SpatialAudioConfig {
                enabled: true,
                doppler_scale: 1.0,
                ..Default::default()
            },
        );
        spatial.set_velocity(Vec3::new(10.0, 0.0, 0.0));

        let doppler = spatial.calculate_doppler_with_stats(Vec3::new(10.0, 0.0, 0.0), Vec3::ZERO);
        assert!(doppler > 1.0);
        assert_eq!(spatial.stats().total_doppler_calculations, 1);
    }

    #[test]
    fn test_doppler_disabled() {
        let mut spatial = SpatialAudio::with_config(
            Vec3::ZERO,
            SpatialAudioConfig {
                enabled: true,
                doppler_scale: 0.0,
                ..Default::default()
            },
        );
        spatial.set_velocity(Vec3::new(10.0, 0.0, 0.0));

        let doppler = spatial.calculate_doppler(Vec3::new(10.0, 0.0, 0.0), Vec3::ZERO);
        assert_eq!(doppler, 1.0);
    }

    #[test]
    fn test_stats_tracking() {
        let mut spatial = SpatialAudio::new(Vec3::ZERO);
        assert_eq!(spatial.stats().total_calculations, 0);

        spatial.calculate_attenuation_with_stats(Vec3::new(5.0, 0.0, 0.0));
        assert_eq!(spatial.stats().total_calculations, 1);
        assert_eq!(spatial.stats().max_distance, 5.0);

        spatial.calculate_attenuation_with_stats(Vec3::new(10.0, 0.0, 0.0));
        assert_eq!(spatial.stats().total_calculations, 2);
        assert_eq!(spatial.stats().max_distance, 10.0);
    }

    #[test]
    fn test_reset_stats() {
        let mut spatial = SpatialAudio::new(Vec3::ZERO);
        spatial.calculate_attenuation_with_stats(Vec3::new(5.0, 0.0, 0.0));
        assert_eq!(spatial.stats().total_calculations, 1);

        spatial.reset_stats();
        assert_eq!(spatial.stats().total_calculations, 0);
    }

    #[test]
    fn test_config_validate_valid() {
        let config = SpatialAudioConfig::default();
        assert!(config.validate().is_ok());
    }

    #[test]
    fn test_config_validate_invalid_reference() {
        let config = SpatialAudioConfig {
            reference_distance: 0.0,
            ..Default::default()
        };
        assert!(config.validate().is_err());
    }

    #[test]
    fn test_config_validate_invalid_max_distance() {
        let config = SpatialAudioConfig {
            reference_distance: 10.0,
            max_distance: 5.0,
            ..Default::default()
        };
        assert!(config.validate().is_err());
    }

    #[test]
    fn test_config_validate_invalid_doppler() {
        let config = SpatialAudioConfig {
            doppler_scale: -1.0,
            ..Default::default()
        };
        assert!(config.validate().is_err());
    }

    #[test]
    fn test_config_builder() {
        let config = SpatialAudioConfig::default()
            .with_reference_distance(5.0)
            .with_max_distance(50.0)
            .with_attenuation_model(AttenuationModel::Linear)
            .with_doppler_scale(0.5);
        assert_eq!(config.reference_distance, 5.0);
        assert_eq!(config.max_distance, 50.0);
        assert_eq!(config.attenuation_model, AttenuationModel::Linear);
        assert_eq!(config.doppler_scale, 0.5);
    }
}
