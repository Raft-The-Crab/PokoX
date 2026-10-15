use crate::error::AudioError;
use crate::listeners::AudioListener;
use crate::mixer::AudioMixer;
use crate::sources::{SoundSource, SoundSourceHandle, SoundSourceRegistry};
use crate::spatial::SpatialAudioConfig;
use glam::Vec3;
use rodio::{Decoder, OutputStream, OutputStreamHandle, Sink};
use std::collections::HashMap;
use std::fs::File;
use std::io::BufReader;

/// Configuration for the audio engine
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct AudioEngineConfig {
    /// Master volume (0.0 to 1.0)
    pub master_volume: f32,
    /// Enable spatial audio by default
    pub enable_spatial: bool,
    /// Default spatial audio configuration
    pub spatial_config: SpatialAudioConfig,
    /// Maximum number of concurrent sounds (0 = unlimited)
    pub max_concurrent_sounds: usize,
    /// Enable statistics tracking
    pub enable_stats: bool,
}

impl Default for AudioEngineConfig {
    fn default() -> Self {
        Self {
            master_volume: 1.0,
            enable_spatial: true,
            spatial_config: SpatialAudioConfig::default(),
            max_concurrent_sounds: 32,
            enable_stats: true,
        }
    }
}

impl AudioEngineConfig {
    /// Validate the configuration
    pub fn validate(&self) -> Result<(), AudioError> {
        if !(0.0..=1.0).contains(&self.master_volume) {
            return Err(AudioError::VolumeOutOfRange(self.master_volume));
        }
        self.spatial_config.validate()?;
        Ok(())
    }

    /// Set master volume
    pub fn with_master_volume(mut self, volume: f32) -> Self {
        self.master_volume = volume;
        self
    }

    /// Enable/disable spatial audio
    pub fn with_spatial(mut self, enabled: bool) -> Self {
        self.enable_spatial = enabled;
        self
    }

    /// Set maximum concurrent sounds
    pub fn with_max_concurrent_sounds(mut self, max: usize) -> Self {
        self.max_concurrent_sounds = max;
        self
    }

    /// Enable/disable statistics
    pub fn with_stats(mut self, enabled: bool) -> Self {
        self.enable_stats = enabled;
        self
    }
}

/// Statistics for the audio engine
#[derive(Debug, Clone, Copy, PartialEq, Default)]
pub struct AudioEngineStats {
    /// Total sounds played since creation
    pub total_played: usize,
    /// Total sounds stopped
    pub total_stopped: usize,
    /// Total sounds paused
    pub total_paused: usize,
    /// Total sounds resumed
    pub total_resumed: usize,
    /// Peak concurrent sounds
    pub peak_concurrent: usize,
}

/// Main audio engine
pub struct AudioEngine {
    /// Output stream handle for rodio
    output_stream_handle: Option<OutputStreamHandle>,
    /// Output stream (kept alive)
    _output_stream: Option<OutputStream>,
    /// Sound source registry
    sources: SoundSourceRegistry,
    /// Active sinks (one per playing sound)
    sinks: HashMap<SoundSourceHandle, Sink>,
    /// Audio mixer
    mixer: AudioMixer,
    /// Configuration
    config: AudioEngineConfig,
    /// Audio listener for spatial audio
    listener: AudioListener,
    /// Statistics
    stats: AudioEngineStats,
}

impl AudioEngine {
    /// Create a new audio engine
    pub fn new() -> Result<Self, AudioError> {
        Self::with_config(AudioEngineConfig::default())
    }

    /// Create a new audio engine with configuration
    pub fn with_config(config: AudioEngineConfig) -> Result<Self, AudioError> {
        config.validate()?;

        let (output_stream, output_stream_handle) = rodio::OutputStream::try_default()
            .map_err(|e| AudioError::InitializationFailed(e.to_string()))?;

        Ok(Self {
            output_stream_handle: Some(output_stream_handle),
            _output_stream: Some(output_stream),
            sources: SoundSourceRegistry::new(),
            sinks: HashMap::new(),
            mixer: AudioMixer::new(),
            config,
            listener: AudioListener::default(),
            stats: AudioEngineStats::default(),
        })
    }

    /// Play a sound from a file
    pub fn play(&mut self, asset_path: &str) -> Result<SoundSourceHandle, AudioError> {
        self.play_on_channel(asset_path, crate::mixer::MixerChannel::Sfx)
    }

