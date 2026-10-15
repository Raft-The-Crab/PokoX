use crate::error::AudioError;
use crate::sources::SoundSourceHandle;
use std::collections::HashMap;

/// Sound effect category
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum SfxCategory {
    /// UI sounds
    Ui,
    /// Footstep sounds
    Footsteps,
    /// Weapon sounds
    Weapons,
    /// Impact sounds
    Impacts,
    /// Ambient sounds
    Ambient,
    /// Custom category
    Custom(u8),
}

/// Sound effect configuration
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct SfxConfig {
    /// Volume variation (0.0 to 1.0)
    pub volume_variation: f32,
    /// Pitch variation (0.0 to 1.0)
    pub pitch_variation: f32,
    /// Minimum volume
    pub min_volume: f32,
    /// Maximum volume
    pub max_volume: f32,
}

impl SfxConfig {
    /// Validate the configuration
    pub fn validate(&self) -> Result<(), AudioError> {
        if !(0.0..=1.0).contains(&self.volume_variation) {
            return Err(AudioError::VolumeOutOfRange(self.volume_variation));
        }
        if !(0.0..=1.0).contains(&self.pitch_variation) {
            return Err(AudioError::PitchOutOfRange(self.pitch_variation));
        }
        if !(0.0..=1.0).contains(&self.min_volume) {
            return Err(AudioError::VolumeOutOfRange(self.min_volume));
        }
        if !(0.0..=1.0).contains(&self.max_volume) {
            return Err(AudioError::VolumeOutOfRange(self.max_volume));
        }
        if self.min_volume > self.max_volume {
            return Err(AudioError::PlaybackFailed(
                "Min volume cannot be greater than max volume".to_string(),
            ));
        }
        Ok(())
    }

    /// Set volume variation
    pub fn with_volume_variation(mut self, variation: f32) -> Self {
        self.volume_variation = variation;
        self
    }

    /// Set pitch variation
    pub fn with_pitch_variation(mut self, variation: f32) -> Self {
        self.pitch_variation = variation;
        self
    }

    /// Set min volume
    pub fn with_min_volume(mut self, volume: f32) -> Self {
        self.min_volume = volume;
        self
    }

    /// Set max volume
    pub fn with_max_volume(mut self, volume: f32) -> Self {
        self.max_volume = volume;
        self
    }
}

impl Default for SfxConfig {
    fn default() -> Self {
        Self {
            volume_variation: 0.1,
            pitch_variation: 0.1,
            min_volume: 0.5,
            max_volume: 1.0,
        }
    }
}

/// Sound effect manager statistics
#[derive(Debug, Clone, Copy, Default)]
pub struct SfxManagerStats {
    /// Total sounds registered
    pub total_sounds: usize,
    /// Sounds per category
    pub sounds_by_category: [usize; 5], // Ui, Footsteps, Weapons, Impacts, Ambient
}

/// Sound effect manager
pub struct SfxManager {
    /// Sound effect configurations by category
    category_configs: HashMap<SfxCategory, SfxConfig>,
    /// Sound to category mapping
    sound_categories: HashMap<SoundSourceHandle, SfxCategory>,
    /// Statistics
    stats: SfxManagerStats,
}

impl SfxManager {
    /// Create a new SFX manager
    pub fn new() -> Self {
        Self {
            category_configs: HashMap::new(),
            sound_categories: HashMap::new(),
            stats: SfxManagerStats::default(),
        }
    }

    /// Get statistics
    pub fn stats(&self) -> SfxManagerStats {
        self.stats
    }

    /// Reset statistics
    pub fn reset_stats(&mut self) {
        self.stats = SfxManagerStats::default();
    }

    /// Update statistics
    fn update_stats(&mut self) {
        self.stats.total_sounds = self.sound_categories.len();
        self.stats.sounds_by_category = [0; 5];
        for category in self.sound_categories.values() {
            match category {
                SfxCategory::Ui => self.stats.sounds_by_category[0] += 1,
                SfxCategory::Footsteps => self.stats.sounds_by_category[1] += 1,
                SfxCategory::Weapons => self.stats.sounds_by_category[2] += 1,
                SfxCategory::Impacts => self.stats.sounds_by_category[3] += 1,
                SfxCategory::Ambient => self.stats.sounds_by_category[4] += 1,
                SfxCategory::Custom(_) => {}
            }
        }
    }

    /// Set configuration for a category
    pub fn set_category_config(
        &mut self,
        category: SfxCategory,
        config: SfxConfig,
    ) -> Result<(), AudioError> {
        config.validate()?;
        self.category_configs.insert(category, config);
        Ok(())
    }

    /// Get configuration for a category
    pub fn get_category_config(&self, category: SfxCategory) -> Option<SfxConfig> {
        self.category_configs.get(&category).copied()
    }

    /// Assign sound to category
    pub fn assign_sound_category(&mut self, handle: SoundSourceHandle, category: SfxCategory) {
        self.sound_categories.insert(handle, category);
        self.update_stats();
    }

    /// Remove sound from category
    pub fn remove_sound_category(&mut self, handle: SoundSourceHandle) -> bool {
        let removed = self.sound_categories.remove(&handle).is_some();
        if removed {
            self.update_stats();
        }
        removed
    }

    /// Get sound's category
    pub fn get_sound_category(&self, handle: SoundSourceHandle) -> Option<SfxCategory> {
        self.sound_categories.get(&handle).copied()
    }

