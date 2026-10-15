use crate::error::AudioError;
use crate::sources::SoundSourceHandle;
use std::collections::HashMap;

/// Audio mixer channel
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum MixerChannel {
    /// Master channel
    Master,
    /// Sound effects channel
    Sfx,
    /// Music channel
    Music,
    /// Voice channel
    Voice,
    /// Ambient channel
    Ambient,
    /// Custom channel with index
    Custom(u8),
}

/// Mixer configuration
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct MixerConfig {
    /// Enable ducking (lowering volume of other channels when one plays)
    pub enable_ducking: bool,
    /// Ducking amount (0.0 to 1.0)
    pub ducking_amount: f32,
    /// Ducking time in seconds
    pub ducking_time: f32,
}

impl MixerConfig {
    /// Validate the configuration
    pub fn validate(&self) -> Result<(), AudioError> {
        if !(0.0..=1.0).contains(&self.ducking_amount) {
            return Err(AudioError::VolumeOutOfRange(self.ducking_amount));
        }
        if self.ducking_time < 0.0 {
            return Err(AudioError::PlaybackFailed(
                "Ducking time must be non-negative".to_string(),
            ));
        }
        Ok(())
    }

    /// Set ducking amount
    pub fn with_ducking_amount(mut self, amount: f32) -> Self {
        self.ducking_amount = amount;
        self
    }

    /// Set ducking time
    pub fn with_ducking_time(mut self, time: f32) -> Self {
        self.ducking_time = time;
        self
    }

    /// Enable/disable ducking
    pub fn with_ducking(mut self, enabled: bool) -> Self {
        self.enable_ducking = enabled;
        self
    }
}

impl Default for MixerConfig {
    fn default() -> Self {
        Self {
            enable_ducking: false,
            ducking_amount: 0.5,
            ducking_time: 0.1,
        }
    }
}

/// Audio mixer statistics
#[derive(Debug, Clone, Copy, Default)]
pub struct MixerStats {
    /// Total sounds mixed
    pub total_sounds_mixed: usize,
    /// Sounds per channel
    pub sounds_by_channel: [usize; 5], // Master, Sfx, Music, Voice, Ambient
}

/// Audio mixer for managing channels and volume
pub struct AudioMixer {
    /// Channel volumes
    channel_volumes: HashMap<MixerChannel, f32>,
    /// Sound to channel mapping
    sound_channels: HashMap<SoundSourceHandle, MixerChannel>,
    /// Configuration
    config: MixerConfig,
    /// Statistics
    stats: MixerStats,
}

impl AudioMixer {
    /// Create a new audio mixer
    pub fn new() -> Self {
        let mut channel_volumes = HashMap::new();
        channel_volumes.insert(MixerChannel::Master, 1.0);
        channel_volumes.insert(MixerChannel::Sfx, 1.0);
        channel_volumes.insert(MixerChannel::Music, 1.0);
        channel_volumes.insert(MixerChannel::Voice, 1.0);
        channel_volumes.insert(MixerChannel::Ambient, 1.0);

        Self {
            channel_volumes,
            sound_channels: HashMap::new(),
            config: MixerConfig::default(),
            stats: MixerStats::default(),
        }
    }

    /// Get statistics
    pub fn stats(&self) -> MixerStats {
        self.stats
    }

    /// Reset statistics
    pub fn reset_stats(&mut self) {
        self.stats = MixerStats::default();
    }

    /// Update statistics
    fn update_stats(&mut self) {
        self.stats.total_sounds_mixed = self.sound_channels.len();
        self.stats.sounds_by_channel = [0; 5];
        for channel in self.sound_channels.values() {
            match channel {
                MixerChannel::Master => self.stats.sounds_by_channel[0] += 1,
                MixerChannel::Sfx => self.stats.sounds_by_channel[1] += 1,
                MixerChannel::Music => self.stats.sounds_by_channel[2] += 1,
                MixerChannel::Voice => self.stats.sounds_by_channel[3] += 1,
                MixerChannel::Ambient => self.stats.sounds_by_channel[4] += 1,
                MixerChannel::Custom(_) => {}
            }
        }
    }

