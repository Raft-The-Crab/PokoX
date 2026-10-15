use crate::effects::{EffectChain, EffectConfig};
use crate::error::AudioError;
use std::collections::HashMap;

/// Configuration for an audio bus
#[derive(Debug, Clone)]
pub struct AudioBusConfig {
    /// Bus name
    pub name: String,
    /// Bus volume
    pub volume: f32,
    /// Enable effects processing
    pub enable_effects: bool,
    /// Enable statistics tracking
    pub enable_stats: bool,
}

impl Default for AudioBusConfig {
    fn default() -> Self {
        Self {
            name: "Master".to_string(),
            volume: 1.0,
            enable_effects: true,
            enable_stats: true,
        }
    }
}

impl AudioBusConfig {
    /// Validate the configuration
    pub fn validate(&self) -> Result<(), AudioError> {
        if !(0.0..=1.0).contains(&self.volume) {
            return Err(AudioError::VolumeOutOfRange(self.volume));
        }
        Ok(())
    }

    /// Set name
    pub fn with_name(mut self, name: String) -> Self {
        self.name = name;
        self
    }

    /// Set volume
    pub fn with_volume(mut self, volume: f32) -> Self {
        self.volume = volume;
        self
    }

    /// Enable/disable effects
    pub fn with_effects(mut self, enabled: bool) -> Self {
        self.enable_effects = enabled;
        self
    }

    /// Enable/disable statistics
    pub fn with_stats(mut self, enabled: bool) -> Self {
        self.enable_stats = enabled;
        self
    }
}

/// Statistics for an audio bus
#[derive(Debug, Clone, Copy, Default)]
pub struct AudioBusStats {
    /// Total samples processed
    pub total_samples: usize,
    /// Peak level
    pub peak_level: f32,
    /// Average level
    pub average_level: f32,
}

/// Send effect configuration
#[derive(Debug, Clone)]
pub struct SendEffect {
    /// Target bus name
    pub target_bus: String,
    /// Send amount (0.0 to 1.0)
    pub send_amount: f32,
    /// Enable pre-fader send
    pub pre_fader: bool,
}

impl SendEffect {
    /// Create a new send effect
    pub fn new(target_bus: String, send_amount: f32) -> Self {
        Self {
            target_bus,
            send_amount,
            pre_fader: false,
        }
    }

    /// Set send amount
    pub fn with_send_amount(mut self, amount: f32) -> Self {
        self.send_amount = amount.clamp(0.0, 1.0);
        self
    }

    /// Set pre-fader
    pub fn with_pre_fader(mut self, pre_fader: bool) -> Self {
        self.pre_fader = pre_fader;
        self
    }

    /// Validate the configuration
    pub fn validate(&self) -> Result<(), AudioError> {
        if self.send_amount < 0.0 || self.send_amount > 1.0 {
            return Err(AudioError::VolumeOutOfRange(self.send_amount));
        }
        Ok(())
    }
}

/// Audio bus for routing and effects
pub struct AudioBus {
    /// Configuration
    config: AudioBusConfig,
    /// Effect chain
    effect_chain: EffectChain,
    /// Send effects
    send_effects: Vec<SendEffect>,
    /// Statistics
    stats: AudioBusStats,
    /// Current level for peak detection
    current_level: f32,
}

impl AudioBus {
    /// Create a new audio bus
    pub fn new() -> Result<Self, AudioError> {
        Self::with_config(AudioBusConfig::default())
    }

    /// Create with custom configuration
    pub fn with_config(config: AudioBusConfig) -> Result<Self, AudioError> {
        config.validate()?;

        Ok(Self {
            config,
            effect_chain: EffectChain::new(),
            send_effects: Vec::new(),
            stats: AudioBusStats::default(),
            current_level: 0.0,
        })
    }