    /// Apply random variation to volume
    pub fn apply_volume_variation(&self, base_volume: f32, handle: SoundSourceHandle) -> f32 {
        let category = self.get_sound_category(handle).unwrap_or(SfxCategory::Ui);
        let config = self.get_category_config(category).unwrap_or_default();

        let variation = (rand::random::<f32>() - 0.5) * 2.0 * config.volume_variation;
        let volume = (base_volume + variation).clamp(config.min_volume, config.max_volume);
        volume.clamp(0.0, 1.0)
    }

    /// Apply random variation to pitch
    pub fn apply_pitch_variation(&self, base_pitch: f32, handle: SoundSourceHandle) -> f32 {
        let category = self.get_sound_category(handle).unwrap_or(SfxCategory::Ui);
        let config = self.get_category_config(category).unwrap_or_default();

        let variation = (rand::random::<f32>() - 0.5) * 2.0 * config.pitch_variation;
        (base_pitch + variation).clamp(0.5, 2.0)
    }
}

impl Default for SfxManager {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_category_config() {
        let mut manager = SfxManager::new();
        let config = SfxConfig::default();
        assert!(manager.set_category_config(SfxCategory::Ui, config).is_ok());
        assert!(manager.get_category_config(SfxCategory::Ui).is_some());
    }

    #[test]
    fn test_assign_sound_category() {
        let mut manager = SfxManager::new();
        let handle = SoundSourceHandle::new(1, 1);
        manager.assign_sound_category(handle, SfxCategory::Footsteps);
        assert_eq!(
            manager.get_sound_category(handle),
            Some(SfxCategory::Footsteps)
        );
    }

    #[test]
    fn test_apply_volume_variation() {
        let mut manager = SfxManager::new();
        let config = SfxConfig {
            volume_variation: 0.0,
            ..Default::default()
        };
        assert!(manager.set_category_config(SfxCategory::Ui, config).is_ok());

        let handle = SoundSourceHandle::new(1, 1);
        manager.assign_sound_category(handle, SfxCategory::Ui);

        let volume = manager.apply_volume_variation(0.8, handle);
        assert_eq!(volume, 0.8);
    }

    #[test]
    fn test_apply_pitch_variation() {
        let mut manager = SfxManager::new();
        let config = SfxConfig {
            pitch_variation: 0.0,
            ..Default::default()
        };
        manager
            .set_category_config(SfxCategory::Ui, config)
            .unwrap();

        let handle = SoundSourceHandle::new(1, 1);
        manager.assign_sound_category(handle, SfxCategory::Ui);

        let pitch = manager.apply_pitch_variation(1.0, handle);
        assert_eq!(pitch, 1.0);
    }

    #[test]
    fn test_config_validate_valid() {
        let config = SfxConfig::default();
        assert!(config.validate().is_ok());
    }

    #[test]
    fn test_config_validate_invalid_volume_variation() {
        let config = SfxConfig {
            volume_variation: 1.5,
            ..Default::default()
        };
        assert!(config.validate().is_err());
    }

    #[test]
    fn test_config_validate_invalid_pitch_variation() {
        let config = SfxConfig {
            pitch_variation: 1.5,
            ..Default::default()
        };
        assert!(config.validate().is_err());
    }

    #[test]
    fn test_config_validate_invalid_min_max() {
        let config = SfxConfig {
            min_volume: 0.8,
            max_volume: 0.5,
            ..Default::default()
        };
        assert!(config.validate().is_err());
    }

    #[test]
    fn test_config_builder() {
        let config = SfxConfig::default()
            .with_volume_variation(0.2)
            .with_pitch_variation(0.3)
            .with_min_volume(0.6)
            .with_max_volume(0.9);
        assert_eq!(config.volume_variation, 0.2);
        assert_eq!(config.pitch_variation, 0.3);
        assert_eq!(config.min_volume, 0.6);
        assert_eq!(config.max_volume, 0.9);
    }

    #[test]
    fn test_set_category_config_invalid() {
        let mut manager = SfxManager::new();
        let config = SfxConfig {
            volume_variation: 1.5,
            ..Default::default()
        };
        assert!(manager
            .set_category_config(SfxCategory::Ui, config)
            .is_err());
    }

    #[test]
    fn test_stats_tracking() {
        let mut manager = SfxManager::new();
        assert_eq!(manager.stats().total_sounds, 0);

        let handle1 = SoundSourceHandle::new(1, 1);
        let handle2 = SoundSourceHandle::new(2, 1);
        manager.assign_sound_category(handle1, SfxCategory::Ui);
        manager.assign_sound_category(handle2, SfxCategory::Footsteps);

        assert_eq!(manager.stats().total_sounds, 2);
        assert_eq!(manager.stats().sounds_by_category[0], 1); // Ui
        assert_eq!(manager.stats().sounds_by_category[1], 1); // Footsteps
    }

    #[test]
    fn test_remove_sound_category() {
        let mut manager = SfxManager::new();
        let handle = SoundSourceHandle::new(1, 1);
        manager.assign_sound_category(handle, SfxCategory::Ui);
        assert_eq!(manager.stats().total_sounds, 1);

        assert!(manager.remove_sound_category(handle));
        assert_eq!(manager.stats().total_sounds, 0);
        assert!(!manager.remove_sound_category(handle));
    }

    #[test]
    fn test_reset_stats() {
        let mut manager = SfxManager::new();
        let handle = SoundSourceHandle::new(1, 1);
        manager.assign_sound_category(handle, SfxCategory::Ui);
        assert_eq!(manager.stats().total_sounds, 1);

        manager.reset_stats();
        assert_eq!(manager.stats().total_sounds, 0);
    }
}