    /// Set channel volume
    pub fn set_channel_volume(
        &mut self,
        channel: MixerChannel,
        volume: f32,
    ) -> Result<(), AudioError> {
        if !(0.0..=1.0).contains(&volume) {
            return Err(AudioError::VolumeOutOfRange(volume));
        }
        self.channel_volumes.insert(channel, volume);
        Ok(())
    }

    /// Get channel volume
    pub fn get_channel_volume(&self, channel: MixerChannel) -> f32 {
        *self.channel_volumes.get(&channel).unwrap_or(&1.0)
    }

    /// Assign sound to channel
    pub fn assign_sound_to_channel(&mut self, handle: SoundSourceHandle, channel: MixerChannel) {
        self.sound_channels.insert(handle, channel);
        self.update_stats();
    }

    /// Remove sound from channel
    pub fn remove_sound_from_channel(&mut self, handle: SoundSourceHandle) {
        let removed = self.sound_channels.remove(&handle).is_some();
        if removed {
            self.update_stats();
        }
    }

    /// Get sound's channel
    pub fn get_sound_channel(&self, handle: SoundSourceHandle) -> Option<MixerChannel> {
        self.sound_channels.get(&handle).copied()
    }

    /// Calculate final volume for a sound (channel volume * master volume)
    pub fn calculate_sound_volume(&self, handle: SoundSourceHandle) -> f32 {
        let channel = self
            .get_sound_channel(handle)
            .unwrap_or(MixerChannel::Master);
        let channel_volume = self.get_channel_volume(channel);
        let master_volume = self.get_channel_volume(MixerChannel::Master);
        channel_volume * master_volume
    }

    /// Get configuration
    pub fn config(&self) -> &MixerConfig {
        &self.config
    }

    /// Set configuration
    pub fn set_config(&mut self, config: MixerConfig) -> Result<(), AudioError> {
        config.validate()?;
        self.config = config;
        Ok(())
    }

    /// Get total sounds mixed
    pub fn total_sounds_mixed(&self) -> usize {
        self.stats.total_sounds_mixed
    }

    /// Get number of sounds in each channel
    pub fn channel_sound_count(&self) -> HashMap<MixerChannel, usize> {
        let mut counts = HashMap::new();
        counts.insert(MixerChannel::Master, self.stats.sounds_by_channel[0]);
        counts.insert(MixerChannel::Sfx, self.stats.sounds_by_channel[1]);
        counts.insert(MixerChannel::Music, self.stats.sounds_by_channel[2]);
        counts.insert(MixerChannel::Voice, self.stats.sounds_by_channel[3]);
        counts.insert(MixerChannel::Ambient, self.stats.sounds_by_channel[4]);
        counts
    }
}

impl Default for AudioMixer {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_channel_volume() {
        let mut mixer = AudioMixer::new();
        assert!(mixer.set_channel_volume(MixerChannel::Sfx, 0.5).is_ok());
        assert_eq!(mixer.get_channel_volume(MixerChannel::Sfx), 0.5);
    }

    #[test]
    fn test_channel_volume_invalid() {
        let mut mixer = AudioMixer::new();
        assert!(mixer.set_channel_volume(MixerChannel::Sfx, 1.5).is_err());
    }

    #[test]
    fn test_assign_sound_to_channel() {
        let mut mixer = AudioMixer::new();
        let handle = SoundSourceHandle::new(1, 1);
        mixer.assign_sound_to_channel(handle, MixerChannel::Music);
        assert_eq!(mixer.get_sound_channel(handle), Some(MixerChannel::Music));
    }

    #[test]
    fn test_calculate_sound_volume() {
        let mut mixer = AudioMixer::new();
        mixer.set_channel_volume(MixerChannel::Master, 0.8).unwrap();
        mixer.set_channel_volume(MixerChannel::Sfx, 0.5).unwrap();

        let handle = SoundSourceHandle::new(1, 1);
        mixer.assign_sound_to_channel(handle, MixerChannel::Sfx);

        let volume = mixer.calculate_sound_volume(handle);
        assert!((volume - 0.4).abs() < 0.01); // 0.8 * 0.5
    }