    /// Process a sample through the bus
    pub fn process(&mut self, sample: f32) -> f32 {
        let mut output = sample * self.config.volume;

        // Update level tracking
        self.current_level = output.abs();
        self.stats.peak_level = self.stats.peak_level.max(self.current_level);

        // Process through effect chain if enabled
        if self.config.enable_effects {
            output = self.effect_chain.process(output);
        }

        // Update statistics
        if self.config.enable_stats {
            self.stats.total_samples += 1;
            // Update average level
            if self.stats.total_samples == 1 {
                self.stats.average_level = self.current_level;
            } else {
                self.stats.average_level = (self.stats.average_level
                    * (self.stats.total_samples - 1) as f32
                    + self.current_level)
                    / self.stats.total_samples as f32;
            }
        }

        output
    }

    /// Add a send effect
    pub fn add_send(&mut self, send: SendEffect) -> Result<(), AudioError> {
        send.validate()?;

        // Check for self-send
        if send.target_bus == self.config.name {
            return Err(AudioError::PlaybackFailed(format!(
                "Bus '{}' cannot send to itself",
                self.config.name
            )));
        }

        // Check for duplicate send to same target
        if self.has_send_to(&send.target_bus) {
            return Err(AudioError::PlaybackFailed(format!(
                "Bus '{}' already has a send to '{}'",
                self.config.name, send.target_bus
            )));
        }

        self.send_effects.push(send);
        Ok(())
    }

    /// Remove a send effect
    pub fn remove_send(&mut self, index: usize) -> Result<(), AudioError> {
        if index >= self.send_effects.len() {
            return Err(AudioError::PlaybackFailed(
                "Send index out of bounds".to_string(),
            ));
        }
        self.send_effects.remove(index);
        Ok(())
    }

    /// Clear all send effects
    pub fn clear_sends(&mut self) {
        self.send_effects.clear();
    }

    /// Check if this bus has a send to the specified target
    pub fn has_send_to(&self, target_bus: &str) -> bool {
        self.send_effects.iter().any(|s| s.target_bus == target_bus)
    }

    /// Get send effects
    pub fn sends(&self) -> &[SendEffect] {
        &self.send_effects
    }

    /// Get effect chain
    pub fn effect_chain(&self) -> &EffectChain {
        &self.effect_chain
    }

    /// Get mutable effect chain
    pub fn effect_chain_mut(&mut self) -> &mut EffectChain {
        &mut self.effect_chain
    }

    /// Get configuration
    pub fn config(&self) -> &AudioBusConfig {
        &self.config
    }

    /// Set configuration
    pub fn set_config(&mut self, config: AudioBusConfig) -> Result<(), AudioError> {
        config.validate()?;
        self.config = config;
        Ok(())
    }

    /// Set volume
    pub fn set_volume(&mut self, volume: f32) -> Result<(), AudioError> {
        if !(0.0..=1.0).contains(&volume) {
            return Err(AudioError::VolumeOutOfRange(volume));
        }
        self.config.volume = volume;
        Ok(())
    }

    /// Get volume
    pub fn volume(&self) -> f32 {
        self.config.volume
    }

    /// Get statistics
    pub fn stats(&self) -> AudioBusStats {
        self.stats
    }

    /// Reset statistics
    pub fn reset_stats(&mut self) {
        self.stats = AudioBusStats::default();
        self.current_level = 0.0;
    }

    /// Add an effect to the bus
    pub fn add_effect(&mut self, effect_config: EffectConfig) -> Result<(), AudioError> {
        let effect = crate::effects::AudioEffect::new(effect_config)?;
        self.effect_chain.add_effect(effect);
        Ok(())
    }
}

impl Default for AudioBus {
    fn default() -> Self {
        Self::new().unwrap()
    }
}

/// Audio bus manager for routing between buses
pub struct AudioBusManager {
    /// Buses indexed by name
    buses: HashMap<String, AudioBus>,
}

impl AudioBusManager {
    /// Create a new bus manager
    pub fn new() -> Self {
        Self {
            buses: HashMap::new(),
        }
    }