    /// Play a sound from a file on a specific mixer channel
    pub fn play_on_channel(
        &mut self,
        asset_path: &str,
        channel: crate::mixer::MixerChannel,
    ) -> Result<SoundSourceHandle, AudioError> {
        // Check concurrent sound limit
        if self.config.max_concurrent_sounds > 0
            && self.sinks.len() >= self.config.max_concurrent_sounds
        {
            return Err(AudioError::PlaybackFailed(
                "Maximum concurrent sounds reached".to_string(),
            ));
        }

        let handle = self.sources.allocate(asset_path.to_string())?;

        // Assign to specified mixer channel
        self.mixer.assign_sound_to_channel(handle, channel);

        if let Some(output_handle) = &self.output_stream_handle {
            // Load the audio file
            let file = File::open(asset_path)
                .map_err(|e| AudioError::LoadFailed(format!("Failed to open file: {}", e)))?;

            let source = Decoder::new(BufReader::new(file))
                .map_err(|e| AudioError::LoadFailed(format!("Failed to decode audio: {}", e)))?;

            // Get source settings
            let source_settings = self.sources.get(handle)?;

            // Create sink
            let sink = Sink::try_new(output_handle)
                .map_err(|e| AudioError::PlaybackFailed(e.to_string()))?;

            // Apply volume (master * channel * source)
            let channel_volume = self.mixer.calculate_sound_volume(handle);
            let final_volume = source_settings.volume * channel_volume * self.config.master_volume;
            sink.set_volume(final_volume);

            // Apply pitch (speed)
            sink.set_speed(source_settings.pitch);

            // Note: Looping support requires re-appending source when sink ends
            // This would need a callback mechanism or polling
            // For now, we store the looping flag in the source but don't implement
            // automatic re-append (complex with rodio's Sink API)

            // Append the source to the sink
            sink.append(source);

            // Start playback
            sink.play();

            self.sinks.insert(handle, sink);

            // Update statistics
            if self.config.enable_stats {
                self.stats.total_played += 1;
                self.stats.peak_concurrent = self.stats.peak_concurrent.max(self.sinks.len());
            }
        }

        Ok(handle)
    }

    /// Stop a sound
    pub fn stop(&mut self, handle: SoundSourceHandle) -> Result<(), AudioError> {
        if let Some(sink) = self.sinks.remove(&handle) {
            sink.stop();
            self.mixer.remove_sound_from_channel(handle);
            if self.config.enable_stats {
                self.stats.total_stopped += 1;
            }
        }
        Ok(())
    }

    /// Pause a sound
    pub fn pause(&mut self, handle: SoundSourceHandle) -> Result<(), AudioError> {
        if let Some(sink) = self.sinks.get(&handle) {
            sink.pause();
            if self.config.enable_stats {
                self.stats.total_paused += 1;
            }
        }
        Ok(())
    }

    /// Resume a paused sound
    pub fn resume(&mut self, handle: SoundSourceHandle) -> Result<(), AudioError> {
        if let Some(sink) = self.sinks.get(&handle) {
            sink.play();
            if self.config.enable_stats {
                self.stats.total_resumed += 1;
            }
        }
        Ok(())
    }

    /// Set volume for a specific sound
    pub fn set_volume(&mut self, handle: SoundSourceHandle, volume: f32) -> Result<(), AudioError> {
        self.sources.get_mut(handle)?.set_volume(volume)?;
        if let Some(sink) = self.sinks.get(&handle) {
            // Apply channel volume and master volume
            let channel_volume = self.mixer.calculate_sound_volume(handle);
            let final_volume = volume * channel_volume * self.config.master_volume;
            sink.set_volume(final_volume);
        }
        Ok(())
    }

    /// Set master volume
    pub fn set_master_volume(&mut self, volume: f32) -> Result<(), AudioError> {
        if !(0.0..=1.0).contains(&volume) {
            return Err(AudioError::VolumeOutOfRange(volume));
        }
        self.config.master_volume = volume;
        // Update all active sinks with master * channel * source volume
        for handle in self.sinks.keys().copied().collect::<Vec<_>>() {
            if let Ok(source) = self.sources.get(handle) {
                if let Some(sink) = self.sinks.get(&handle) {
                    let channel_volume = self.mixer.calculate_sound_volume(handle);
                    let final_volume = source.volume * channel_volume * volume;
                    sink.set_volume(final_volume);
                }
            }
        }
        Ok(())
    }

    /// Get a sound source
    pub fn get_source(&self, handle: SoundSourceHandle) -> Result<&SoundSource, AudioError> {
        self.sources.get(handle)
    }

    /// Get a mutable sound source
    pub fn get_source_mut(
        &mut self,
        handle: SoundSourceHandle,
    ) -> Result<&mut SoundSource, AudioError> {
        self.sources.get_mut(handle)
    }

    /// Get the audio mixer
    pub fn mixer(&self) -> &AudioMixer {
        &self.mixer
    }

    /// Get a mutable reference to the audio mixer
    pub fn mixer_mut(&mut self) -> &mut AudioMixer {
        &mut self.mixer
    }

