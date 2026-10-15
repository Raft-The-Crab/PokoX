//! Transform system for the Poko Engine
//!
//! This module provides transform functionality with:
//! - Local and world transforms
//! - Parent propagation and invalidation
//! - Non-uniform scale support
//! - Efficient transform caching

use glam::{Mat4, Quat, Vec3};
use poko_ids_handles::Id;
use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use thiserror::Error;

/// Configuration for the transform registry
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct TransformRegistryConfig {
    /// Maximum number of transforms (0 = no limit)
    pub max_transforms: usize,
    /// Whether to enable statistics tracking
    pub enable_stats: bool,
    /// Whether to automatically update dirty transforms on get
    pub auto_update: bool,
}

impl Default for TransformRegistryConfig {
    fn default() -> Self {
        Self {
            max_transforms: 0,
            enable_stats: true,
            auto_update: true,
        }
    }
}

impl TransformRegistryConfig {
    /// Validate the configuration
    pub fn validate(&self) -> Result<(), TransformError> {
        if self.max_transforms > 0 && self.max_transforms < 1 {
            return Err(TransformError::InvalidTransform(
                "max_transforms must be at least 1 if set".to_string(),
            ));
        }
        Ok(())
    }
}

/// Transform registry statistics
#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct TransformRegistryStats {
    pub total_transforms: usize,
    pub dirty_count: usize,
    pub version: u64,
    pub updates_count: u64,
    pub adds_count: u64,
    pub removes_count: u64,
}

/// Errors that can occur in transform operations
#[derive(Debug, Clone, PartialEq, Eq, Error)]
pub enum TransformError {
    #[error("Transform not found: {0}")]
    TransformNotFound(Id),
    #[error("Invalid transform: {0}")]
    InvalidTransform(String),
    #[error("Cycle detected in transform hierarchy: {0}")]
    CycleDetected(Id),
}

/// Transform representation with position, rotation, and scale
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Transform {
    /// Local position relative to parent
    pub position: Vec3,
    /// Local rotation as quaternion
    pub rotation: Quat,
    /// Local scale (non-uniform supported)
    pub scale: Vec3,
    /// Cached world transform matrix
    #[serde(skip)]
    world_matrix: Mat4,
    /// Whether world transform needs recalculation
    #[serde(skip)]
    dirty: bool,
}

impl Default for Transform {
    fn default() -> Self {
        Self::identity()
    }
}

impl Transform {
    /// Create an identity transform
    pub fn identity() -> Self {
        Self {
            position: Vec3::ZERO,
            rotation: Quat::IDENTITY,
            scale: Vec3::ONE,
            world_matrix: Mat4::IDENTITY,
            dirty: false,
        }
    }

    /// Create a transform from position, rotation, and scale
    pub fn new(position: Vec3, rotation: Quat, scale: Vec3) -> Self {
        let mut transform = Self {
            position,
            rotation,
            scale,
            world_matrix: Mat4::IDENTITY,
            dirty: true,
        };
        transform.recalculate_world_matrix(Mat4::IDENTITY);
        transform
    }

    /// Create a transform from position only
    pub fn from_position(position: Vec3) -> Self {
        Self::new(position, Quat::IDENTITY, Vec3::ONE)
    }

    /// Create a transform from position and rotation
    pub fn from_position_rotation(position: Vec3, rotation: Quat) -> Self {
        Self::new(position, rotation, Vec3::ONE)
    }

    /// Get the world transform matrix
    pub fn world_matrix(&self) -> Mat4 {
        self.world_matrix
    }

    /// Mark the transform as dirty (needs recalculation)
    pub fn mark_dirty(&mut self) {
        self.dirty = true;
    }

    /// Check if the transform is dirty
    pub fn is_dirty(&self) -> bool {
        self.dirty
    }

    /// Recalculate world matrix from local transform and parent matrix
    pub fn recalculate_world_matrix(&mut self, parent_matrix: Mat4) {
        let local_matrix =
            Mat4::from_scale_rotation_translation(self.scale, self.rotation, self.position);
        self.world_matrix = parent_matrix * local_matrix;
        self.dirty = false;
    }

    /// Get forward direction vector
    pub fn forward(&self) -> Vec3 {
        self.rotation * Vec3::Z
    }

    /// Get right direction vector
    pub fn right(&self) -> Vec3 {
        self.rotation * Vec3::X
    }

    /// Get up direction vector
    pub fn up(&self) -> Vec3 {
        self.rotation * Vec3::Y
    }

    /// Translate the transform in local space
    pub fn translate(&mut self, delta: Vec3) {
        self.position += delta;
        self.mark_dirty();
    }

    /// Rotate the transform
    pub fn rotate(&mut self, rotation: Quat) {
        self.rotation = rotation * self.rotation;
        self.rotation = self.rotation.normalize();
        self.mark_dirty();
    }