    /// Add a bus
    pub fn add_bus(&mut self, bus: AudioBus) -> Result<(), AudioError> {
        let name = bus.config().name.clone();
        if self.buses.contains_key(&name) {
            return Err(AudioError::PlaybackFailed(format!(
                "Bus '{}' already exists",
                name
            )));
        }

        self.buses.insert(name, bus);
        Ok(())
    }

    /// Remove a bus
    pub fn remove_bus(&mut self, name: &str) -> Result<(), AudioError> {
        if !self.buses.contains_key(name) {
            return Err(AudioError::PlaybackFailed(format!(
                "Bus '{}' not found",
                name
            )));
        }
        self.buses.remove(name);
        Ok(())
    }

    /// Get a bus by name
    pub fn get_bus(&self, name: &str) -> Result<&AudioBus, AudioError> {
        self.buses
            .get(name)
            .ok_or_else(|| AudioError::PlaybackFailed(format!("Bus '{}' not found", name)))
    }

    /// Get a mutable bus by name
    pub fn get_bus_mut(&mut self, name: &str) -> Result<&mut AudioBus, AudioError> {
        self.buses
            .get_mut(name)
            .ok_or_else(|| AudioError::PlaybackFailed(format!("Bus '{}' not found", name)))
    }

    /// Process audio through a specific bus
    pub fn process_bus(&mut self, name: &str, sample: f32) -> Result<f32, AudioError> {
        let bus = self.get_bus_mut(name)?;
        Ok(bus.process(sample))
    }

    /// Process audio with send effects
    pub fn process_with_sends(&mut self, source_bus: &str, sample: f32) -> Result<f32, AudioError> {
        // Collect send information first
        let sends: Vec<(String, f32, bool)> = {
            let source = self.get_bus(source_bus)?;
            source
                .sends()
                .iter()
                .map(|s| (s.target_bus.clone(), s.send_amount, s.pre_fader))
                .collect()
        };

        // Process through source bus
        let output = self.process_bus(source_bus, sample)?;

        // Apply send effects
        for (target_bus, send_amount, pre_fader) in sends {
            if let Ok(target) = self.get_bus_mut(&target_bus) {
                let send_input = if pre_fader { sample } else { output };
                target.process(send_input * send_amount);
            }
        }

        Ok(output)
    }

    /// Get all bus names
    pub fn bus_names(&self) -> Vec<String> {
        self.buses.keys().cloned().collect()
    }

    /// Get number of buses
    pub fn len(&self) -> usize {
        self.buses.len()
    }

    /// Check if manager is empty
    pub fn is_empty(&self) -> bool {
        self.buses.is_empty()
    }

    /// Check if adding a send would create a cycle
    pub fn would_create_cycle(&self, source_bus: &str, target_bus: &str) -> bool {
        if source_bus == target_bus {
            return true; // Self-send is a cycle
        }

        // BFS to check if target_bus can reach source_bus
        let mut visited = std::collections::HashSet::new();
        let mut queue = vec![target_bus.to_string()];

        while let Some(current) = queue.pop() {
            if current == source_bus {
                return true;
            }
            if visited.contains(&current) {
                continue;
            }
            visited.insert(current.clone());

            if let Ok(bus) = self.get_bus(&current) {
                for send in bus.sends() {
                    queue.push(send.target_bus.clone());
                }
            }
        }

        false
    }

    /// Validate all sends in the manager (check target existence and cycles)
    pub fn validate_sends(&self) -> Result<(), AudioError> {
        for (name, bus) in &self.buses {
            for send in bus.sends() {
                // Check if target bus exists
                if !self.buses.contains_key(&send.target_bus) {
                    return Err(AudioError::PlaybackFailed(format!(
                        "Bus '{}' has send to non-existent bus '{}'",
                        name, send.target_bus
                    )));
                }

                // Check for cycles
                if self.would_create_cycle(name, &send.target_bus) {
                    return Err(AudioError::PlaybackFailed(format!(
                        "Bus '{}' send to '{}' would create a cycle",
                        name, send.target_bus
                    )));
                }
            }
        }
        Ok(())
    }

