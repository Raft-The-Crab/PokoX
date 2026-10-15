use crate::error::AudioError;
use rodio::{Decoder, Source};
use std::fs::File;
use std::io::BufReader;

/// Audio codec/format types
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum AudioCodec {
    /// WAV format
    Wav,
    /// OGG Vorbis format
    Ogg,
    /// MP3 format
    Mp3,
    /// FLAC format
    Flac,
    /// Unknown format
    Unknown,
}

impl AudioCodec {
    /// Detect codec from file extension
    pub fn from_extension(ext: &str) -> Self {
        match ext.to_lowercase().as_str() {
            "wav" => AudioCodec::Wav,
            "ogg" => AudioCodec::Ogg,
            "mp3" => AudioCodec::Mp3,
            "flac" => AudioCodec::Flac,
            _ => AudioCodec::Unknown,
        }
    }

    /// Get file extension for codec
    pub fn extension(&self) -> &'static str {
        match self {
            AudioCodec::Wav => "wav",
            AudioCodec::Ogg => "ogg",
            AudioCodec::Mp3 => "mp3",
            AudioCodec::Flac => "flac",
            AudioCodec::Unknown => "bin",
        }
    }

    /// Check if codec is supported
    pub fn is_supported(&self) -> bool {
        matches!(
            self,
            AudioCodec::Wav | AudioCodec::Ogg | AudioCodec::Mp3 | AudioCodec::Flac
        )
    }
}

/// Audio stream information
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct AudioStreamInfo {
    /// Sample rate in Hz
    pub sample_rate: u32,
    /// Number of channels (1 = mono, 2 = stereo)
    pub channels: u16,
    /// Bits per sample
    pub bits_per_sample: u16,
    /// Duration in seconds
    pub duration: f32,
}

impl AudioStreamInfo {
    /// Validate the stream info
    pub fn validate(&self) -> Result<(), AudioError> {
        if self.sample_rate == 0 {
            return Err(AudioError::PlaybackFailed(
                "Sample rate must be positive".to_string(),
            ));
        }
        if self.channels == 0 {
            return Err(AudioError::PlaybackFailed(
                "Channels must be at least 1".to_string(),
            ));
        }
        if self.bits_per_sample == 0 {
            return Err(AudioError::PlaybackFailed(
                "Bits per sample must be positive".to_string(),
            ));
        }
        if self.duration < 0.0 {
            return Err(AudioError::PlaybackFailed(
                "Duration must be non-negative".to_string(),
            ));
        }
        Ok(())
    }

    /// Calculate data size in bytes
    pub fn calculate_size_bytes(&self) -> u64 {
        let samples_per_second = self.sample_rate as u64;
        let total_samples = samples_per_second * self.duration as u64;
        let bytes_per_sample = (self.bits_per_sample / 8) as u64;
        total_samples * bytes_per_sample * self.channels as u64
    }

    /// Calculate bitrate in kbps
    pub fn calculate_bitrate(&self) -> u32 {
        if self.duration == 0.0 {
            return 0;
        }
        let size_bits = self.calculate_size_bytes() * 8;
        (size_bits as f32 / self.duration) as u32
    }
}

impl Default for AudioStreamInfo {
    fn default() -> Self {
        Self {
            sample_rate: 44100,
            channels: 2,
            bits_per_sample: 16,
            duration: 0.0,
        }
    }
}

/// Audio decoder
pub struct AudioDecoder;

impl AudioDecoder {
    /// Get stream info from file
    pub fn get_stream_info(path: &str) -> Result<AudioStreamInfo, AudioError> {
        let file = File::open(path)
            .map_err(|e| AudioError::LoadFailed(format!("Failed to open file: {}", e)))?;

        let decoder = Decoder::new(BufReader::new(file))
            .map_err(|e| AudioError::LoadFailed(format!("Failed to decode audio: {}", e)))?;

        let sample_rate = decoder.sample_rate();
        let channels = decoder.channels();
        let duration_opt = decoder.total_duration();

        // Estimate bits per sample (rodio doesn't expose this directly, use 16 as default)
        let bits_per_sample = 16;

        let duration = duration_opt.map(|d| d.as_secs_f32()).unwrap_or(0.0);

        let info = AudioStreamInfo {
            sample_rate,
            channels,
            bits_per_sample,
            duration,
        };

        info.validate()?;
        Ok(info)
    }