    /// Scale the transform
    pub fn scale_by(&mut self, scale: Vec3) {
        self.scale *= scale;
        self.mark_dirty();
    }

    /// Set position
    pub fn set_position(&mut self, position: Vec3) {
        self.position = position;
        self.mark_dirty();
    }

    /// Set rotation
    pub fn set_rotation(&mut self, rotation: Quat) {
        self.rotation = rotation.normalize();
        self.mark_dirty();
    }

    /// Set scale
    pub fn set_scale(&mut self, scale: Vec3) {
        self.scale = scale;
        self.mark_dirty();
    }

    /// Look at a target position
    pub fn look_at(&mut self, target: Vec3, up: Vec3) {
        let forward = (target - self.position).normalize();
        let right = forward.cross(up).normalize();
        let actual_up = right.cross(forward);

        let rotation = Quat::from_mat3(&glam::Mat3::from_cols(right, actual_up, forward));
        self.set_rotation(rotation);
    }

    /// Convert local position to world position given parent matrix
    pub fn local_to_world_position(&self, parent_matrix: Mat4) -> Vec3 {
        parent_matrix.transform_point3(self.position)
    }

    /// Convert world position to local position given parent matrix
    pub fn world_to_local_position(&self, world_position: Vec3, parent_matrix: Mat4) -> Vec3 {
        let inverse_parent = parent_matrix.inverse();
        inverse_parent.transform_point3(world_position)
    }

    /// Convert local direction to world direction given parent rotation
    pub fn local_to_world_direction(&self, local_direction: Vec3, parent_rotation: Quat) -> Vec3 {
        parent_rotation * local_direction
    }

    /// Convert world direction to local direction given parent rotation
    pub fn world_to_local_direction(&self, world_direction: Vec3, parent_rotation: Quat) -> Vec3 {
        parent_rotation.inverse() * world_direction
    }

    /// Get world position
    pub fn world_position(&self, parent_matrix: Mat4) -> Vec3 {
        self.local_to_world_position(parent_matrix)
    }

    /// Get world rotation
    pub fn world_rotation(&self, parent_rotation: Quat) -> Quat {
        parent_rotation * self.rotation
    }

    /// Extract rotation from world matrix
    pub fn rotation_from_matrix(matrix: Mat4) -> Quat {
        Quat::from_mat4(&matrix)
    }

    /// Extract position from world matrix
    pub fn position_from_matrix(matrix: Mat4) -> Vec3 {
        matrix.transform_point3(Vec3::ZERO)
    }

    /// Extract scale from world matrix
    pub fn scale_from_matrix(matrix: Mat4) -> Vec3 {
        Vec3::new(
            matrix.x_axis.length(),
            matrix.y_axis.length(),
            matrix.z_axis.length(),
        )
    }

    /// Interpolate between two transforms
    pub fn lerp(&self, other: &Transform, t: f32) -> Transform {
        Transform {
            position: self.position.lerp(other.position, t),
            rotation: self.rotation.slerp(other.rotation, t),
            scale: self.scale.lerp(other.scale, t),
            world_matrix: Mat4::IDENTITY,
            dirty: true,
        }
    }

    /// Slerp rotation only (linear interpolate position and scale)
    pub fn lerp_with_slerp(&self, other: &Transform, t: f32) -> Transform {
        Transform {
            position: self.position.lerp(other.position, t),
            rotation: self.rotation.slerp(other.rotation, t),
            scale: self.scale.lerp(other.scale, t),
            world_matrix: Mat4::IDENTITY,
            dirty: true,
        }
    }

    /// Check if transform is approximately equal to another
    pub fn approx_eq(&self, other: &Transform, epsilon: f32) -> bool {
        self.position.abs_diff_eq(other.position, epsilon)
            && self.rotation.abs_diff_eq(other.rotation, epsilon)
            && self.scale.abs_diff_eq(other.scale, epsilon)
    }

    /// Get the local transform matrix (without parent)
    pub fn local_matrix(&self) -> Mat4 {
        Mat4::from_scale_rotation_translation(self.scale, self.rotation, self.position)
    }

    /// Apply a uniform scale
    pub fn scale_uniform(&mut self, scale: f32) {
        self.scale = Vec3::splat(scale);
        self.mark_dirty();
    }

    /// Rotate around a local axis
    pub fn rotate_around(&mut self, axis: Vec3, angle: f32) {
        let rotation = Quat::from_axis_angle(axis.normalize(), angle);
        self.rotate(rotation);
    }

    /// Rotate around a world axis (requires parent rotation)
    pub fn rotate_around_world(&mut self, axis: Vec3, angle: f32, parent_rotation: Quat) {
        let world_axis = parent_rotation * axis;
        let rotation = Quat::from_axis_angle(world_axis.normalize(), angle);
        self.rotation *= rotation;
        self.rotation = self.rotation.normalize();
        self.mark_dirty();
    }