    /// Update parameter smoothing for all buses
    pub fn update_smoothing(&mut self, smoothing_factor: f32) {
        for bus in self.buses.values_mut() {
            bus.effect_chain_mut().update_smoothing(smoothing_factor);
        }
    }
}

impl Default for AudioBusManager {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_bus_config_default() {
        let config = AudioBusConfig::default();
        assert_eq!(config.name, "Master");
        assert_eq!(config.volume, 1.0);
        assert!(config.enable_effects);
        assert!(config.enable_stats);
    }

    #[test]
    fn test_bus_config_validate_valid() {
        let config = AudioBusConfig::default();
        assert!(config.validate().is_ok());
    }

    #[test]
    fn test_bus_config_validate_invalid_volume() {
        let config = AudioBusConfig {
            volume: 1.5,
            ..Default::default()
        };
        assert!(config.validate().is_err());
    }

    #[test]
    fn test_bus_config_builder() {
        let config = AudioBusConfig::default()
            .with_name("Reverb".to_string())
            .with_volume(0.8)
            .with_effects(false)
            .with_stats(false);
        assert_eq!(config.name, "Reverb");
        assert_eq!(config.volume, 0.8);
        assert!(!config.enable_effects);
        assert!(!config.enable_stats);
    }

    #[test]
    fn test_send_effect_creation() {
        let send = SendEffect::new("Reverb".to_string(), 0.5);
        assert_eq!(send.target_bus, "Reverb");
        assert_eq!(send.send_amount, 0.5);
        assert!(!send.pre_fader);
    }

    #[test]
    fn test_send_effect_builder() {
        let send = SendEffect::new("Reverb".to_string(), 0.5)
            .with_send_amount(0.8)
            .with_pre_fader(true);
        assert_eq!(send.send_amount, 0.8);
        assert!(send.pre_fader);
    }

    #[test]
    fn test_send_effect_validate_valid() {
        let send = SendEffect::new("Reverb".to_string(), 0.5);
        assert!(send.validate().is_ok());
    }

    #[test]
    fn test_send_effect_validate_invalid() {
        let send = SendEffect::new("Reverb".to_string(), 1.5);
        assert!(send.validate().is_err());
    }

    #[test]
    fn test_audio_bus_creation() {
        let bus = AudioBus::new().unwrap();
        assert_eq!(bus.config().name, "Master");
        assert_eq!(bus.volume(), 1.0);
    }

    #[test]
    fn test_audio_bus_process() {
        let mut bus = AudioBus::new().unwrap();
        let output = bus.process(0.5);
        assert!(output.abs() <= 1.0);
    }

    #[test]
    fn test_audio_bus_set_volume() {
        let mut bus = AudioBus::new().unwrap();
        assert!(bus.set_volume(0.5).is_ok());
        assert_eq!(bus.volume(), 0.5);
    }

    #[test]
    fn test_audio_bus_set_volume_invalid() {
        let mut bus = AudioBus::new().unwrap();
        assert!(bus.set_volume(1.5).is_err());
    }

    #[test]
    fn test_audio_bus_add_send() {
        let mut bus = AudioBus::new().unwrap();
        let send = SendEffect::new("Reverb".to_string(), 0.5);
        assert!(bus.add_send(send).is_ok());
        assert_eq!(bus.sends().len(), 1);
    }

    #[test]
    fn test_audio_bus_add_send_duplicate() {
        let mut bus = AudioBus::new().unwrap();
        let send = SendEffect::new("Reverb".to_string(), 0.5);
        bus.add_send(send.clone()).unwrap();
        assert!(bus.add_send(send).is_err());
    }

