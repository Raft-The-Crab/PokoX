use crate::error::AudioError;
use std::sync::Arc;
use std::sync::Mutex;

/// Configuration for audio capture
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct AudioCaptureConfig {
    /// Sample rate in Hz
    pub sample_rate: u32,
    /// Number of channels (1 = mono, 2 = stereo)
    pub channels: u16,
    /// Buffer size in frames
    pub buffer_size: usize,
    /// Enable statistics tracking
    pub enable_stats: bool,
}

impl Default for AudioCaptureConfig {
    fn default() -> Self {
        Self {
            sample_rate: 44100,
            channels: 2,
            buffer_size: 1024,
            enable_stats: true,
        }
    }
}

impl AudioCaptureConfig {
    /// Validate the configuration
    pub fn validate(&self) -> Result<(), AudioError> {
        if self.sample_rate < 8000 || self.sample_rate > 192000 {
            return Err(AudioError::PlaybackFailed(
                "Sample rate must be between 8000 and 192000 Hz".to_string(),
            ));
        }
        if self.channels < 1 || self.channels > 2 {
            return Err(AudioError::PlaybackFailed(
                "Channels must be 1 (mono) or 2 (stereo)".to_string(),
            ));
        }
        if self.buffer_size < 64 || self.buffer_size > 8192 {
            return Err(AudioError::PlaybackFailed(
                "Buffer size must be between 64 and 8192 frames".to_string(),
            ));
        }
        Ok(())
    }

    /// Set sample rate
    pub fn with_sample_rate(mut self, rate: u32) -> Self {
        self.sample_rate = rate;
        self
    }

    /// Set channels
    pub fn with_channels(mut self, channels: u16) -> Self {
        self.channels = channels;
        self
    }

    /// Set buffer size
    pub fn with_buffer_size(mut self, size: usize) -> Self {
        self.buffer_size = size;
        self
    }

    /// Enable/disable statistics
    pub fn with_stats(mut self, enabled: bool) -> Self {
        self.enable_stats = enabled;
        self
    }
}

/// Statistics for audio capture
#[derive(Debug, Clone, Copy, Default)]
pub struct AudioCaptureStats {
    /// Total frames captured
    pub total_frames: usize,
    /// Total capture duration in seconds
    pub total_duration: f32,
    /// Buffer underruns
    pub buffer_underruns: usize,
    /// Buffer overruns
    pub buffer_overruns: usize,
}

/// Audio capture for microphone input
pub struct AudioCapture {
    /// Configuration
    config: AudioCaptureConfig,
    /// Captured audio data buffer
    buffer: Arc<Mutex<Vec<f32>>>,
    /// Statistics
    stats: AudioCaptureStats,
    /// Is currently capturing
    capturing: bool,
}

impl AudioCapture {
    /// Create a new audio capture instance
    pub fn new() -> Result<Self, AudioError> {
        Self::with_config(AudioCaptureConfig::default())
    }

    /// Create with custom configuration
    pub fn with_config(config: AudioCaptureConfig) -> Result<Self, AudioError> {
        config.validate()?;

        Ok(Self {
            config,
            buffer: Arc::new(Mutex::new(Vec::with_capacity(config.buffer_size))),
            stats: AudioCaptureStats::default(),
            capturing: false,
        })
    }

    /// Start capturing audio
    pub fn start(&mut self) -> Result<(), AudioError> {
        if self.capturing {
            return Err(AudioError::PlaybackFailed("Already capturing".to_string()));
        }

        // Note: Actual capture implementation would use rodio's input stream
        // or a dedicated audio input backend (e.g., cpal).
        // For now, this sets the capturing state flag.
        // TODO: Implement real device capture when rodio or cpal input is integrated.

        self.capturing = true;
        Ok(())
    }

    /// Stop capturing audio
    pub fn stop(&mut self) -> Result<(), AudioError> {
        if !self.capturing {
            return Err(AudioError::PlaybackFailed(
                "Not currently capturing".to_string(),
            ));
        }

        self.capturing = false;
        Ok(())
    }

    /// Get captured audio data
    pub fn get_buffer(&self) -> Vec<f32> {
        let buffer = self.buffer.lock().unwrap();
        buffer.clone()
    }

    /// Clear the capture buffer
    pub fn clear_buffer(&mut self) {
        let mut buffer = self.buffer.lock().unwrap();
        buffer.clear();
    }

    /// Check if currently capturing
    pub fn is_capturing(&self) -> bool {
        self.capturing
    }

    /// Get configuration
    pub fn config(&self) -> &AudioCaptureConfig {
        &self.config
    }