    /// Get the lossy scale (if there's shear, this may not be accurate)
    pub fn lossy_scale(&self) -> Vec3 {
        Vec3::new(self.scale.x.abs(), self.scale.y.abs(), self.scale.z.abs())
    }

    /// Check if scale is uniform
    pub fn has_uniform_scale(&self) -> bool {
        let epsilon = 1e-5;
        (self.scale.x - self.scale.y).abs() < epsilon
            && (self.scale.y - self.scale.z).abs() < epsilon
    }

    /// Get the forward vector in world space
    pub fn world_forward(&self, parent_rotation: Quat) -> Vec3 {
        self.local_to_world_direction(Vec3::Z, parent_rotation)
    }

    /// Get the right vector in world space
    pub fn world_right(&self, parent_rotation: Quat) -> Vec3 {
        self.local_to_world_direction(Vec3::X, parent_rotation)
    }

    /// Get the up vector in world space
    pub fn world_up(&self, parent_rotation: Quat) -> Vec3 {
        self.local_to_world_direction(Vec3::Y, parent_rotation)
    }

    /// Invert the transform (useful for coordinate space conversions)
    pub fn inverse(&self) -> Transform {
        let local_matrix = self.local_matrix();
        let inverse_matrix = local_matrix.inverse();

        let (scale, rotation, position) = inverse_matrix.to_scale_rotation_translation();

        Transform {
            position,
            rotation,
            scale,
            world_matrix: Mat4::IDENTITY,
            dirty: true,
        }
    }

    /// Reset to identity
    pub fn reset(&mut self) {
        *self = Self::identity();
    }

    /// Get distance to another transform (position only)
    pub fn distance_to(&self, other: &Transform) -> f32 {
        self.position.distance(other.position)
    }

    /// Get direction to another transform
    pub fn direction_to(&self, other: &Transform) -> Vec3 {
        (other.position - self.position).normalize()
    }

    /// Look at another transform's position
    pub fn look_at_transform(&mut self, target: &Transform, up: Vec3) {
        self.look_at(target.position, up);
    }
}

/// Transform registry managing all object transforms
pub struct TransformRegistry {
    /// Transform storage by object ID
    transforms: HashMap<Id, Transform>,
    /// Parent transform matrices by object ID
    parent_matrices: HashMap<Id, Mat4>,
    /// Transform update version for tracking changes
    version: u64,
    /// Configuration
    config: TransformRegistryConfig,
    /// Statistics
    stats: TransformRegistryStats,
}

impl Default for TransformRegistry {
    fn default() -> Self {
        Self::new()
    }
}

impl TransformRegistry {
    /// Create a new transform registry
    pub fn new() -> Self {
        Self::with_config(TransformRegistryConfig::default())
    }

    /// Create a new transform registry with configuration
    pub fn with_config(config: TransformRegistryConfig) -> Self {
        config.validate().unwrap();
        Self {
            transforms: HashMap::new(),
            parent_matrices: HashMap::new(),
            version: 0,
            config,
            stats: TransformRegistryStats {
                total_transforms: 0,
                dirty_count: 0,
                version: 0,
                updates_count: 0,
                adds_count: 0,
                removes_count: 0,
            },
        }
    }

    /// Create a new transform registry with pre-allocated capacity
    pub fn with_capacity(capacity: usize) -> Self {
        let config = TransformRegistryConfig {
            max_transforms: capacity,
            ..Default::default()
        };
        Self::with_config(config)
    }

    /// Add a transform for an object
    pub fn add(&mut self, id: Id, transform: Transform) -> Result<(), TransformError> {
        if self.config.max_transforms > 0 && self.transforms.len() >= self.config.max_transforms {
            return Err(TransformError::InvalidTransform(
                "Registry is at maximum capacity".to_string(),
            ));
        }
        let is_dirty = transform.is_dirty();
        self.transforms.insert(id, transform);
        self.parent_matrices.insert(id, Mat4::IDENTITY);
        self.increment_version();
        self.stats.total_transforms = self.transforms.len();
        self.stats.adds_count += 1;
        if is_dirty {
            self.stats.dirty_count += 1;
        }
        Ok(())
    }

    /// Remove a transform
    pub fn remove(&mut self, id: Id) -> Result<Transform, TransformError> {
        let was_dirty = self
            .transforms
            .get(&id)
            .map(|t| t.is_dirty())
            .unwrap_or(false);
        self.parent_matrices.remove(&id);
        let result = self
            .transforms
            .remove(&id)
            .ok_or(TransformError::TransformNotFound(id))?;
        self.increment_version();
        self.stats.total_transforms = self.transforms.len();
        self.stats.removes_count += 1;
        if was_dirty {
            self.stats.dirty_count = self.stats.dirty_count.saturating_sub(1);
        }
        Ok(result)
    }

