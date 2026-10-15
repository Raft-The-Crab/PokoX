use poko_ids_handles::RegistryError;
use thiserror::Error;

/// Errors that can occur in the audio system
#[derive(Debug, Clone, PartialEq, Error)]
pub enum AudioError {
    #[error("Audio device not found")]
    DeviceNotFound,
    #[error("Failed to initialize audio engine: {0}")]
    InitializationFailed(String),
    #[error("Sound not found: {0}")]
    SoundNotFound(String),
    #[error("Invalid sound handle")]
    InvalidHandle,
    #[error("Failed to load sound: {0}")]
    LoadFailed(String),
    #[error("Sound playback failed: {0}")]
    PlaybackFailed(String),
    #[error("Audio stream error: {0}")]
    StreamError(String),
    #[error("Volume out of range: {0}")]
    VolumeOutOfRange(f32),
    #[error("Pitch out of range: {0}")]
    PitchOutOfRange(f32),
    #[error("Invalid audio format")]
    InvalidFormat,
    #[error("Registry error: {0}")]
    RegistryError(#[from] RegistryError),
}
