use crate::error::AudioError;
use crate::spatial::SpatialAudio;
use glam::Vec3;
use poko_ids_handles::{Handle, HandleRegistry, Id, IdType};
use std::collections::HashMap;

/// Unique handle for a sound source
pub type SoundSourceHandle = Handle;

/// Sound source state
#[derive(Debug, Clone)]
pub struct SoundSource {
    /// Unique ID
    pub id: Id,
    /// Handle for registry access
    pub handle: SoundSourceHandle,
    /// Sound asset path
    pub asset_path: String,
    /// Volume (0.0 to 1.0)
    pub volume: f32,
    /// Pitch multiplier (0.5 to 2.0)
    pub pitch: f32,
    /// Whether sound is looping
    pub looping: bool,
    /// Whether sound is paused
    pub paused: bool,
    /// Spatial audio state (if applicable)
    pub spatial: Option<SpatialAudio>,
    /// Play position in seconds
    pub position: f32,
}

impl SoundSource {
    /// Create a new sound source
    pub fn new(handle: SoundSourceHandle, asset_path: String) -> Self {
        Self {
            id: Id::new(IdType::Sound),
            handle,
            asset_path,
            volume: 1.0,
            pitch: 1.0,
            looping: false,
            paused: false,
            spatial: None,
            position: 0.0,
        }
    }

    /// Set volume
    pub fn set_volume(&mut self, volume: f32) -> Result<(), AudioError> {
        if !(0.0..=1.0).contains(&volume) {
            return Err(AudioError::VolumeOutOfRange(volume));
        }
        self.volume = volume;
        Ok(())
    }

    /// Set pitch
    pub fn set_pitch(&mut self, pitch: f32) -> Result<(), AudioError> {
        if !(0.5..=2.0).contains(&pitch) {
            return Err(AudioError::PitchOutOfRange(pitch));
        }
        self.pitch = pitch;
        Ok(())
    }

    /// Enable looping
    pub fn set_looping(&mut self, looping: bool) {
        self.looping = looping;
    }

    /// Pause/unpause
    pub fn set_paused(&mut self, paused: bool) {
        self.paused = paused;
    }

    /// Set spatial audio
    pub fn set_spatial(&mut self, spatial: SpatialAudio) {
        self.spatial = Some(spatial);
    }

    /// Clear spatial audio (make it 2D)
    pub fn clear_spatial(&mut self) {
        self.spatial = None;
    }

    /// Update spatial position
    pub fn update_position(&mut self, position: Vec3) {
        if let Some(ref mut spatial) = self.spatial {
            spatial.set_position(position);
        }
    }

    /// Update spatial velocity
    pub fn update_velocity(&mut self, velocity: Vec3) {
        if let Some(ref mut spatial) = self.spatial {
            spatial.set_velocity(velocity);
        }
    }

    /// Calculate spatial attenuation (immutable)
    pub fn calculate_spatial_attenuation(&self, listener_position: Vec3) -> f32 {
        if let Some(spatial) = &self.spatial {
            spatial.calculate_attenuation(listener_position)
        } else {
            1.0
        }
    }

    /// Calculate spatial attenuation with statistics tracking
    pub fn calculate_spatial_attenuation_with_stats(&mut self, listener_position: Vec3) -> f32 {
        if let Some(ref mut spatial) = self.spatial {
            spatial.calculate_attenuation_with_stats(listener_position)
        } else {
            1.0
        }
    }

    /// Calculate doppler shift
    pub fn calculate_doppler_shift(
        &mut self,
        listener_position: Vec3,
        listener_velocity: Vec3,
    ) -> f32 {
        if let Some(ref mut spatial) = self.spatial {
            spatial.calculate_doppler(listener_position, listener_velocity)
        } else {
            1.0
        }
    }
}

/// Sound source registry statistics
#[derive(Debug, Clone, Copy, Default)]
pub struct SoundSourceRegistryStats {
    /// Total sound sources
    pub total_sources: usize,
    /// Number of spatial sources
    pub spatial_sources: usize,
    /// Number of looping sources
    pub looping_sources: usize,
    /// Number of paused sources
    pub paused_sources: usize,
}

/// Registry for managing sound sources
pub struct SoundSourceRegistry {
    sources: HandleRegistry<SoundSource>,
    id_to_handle: HashMap<Id, SoundSourceHandle>,
    stats: SoundSourceRegistryStats,
}

impl SoundSourceRegistry {
    /// Create a new sound source registry
    pub fn new() -> Self {
        Self {
            sources: HandleRegistry::new(),
            id_to_handle: HashMap::new(),
            stats: SoundSourceRegistryStats::default(),
        }
    }

    /// Get statistics
    pub fn stats(&self) -> SoundSourceRegistryStats {
        self.stats
    }