    /// Validate audio file format
    pub fn validate_format(path: &str) -> Result<AudioCodec, AudioError> {
        let ext = path.rsplit('.').next().ok_or(AudioError::InvalidFormat)?;
        let codec = AudioCodec::from_extension(ext);

        if !codec.is_supported() {
            return Err(AudioError::InvalidFormat);
        }

        Ok(codec)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_codec_from_extension() {
        assert_eq!(AudioCodec::from_extension("wav"), AudioCodec::Wav);
        assert_eq!(AudioCodec::from_extension("WAV"), AudioCodec::Wav);
        assert_eq!(AudioCodec::from_extension("ogg"), AudioCodec::Ogg);
        assert_eq!(AudioCodec::from_extension("mp3"), AudioCodec::Mp3);
        assert_eq!(AudioCodec::from_extension("flac"), AudioCodec::Flac);
        assert_eq!(AudioCodec::from_extension("xyz"), AudioCodec::Unknown);
    }

    #[test]
    fn test_codec_extension() {
        assert_eq!(AudioCodec::Wav.extension(), "wav");
        assert_eq!(AudioCodec::Ogg.extension(), "ogg");
        assert_eq!(AudioCodec::Mp3.extension(), "mp3");
        assert_eq!(AudioCodec::Flac.extension(), "flac");
    }

    #[test]
    fn test_codec_supported() {
        assert!(AudioCodec::Wav.is_supported());
        assert!(AudioCodec::Ogg.is_supported());
        assert!(AudioCodec::Mp3.is_supported());
        assert!(AudioCodec::Flac.is_supported());
        assert!(!AudioCodec::Unknown.is_supported());
    }

    #[test]
    fn test_validate_format_valid() {
        assert!(AudioDecoder::validate_format("test.wav").is_ok());
        assert!(AudioDecoder::validate_format("test.ogg").is_ok());
        assert!(AudioDecoder::validate_format("test.mp3").is_ok());
    }

    #[test]
    fn test_validate_format_invalid() {
        assert!(AudioDecoder::validate_format("test.xyz").is_err());
        assert!(AudioDecoder::validate_format("test").is_err());
    }

    #[test]
    fn test_stream_info_validate_valid() {
        let info = AudioStreamInfo::default();
        assert!(info.validate().is_ok());
    }

    #[test]
    fn test_stream_info_validate_invalid_sample_rate() {
        let info = AudioStreamInfo {
            sample_rate: 0,
            ..Default::default()
        };
        assert!(info.validate().is_err());
    }

    #[test]
    fn test_stream_info_validate_invalid_channels() {
        let info = AudioStreamInfo {
            channels: 0,
            ..Default::default()
        };
        assert!(info.validate().is_err());
    }

    #[test]
    fn test_stream_info_validate_invalid_duration() {
        let info = AudioStreamInfo {
            duration: -1.0,
            ..Default::default()
        };
        assert!(info.validate().is_err());
    }

    #[test]
    fn test_calculate_size_bytes() {
        let info = AudioStreamInfo {
            sample_rate: 44100,
            channels: 2,
            bits_per_sample: 16,
            duration: 10.0,
        };
        let size = info.calculate_size_bytes();
        // 44100 * 10 * 2 * (16/8) = 44100 * 10 * 2 * 2 = 1,764,000 bytes
        assert_eq!(size, 1_764_000);
    }

    #[test]
    fn test_calculate_bitrate() {
        let info = AudioStreamInfo {
            sample_rate: 44100,
            channels: 2,
            bits_per_sample: 16,
            duration: 10.0,
        };
        let bitrate = info.calculate_bitrate();
        // 1,764,000 bytes * 8 bits = 14,112,000 bits / 10 seconds = 1,411,200 bps
        assert_eq!(bitrate, 1_411_200);
    }

    #[test]
    fn test_calculate_bitrate_zero_duration() {
        let info = AudioStreamInfo {
            duration: 0.0,
            ..Default::default()
        };
        assert_eq!(info.calculate_bitrate(), 0);
    }

    #[test]
    fn test_get_stream_info_nonexistent_file() {
        assert!(AudioDecoder::get_stream_info("nonexistent.wav").is_err());
    }
}