    #[test]
    fn test_config_validate_valid() {
        let config = MixerConfig::default();
        assert!(config.validate().is_ok());
    }

    #[test]
    fn test_config_validate_invalid_ducking() {
        let config = MixerConfig {
            ducking_amount: 1.5,
            ..Default::default()
        };
        assert!(config.validate().is_err());
    }

    #[test]
    fn test_config_validate_invalid_time() {
        let config = MixerConfig {
            ducking_time: -1.0,
            ..Default::default()
        };
        assert!(config.validate().is_err());
    }

    #[test]
    fn test_config_builder() {
        let config = MixerConfig::default()
            .with_ducking_amount(0.3)
            .with_ducking_time(0.2)
            .with_ducking(true);
        assert_eq!(config.ducking_amount, 0.3);
        assert_eq!(config.ducking_time, 0.2);
        assert!(config.enable_ducking);
    }

    #[test]
    fn test_remove_sound_from_channel() {
        let mut mixer = AudioMixer::new();
        let handle = SoundSourceHandle::new(1, 1);
        mixer.assign_sound_to_channel(handle, MixerChannel::Music);
        mixer.remove_sound_from_channel(handle);
        assert!(mixer.get_sound_channel(handle).is_none());
    }

    #[test]
    fn test_total_sounds_mixed() {
        let mut mixer = AudioMixer::new();
        let handle1 = SoundSourceHandle::new(1, 1);
        let handle2 = SoundSourceHandle::new(2, 1);
        mixer.assign_sound_to_channel(handle1, MixerChannel::Sfx);
        mixer.assign_sound_to_channel(handle2, MixerChannel::Music);
        assert_eq!(mixer.total_sounds_mixed(), 2);
    }

    #[test]
    fn test_channel_sound_count() {
        let mut mixer = AudioMixer::new();
        let handle1 = SoundSourceHandle::new(1, 1);
        let handle2 = SoundSourceHandle::new(2, 1);
        mixer.assign_sound_to_channel(handle1, MixerChannel::Sfx);
        mixer.assign_sound_to_channel(handle2, MixerChannel::Sfx);

        let counts = mixer.channel_sound_count();
        assert_eq!(*counts.get(&MixerChannel::Sfx).unwrap_or(&0), 2);
    }

    #[test]
    fn test_set_config_valid() {
        let mut mixer = AudioMixer::new();
        let config = MixerConfig::default();
        assert!(mixer.set_config(config).is_ok());
    }

    #[test]
    fn test_set_config_invalid() {
        let mut mixer = AudioMixer::new();
        let config = MixerConfig {
            ducking_amount: 1.5,
            ..Default::default()
        };
        assert!(mixer.set_config(config).is_err());
    }

    #[test]
    fn test_stats_tracking() {
        let mut mixer = AudioMixer::new();
        assert_eq!(mixer.stats().total_sounds_mixed, 0);

        let handle1 = SoundSourceHandle::new(1, 1);
        let handle2 = SoundSourceHandle::new(2, 1);
        mixer.assign_sound_to_channel(handle1, MixerChannel::Sfx);
        mixer.assign_sound_to_channel(handle2, MixerChannel::Music);

        assert_eq!(mixer.stats().total_sounds_mixed, 2);
        assert_eq!(mixer.stats().sounds_by_channel[1], 1); // Sfx
        assert_eq!(mixer.stats().sounds_by_channel[2], 1); // Music
    }

    #[test]
    fn test_reset_stats() {
        let mut mixer = AudioMixer::new();
        let handle = SoundSourceHandle::new(1, 1);
        mixer.assign_sound_to_channel(handle, MixerChannel::Sfx);
        assert_eq!(mixer.stats().total_sounds_mixed, 1);

        mixer.reset_stats();
        assert_eq!(mixer.stats().total_sounds_mixed, 0);
    }
}