    /// Reset statistics
    pub fn reset_stats(&mut self) {
        self.stats = SoundSourceRegistryStats::default();
    }

    /// Update statistics
    fn update_stats(&mut self) {
        self.stats.total_sources = self.sources.len();
        self.stats.spatial_sources = 0;
        self.stats.looping_sources = 0;
        self.stats.paused_sources = 0;

        for source in self.sources.values() {
            if source.spatial.is_some() {
                self.stats.spatial_sources += 1;
            }
            if source.looping {
                self.stats.looping_sources += 1;
            }
            if source.paused {
                self.stats.paused_sources += 1;
            }
        }
    }

    /// Allocate a new sound source
    pub fn allocate(&mut self, asset_path: String) -> Result<SoundSourceHandle, AudioError> {
        let handle = self
            .sources
            .allocate(SoundSource::new(Handle::null(), asset_path))?;

        // Update the handle in the source
        if let Ok(source) = self.sources.get_mut(handle) {
            source.handle = handle;
            let id = source.id;
            self.id_to_handle.insert(id, handle);
        }

        self.update_stats();
        Ok(handle)
    }

    /// Get a sound source by handle
    pub fn get(&self, handle: SoundSourceHandle) -> Result<&SoundSource, AudioError> {
        self.sources
            .get(handle)
            .map_err(|_| AudioError::InvalidHandle)
    }

    /// Get a mutable sound source by handle
    pub fn get_mut(&mut self, handle: SoundSourceHandle) -> Result<&mut SoundSource, AudioError> {
        self.sources
            .get_mut(handle)
            .map_err(|_| AudioError::InvalidHandle)
    }

    /// Get a sound source by ID
    pub fn get_by_id(&self, id: Id) -> Result<&SoundSource, AudioError> {
        let handle = self
            .id_to_handle
            .get(&id)
            .ok_or(AudioError::SoundNotFound(id.to_string()))?;
        self.get(*handle)
    }

    /// Get a mutable sound source by ID
    pub fn get_mut_by_id(&mut self, id: Id) -> Result<&mut SoundSource, AudioError> {
        let handle = self
            .id_to_handle
            .get(&id)
            .ok_or(AudioError::SoundNotFound(id.to_string()))?;
        self.get_mut(*handle)
    }

    /// Remove a sound source
    pub fn remove(&mut self, handle: SoundSourceHandle) -> Result<(), AudioError> {
        let id = self.get(handle)?.id;
        self.id_to_handle.remove(&id);
        self.sources
            .remove(handle)
            .map_err(|_| AudioError::InvalidHandle)?;
        self.update_stats();
        Ok(())
    }

    /// Get all sound sources
    pub fn all(&self) -> Vec<&SoundSource> {
        self.sources.values()
    }

    /// Get number of sound sources
    pub fn len(&self) -> usize {
        self.sources.len()
    }

    /// Check if registry is empty
    pub fn is_empty(&self) -> bool {
        self.sources.is_empty()
    }

    /// Manually update statistics (call after modifying source properties)
    pub fn update_statistics(&mut self) {
        self.update_stats();
    }
}

impl Default for SoundSourceRegistry {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_sound_source_creation() {
        let handle = Handle::null();
        let source = SoundSource::new(handle, "test.wav".to_string());
        assert_eq!(source.asset_path, "test.wav");
        assert_eq!(source.volume, 1.0);
        assert_eq!(source.pitch, 1.0);
        assert!(!source.looping);
        assert!(!source.paused);
    }

    #[test]
    fn test_set_volume_valid() {
        let handle = Handle::null();
        let mut source = SoundSource::new(handle, "test.wav".to_string());
        assert!(source.set_volume(0.5).is_ok());
        assert_eq!(source.volume, 0.5);
    }

    #[test]
    fn test_set_volume_invalid() {
        let handle = Handle::null();
        let mut source = SoundSource::new(handle, "test.wav".to_string());
        assert!(source.set_volume(1.5).is_err());
        assert!(source.set_volume(-0.1).is_err());
    }

    #[test]
    fn test_set_pitch_valid() {
        let handle = Handle::null();
        let mut source = SoundSource::new(handle, "test.wav".to_string());
        assert!(source.set_pitch(1.5).is_ok());
        assert_eq!(source.pitch, 1.5);
    }

    #[test]
    fn test_set_pitch_invalid() {
        let handle = Handle::null();
        let mut source = SoundSource::new(handle, "test.wav".to_string());
        assert!(source.set_pitch(2.5).is_err());
        assert!(source.set_pitch(0.3).is_err());
    }

    #[test]
    fn test_looping() {
        let handle = Handle::null();
        let mut source = SoundSource::new(handle, "test.wav".to_string());
        source.set_looping(true);
        assert!(source.looping);
    }

