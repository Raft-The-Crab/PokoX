use crate::error::AudioError;
use crate::sources::SoundSourceHandle;
use std::collections::HashMap;

/// Music track state
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum MusicState {
    /// Stopped
    Stopped,
    /// Playing
    Playing,
    /// Paused
    Paused,
    /// Fading in
    FadingIn,
    /// Fading out
    FadingOut,
}

/// Music playback configuration
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct MusicConfig {
    /// Fade in duration in seconds
    pub fade_in_duration: f32,
    /// Fade out duration in seconds
    pub fade_out_duration: f32,
    /// Crossfade between tracks
    pub crossfade: bool,
    /// Loop music
    pub loop_track: bool,
}

impl MusicConfig {
    /// Validate the configuration
    pub fn validate(&self) -> Result<(), AudioError> {
        if self.fade_in_duration < 0.0 {
            return Err(AudioError::PlaybackFailed(
                "Fade in duration must be non-negative".to_string(),
            ));
        }
        if self.fade_out_duration < 0.0 {
            return Err(AudioError::PlaybackFailed(
                "Fade out duration must be non-negative".to_string(),
            ));
        }
        Ok(())
    }

    /// Set fade in duration
    pub fn with_fade_in(mut self, duration: f32) -> Self {
        self.fade_in_duration = duration;
        self
    }

    /// Set fade out duration
    pub fn with_fade_out(mut self, duration: f32) -> Self {
        self.fade_out_duration = duration;
        self
    }

    /// Enable/disable crossfade
    pub fn with_crossfade(mut self, enabled: bool) -> Self {
        self.crossfade = enabled;
        self
    }

    /// Enable/disable looping
    pub fn with_looping(mut self, enabled: bool) -> Self {
        self.loop_track = enabled;
        self
    }
}

impl Default for MusicConfig {
    fn default() -> Self {
        Self {
            fade_in_duration: 1.0,
            fade_out_duration: 1.0,
            crossfade: true,
            loop_track: true,
        }
    }
}

/// Music manager statistics
#[derive(Debug, Clone, Copy, Default)]
pub struct MusicManagerStats {
    /// Total tracks played
    pub total_tracks_played: usize,
    /// Total play time in seconds
    pub total_play_time: f32,
    /// Number of tracks in library
    pub library_size: usize,
}

/// Music manager
pub struct MusicManager {
    /// Current music state
    current_state: MusicState,
    /// Current track handle
    current_track: Option<SoundSourceHandle>,
    /// Next track handle (for crossfade)
    next_track: Option<SoundSourceHandle>,
    /// Configuration
    config: MusicConfig,
    /// Track library
    track_library: HashMap<String, SoundSourceHandle>,
    /// Total tracks played
    total_tracks_played: usize,
    /// Current track position in seconds
    current_position: f32,
    /// Total play time in seconds
    total_play_time: f32,
    /// Statistics
    stats: MusicManagerStats,
}

impl MusicManager {
    /// Create a new music manager
    pub fn new() -> Self {
        Self {
            current_state: MusicState::Stopped,
            current_track: None,
            next_track: None,
            config: MusicConfig::default(),
            track_library: HashMap::new(),
            total_tracks_played: 0,
            current_position: 0.0,
            total_play_time: 0.0,
            stats: MusicManagerStats::default(),
        }
    }

    /// Get statistics
    pub fn stats(&self) -> MusicManagerStats {
        self.stats
    }

    /// Reset statistics
    pub fn reset_stats(&mut self) {
        self.stats = MusicManagerStats::default();
    }

    /// Update statistics
    fn update_stats(&mut self) {
        self.stats.total_tracks_played = self.total_tracks_played;
        self.stats.total_play_time = self.total_play_time;
        self.stats.library_size = self.track_library.len();
    }

    /// Play a track by name
    pub fn play_track(&mut self, track_name: &str) -> Result<(), AudioError> {
        let handle = self
            .track_library
            .get(track_name)
            .ok_or_else(|| AudioError::SoundNotFound(track_name.to_string()))?;

        self.current_track = Some(*handle);
        self.current_state = MusicState::Playing;
        self.current_position = 0.0;
        self.total_tracks_played += 1;
        self.update_stats();
        Ok(())
    }

    /// Stop current track
    pub fn stop(&mut self) {
        if self.current_state == MusicState::Playing {
            self.total_play_time += self.current_position;
        }
        self.current_track = None;
        self.next_track = None;
        self.current_state = MusicState::Stopped;
        self.current_position = 0.0;
        self.update_stats();
    }

    /// Pause current track
    pub fn pause(&mut self) {
        if self.current_state == MusicState::Playing {
            self.total_play_time += self.current_position;
            self.current_state = MusicState::Paused;
            self.update_stats();
        }
    }

    /// Resume paused track
    pub fn resume(&mut self) {
        if self.current_state == MusicState::Paused {
            self.current_state = MusicState::Playing;
            self.update_stats();
        }
    }

    /// Get current state
    pub fn state(&self) -> MusicState {
        self.current_state
    }

    /// Get configuration
    pub fn config(&self) -> &MusicConfig {
        &self.config
    }

    /// Set configuration
    pub fn set_config(&mut self, config: MusicConfig) -> Result<(), AudioError> {
        config.validate()?;
        self.config = config;
        Ok(())
    }

    /// Get current position
    pub fn current_position(&self) -> f32 {
        self.current_position
    }

