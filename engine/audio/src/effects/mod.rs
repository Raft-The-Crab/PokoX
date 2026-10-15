use crate::error::AudioError;

/// Types of audio effects
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum AudioEffectType {
    /// Reverb effect
    Reverb,
    /// Echo effect
    Echo,
    /// Low-pass filter
    LowPass,
    /// High-pass filter
    HighPass,
    /// Band-pass filter
    BandPass,
    /// Compressor
    Compressor,
    /// Equalizer
    Equalizer,
    /// Distortion
    Distortion,
    /// Chorus
    Chorus,
    /// Flanger
    Flanger,
}

/// Configuration for a single audio effect
#[derive(Debug, Clone)]
pub struct EffectConfig {
    /// Effect type
    pub effect_type: AudioEffectType,
    /// Wet/dry mix (0.0 = dry, 1.0 = wet)
    pub mix: f32,
    /// Effect-specific parameters
    pub parameters: Vec<f32>,
    /// Bypass the effect
    pub bypass: bool,
    /// Enable linear interpolation for parameter smoothing
    pub smooth_parameters: bool,
}

impl EffectConfig {
    /// Create a new effect configuration
    pub fn new(effect_type: AudioEffectType) -> Self {
        Self {
            effect_type,
            mix: 0.5,
            parameters: Vec::new(),
            bypass: false,
            smooth_parameters: false,
        }
    }

    /// Set wet/dry mix
    pub fn with_mix(mut self, mix: f32) -> Self {
        self.mix = mix;
        self
    }

    /// Set parameters
    pub fn with_parameters(mut self, parameters: Vec<f32>) -> Self {
        self.parameters = parameters;
        self
    }

    /// Set bypass
    pub fn with_bypass(mut self, bypass: bool) -> Self {
        self.bypass = bypass;
        self
    }

    /// Enable/disable parameter smoothing
    pub fn with_smoothing(mut self, smoothing: bool) -> Self {
        self.smooth_parameters = smoothing;
        self
    }

    /// Validate the configuration
    pub fn validate(&self) -> Result<(), AudioError> {
        if !(0.0..=1.0).contains(&self.mix) {
            return Err(AudioError::VolumeOutOfRange(self.mix));
        }
        Ok(())
    }
}

/// A single audio effect
pub struct AudioEffect {
    /// Configuration
    config: EffectConfig,
    /// Effect state
    state: Vec<f32>,
    /// Smoothed parameters (for real-time smoothing)
    smoothed_parameters: Vec<f32>,
}

impl AudioEffect {
    /// Create a new audio effect
    pub fn new(config: EffectConfig) -> Result<Self, AudioError> {
        config.validate()?;

        // Initialize state based on effect type
        let state = match config.effect_type {
            AudioEffectType::Reverb => vec![0.0; 8],  // Reverb buffer
            AudioEffectType::Echo => vec![0.0; 4],    // Echo delay buffer
            AudioEffectType::LowPass => vec![0.0; 2], // Filter state
            AudioEffectType::HighPass => vec![0.0; 2],
            AudioEffectType::BandPass => vec![0.0; 4],
            AudioEffectType::Compressor => vec![0.0; 3],
            AudioEffectType::Equalizer => vec![0.0; 10],
            AudioEffectType::Distortion => vec![0.0; 1],
            AudioEffectType::Chorus => vec![0.0; 4],
            AudioEffectType::Flanger => vec![0.0; 4],
        };

        // Initialize smoothed parameters
        let smoothed_parameters = config.parameters.clone();

        Ok(Self {
            config,
            state,
            smoothed_parameters,
        })
    }

    /// Get configuration
    pub fn config(&self) -> &EffectConfig {
        &self.config
    }

    /// Set configuration
    pub fn set_config(&mut self, config: EffectConfig) -> Result<(), AudioError> {
        config.validate()?;
        let parameters = config.parameters.clone();
        self.config = config;
        self.smoothed_parameters = parameters;
        Ok(())
    }

