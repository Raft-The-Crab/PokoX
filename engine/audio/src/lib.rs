//! Audio system for the Poko Engine
//!
//! This module provides a comprehensive audio system built on rodio:
//! - Sound source management with 3D spatial audio
//! - Audio mixing and volume control
//! - Sound effects and music playback
//! - Resource management for audio assets
//! - Audio capture and microphone input
//! - Audio effects (reverb, echo, filters)
//! - Audio bus/send effects system
//! - Asset hot-reloading
//! - Integration with the engine's event system

pub use bus::{AudioBus, AudioBusConfig, AudioBusStats, SendEffect};
pub use capture::{AudioCapture, AudioCaptureConfig, AudioCaptureStats};
pub use codecs::{AudioCodec, AudioDecoder, AudioStreamInfo};
pub use effects::{AudioEffect, AudioEffectType, EffectChain, EffectConfig};
pub use engine::{AudioEngine, AudioEngineConfig, AudioEngineStats};
pub use error::AudioError;
pub use listeners::{AudioListener, AudioListenerConfig};
pub use mixer::{AudioMixer, MixerChannel, MixerConfig, MixerStats};
pub use music::{MusicConfig, MusicManager, MusicManagerStats, MusicState};
pub use sfx::{SfxCategory, SfxConfig, SfxManager, SfxManagerStats};
pub use sources::{SoundSource, SoundSourceHandle, SoundSourceRegistry, SoundSourceRegistryStats};
pub use spatial::{AttenuationModel, SpatialAudio, SpatialAudioConfig, SpatialAudioStats};
pub use volume::VolumeControl;

pub mod bus;
pub mod capture;
pub mod codecs;
pub mod effects;
pub mod engine;
pub mod error;
pub mod listeners;
pub mod mixer;
pub mod music;
pub mod sfx;
pub mod sources;
pub mod spatial;
pub mod volume;