    #[test]
    fn test_audio_bus_add_send_self() {
        let mut bus = AudioBus::new().unwrap();
        let send = SendEffect::new("Master".to_string(), 0.5);
        assert!(bus.add_send(send).is_err());
    }

    #[test]
    fn test_audio_bus_remove_send() {
        let mut bus = AudioBus::new().unwrap();
        let send = SendEffect::new("Reverb".to_string(), 0.5);
        bus.add_send(send).unwrap();
        assert!(bus.remove_send(0).is_ok());
        assert_eq!(bus.sends().len(), 0);
    }

    #[test]
    fn test_audio_bus_add_effect() {
        let mut bus = AudioBus::new().unwrap();
        let config = EffectConfig::new(crate::effects::AudioEffectType::Reverb);
        assert!(bus.add_effect(config).is_ok());
        assert_eq!(bus.effect_chain().len(), 1);
    }

    #[test]
    fn test_audio_bus_stats() {
        let mut bus = AudioBus::new().unwrap();
        bus.process(0.5);
        let stats = bus.stats();
        assert_eq!(stats.total_samples, 1);
    }

    #[test]
    fn test_audio_bus_reset_stats() {
        let mut bus = AudioBus::new().unwrap();
        bus.process(0.5);
        bus.reset_stats();
        let stats = bus.stats();
        assert_eq!(stats.total_samples, 0);
    }

    #[test]
    fn test_bus_manager_creation() {
        let manager = AudioBusManager::new();
        assert!(manager.is_empty());
    }

    #[test]
    fn test_bus_manager_add_bus() {
        let mut manager = AudioBusManager::new();
        let bus = AudioBus::new().unwrap();
        assert!(manager.add_bus(bus).is_ok());
        assert_eq!(manager.len(), 1);
    }

    #[test]
    fn test_bus_manager_add_duplicate() {
        let mut manager = AudioBusManager::new();
        let bus = AudioBus::new().unwrap();
        manager.add_bus(bus).unwrap();
        let bus2 = AudioBus::new().unwrap();
        assert!(manager.add_bus(bus2).is_err());
    }

    #[test]
    fn test_bus_manager_remove_bus() {
        let mut manager = AudioBusManager::new();
        let bus = AudioBus::new().unwrap();
        manager.add_bus(bus).unwrap();
        assert!(manager.remove_bus("Master").is_ok());
        assert!(manager.is_empty());
    }

    #[test]
    fn test_bus_manager_process_bus() {
        let mut manager = AudioBusManager::new();
        let bus = AudioBus::new().unwrap();
        manager.add_bus(bus).unwrap();
        let output = manager.process_bus("Master", 0.5).unwrap();
        assert!(output.abs() <= 1.0);
    }

    #[test]
    fn test_bus_manager_bus_names() {
        let mut manager = AudioBusManager::new();
        let bus = AudioBus::new().unwrap();
        manager.add_bus(bus).unwrap();
        let names = manager.bus_names();
        assert_eq!(names.len(), 1);
        assert!(names.contains(&"Master".to_string()));
    }

    #[test]
    fn test_bus_manager_clear_sends() {
        let mut bus = AudioBus::new().unwrap();
        let send = SendEffect::new("Reverb".to_string(), 0.5);
        bus.add_send(send).unwrap();
        assert_eq!(bus.sends().len(), 1);
        bus.clear_sends();
        assert_eq!(bus.sends().len(), 0);
    }

    #[test]
    fn test_bus_manager_has_send_to() {
        let mut bus = AudioBus::new().unwrap();
        let send = SendEffect::new("Reverb".to_string(), 0.5);
        bus.add_send(send).unwrap();
        assert!(bus.has_send_to("Reverb"));
        assert!(!bus.has_send_to("Other"));
    }

    #[test]
    fn test_bus_manager_would_create_cycle_self() {
        let manager = AudioBusManager::new();
        assert!(manager.would_create_cycle("Master", "Master"));
    }