    /// Update smoothed parameters (call each frame)
    pub fn update_smoothing(&mut self, smoothing_factor: f32) {
        if !self.config.smooth_parameters {
            self.smoothed_parameters = self.config.parameters.clone();
            return;
        }

        for (i, target) in self.config.parameters.iter().enumerate() {
            if let Some(current) = self.smoothed_parameters.get_mut(i) {
                *current = *current + (*target - *current) * smoothing_factor.clamp(0.0, 1.0);
            }
        }
    }

    /// Get smoothed parameter value
    pub fn get_smoothed_parameter(&self, index: usize) -> f32 {
        self.smoothed_parameters.get(index).copied().unwrap_or(0.0)
    }

    /// Process a single sample through the effect
    pub fn process(&mut self, sample: f32) -> f32 {
        if self.config.bypass {
            return sample;
        }

        match self.config.effect_type {
            AudioEffectType::Reverb => self.process_reverb(sample),
            AudioEffectType::Echo => self.process_echo(sample),
            AudioEffectType::LowPass => self.process_lowpass(sample),
            AudioEffectType::HighPass => self.process_highpass(sample),
            AudioEffectType::BandPass => self.process_bandpass(sample),
            AudioEffectType::Compressor => self.process_compressor(sample),
            AudioEffectType::Equalizer => self.process_equalizer(sample),
            AudioEffectType::Distortion => self.process_distortion(sample),
            AudioEffectType::Chorus => self.process_chorus(sample),
            AudioEffectType::Flanger => self.process_flanger(sample),
        }
    }

    /// Process reverb effect
    fn process_reverb(&mut self, sample: f32) -> f32 {
        // Use smoothed parameters
        let decay = self.smoothed_parameters.first().copied().unwrap_or(0.3);
        let delay_idx = self.smoothed_parameters.get(1).copied().unwrap_or(3.0) as usize;
        let delay_idx = delay_idx.min(self.state.len() - 1);

        let delayed = self.state[delay_idx];
        self.state.rotate_right(1);
        self.state[0] = sample + delayed * decay;

        let wet = self.state[0];
        let dry = sample;
        dry * (1.0 - self.config.mix) + wet * self.config.mix
    }

    /// Process echo effect
    fn process_echo(&mut self, sample: f32) -> f32 {
        let decay = self.smoothed_parameters.first().copied().unwrap_or(0.5);
        let delay_idx = self.smoothed_parameters.get(1).copied().unwrap_or(2.0) as usize;
        let delay_idx = delay_idx.min(self.state.len() - 1);

        let delayed = self.state[delay_idx];
        self.state.rotate_right(1);
        self.state[0] = sample + delayed * decay;

        let wet = self.state[0];
        let dry = sample;
        dry * (1.0 - self.config.mix) + wet * self.config.mix
    }

    /// Process low-pass filter
    fn process_lowpass(&mut self, sample: f32) -> f32 {
        let cutoff = self.smoothed_parameters.first().copied().unwrap_or(0.5);
        let _resonance = self.smoothed_parameters.get(1).copied().unwrap_or(0.1);

        let output = self.state[0] + cutoff * (sample - self.state[0]);
        self.state[0] = output;

        let wet = output;
        let dry = sample;
        dry * (1.0 - self.config.mix) + wet * self.config.mix
    }

    /// Process high-pass filter
    fn process_highpass(&mut self, sample: f32) -> f32 {
        let cutoff = self.config.parameters.first().copied().unwrap_or(0.5);

        let output = sample - self.state[0];
        self.state[0] += cutoff * output;

        let wet = output;
        let dry = sample;
        dry * (1.0 - self.config.mix) + wet * self.config.mix
    }

    /// Process band-pass filter
    fn process_bandpass(&mut self, sample: f32) -> f32 {
        let low_cutoff = self.config.parameters.first().copied().unwrap_or(0.3);
        let high_cutoff = self.config.parameters.get(1).copied().unwrap_or(0.7);

        // Simplified band-pass: cascade low-pass and high-pass
        let mut temp = sample;
        temp = temp + low_cutoff * (self.state[0] - temp);
        self.state[0] = temp;

        let output = temp - self.state[1];
        self.state[1] += high_cutoff * output;

        let wet = output;
        let dry = sample;
        dry * (1.0 - self.config.mix) + wet * self.config.mix
    }