    /// Get the audio listener
    pub fn listener(&self) -> &AudioListener {
        &self.listener
    }

    /// Get a mutable reference to the audio listener
    pub fn listener_mut(&mut self) -> &mut AudioListener {
        &mut self.listener
    }

    /// Set listener position for spatial audio
    pub fn set_listener_position(&mut self, position: Vec3) {
        self.listener.set_position(position);
    }

    /// Set listener velocity for Doppler effect
    pub fn set_listener_velocity(&mut self, velocity: Vec3) {
        self.listener.set_velocity(velocity);
    }

    /// Update spatial audio for all sources
    pub fn update_spatial(&mut self) {
        for handle in self.sinks.keys().copied().collect::<Vec<_>>() {
            if let Ok(source) = self.sources.get(handle) {
                if let Some(sink) = self.sinks.get(&handle) {
                    // Calculate spatial attenuation
                    let attenuation = source.calculate_spatial_attenuation(self.listener.position);

                    // Apply channel volume
                    let channel_volume = self.mixer.calculate_sound_volume(handle);

                    // Apply master volume, channel volume, and spatial attenuation
                    let final_volume = attenuation * channel_volume * self.config.master_volume;
                    sink.set_volume(final_volume);

                    // Apply Doppler effect to pitch if spatial audio is enabled
                    if self.config.enable_spatial {
                        if let Some(spatial) = &source.spatial {
                            let doppler_factor = spatial
                                .calculate_doppler(self.listener.position, self.listener.velocity);
                            // Apply doppler to the sink's speed (affects pitch)
                            let current_speed = sink.speed();
                            sink.set_speed(current_speed * doppler_factor);
                        }
                    }
                }
            }
        }
    }

    /// Clean up finished sounds (sinks that have finished playing)
    pub fn cleanup_finished(&mut self) {
        let mut finished_handles = Vec::new();
        for (handle, sink) in &self.sinks {
            if sink.empty() {
                finished_handles.push(*handle);
            }
        }
        for handle in finished_handles {
            self.sinks.remove(&handle);
            self.mixer.remove_sound_from_channel(handle);
            if self.config.enable_stats {
                self.stats.total_stopped += 1;
            }
        }
    }

    /// Get number of active sounds
    pub fn active_sound_count(&self) -> usize {
        self.sinks.len()
    }

    /// Check if a sound is currently playing
    pub fn is_playing(&self, handle: SoundSourceHandle) -> bool {
        self.sinks.contains_key(&handle)
    }

    /// Stop all sounds
    pub fn stop_all(&mut self) {
        for sink in self.sinks.values() {
            sink.stop();
        }
        self.sinks.clear();
    }

    /// Get configuration
    pub fn config(&self) -> &AudioEngineConfig {
        &self.config
    }

    /// Set configuration
    pub fn set_config(&mut self, config: AudioEngineConfig) -> Result<(), AudioError> {
        config.validate()?;
        self.config = config;
        Ok(())
    }

    /// Get statistics
    pub fn stats(&self) -> AudioEngineStats {
        self.stats
    }

    /// Reset statistics
    pub fn reset_stats(&mut self) {
        self.stats = AudioEngineStats::default();
    }
}