    #[test]
    fn test_bus_manager_would_create_cycle_direct() {
        let mut manager = AudioBusManager::new();
        let mut bus1 =
            AudioBus::with_config(AudioBusConfig::default().with_name("Bus1".to_string())).unwrap();
        let mut bus2 =
            AudioBus::with_config(AudioBusConfig::default().with_name("Bus2".to_string())).unwrap();

        bus1.add_send(SendEffect::new("Bus2".to_string(), 0.5))
            .unwrap();
        bus2.add_send(SendEffect::new("Bus1".to_string(), 0.5))
            .unwrap();

        manager.add_bus(bus1).unwrap();
        manager.add_bus(bus2).unwrap();

        assert!(manager.would_create_cycle("Bus1", "Bus2"));
    }

    #[test]
    fn test_bus_manager_would_create_cycle_indirect() {
        let mut manager = AudioBusManager::new();
        let mut bus1 =
            AudioBus::with_config(AudioBusConfig::default().with_name("Bus1".to_string())).unwrap();
        let mut bus2 =
            AudioBus::with_config(AudioBusConfig::default().with_name("Bus2".to_string())).unwrap();
        let mut bus3 =
            AudioBus::with_config(AudioBusConfig::default().with_name("Bus3".to_string())).unwrap();

        bus1.add_send(SendEffect::new("Bus2".to_string(), 0.5))
            .unwrap();
        bus2.add_send(SendEffect::new("Bus3".to_string(), 0.5))
            .unwrap();
        bus3.add_send(SendEffect::new("Bus1".to_string(), 0.5))
            .unwrap();

        manager.add_bus(bus1).unwrap();
        manager.add_bus(bus2).unwrap();
        manager.add_bus(bus3).unwrap();

        assert!(manager.would_create_cycle("Bus1", "Bus3"));
    }

    #[test]
    fn test_bus_manager_validate_sends_missing_target() {
        let mut manager = AudioBusManager::new();
        let mut bus = AudioBus::new().unwrap();
        bus.add_send(SendEffect::new("NonExistent".to_string(), 0.5))
            .unwrap();
        manager.add_bus(bus).unwrap();

        assert!(manager.validate_sends().is_err());
    }

    #[test]
    fn test_bus_manager_validate_sends_cycle() {
        let mut manager = AudioBusManager::new();
        let mut bus1 =
            AudioBus::with_config(AudioBusConfig::default().with_name("Bus1".to_string())).unwrap();
        let mut bus2 =
            AudioBus::with_config(AudioBusConfig::default().with_name("Bus2".to_string())).unwrap();

        bus1.add_send(SendEffect::new("Bus2".to_string(), 0.5))
            .unwrap();
        bus2.add_send(SendEffect::new("Bus1".to_string(), 0.5))
            .unwrap();

        manager.add_bus(bus1).unwrap();
        manager.add_bus(bus2).unwrap();

        assert!(manager.validate_sends().is_err());
    }

    #[test]
    fn test_bus_manager_validate_sends_valid() {
        let mut manager = AudioBusManager::new();
        let mut bus1 =
            AudioBus::with_config(AudioBusConfig::default().with_name("Bus1".to_string())).unwrap();
        let bus2 =
            AudioBus::with_config(AudioBusConfig::default().with_name("Bus2".to_string())).unwrap();

        bus1.add_send(SendEffect::new("Bus2".to_string(), 0.5))
            .unwrap();

        manager.add_bus(bus1).unwrap();
        manager.add_bus(bus2).unwrap();

        assert!(manager.validate_sends().is_ok());
    }

    #[test]
    fn test_bus_manager_update_smoothing() {
        let mut manager = AudioBusManager::new();
        let bus = AudioBus::new().unwrap();
        manager.add_bus(bus).unwrap();

        // Should not panic
        manager.update_smoothing(0.5);
    }
}