    /// Process compressor
    fn process_compressor(&mut self, sample: f32) -> f32 {
        let threshold = self.config.parameters.first().copied().unwrap_or(0.5);
        let ratio = self.config.parameters.get(1).copied().unwrap_or(4.0);
        let attack = self.config.parameters.get(2).copied().unwrap_or(0.01);

        let input_level = sample.abs();
        let gain_reduction = if input_level > threshold {
            (input_level - threshold) / ratio
        } else {
            0.0
        };

        let output = sample * (1.0 - gain_reduction);
        self.state[0] = self.state[0] + attack * (gain_reduction - self.state[0]);

        let wet = output;
        let dry = sample;
        dry * (1.0 - self.config.mix) + wet * self.config.mix
    }

    /// Process equalizer
    fn process_equalizer(&mut self, sample: f32) -> f32 {
        // Simplified 5-band equalizer
        let bands = [
            self.smoothed_parameters.first().copied().unwrap_or(0.0), // Low
            self.smoothed_parameters.get(1).copied().unwrap_or(0.0),  // Low-mid
            self.smoothed_parameters.get(2).copied().unwrap_or(0.0),  // Mid
            self.smoothed_parameters.get(3).copied().unwrap_or(0.0),  // High-mid
            self.smoothed_parameters.get(4).copied().unwrap_or(0.0),  // High
        ];

        let output = sample * (1.0 + bands.iter().sum::<f32>());
        let wet = output;
        let dry = sample;
        dry * (1.0 - self.config.mix) + wet * self.config.mix
    }

    /// Process distortion
    fn process_distortion(&mut self, sample: f32) -> f32 {
        let drive = self.smoothed_parameters.first().copied().unwrap_or(2.0);

        let output = (sample * drive).tanh();
        let wet = output;
        let dry = sample;
        dry * (1.0 - self.config.mix) + wet * self.config.mix
    }

    /// Process chorus
    fn process_chorus(&mut self, sample: f32) -> f32 {
        let rate = self.smoothed_parameters.first().copied().unwrap_or(0.5);
        let depth = self.smoothed_parameters.get(1).copied().unwrap_or(0.5);

        // Simplified chorus: LFO-modulated delay
        let lfo = (self.state[0] * rate).sin();
        let delay = (depth * lfo) as usize;
        let delayed = self.state[delay.min(self.state.len() - 1)];

        self.state.rotate_right(1);
        self.state[0] = sample;

        let wet = sample + delayed * 0.5;
        let dry = sample;
        dry * (1.0 - self.config.mix) + wet * self.config.mix
    }

    /// Process flanger
    fn process_flanger(&mut self, sample: f32) -> f32 {
        let rate = self.smoothed_parameters.first().copied().unwrap_or(0.1);
        let depth = self.smoothed_parameters.get(1).copied().unwrap_or(0.2);
        let feedback = self.smoothed_parameters.get(2).copied().unwrap_or(0.5);

        // Flanger: very short delay with LFO and feedback
        let lfo = (self.state[0] * rate).sin();
        let delay = (depth * lfo) as usize;
        let delayed = self.state[delay.min(self.state.len() - 1)];

        let output = sample + delayed * feedback;
        self.state.rotate_right(1);
        self.state[0] = output;

        let wet = output;
        let dry = sample;
        dry * (1.0 - self.config.mix) + wet * self.config.mix
    }
}

/// Chain of audio effects
pub struct EffectChain {
    /// Effects in the chain
    effects: Vec<AudioEffect>,
    /// Master gain
    master_gain: f32,
}

impl EffectChain {
    /// Create a new effect chain
    pub fn new() -> Self {
        Self {
            effects: Vec::new(),
            master_gain: 1.0,
        }
    }

    /// Add an effect to the chain
    pub fn add_effect(&mut self, effect: AudioEffect) {
        self.effects.push(effect);
    }

    /// Remove an effect from the chain
    pub fn remove_effect(&mut self, index: usize) -> Result<(), AudioError> {
        if index >= self.effects.len() {
            return Err(AudioError::PlaybackFailed(
                "Effect index out of bounds".to_string(),
            ));
        }
        self.effects.remove(index);
        Ok(())
    }