    #[test]
    fn test_paused() {
        let handle = Handle::null();
        let mut source = SoundSource::new(handle, "test.wav".to_string());
        source.set_paused(true);
        assert!(source.paused);
    }

    #[test]
    fn test_spatial() {
        let handle = Handle::null();
        let mut source = SoundSource::new(handle, "test.wav".to_string());
        let spatial = SpatialAudio::new(Vec3::new(1.0, 2.0, 3.0));
        source.set_spatial(spatial);
        assert!(source.spatial.is_some());
    }

    #[test]
    fn test_clear_spatial() {
        let handle = Handle::null();
        let mut source = SoundSource::new(handle, "test.wav".to_string());
        let spatial = SpatialAudio::new(Vec3::ZERO);
        source.set_spatial(spatial);
        source.clear_spatial();
        assert!(source.spatial.is_none());
    }

    #[test]
    fn test_registry_allocation() {
        let mut registry = SoundSourceRegistry::new();
        let handle = registry.allocate("test.wav".to_string()).unwrap();
        assert!(registry.get(handle).is_ok());
    }

    #[test]
    fn test_registry_get_invalid() {
        let registry = SoundSourceRegistry::new();
        assert!(registry.get(Handle::null()).is_err());
    }

    #[test]
    fn test_registry_remove() {
        let mut registry = SoundSourceRegistry::new();
        let handle = registry.allocate("test.wav".to_string()).unwrap();
        assert!(registry.remove(handle).is_ok());
        assert!(registry.get(handle).is_err());
    }

    #[test]
    fn test_registry_len() {
        let mut registry = SoundSourceRegistry::new();
        assert_eq!(registry.len(), 0);
        registry.allocate("test1.wav".to_string()).unwrap();
        registry.allocate("test2.wav".to_string()).unwrap();
        assert_eq!(registry.len(), 2);
    }

    #[test]
    fn test_stats_tracking() {
        let mut registry = SoundSourceRegistry::new();
        assert_eq!(registry.stats().total_sources, 0);

        let handle1 = registry.allocate("test1.wav".to_string()).unwrap();
        let handle2 = registry.allocate("test2.wav".to_string()).unwrap();
        assert_eq!(registry.stats().total_sources, 2);

        // Make one spatial
        if let Ok(source) = registry.get_mut(handle1) {
            source.set_spatial(SpatialAudio::new(Vec3::ZERO));
        }
        registry.update_statistics();
        assert_eq!(registry.stats().spatial_sources, 1);

        // Make one looping
        if let Ok(source) = registry.get_mut(handle2) {
            source.set_looping(true);
        }
        registry.update_statistics();
        assert_eq!(registry.stats().looping_sources, 1);

        // Make one paused
        if let Ok(source) = registry.get_mut(handle1) {
            source.set_paused(true);
        }
        registry.update_statistics();
        assert_eq!(registry.stats().paused_sources, 1);
    }

    #[test]
    fn test_reset_stats() {
        let mut registry = SoundSourceRegistry::new();
        registry.allocate("test.wav".to_string()).unwrap();
        assert_eq!(registry.stats().total_sources, 1);

        registry.reset_stats();
        assert_eq!(registry.stats().total_sources, 0);
    }

    #[test]
    fn test_calculate_spatial_attenuation() {
        let handle = Handle::null();
        let mut source = SoundSource::new(handle, "test.wav".to_string());
        source.set_spatial(SpatialAudio::new(Vec3::ZERO));

        let attenuation = source.calculate_spatial_attenuation(Vec3::new(5.0, 0.0, 0.0));
        assert!(attenuation > 0.0 && attenuation <= 1.0);
    }

    #[test]
    fn test_calculate_spatial_attenuation_no_spatial() {
        let handle = Handle::null();
        let source = SoundSource::new(handle, "test.wav".to_string());

        let attenuation = source.calculate_spatial_attenuation(Vec3::new(5.0, 0.0, 0.0));
        assert_eq!(attenuation, 1.0);
    }

    #[test]
    fn test_calculate_doppler_shift() {
        let handle = Handle::null();
        let mut source = SoundSource::new(handle, "test.wav".to_string());
        source.set_spatial(SpatialAudio::new(Vec3::ZERO));
        source.update_velocity(Vec3::new(10.0, 0.0, 0.0));

        let doppler = source.calculate_doppler_shift(Vec3::new(10.0, 0.0, 0.0), Vec3::ZERO);
        assert!(doppler > 1.0);
    }

    #[test]
    fn test_calculate_doppler_shift_no_spatial() {
        let handle = Handle::null();
        let mut source = SoundSource::new(handle, "test.wav".to_string());

        let doppler = source.calculate_doppler_shift(Vec3::new(10.0, 0.0, 0.0), Vec3::ZERO);
        assert_eq!(doppler, 1.0);
    }
}