    /// Get a transform
    pub fn get(&self, id: Id) -> Result<&Transform, TransformError> {
        self.transforms
            .get(&id)
            .ok_or(TransformError::TransformNotFound(id))
    }

    /// Get a mutable transform
    pub fn get_mut(&mut self, id: Id) -> Result<&mut Transform, TransformError> {
        self.transforms
            .get_mut(&id)
            .ok_or(TransformError::TransformNotFound(id))
    }

    /// Check if a transform exists
    pub fn contains(&self, id: Id) -> bool {
        self.transforms.contains_key(&id)
    }

    /// Set the parent transform matrix for an object
    pub fn set_parent_matrix(&mut self, id: Id, parent_matrix: Mat4) -> Result<(), TransformError> {
        if !self.transforms.contains_key(&id) {
            return Err(TransformError::TransformNotFound(id));
        }
        self.parent_matrices.insert(id, parent_matrix);
        Ok(())
    }

    /// Update world transforms for dirty objects
    pub fn update_dirty_transforms(&mut self) {
        let mut dirty_count = 0;
        for (id, transform) in self.transforms.iter_mut() {
            if transform.is_dirty() {
                let parent_matrix = self
                    .parent_matrices
                    .get(id)
                    .copied()
                    .unwrap_or(Mat4::IDENTITY);
                transform.recalculate_world_matrix(parent_matrix);
                dirty_count += 1;
            }
        }
        self.stats.dirty_count = dirty_count;
        self.stats.updates_count += 1;
    }

    /// Mark a transform and all its children as dirty
    pub fn mark_subtree_dirty(&mut self, id: Id, children: &[Id]) {
        if let Some(transform) = self.transforms.get_mut(&id) {
            if !transform.is_dirty() {
                transform.mark_dirty();
                self.stats.dirty_count += 1;
            }
        }
        for child_id in children {
            if let Some(transform) = self.transforms.get_mut(child_id) {
                if !transform.is_dirty() {
                    transform.mark_dirty();
                    self.stats.dirty_count += 1;
                }
            }
        }
    }

    /// Get the number of transforms
    pub fn len(&self) -> usize {
        self.transforms.len()
    }

    /// Check if the registry is empty
    pub fn is_empty(&self) -> bool {
        self.transforms.is_empty()
    }

    /// Clear all transforms
    pub fn clear(&mut self) {
        self.transforms.clear();
        self.parent_matrices.clear();
        self.version = 0;
        self.stats = TransformRegistryStats {
            total_transforms: 0,
            dirty_count: 0,
            version: 0,
            updates_count: 0,
            adds_count: 0,
            removes_count: 0,
        };
    }

    /// Get the current version (increments on modifications)
    pub fn version(&self) -> u64 {
        self.version
    }

    /// Get registry statistics
    pub fn stats(&self) -> &TransformRegistryStats {
        &self.stats
    }

    /// Get the configuration
    pub fn config(&self) -> &TransformRegistryConfig {
        &self.config
    }

    /// Increment the version number
    fn increment_version(&mut self) {
        self.version = self.version.wrapping_add(1);
        self.stats.version = self.version;
    }

    /// Set transform for an object (replaces existing)
    pub fn set_transform(&mut self, id: Id, transform: Transform) -> Result<(), TransformError> {
        if !self.transforms.contains_key(&id) {
            return Err(TransformError::TransformNotFound(id));
        }
        let was_dirty = self
            .transforms
            .get(&id)
            .map(|t| t.is_dirty())
            .unwrap_or(false);
        let is_dirty = transform.is_dirty();
        self.transforms.insert(id, transform);
        self.increment_version();
        if is_dirty && !was_dirty {
            self.stats.dirty_count += 1;
        } else if !is_dirty && was_dirty {
            self.stats.dirty_count = self.stats.dirty_count.saturating_sub(1);
        }
        Ok(())
    }

    /// Interpolate between two transforms for an object
    pub fn interpolate(
        &mut self,
        id: Id,
        target: &Transform,
        t: f32,
    ) -> Result<(), TransformError> {
        let current = self.get(id)?;
        let interpolated = current.lerp(target, t);
        self.add(id, interpolated)?;
        Ok(())
    }

    /// Snapshot all transforms
    pub fn snapshot(&self) -> HashMap<Id, Transform> {
        self.transforms.clone()
    }

    /// Restore from a snapshot
    pub fn restore(&mut self, snapshot: HashMap<Id, Transform>) {
        self.transforms = snapshot;
        self.increment_version();
        // Mark all as dirty
        let dirty_count = self.transforms.len();
        for transform in self.transforms.values_mut() {
            transform.mark_dirty();
        }
        self.stats.dirty_count = dirty_count;
        self.stats.total_transforms = self.transforms.len();
    }