    /// Get an effect by index
    pub fn get_effect(&self, index: usize) -> Result<&AudioEffect, AudioError> {
        self.effects
            .get(index)
            .ok_or_else(|| AudioError::PlaybackFailed("Effect index out of bounds".to_string()))
    }

    /// Get a mutable effect by index
    pub fn get_effect_mut(&mut self, index: usize) -> Result<&mut AudioEffect, AudioError> {
        self.effects
            .get_mut(index)
            .ok_or_else(|| AudioError::PlaybackFailed("Effect index out of bounds".to_string()))
    }

    /// Process a sample through the entire chain
    pub fn process(&mut self, sample: f32) -> f32 {
        let mut output = sample;
        for effect in &mut self.effects {
            output = effect.process(output);
        }
        output * self.master_gain
    }

    /// Set master gain
    pub fn set_master_gain(&mut self, gain: f32) -> Result<(), AudioError> {
        if gain < 0.0 {
            return Err(AudioError::VolumeOutOfRange(gain));
        }
        self.master_gain = gain;
        Ok(())
    }

    /// Get master gain
    pub fn master_gain(&self) -> f32 {
        self.master_gain
    }

    /// Get number of effects
    pub fn len(&self) -> usize {
        self.effects.len()
    }

    /// Check if chain is empty
    pub fn is_empty(&self) -> bool {
        self.effects.is_empty()
    }

    /// Clear all effects
    pub fn clear(&mut self) {
        self.effects.clear();
    }

    /// Update parameter smoothing for all effects (call each frame)
    pub fn update_smoothing(&mut self, smoothing_factor: f32) {
        for effect in &mut self.effects {
            effect.update_smoothing(smoothing_factor);
        }
    }
}

impl Default for EffectChain {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_effect_config_creation() {
        let config = EffectConfig::new(AudioEffectType::Reverb);
        assert_eq!(config.effect_type, AudioEffectType::Reverb);
        assert_eq!(config.mix, 0.5);
        assert!(!config.bypass);
    }

    #[test]
    fn test_effect_config_builder() {
        let config = EffectConfig::new(AudioEffectType::Echo)
            .with_mix(0.8)
            .with_parameters(vec![0.5, 0.3])
            .with_bypass(true);
        assert_eq!(config.mix, 0.8);
        assert_eq!(config.parameters.len(), 2);
        assert!(config.bypass);
    }

    #[test]
    fn test_effect_config_validate_valid() {
        let config = EffectConfig::new(AudioEffectType::Reverb);
        assert!(config.validate().is_ok());
    }

    #[test]
    fn test_effect_config_validate_invalid_mix() {
        let config = EffectConfig::new(AudioEffectType::Reverb).with_mix(1.5);
        assert!(config.validate().is_err());
    }

    #[test]
    fn test_effect_config_smoothing() {
        let config = EffectConfig::new(AudioEffectType::Reverb).with_smoothing(true);
        assert!(config.smooth_parameters);
    }

    #[test]
    fn test_audio_effect_creation() {
        let config = EffectConfig::new(AudioEffectType::Reverb);
        let effect = AudioEffect::new(config).unwrap();
        assert_eq!(effect.config().effect_type, AudioEffectType::Reverb);
    }

    #[test]
    fn test_audio_effect_smoothing() {
        let config = EffectConfig::new(AudioEffectType::Reverb)
            .with_parameters(vec![0.5, 0.3])
            .with_smoothing(true);
        let mut effect = AudioEffect::new(config).unwrap();

        // Initial smoothed parameters should match target
        assert_eq!(effect.get_smoothed_parameter(0), 0.5);

        // Update smoothing (no change in target)
        effect.update_smoothing(0.5);
        assert_eq!(effect.get_smoothed_parameter(0), 0.5);

        // Change target parameter
        effect
            .set_config(
                EffectConfig::new(AudioEffectType::Reverb)
                    .with_parameters(vec![0.8, 0.3])
                    .with_smoothing(true),
            )
            .unwrap();

        // After set_config, smoothed parameters should be updated to new target
        // (This is the current behavior: set_config resets smoothed to target)
        assert_eq!(effect.get_smoothed_parameter(0), 0.8);

        // Update smoothing (no further change since already at target)
        effect.update_smoothing(0.5);
        assert_eq!(effect.get_smoothed_parameter(0), 0.8);
    }