    /// Set current position
    pub fn seek(&mut self, position: f32) {
        self.current_position = position.max(0.0);
    }

    /// Get total tracks played
    pub fn total_tracks_played(&self) -> usize {
        self.stats.total_tracks_played
    }

    /// Get number of tracks in library
    pub fn library_size(&self) -> usize {
        self.stats.library_size
    }

    /// Add track to library
    pub fn add_track(&mut self, name: String, handle: SoundSourceHandle) {
        self.track_library.insert(name, handle);
        self.update_stats();
    }

    /// Remove track from library
    pub fn remove_track(&mut self, name: &str) -> bool {
        let removed = self.track_library.remove(name).is_some();
        if removed {
            self.update_stats();
        }
        removed
    }
}

impl Default for MusicManager {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_music_state() {
        let manager = MusicManager::new();
        assert_eq!(manager.state(), MusicState::Stopped);
    }

    #[test]
    fn test_play_track_not_found() {
        let mut manager = MusicManager::new();
        assert!(manager.play_track("nonexistent").is_err());
    }

    #[test]
    fn test_stop() {
        let mut manager = MusicManager::new();
        manager.stop();
        assert_eq!(manager.state(), MusicState::Stopped);
    }

    #[test]
    fn test_pause_resume() {
        let mut manager = MusicManager::new();
        manager.current_state = MusicState::Playing;
        manager.pause();
        assert_eq!(manager.state(), MusicState::Paused);
        manager.resume();
        assert_eq!(manager.state(), MusicState::Playing);
    }

    #[test]
    fn test_add_remove_track() {
        let mut manager = MusicManager::new();
        let handle = SoundSourceHandle::new(1, 1);
        manager.add_track("test".to_string(), handle);
        assert!(manager.remove_track("test"));
        assert!(!manager.remove_track("test"));
    }

    #[test]
    fn test_config_validate_valid() {
        let config = MusicConfig::default();
        assert!(config.validate().is_ok());
    }

    #[test]
    fn test_config_validate_invalid_fade_in() {
        let config = MusicConfig {
            fade_in_duration: -1.0,
            ..Default::default()
        };
        assert!(config.validate().is_err());
    }

    #[test]
    fn test_config_validate_invalid_fade_out() {
        let config = MusicConfig {
            fade_out_duration: -1.0,
            ..Default::default()
        };
        assert!(config.validate().is_err());
    }

    #[test]
    fn test_config_builder() {
        let config = MusicConfig::default()
            .with_fade_in(2.0)
            .with_fade_out(3.0)
            .with_crossfade(false)
            .with_looping(false);
        assert_eq!(config.fade_in_duration, 2.0);
        assert_eq!(config.fade_out_duration, 3.0);
        assert!(!config.crossfade);
        assert!(!config.loop_track);
    }

    #[test]
    fn test_total_tracks_played() {
        let mut manager = MusicManager::new();
        let handle = SoundSourceHandle::new(1, 1);
        manager.add_track("test".to_string(), handle);
        manager.play_track("test").unwrap();
        assert_eq!(manager.stats().total_tracks_played, 1);
    }

    #[test]
    fn test_current_position() {
        let mut manager = MusicManager::new();
        assert_eq!(manager.current_position(), 0.0);
        manager.seek(10.0);
        assert_eq!(manager.current_position(), 10.0);
    }

    #[test]
    fn test_seek_negative() {
        let mut manager = MusicManager::new();
        manager.seek(-5.0);
        assert_eq!(manager.current_position(), 0.0);
    }

    #[test]
    fn test_library_size() {
        let mut manager = MusicManager::new();
        let handle = SoundSourceHandle::new(1, 1);
        manager.add_track("test".to_string(), handle);
        assert_eq!(manager.library_size(), 1);
    }

    #[test]
    fn test_set_config_valid() {
        let mut manager = MusicManager::new();
        let config = MusicConfig::default();
        assert!(manager.set_config(config).is_ok());
    }

    #[test]
    fn test_set_config_invalid() {
        let mut manager = MusicManager::new();
        let config = MusicConfig {
            fade_in_duration: -1.0,
            ..Default::default()
        };
        assert!(manager.set_config(config).is_err());
    }

    #[test]
    fn test_stats_tracking() {
        let mut manager = MusicManager::new();
        let handle = SoundSourceHandle::new(1, 1);
        manager.add_track("test".to_string(), handle);
        assert_eq!(manager.stats().library_size, 1);

        manager.play_track("test").unwrap();
        assert_eq!(manager.stats().total_tracks_played, 1);
    }

    #[test]
    fn test_total_play_time() {
        let mut manager = MusicManager::new();
        let handle = SoundSourceHandle::new(1, 1);
        manager.add_track("test".to_string(), handle);
        manager.play_track("test").unwrap();
        manager.current_position = 10.0;
        manager.stop();
        assert_eq!(manager.stats().total_play_time, 10.0);
    }

    #[test]
    fn test_reset_stats() {
        let mut manager = MusicManager::new();
        let handle = SoundSourceHandle::new(1, 1);
        manager.add_track("test".to_string(), handle);
        manager.play_track("test").unwrap();
        assert_eq!(manager.stats().total_tracks_played, 1);

        manager.reset_stats();
        assert_eq!(manager.stats().total_tracks_played, 0);
        assert_eq!(manager.stats().library_size, 0);
    }
}