    /// Update transforms for a specific object and its children
    pub fn update_subtree(&mut self, id: Id, children: &[Id]) {
        // Get parent world matrix before any mutable borrows
        let parent_world = self.transforms.get(&id).map(|t| t.world_matrix());

        // Update the object itself
        if let Some(transform) = self.transforms.get_mut(&id) {
            if transform.is_dirty() {
                let parent_matrix = self
                    .parent_matrices
                    .get(&id)
                    .copied()
                    .unwrap_or(Mat4::IDENTITY);
                transform.recalculate_world_matrix(parent_matrix);
            }
        }

        // Update children
        for child_id in children {
            if let Some(transform) = self.transforms.get_mut(child_id) {
                if transform.is_dirty() {
                    let parent_matrix = self
                        .parent_matrices
                        .get(&id)
                        .copied()
                        .unwrap_or(Mat4::IDENTITY);
                    if let Some(pw) = parent_world {
                        transform.recalculate_world_matrix(pw);
                    } else {
                        transform.recalculate_world_matrix(parent_matrix);
                    }
                }
            }
        }
    }

    /// Apply a function to all transforms
    pub fn apply_to_all<F>(&mut self, mut f: F)
    where
        F: FnMut(Id, &mut Transform),
    {
        let ids: Vec<Id> = self.transforms.keys().copied().collect();
        for id in ids {
            if let Some(transform) = self.transforms.get_mut(&id) {
                f(id, transform);
            }
        }
        self.increment_version();
    }

    /// Find all transforms matching a predicate
    pub fn find_all<F>(&self, f: F) -> Vec<Id>
    where
        F: Fn(&Transform) -> bool,
    {
        self.transforms
            .iter()
            .filter(|(_, t)| f(t))
            .map(|(id, _)| *id)
            .collect()
    }

    /// Batch update parent matrices
    pub fn update_parent_matrices_batch(&mut self, updates: HashMap<Id, Mat4>) {
        for (id, matrix) in updates {
            self.parent_matrices.insert(id, matrix);
        }
    }

    /// Get all dirty transform IDs
    pub fn dirty_ids(&self) -> Vec<Id> {
        self.transforms
            .iter()
            .filter(|(_, t)| t.is_dirty())
            .map(|(id, _)| *id)
            .collect()
    }