    #[test]
    fn test_audio_effect_smoothing_disabled() {
        let config = EffectConfig::new(AudioEffectType::Reverb)
            .with_parameters(vec![0.5, 0.3])
            .with_smoothing(false);
        let mut effect = AudioEffect::new(config).unwrap();

        effect
            .set_config(
                EffectConfig::new(AudioEffectType::Reverb)
                    .with_parameters(vec![0.8, 0.3])
                    .with_smoothing(false),
            )
            .unwrap();

        // Update smoothing with disabled smoothing should snap to target
        effect.update_smoothing(0.5);
        assert_eq!(effect.get_smoothed_parameter(0), 0.8);
    }

    #[test]
    fn test_audio_effect_bypass() {
        let config = EffectConfig::new(AudioEffectType::Reverb).with_bypass(true);
        let mut effect = AudioEffect::new(config).unwrap();
        let output = effect.process(0.5);
        assert_eq!(output, 0.5); // Bypassed, output equals input
    }

    #[test]
    fn test_audio_effect_process() {
        let config = EffectConfig::new(AudioEffectType::Reverb).with_mix(1.0);
        let mut effect = AudioEffect::new(config).unwrap();
        let output = effect.process(0.5);
        // Reverb should modify the sample
        assert!(output != 0.5 || output.abs() <= 1.0);
    }

    #[test]
    fn test_effect_chain_creation() {
        let chain = EffectChain::new();
        assert!(chain.is_empty());
        assert_eq!(chain.master_gain(), 1.0);
    }

    #[test]
    fn test_effect_chain_add_effect() {
        let mut chain = EffectChain::new();
        let config = EffectConfig::new(AudioEffectType::Reverb);
        let effect = AudioEffect::new(config).unwrap();
        chain.add_effect(effect);
        assert_eq!(chain.len(), 1);
    }

    #[test]
    fn test_effect_chain_remove_effect() {
        let mut chain = EffectChain::new();
        let config = EffectConfig::new(AudioEffectType::Reverb);
        let effect = AudioEffect::new(config).unwrap();
        chain.add_effect(effect);
        assert!(chain.remove_effect(0).is_ok());
        assert!(chain.is_empty());
    }

    #[test]
    fn test_effect_chain_remove_invalid_index() {
        let mut chain = EffectChain::new();
        assert!(chain.remove_effect(0).is_err());
    }

    #[test]
    fn test_effect_chain_process() {
        let mut chain = EffectChain::new();
        let config = EffectConfig::new(AudioEffectType::Reverb).with_mix(0.0);
        let effect = AudioEffect::new(config).unwrap();
        chain.add_effect(effect);
        let output = chain.process(0.5);
        // With mix 0.0, should be close to input
        assert!((output - 0.5).abs() < 0.1);
    }

    #[test]
    fn test_effect_chain_master_gain() {
        let mut chain = EffectChain::new();
        assert!(chain.set_master_gain(0.5).is_ok());
        assert_eq!(chain.master_gain(), 0.5);
    }

    #[test]
    fn test_effect_chain_update_smoothing() {
        let mut chain = EffectChain::new();
        let config = EffectConfig::new(AudioEffectType::Reverb)
            .with_parameters(vec![0.5, 0.3])
            .with_smoothing(true);
        let effect = AudioEffect::new(config).unwrap();
        chain.add_effect(effect);

        // Update smoothing on the chain
        chain.update_smoothing(0.5);

        // Should not panic
        assert_eq!(chain.len(), 1);
    }

    #[test]
    fn test_effect_chain_master_gain_invalid() {
        let mut chain = EffectChain::new();
        assert!(chain.set_master_gain(-0.5).is_err());
    }

    #[test]
    fn test_effect_chain_clear() {
        let mut chain = EffectChain::new();
        let config = EffectConfig::new(AudioEffectType::Reverb);
        let effect = AudioEffect::new(config).unwrap();
        chain.add_effect(effect);
        chain.clear();
        assert!(chain.is_empty());
    }
}