impl Default for AudioEngine {
    fn default() -> Self {
        Self::new().unwrap()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_config_default() {
        let config = AudioEngineConfig::default();
        assert_eq!(config.master_volume, 1.0);
        assert!(config.enable_spatial);
        assert_eq!(config.max_concurrent_sounds, 32);
        assert!(config.enable_stats);
    }

    #[test]
    fn test_config_validate_valid() {
        let config = AudioEngineConfig::default();
        assert!(config.validate().is_ok());
    }

    #[test]
    fn test_config_validate_invalid_volume() {
        let config = AudioEngineConfig {
            master_volume: 1.5,
            ..Default::default()
        };
        assert!(config.validate().is_err());
    }

    #[test]
    fn test_config_builder() {
        let config = AudioEngineConfig::default()
            .with_master_volume(0.5)
            .with_spatial(false)
            .with_max_concurrent_sounds(16)
            .with_stats(false);
        assert_eq!(config.master_volume, 0.5);
        assert!(!config.enable_spatial);
        assert_eq!(config.max_concurrent_sounds, 16);
        assert!(!config.enable_stats);
    }

    #[test]
    fn test_set_master_volume_valid() {
        let mut engine = AudioEngine::new().unwrap();
        assert!(engine.set_master_volume(0.5).is_ok());
    }

    #[test]
    fn test_set_master_volume_invalid() {
        let mut engine = AudioEngine::new().unwrap();
        assert!(engine.set_master_volume(1.5).is_err());
    }

    #[test]
    fn test_mixer_integration() {
        let mut engine = AudioEngine::new().unwrap();
        assert!(engine
            .mixer_mut()
            .set_channel_volume(crate::mixer::MixerChannel::Music, 0.5)
            .is_ok());
        assert_eq!(
            engine
                .mixer()
                .get_channel_volume(crate::mixer::MixerChannel::Music),
            0.5
        );
    }

    #[test]
    fn test_listener_position() {
        let mut engine = AudioEngine::new().unwrap();
        engine.set_listener_position(Vec3::new(1.0, 2.0, 3.0));
        assert_eq!(engine.listener().position, Vec3::new(1.0, 2.0, 3.0));
    }

    #[test]
    fn test_listener_velocity() {
        let mut engine = AudioEngine::new().unwrap();
        engine.set_listener_velocity(Vec3::new(1.0, 2.0, 3.0));
        assert_eq!(engine.listener().velocity, Vec3::new(1.0, 2.0, 3.0));
    }

    #[test]
    fn test_active_sound_count() {
        let engine = AudioEngine::new().unwrap();
        assert_eq!(engine.active_sound_count(), 0);
    }

    #[test]
    fn test_is_playing() {
        let engine = AudioEngine::new().unwrap();
        let handle = SoundSourceHandle::new(1, 1);
        assert!(!engine.is_playing(handle));
    }

    #[test]
    fn test_stop_all() {
        let mut engine = AudioEngine::new().unwrap();
        engine.stop_all();
        assert_eq!(engine.active_sound_count(), 0);
    }

    #[test]
    fn test_config_accessor() {
        let engine = AudioEngine::new().unwrap();
        let config = engine.config();
        assert_eq!(config.master_volume, 1.0);
    }

    #[test]
    fn test_set_config_valid() {
        let mut engine = AudioEngine::new().unwrap();
        let new_config = AudioEngineConfig::default().with_master_volume(0.5);
        assert!(engine.set_config(new_config).is_ok());
        assert_eq!(engine.config().master_volume, 0.5);
    }

    #[test]
    fn test_set_config_invalid() {
        let mut engine = AudioEngine::new().unwrap();
        let new_config = AudioEngineConfig {
            master_volume: 1.5,
            ..Default::default()
        };
        assert!(engine.set_config(new_config).is_err());
    }

    #[test]
    fn test_stats() {
        let engine = AudioEngine::new().unwrap();
        let stats = engine.stats();
        assert_eq!(stats.total_played, 0);
        assert_eq!(stats.total_stopped, 0);
    }

    #[test]
    fn test_reset_stats() {
        let mut engine = AudioEngine::new().unwrap();
        engine.reset_stats();
        let stats = engine.stats();
        assert_eq!(stats.total_played, 0);
    }

    #[test]
    fn test_max_concurrent_sounds() {
        let config = AudioEngineConfig::default()
            .with_max_concurrent_sounds(2)
            .with_stats(false);
        let mut engine = AudioEngine::with_config(config).unwrap();

        // Test limit logic without actual file loading
        // Manually add sinks to test the limit
        use rodio::{OutputStream, Sink};
        if let Ok((_stream, handle)) = OutputStream::try_default() {
            engine.output_stream_handle = Some(handle);

            // Add first two sinks (should succeed)
            for i in 0..2 {
                let sink = Sink::try_new(engine.output_stream_handle.as_ref().unwrap()).unwrap();
                engine
                    .sinks
                    .insert(SoundSourceHandle::new(i as u16, 1), sink);
            }

            // Third should fail due to limit
            let result = engine.play("test3.wav");
            assert!(result.is_err());
            assert!(matches!(result.unwrap_err(), AudioError::PlaybackFailed(_)));
        }
    }

    #[test]
    fn test_cleanup_finished() {
        let mut engine = AudioEngine::new().unwrap();
        engine.stop_all();
        engine.cleanup_finished();
        assert_eq!(engine.active_sound_count(), 0);
    }

    #[test]
    fn test_spatial_update_with_doppler() {
        let mut engine = AudioEngine::new().unwrap();
        engine.set_listener_position(Vec3::new(0.0, 0.0, 0.0));
        engine.set_listener_velocity(Vec3::new(5.0, 0.0, 0.0));
        // Test that spatial update runs without error
        engine.update_spatial();
    }

    #[test]
    fn test_listener_accessor() {
        let engine = AudioEngine::new().unwrap();
        let listener = engine.listener();
        assert_eq!(listener.position, Vec3::ZERO);
    }

    #[test]
    fn test_listener_mut() {
        let mut engine = AudioEngine::new().unwrap();
        engine.listener_mut().set_position(Vec3::new(1.0, 2.0, 3.0));
        assert_eq!(engine.listener().position, Vec3::new(1.0, 2.0, 3.0));
    }
}