    /// Set configuration
    pub fn set_config(&mut self, config: AudioCaptureConfig) -> Result<(), AudioError> {
        config.validate()?;
        self.config = config;
        Ok(())
    }

    /// Get statistics
    pub fn stats(&self) -> AudioCaptureStats {
        self.stats
    }

    /// Reset statistics
    pub fn reset_stats(&mut self) {
        self.stats = AudioCaptureStats::default();
    }

    /// Get shared buffer reference (for external processing)
    pub fn buffer(&self) -> Arc<Mutex<Vec<f32>>> {
        Arc::clone(&self.buffer)
    }
}

impl Default for AudioCapture {
    fn default() -> Self {
        Self::new().unwrap()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_config_default() {
        let config = AudioCaptureConfig::default();
        assert_eq!(config.sample_rate, 44100);
        assert_eq!(config.channels, 2);
        assert_eq!(config.buffer_size, 1024);
        assert!(config.enable_stats);
    }

    #[test]
    fn test_config_validate_valid() {
        let config = AudioCaptureConfig::default();
        assert!(config.validate().is_ok());
    }

    #[test]
    fn test_config_validate_invalid_sample_rate() {
        let config = AudioCaptureConfig {
            sample_rate: 100,
            ..Default::default()
        };
        assert!(config.validate().is_err());
    }

    #[test]
    fn test_config_validate_invalid_channels() {
        let config = AudioCaptureConfig {
            channels: 3,
            ..Default::default()
        };
        assert!(config.validate().is_err());
    }

    #[test]
    fn test_config_validate_invalid_buffer_size() {
        let config = AudioCaptureConfig {
            buffer_size: 32,
            ..Default::default()
        };
        assert!(config.validate().is_err());
    }

    #[test]
    fn test_config_builder() {
        let config = AudioCaptureConfig::default()
            .with_sample_rate(48000)
            .with_channels(1)
            .with_buffer_size(2048)
            .with_stats(false);
        assert_eq!(config.sample_rate, 48000);
        assert_eq!(config.channels, 1);
        assert_eq!(config.buffer_size, 2048);
        assert!(!config.enable_stats);
    }

    #[test]
    fn test_capture_creation() {
        let capture = AudioCapture::new().unwrap();
        assert!(!capture.is_capturing());
    }

    #[test]
    fn test_capture_start_stop() {
        let mut capture = AudioCapture::new().unwrap();
        assert!(capture.start().is_ok());
        assert!(capture.is_capturing());
        assert!(capture.stop().is_ok());
        assert!(!capture.is_capturing());
    }

    #[test]
    fn test_capture_start_twice() {
        let mut capture = AudioCapture::new().unwrap();
        assert!(capture.start().is_ok());
        assert!(capture.start().is_err());
    }

    #[test]
    fn test_capture_stop_when_not_capturing() {
        let mut capture = AudioCapture::new().unwrap();
        assert!(capture.stop().is_err());
    }

    #[test]
    fn test_get_buffer() {
        let capture = AudioCapture::new().unwrap();
        let buffer = capture.get_buffer();
        assert!(buffer.is_empty());
    }

    #[test]
    fn test_clear_buffer() {
        let mut capture = AudioCapture::new().unwrap();
        capture.clear_buffer();
        let buffer = capture.get_buffer();
        assert!(buffer.is_empty());
    }

    #[test]
    fn test_config_accessor() {
        let capture = AudioCapture::new().unwrap();
        let config = capture.config();
        assert_eq!(config.sample_rate, 44100);
    }

    #[test]
    fn test_set_config_valid() {
        let mut capture = AudioCapture::new().unwrap();
        let new_config = AudioCaptureConfig::default().with_sample_rate(48000);
        assert!(capture.set_config(new_config).is_ok());
        assert_eq!(capture.config().sample_rate, 48000);
    }

    #[test]
    fn test_set_config_invalid() {
        let mut capture = AudioCapture::new().unwrap();
        let new_config = AudioCaptureConfig {
            sample_rate: 100,
            ..Default::default()
        };
        assert!(capture.set_config(new_config).is_err());
    }

    #[test]
    fn test_stats() {
        let capture = AudioCapture::new().unwrap();
        let stats = capture.stats();
        assert_eq!(stats.total_frames, 0);
    }

    #[test]
    fn test_reset_stats() {
        let mut capture = AudioCapture::new().unwrap();
        capture.reset_stats();
        let stats = capture.stats();
        assert_eq!(stats.total_frames, 0);
    }
}