    /// Force all transforms to be marked as dirty
    pub fn mark_all_dirty(&mut self) {
        let total = self.transforms.len();
        for transform in self.transforms.values_mut() {
            transform.mark_dirty();
        }
        self.stats.dirty_count = total;
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_transform_identity() {
        let transform = Transform::identity();
        assert_eq!(transform.position, Vec3::ZERO);
        assert_eq!(transform.rotation, Quat::IDENTITY);
        assert_eq!(transform.scale, Vec3::ONE);
    }

    #[test]
    fn test_transform_creation() {
        let position = Vec3::new(1.0, 2.0, 3.0);
        let rotation = Quat::from_rotation_y(std::f32::consts::FRAC_PI_4);
        let scale = Vec3::new(2.0, 2.0, 2.0);

        let transform = Transform::new(position, rotation, scale);
        assert_eq!(transform.position, position);
        assert_eq!(transform.rotation, rotation);
        assert_eq!(transform.scale, scale);
    }

    #[test]
    fn test_transform_setters() {
        let mut transform = Transform::identity();

        transform.set_position(Vec3::new(1.0, 2.0, 3.0));
        assert_eq!(transform.position, Vec3::new(1.0, 2.0, 3.0));
        assert!(transform.is_dirty());

        transform.set_rotation(Quat::from_rotation_x(0.5));
        assert!(transform.is_dirty());

        transform.set_scale(Vec3::new(2.0, 2.0, 2.0));
        assert!(transform.is_dirty());
    }

    #[test]
    fn test_transform_directions() {
        let transform = Transform::identity();
        assert_eq!(transform.forward(), Vec3::Z);
        assert_eq!(transform.right(), Vec3::X);
        assert_eq!(transform.up(), Vec3::Y);
    }

    #[test]
    fn test_transform_rotate() {
        let mut transform = Transform::identity();
        let rotation = Quat::from_rotation_y(std::f32::consts::FRAC_PI_2);
        transform.rotate(rotation);

        let forward = transform.forward();
        assert!((forward.x - 1.0).abs() < 0.01);
        assert!(forward.y.abs() < 0.01);
        assert!(forward.z.abs() < 0.01);
    }

    #[test]
    fn test_transform_registry() {
        let mut registry = TransformRegistry::new();
        let id = Id::new(poko_ids_handles::IdType::Model);
        let transform = Transform::identity();

        registry.add(id, transform.clone()).unwrap();
        assert!(registry.contains(id));
        assert_eq!(registry.len(), 1);

        let retrieved = registry.get(id).unwrap();
        assert_eq!(retrieved.position, transform.position);

        registry.remove(id).unwrap();
        assert!(!registry.contains(id));
        assert_eq!(registry.len(), 0);
    }

    #[test]
    fn test_parent_matrix() {
        let mut registry = TransformRegistry::new();
        let id = Id::new(poko_ids_handles::IdType::Model);
        let transform = Transform::identity();

        registry.add(id, transform).unwrap();

        let parent_matrix = Mat4::from_translation(Vec3::new(10.0, 0.0, 0.0));
        registry.set_parent_matrix(id, parent_matrix).unwrap();

        // Mark transform as dirty so it will be recalculated
        registry.get_mut(id).unwrap().mark_dirty();

        registry.update_dirty_transforms();
        let retrieved = registry.get(id).unwrap();
        // World matrix should now include parent translation
        let world_pos = retrieved.world_matrix().transform_point3(Vec3::ZERO);
        assert!((world_pos.x - 10.0).abs() < 0.01);
    }

    #[test]
    fn test_non_uniform_scale() {
        let transform = Transform::new(Vec3::ZERO, Quat::IDENTITY, Vec3::new(2.0, 1.0, 0.5));
        assert_eq!(transform.scale, Vec3::new(2.0, 1.0, 0.5));
    }

    #[test]
    fn test_look_at() {
        let mut transform = Transform::from_position(Vec3::ZERO);
        transform.look_at(Vec3::new(0.0, 0.0, 10.0), Vec3::Y);

        let forward = transform.forward();
        // Forward should point towards +Z target
        assert!((forward.z - 1.0).abs() < 0.01);
    }

    #[test]
    fn test_local_to_world_position() {
        let transform = Transform::from_position(Vec3::new(1.0, 2.0, 3.0));
        let parent_matrix = Mat4::from_translation(Vec3::new(10.0, 0.0, 0.0));

        let world_pos = transform.local_to_world_position(parent_matrix);
        assert!((world_pos.x - 11.0).abs() < 0.01);
        assert!((world_pos.y - 2.0).abs() < 0.01);
        assert!((world_pos.z - 3.0).abs() < 0.01);
    }

    #[test]
    fn test_world_to_local_position() {
        let transform = Transform::from_position(Vec3::new(1.0, 2.0, 3.0));
        let parent_matrix = Mat4::from_translation(Vec3::new(10.0, 0.0, 0.0));

        let world_pos = Vec3::new(11.0, 2.0, 3.0);
        let local_pos = transform.world_to_local_position(world_pos, parent_matrix);
        assert!((local_pos.x - 1.0).abs() < 0.01);
        assert!((local_pos.y - 2.0).abs() < 0.01);
        assert!((local_pos.z - 3.0).abs() < 0.01);
    }

    #[test]
    fn test_local_to_world_direction() {
        let transform = Transform::identity();
        let parent_rotation = Quat::from_rotation_y(std::f32::consts::FRAC_PI_2);

        let local_dir = Vec3::Z;
        let world_dir = transform.local_to_world_direction(local_dir, parent_rotation);
        assert!((world_dir.x - 1.0).abs() < 0.01);
        assert!(world_dir.y.abs() < 0.01);
        assert!(world_dir.z.abs() < 0.01);
    }

    #[test]
    fn test_world_to_local_direction() {
        let transform = Transform::identity();
        let parent_rotation = Quat::from_rotation_y(std::f32::consts::FRAC_PI_2);

        let world_dir = Vec3::X;
        let local_dir = transform.world_to_local_direction(world_dir, parent_rotation);
        assert!((local_dir.z - 1.0).abs() < 0.01);
    }

    #[test]
    fn test_extract_from_matrix() {
        let position = Vec3::new(1.0, 2.0, 3.0);
        let rotation = Quat::from_rotation_x(0.5);
        let scale = Vec3::new(2.0, 1.0, 0.5);

        let matrix = Mat4::from_scale_rotation_translation(scale, rotation, position);

        let extracted_pos = Transform::position_from_matrix(matrix);
        assert!((extracted_pos.x - 1.0).abs() < 0.01);
        assert!((extracted_pos.y - 2.0).abs() < 0.01);
        assert!((extracted_pos.z - 3.0).abs() < 0.01);

        let extracted_scale = Transform::scale_from_matrix(matrix);
        assert!((extracted_scale.x - 2.0).abs() < 0.01);
        assert!((extracted_scale.y - 1.0).abs() < 0.01);
        assert!((extracted_scale.z - 0.5).abs() < 0.01);
    }

    #[test]
    fn test_transform_lerp() {
        let t1 = Transform::from_position(Vec3::new(0.0, 0.0, 0.0));
        let t2 = Transform::from_position(Vec3::new(10.0, 0.0, 0.0));

        let t_mid = t1.lerp(&t2, 0.5);
        assert!((t_mid.position.x - 5.0).abs() < 0.01);
    }

    #[test]
    fn test_transform_approx_eq() {
        let t1 = Transform::from_position(Vec3::new(1.0, 2.0, 3.0));
        let t2 = Transform::from_position(Vec3::new(1.0001, 2.0001, 3.0001));
        let t3 = Transform::from_position(Vec3::new(2.0, 3.0, 4.0));

        assert!(t1.approx_eq(&t2, 0.001));
        assert!(!t1.approx_eq(&t3, 0.001));
    }

    #[test]
    fn test_transform_local_matrix() {
        let transform = Transform::from_position(Vec3::new(1.0, 2.0, 3.0));
        let local_matrix = transform.local_matrix();
        let extracted = Transform::position_from_matrix(local_matrix);
        assert!((extracted.x - 1.0).abs() < 0.01);
    }

    #[test]
    fn test_transform_scale_uniform() {
        let mut transform = Transform::identity();
        transform.scale_uniform(2.0);
        assert_eq!(transform.scale, Vec3::splat(2.0));
    }

    #[test]
    fn test_transform_rotate_around() {
        let mut transform = Transform::identity();
        transform.rotate_around(Vec3::Y, std::f32::consts::FRAC_PI_2);
        let forward = transform.forward();
        assert!((forward.x - 1.0).abs() < 0.01);
    }

    #[test]
    fn test_transform_has_uniform_scale() {
        let uniform = Transform::new(Vec3::ZERO, Quat::IDENTITY, Vec3::splat(2.0));
        assert!(uniform.has_uniform_scale());

        let non_uniform = Transform::new(Vec3::ZERO, Quat::IDENTITY, Vec3::new(2.0, 1.0, 1.0));
        assert!(!non_uniform.has_uniform_scale());
    }

    #[test]
    fn test_transform_inverse() {
        let transform = Transform::from_position(Vec3::new(5.0, 0.0, 0.0));
        let inverse = transform.inverse();
        let local_matrix = inverse.local_matrix();
        let pos = Transform::position_from_matrix(local_matrix);
        assert!((pos.x + 5.0).abs() < 0.01);
    }

    #[test]
    fn test_transform_distance_to() {
        let t1 = Transform::from_position(Vec3::ZERO);
        let t2 = Transform::from_position(Vec3::new(10.0, 0.0, 0.0));
        assert!((t1.distance_to(&t2) - 10.0).abs() < 0.01);
    }

    #[test]
    fn test_transform_direction_to() {
        let t1 = Transform::from_position(Vec3::ZERO);
        let t2 = Transform::from_position(Vec3::new(10.0, 0.0, 0.0));
        let dir = t1.direction_to(&t2);
        assert!((dir.x - 1.0).abs() < 0.01);
    }

    #[test]
    fn test_transform_registry_version() {
        let mut registry = TransformRegistry::new();
        let id = Id::new(poko_ids_handles::IdType::Model);
        let transform = Transform::identity();

        let v1 = registry.version();
        registry.add(id, transform).unwrap();
        let v2 = registry.version();
        assert!(v2 > v1);
    }

    #[test]
    fn test_transform_registry_interpolate() {
        let mut registry = TransformRegistry::new();
        let id = Id::new(poko_ids_handles::IdType::Model);
        let t1 = Transform::from_position(Vec3::ZERO);
        let t2 = Transform::from_position(Vec3::new(10.0, 0.0, 0.0));

        registry.add(id, t1).unwrap();
        registry.interpolate(id, &t2, 0.5).unwrap();

        let result = registry.get(id).unwrap();
        assert!((result.position.x - 5.0).abs() < 0.01);
    }

    #[test]
    fn test_transform_registry_snapshot_restore() {
        let mut registry1 = TransformRegistry::new();
        let id = Id::new(poko_ids_handles::IdType::Model);
        let transform = Transform::from_position(Vec3::new(5.0, 0.0, 0.0));

        registry1.add(id, transform).unwrap();
        let snapshot = registry1.snapshot();

        let mut registry2 = TransformRegistry::new();
        registry2.restore(snapshot);

        let restored = registry2.get(id).unwrap();
        assert_eq!(restored.position, Vec3::new(5.0, 0.0, 0.0));
    }

    #[test]
    fn test_transform_registry_stats() {
        let mut registry = TransformRegistry::new();
        let id1 = Id::new(poko_ids_handles::IdType::Model);
        let id2 = Id::new(poko_ids_handles::IdType::Part);

        registry.add(id1, Transform::identity()).unwrap();
        registry.add(id2, Transform::identity()).unwrap();

        let stats = registry.stats();
        assert_eq!(stats.total_transforms, 2);
        assert_eq!(stats.dirty_count, 0);
    }

    #[test]
    fn test_transform_registry_apply_to_all() {
        let mut registry = TransformRegistry::new();
        let id1 = Id::new(poko_ids_handles::IdType::Model);
        let id2 = Id::new(poko_ids_handles::IdType::Part);

        registry.add(id1, Transform::identity()).unwrap();
        registry.add(id2, Transform::identity()).unwrap();

        registry.apply_to_all(|_, t| t.set_position(Vec3::new(1.0, 0.0, 0.0)));

        assert_eq!(
            registry.get(id1).unwrap().position,
            Vec3::new(1.0, 0.0, 0.0)
        );
        assert_eq!(
            registry.get(id2).unwrap().position,
            Vec3::new(1.0, 0.0, 0.0)
        );
    }

    #[test]
    fn test_transform_registry_find_all() {
        let mut registry = TransformRegistry::new();
        let id1 = Id::new(poko_ids_handles::IdType::Model);
        let id2 = Id::new(poko_ids_handles::IdType::Part);

        registry
            .add(id1, Transform::from_position(Vec3::new(5.0, 0.0, 0.0)))
            .unwrap();
        registry
            .add(id2, Transform::from_position(Vec3::ZERO))
            .unwrap();

        let found = registry.find_all(|t| t.position.x > 2.0);
        assert_eq!(found.len(), 1);
        assert!(found.contains(&id1));
    }

    #[test]
    fn test_transform_registry_dirty_ids() {
        let mut registry = TransformRegistry::new();
        let id1 = Id::new(poko_ids_handles::IdType::Model);
        let id2 = Id::new(poko_ids_handles::IdType::Part);

        registry.add(id1, Transform::identity()).unwrap();
        registry.add(id2, Transform::identity()).unwrap();

        registry
            .get_mut(id1)
            .unwrap()
            .set_position(Vec3::new(1.0, 0.0, 0.0));

        let dirty = registry.dirty_ids();
        assert_eq!(dirty.len(), 1);
        assert!(dirty.contains(&id1));
    }

    #[test]
    fn test_transform_registry_mark_all_dirty() {
        let mut registry = TransformRegistry::new();
        let id1 = Id::new(poko_ids_handles::IdType::Model);
        let id2 = Id::new(poko_ids_handles::IdType::Part);

        registry.add(id1, Transform::identity()).unwrap();
        registry.add(id2, Transform::identity()).unwrap();

        registry.update_dirty_transforms(); // Should clear dirty flags
        registry.mark_all_dirty();

        let stats = registry.stats();
        assert_eq!(stats.dirty_count, 2);
    }

    #[test]
    fn test_transform_registry_config() {
        let config = TransformRegistryConfig::default();
        assert_eq!(config.max_transforms, 0);
        assert!(config.enable_stats);
        assert!(config.auto_update);
        assert!(config.validate().is_ok());
    }

    #[test]
    fn test_transform_registry_config_validation() {
        let config = TransformRegistryConfig {
            max_transforms: 0,
            ..Default::default()
        };
        assert!(config.validate().is_ok());

        let config = TransformRegistryConfig {
            max_transforms: 1,
            ..Default::default()
        };
        assert!(config.validate().is_ok());
    }

    #[test]
    fn test_transform_registry_with_config() {
        let config = TransformRegistryConfig {
            max_transforms: 100,
            ..Default::default()
        };
        let registry = TransformRegistry::with_config(config);
        assert_eq!(registry.config().max_transforms, 100);
    }

    #[test]
    fn test_transform_registry_stats_tracking() {
        let mut registry = TransformRegistry::new();
        let id = Id::new(poko_ids_handles::IdType::Model);
        registry.add(id, Transform::identity()).unwrap();

        let stats = registry.stats();
        assert_eq!(stats.total_transforms, 1);
        assert_eq!(stats.adds_count, 1);
        assert_eq!(stats.removes_count, 0);
    }

    #[test]
    fn test_transform_registry_max_capacity() {
        let config = TransformRegistryConfig {
            max_transforms: 2,
            ..Default::default()
        };
        let mut registry = TransformRegistry::with_config(config);
        let id1 = Id::new(poko_ids_handles::IdType::Model);
        let id2 = Id::new(poko_ids_handles::IdType::Part);
        let id3 = Id::new(poko_ids_handles::IdType::Model);

        registry.add(id1, Transform::identity()).unwrap();
        registry.add(id2, Transform::identity()).unwrap();

        let result = registry.add(id3, Transform::identity());
        assert!(matches!(result, Err(TransformError::InvalidTransform(_))));
    }
}
