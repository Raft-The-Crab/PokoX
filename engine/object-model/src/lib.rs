//! Object model for the Poko Engine
//!
//! This module provides the Workspace → Model → Part object hierarchy with:
//! - Parenting relationships with cycle prevention
//! - Lifecycle events (create, destroy, reparent)
//! - Child enumeration and hierarchy traversal
//! - Integration with the IDs/handles system

use poko_ids_handles::{Handle, HandleRegistry, Id, IdType, RegistryError, RegistryStats};
use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use thiserror::Error;

/// Errors that can occur in the object model
#[derive(Debug, Clone, PartialEq, Eq, Error)]
pub enum ObjectModelError {
    #[error("Parent not found: {0}")]
    ParentNotFound(Id),
    #[error("Cycle detected in hierarchy: {0} would create a cycle")]
    CycleDetected(Id),
    #[error("Object not found: {0}")]
    ObjectNotFound(Id),
    #[error("Registry error: {0}")]
    RegistryError(#[from] RegistryError),
    #[error("Invalid operation: {0}")]
    InvalidOperation(String),
}

/// Object lifecycle events
#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub enum LifecycleEvent {
    /// Object was created
    Created { id: Id },
    /// Object was destroyed
    Destroyed { id: Id },
    /// Object was reparented
    Reparented {
        id: Id,
        old_parent: Option<Id>,
        new_parent: Option<Id>,
    },
    /// Object was modified
    Modified { id: Id },
    /// Object name was changed
    Renamed {
        id: Id,
        old_name: String,
        new_name: String,
    },
    /// Child was added to an object
    ChildAdded { parent_id: Id, child_id: Id },
    /// Child was removed from an object
    ChildRemoved { parent_id: Id, child_id: Id },
}

/// Object type for hierarchy classification
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum ObjectType {
    Workspace,
    Model,
    Part,
}

/// Core object data shared across all object types
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ObjectData {
    /// Unique ID for the object
    pub id: Id,
    /// Object type
    pub object_type: ObjectType,
    /// Parent ID (if any)
    pub parent: Option<Id>,
    /// Child IDs
    pub children: Vec<Id>,
    /// Name of the object
    pub name: String,
    /// Whether the object is marked for destruction
    pub destroyed: bool,
    /// Custom metadata key-value pairs
    pub metadata: HashMap<String, String>,
}

impl ObjectData {
    /// Create new object data
    pub fn new(id: Id, object_type: ObjectType, name: String) -> Self {
        Self {
            id,
            object_type,
            parent: None,
            children: Vec::new(),
            name,
            destroyed: false,
            metadata: HashMap::new(),
        }
    }

    /// Check if this object has a parent
    pub fn has_parent(&self) -> bool {
        self.parent.is_some()
    }

    /// Check if this object has children
    pub fn has_children(&self) -> bool {
        !self.children.is_empty()
    }

    /// Get the number of children
    pub fn child_count(&self) -> usize {
        self.children.len()
    }
}

/// Registry statistics
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct RegistryStatistics {
    pub total_objects: usize,
    pub workspaces: usize,
    pub models: usize,
    pub parts: usize,
    pub total_children: usize,
    pub max_depth: usize,
}

/// Workspace: Top-level container for models
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Workspace {
    /// Handle to the workspace object
    pub handle: Handle,
    /// Workspace data
    pub data: ObjectData,
}

impl Workspace {
    /// Create a new workspace
    pub fn new(handle: Handle, name: String) -> Self {
        let id = Id::new(IdType::Workspace);
        Self {
            handle,
            data: ObjectData::new(id, ObjectType::Workspace, name),
        }
    }
}

/// Model: Container for parts within a workspace
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Model {
    /// Handle to the model object
    pub handle: Handle,
    /// Model data
    pub data: ObjectData,
}

impl Model {
    /// Create a new model
    pub fn new(handle: Handle, name: String) -> Self {
        let id = Id::new(IdType::Model);
        Self {
            handle,
            data: ObjectData::new(id, ObjectType::Model, name),
        }
    }
}

/// Part: Individual object within a model
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Part {
    /// Handle to the part object
    pub handle: Handle,
    /// Part data
    pub data: ObjectData,
}

impl Part {
    /// Create a new part
    pub fn new(handle: Handle, name: String) -> Self {
        let id = Id::new(IdType::Part);
        Self {
            handle,
            data: ObjectData::new(id, ObjectType::Part, name),
        }
    }
}

/// Serializable representation of an object (without runtime handle)
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct SerializableObject {
    pub id: Id,
    pub object_type: ObjectType,
    pub parent: Option<Id>,
    pub children: Vec<Id>,
    pub name: String,
    pub destroyed: bool,
    pub metadata: HashMap<String, String>,
}

impl From<Object> for SerializableObject {
    fn from(obj: Object) -> Self {
        let data = obj.data();
        SerializableObject {
            id: data.id,
            object_type: data.object_type,
            parent: data.parent,
            children: data.children.clone(),
            name: data.name.clone(),
            destroyed: data.destroyed,
            metadata: data.metadata.clone(),
        }
    }
}

/// Unified object enum for the registry
#[derive(Debug, Clone)]
pub enum Object {
    Workspace(Workspace),
    Model(Model),
    Part(Part),
}

impl Object {
    /// Get the object's ID
    pub fn id(&self) -> Id {
        match self {
            Object::Workspace(w) => w.data.id,
            Object::Model(m) => m.data.id,
            Object::Part(p) => p.data.id,
        }
    }

    /// Get the object's handle
    pub fn handle(&self) -> Handle {
        match self {
            Object::Workspace(w) => w.handle,
            Object::Model(m) => m.handle,
            Object::Part(p) => p.handle,
        }
    }

    /// Get the object's data
    pub fn data(&self) -> &ObjectData {
        match self {
            Object::Workspace(w) => &w.data,
            Object::Model(m) => &m.data,
            Object::Part(p) => &p.data,
        }
    }

    /// Get mutable object data
    pub fn data_mut(&mut self) -> &mut ObjectData {
        match self {
            Object::Workspace(w) => &mut w.data,
            Object::Model(m) => &mut m.data,
            Object::Part(p) => &mut p.data,
        }
    }

    /// Get the object's type
    pub fn object_type(&self) -> ObjectType {
        self.data().object_type
    }
}

/// Registry for managing the object hierarchy
pub struct ObjectRegistry {
    /// Object storage
    objects: HandleRegistry<Object>,
    /// ID to handle mapping for fast lookup
    id_to_handle: HashMap<Id, Handle>,
    /// Lifecycle event listeners
    event_listeners: Vec<Box<dyn Fn(LifecycleEvent) + Send>>,
}

impl ObjectRegistry {
    /// Create a new object registry
    pub fn new() -> Self {
        Self {
            objects: HandleRegistry::new(),
            id_to_handle: HashMap::new(),
            event_listeners: Vec::new(),
        }
    }

    /// Create a new workspace
    pub fn create_workspace(&mut self, name: String) -> Result<Workspace, ObjectModelError> {
        let handle = self.objects.allocate(Object::Workspace(Workspace::new(
            Handle::null(), // Will be set after allocation
            name.clone(),
        )))?;

        // Update the handle in the workspace
        if let Some(obj) = self.objects.get_mut(handle).ok() {
            if let Object::Workspace(workspace) = obj {
                workspace.handle = handle;
                let id = workspace.data.id;
                self.id_to_handle.insert(id, handle);
                self.emit_event(LifecycleEvent::Created { id });
            }
        }

        let workspace = self.objects.get(handle)?.clone();
        if let Object::Workspace(w) = workspace {
            Ok(w)
        } else {
            Err(ObjectModelError::InvalidOperation(
                "Expected workspace".to_string(),
            ))
        }
    }

    /// Create a new model
    pub fn create_model(&mut self, parent: Id, name: String) -> Result<Model, ObjectModelError> {
        let handle = self
            .objects
            .allocate(Object::Model(Model::new(Handle::null(), name.clone())))?;

        // Update the handle and set parent
        if let Some(obj) = self.objects.get_mut(handle).ok() {
            if let Object::Model(model) = obj {
                model.handle = handle;
                model.data.parent = Some(parent);
                let id = model.data.id;
                self.id_to_handle.insert(id, handle);

                // Add to parent's children
                if let Some(parent_handle) = self.id_to_handle.get(&parent) {
                    if let Some(parent_obj) = self.objects.get_mut(*parent_handle).ok() {
                        parent_obj.data_mut().children.push(id);
                        self.emit_event(LifecycleEvent::ChildAdded {
                            parent_id: parent,
                            child_id: id,
                        });
                    }
                }

                self.emit_event(LifecycleEvent::Created { id });
            }
        }

        let model = self.objects.get(handle)?.clone();
        if let Object::Model(m) = model {
            Ok(m)
        } else {
            Err(ObjectModelError::InvalidOperation(
                "Expected model".to_string(),
            ))
        }
    }

    /// Create a new part
    pub fn create_part(&mut self, parent: Id, name: String) -> Result<Part, ObjectModelError> {
        let handle = self
            .objects
            .allocate(Object::Part(Part::new(Handle::null(), name.clone())))?;

        // Update the handle and set parent
        if let Some(obj) = self.objects.get_mut(handle).ok() {
            if let Object::Part(part) = obj {
                part.handle = handle;
                part.data.parent = Some(parent);
                let id = part.data.id;
                self.id_to_handle.insert(id, handle);

                // Add to parent's children
                if let Some(parent_handle) = self.id_to_handle.get(&parent) {
                    if let Some(parent_obj) = self.objects.get_mut(*parent_handle).ok() {
                        parent_obj.data_mut().children.push(id);
                        self.emit_event(LifecycleEvent::ChildAdded {
                            parent_id: parent,
                            child_id: id,
                        });
                    }
                }

                self.emit_event(LifecycleEvent::Created { id });
            }
        }

        let part = self.objects.get(handle)?.clone();
        if let Object::Part(p) = part {
            Ok(p)
        } else {
            Err(ObjectModelError::InvalidOperation(
                "Expected part".to_string(),
            ))
        }
    }

    /// Get an object by ID
    pub fn get_object(&self, id: Id) -> Result<Object, ObjectModelError> {
        let handle = self
            .id_to_handle
            .get(&id)
            .ok_or(ObjectModelError::ObjectNotFound(id))?;
        let obj = self.objects.get(*handle)?;
        Ok(obj.clone())
    }

    /// Check for cycles before setting parent
    fn check_cycle(&self, child_id: Id, parent_id: Id) -> Result<(), ObjectModelError> {
        let mut current = Some(parent_id);
        let mut visited = std::collections::HashSet::new();

        while let Some(curr_id) = current {
            if curr_id == child_id {
                return Err(ObjectModelError::CycleDetected(child_id));
            }
            if visited.contains(&curr_id) {
                break; // Already checked this path
            }
            visited.insert(curr_id);

            current = self
                .get_object(curr_id)
                .ok()
                .and_then(|obj| obj.data().parent);
        }

        Ok(())
    }

    /// Reparent an object
    pub fn reparent(&mut self, id: Id, new_parent: Option<Id>) -> Result<(), ObjectModelError> {
        let handle = self
            .id_to_handle
            .get(&id)
            .ok_or(ObjectModelError::ObjectNotFound(id))?;

        // Get current object
        let obj = self.objects.get(*handle)?;
        let old_parent = obj.data().parent;

        // Check for cycles if setting a new parent
        if let Some(new_parent_id) = new_parent {
            if !self.id_to_handle.contains_key(&new_parent_id) {
                return Err(ObjectModelError::ParentNotFound(new_parent_id));
            }
            self.check_cycle(id, new_parent_id)?;
        }

        // Remove from old parent's children
        if let Some(old_parent_id) = old_parent {
            if let Some(old_parent_handle) = self.id_to_handle.get(&old_parent_id) {
                if let Some(parent_obj) = self.objects.get_mut(*old_parent_handle).ok() {
                    parent_obj
                        .data_mut()
                        .children
                        .retain(|&child_id| child_id != id);
                    self.emit_event(LifecycleEvent::ChildRemoved {
                        parent_id: old_parent_id,
                        child_id: id,
                    });
                }
            }
        }

        // Update object's parent
        if let Some(obj) = self.objects.get_mut(*handle).ok() {
            obj.data_mut().parent = new_parent;
        }

        // Add to new parent's children
        if let Some(new_parent_id) = new_parent {
            if let Some(new_parent_handle) = self.id_to_handle.get(&new_parent_id) {
                if let Some(parent_obj) = self.objects.get_mut(*new_parent_handle).ok() {
                    parent_obj.data_mut().children.push(id);
                    self.emit_event(LifecycleEvent::ChildAdded {
                        parent_id: new_parent_id,
                        child_id: id,
                    });
                }
            }
        }

        self.emit_event(LifecycleEvent::Reparented {
            id,
            old_parent,
            new_parent,
        });

        Ok(())
    }

    /// Destroy an object and its children (cascade)
    pub fn destroy(&mut self, id: Id) -> Result<(), ObjectModelError> {
        let handle = self
            .id_to_handle
            .get(&id)
            .ok_or(ObjectModelError::ObjectNotFound(id))?
            .clone();

        // Get children before destruction
        let children: Vec<Id> = self.objects.get(handle)?.data().children.clone();

        // Destroy children first (recursive)
        for child_id in children {
            self.destroy(child_id)?;
        }

        // Remove from parent's children
        let parent_id = {
            let obj = self.objects.get(handle)?;
            obj.data().parent
        };

        if let Some(parent_id) = parent_id {
            if let Some(parent_handle) = self.id_to_handle.get(&parent_id).cloned() {
                if let Some(parent_obj) = self.objects.get_mut(parent_handle).ok() {
                    parent_obj
                        .data_mut()
                        .children
                        .retain(|&child_id| child_id != id);
                }
            }
        }

        // Remove from registry
        self.id_to_handle.remove(&id);
        self.objects.remove(handle)?;

        self.emit_event(LifecycleEvent::Destroyed { id });

        Ok(())
    }

    /// Get all children of an object
    pub fn get_children(&self, id: Id) -> Result<Vec<Object>, ObjectModelError> {
        let handle = self
            .id_to_handle
            .get(&id)
            .ok_or(ObjectModelError::ObjectNotFound(id))?;

        let obj = self.objects.get(*handle)?;
        let mut children = Vec::new();

        for child_id in &obj.data().children {
            if let Ok(child) = self.get_object(*child_id) {
                children.push(child);
            }
        }

        Ok(children)
    }

    /// Add a lifecycle event listener
    pub fn add_event_listener<F>(&mut self, listener: F)
    where
        F: Fn(LifecycleEvent) + Send + 'static,
    {
        self.event_listeners.push(Box::new(listener));
    }

    /// Emit a lifecycle event
    fn emit_event(&self, event: LifecycleEvent) {
        for listener in &self.event_listeners {
            listener(event.clone());
        }
    }

    /// Get registry statistics
    pub fn stats(&self) -> RegistryStats {
        self.objects.stats()
    }

    /// Get the depth of an object in the hierarchy (0 for workspace)
    pub fn get_depth(&self, id: Id) -> Result<usize, ObjectModelError> {
        let mut depth = 0;
        let mut current = self.get_object(id)?.data().parent;

        while let Some(parent_id) = current {
            depth += 1;
            current = self.get_object(parent_id)?.data().parent;
        }

        Ok(depth)
    }

    /// Get the path from workspace to an object
    pub fn get_path(&self, id: Id) -> Result<Vec<Id>, ObjectModelError> {
        let mut path = Vec::new();
        let mut current = Some(id);

        while let Some(curr_id) = current {
            let obj = self.get_object(curr_id)?;
            current = obj.data().parent;
            path.insert(0, curr_id);
        }

        Ok(path)
    }

    /// Traverse hierarchy depth-first starting from an object
    pub fn traverse_depth_first<F>(&self, id: Id, mut visitor: F) -> Result<(), ObjectModelError>
    where
        F: FnMut(&Object) -> bool,
    {
        self.traverse_depth_first_impl(id, &mut visitor)
    }

    fn traverse_depth_first_impl<F>(&self, id: Id, visitor: &mut F) -> Result<(), ObjectModelError>
    where
        F: FnMut(&Object) -> bool,
    {
        let obj = self.get_object(id)?;
        if !visitor(&obj) {
            return Ok(());
        }

        let children: Vec<Id> = obj.data().children.clone();
        for child_id in children {
            self.traverse_depth_first_impl(child_id, visitor)?;
        }

        Ok(())
    }

    /// Traverse hierarchy breadth-first starting from an object
    pub fn traverse_breadth_first<F>(&self, id: Id, mut visitor: F) -> Result<(), ObjectModelError>
    where
        F: FnMut(&Object) -> bool,
    {
        let mut queue = vec![id];

        while let Some(curr_id) = queue.pop() {
            let obj = self.get_object(curr_id)?;
            if !visitor(&obj) {
                continue;
            }

            let children: Vec<Id> = obj.data().children.clone();
            queue.extend(children);
        }

        Ok(())
    }

    /// Get all descendants of an object (not including the object itself)
    pub fn get_descendants(&self, id: Id) -> Result<Vec<Object>, ObjectModelError> {
        let mut descendants = Vec::new();
        self.traverse_depth_first(id, |obj| {
            if obj.id() != id {
                descendants.push(obj.clone());
                true
            } else {
                true
            }
        })?;
        Ok(descendants)
    }

    /// Get all ancestors of an object (not including the object itself)
    pub fn get_ancestors(&self, id: Id) -> Result<Vec<Object>, ObjectModelError> {
        let mut ancestors = Vec::new();
        let mut current = self.get_object(id)?.data().parent;

        while let Some(parent_id) = current {
            let parent = self.get_object(parent_id)?;
            ancestors.push(parent.clone());
            current = parent.data().parent;
        }

        ancestors.reverse();
        Ok(ancestors)
    }

    /// Validate hierarchy integrity
    pub fn validate_hierarchy(&self) -> Result<Vec<String>, ObjectModelError> {
        let mut errors = Vec::new();

        // Check all objects
        for handle in self.objects.handles() {
            let obj = self.objects.get(handle)?;
            let data = obj.data();

            // Check parent exists
            if let Some(parent_id) = data.parent {
                if !self.id_to_handle.contains_key(&parent_id) {
                    errors.push(format!(
                        "Object {} has non-existent parent {}",
                        data.id, parent_id
                    ));
                }
            }

            // Check all children exist and have this as parent
            for child_id in &data.children {
                if !self.id_to_handle.contains_key(child_id) {
                    errors.push(format!(
                        "Object {} has non-existent child {}",
                        data.id, child_id
                    ));
                } else {
                    let child = self.get_object(*child_id)?;
                    if child.data().parent != Some(data.id) {
                        errors.push(format!(
                            "Object {} lists child {} but child has different parent",
                            data.id, child_id
                        ));
                    }
                }
            }
        }

        Ok(errors)
    }

    /// Rename an object
    pub fn rename(&mut self, id: Id, new_name: String) -> Result<(), ObjectModelError> {
        let handle = self
            .id_to_handle
            .get(&id)
            .ok_or(ObjectModelError::ObjectNotFound(id))?
            .clone();

        let old_name = {
            let obj = self.objects.get(handle)?;
            obj.data().name.clone()
        };

        if let Some(obj) = self.objects.get_mut(handle).ok() {
            obj.data_mut().name = new_name.clone();
        }

        self.emit_event(LifecycleEvent::Renamed {
            id,
            old_name,
            new_name,
        });

        Ok(())
    }

    /// Serialize entire hierarchy to JSON
    pub fn serialize_hierarchy(&self) -> Result<String, ObjectModelError> {
        let mut all_objects: Vec<SerializableObject> = Vec::new();

        // Find all workspaces and traverse their hierarchies
        for handle in self.objects.handles() {
            if let Ok(obj) = self.objects.get(handle) {
                if obj.object_type() == ObjectType::Workspace {
                    // Traverse depth-first and collect all objects
                    self.traverse_depth_first_impl(obj.id(), &mut |o| {
                        all_objects.push(SerializableObject::from(o.clone()));
                        true
                    })
                    .ok();
                }
            }
        }

        serde_json::to_string(&all_objects)
            .map_err(|e| ObjectModelError::InvalidOperation(format!("Serialization failed: {}", e)))
    }

    /// Deserialize hierarchy from JSON and merge into registry
    pub fn deserialize_hierarchy(&mut self, json: &str) -> Result<(), ObjectModelError> {
        let serializable: Vec<SerializableObject> = serde_json::from_str(json).map_err(|e| {
            ObjectModelError::InvalidOperation(format!("Deserialization failed: {}", e))
        })?;

        // Collect all IDs from serialized data for dependency resolution
        let mut all_objects: HashMap<Id, SerializableObject> = HashMap::new();
        for obj in serializable {
            all_objects.insert(obj.id, obj);
        }

        // First pass: create all objects (without establishing parent-child links yet)
        for serial_obj in all_objects.values() {
            if self.id_to_handle.contains_key(&serial_obj.id) {
                continue; // Skip if already exists
            }

            let data = ObjectData {
                id: serial_obj.id,
                object_type: serial_obj.object_type,
                parent: None,         // Will set in second pass
                children: Vec::new(), // Will populate in second pass
                name: serial_obj.name.clone(),
                destroyed: serial_obj.destroyed,
                metadata: serial_obj.metadata.clone(),
            };

            match serial_obj.object_type {
                ObjectType::Workspace => {
                    let workspace = Workspace {
                        handle: Handle::new(0, 0),
                        data,
                    };
                    let handle = self.objects.allocate(Object::Workspace(workspace))?;
                    self.id_to_handle.insert(serial_obj.id, handle);
                }
                ObjectType::Model => {
                    let model = Model {
                        handle: Handle::new(0, 0),
                        data,
                    };
                    let handle = self.objects.allocate(Object::Model(model))?;
                    self.id_to_handle.insert(serial_obj.id, handle);
                }
                ObjectType::Part => {
                    let part = Part {
                        handle: Handle::new(0, 0),
                        data,
                    };
                    let handle = self.objects.allocate(Object::Part(part))?;
                    self.id_to_handle.insert(serial_obj.id, handle);
                }
            }
        }

        // Second pass: establish parent-child relationships
        for serial_obj in all_objects.values() {
            if let Some(parent_id) = serial_obj.parent {
                if let (Some(child_handle), Some(parent_handle)) = (
                    self.id_to_handle.get(&serial_obj.id),
                    self.id_to_handle.get(&parent_id),
                ) {
                    // Set child's parent
                    if let Some(child_obj) = self.objects.get_mut(*child_handle).ok() {
                        child_obj.data_mut().parent = Some(parent_id);
                    }
                    // Add child to parent's children list
                    if let Some(parent_obj) = self.objects.get_mut(*parent_handle).ok() {
                        if !parent_obj.data().children.contains(&serial_obj.id) {
                            parent_obj.data_mut().children.push(serial_obj.id);
                        }
                    }
                }
            }
        }

        Ok(())
    }

    /// Get registry statistics
    pub fn get_stats(&self) -> RegistryStatistics {
        let mut workspaces = 0;
        let mut models = 0;
        let mut parts = 0;
        let mut total_children = 0;
        let mut max_depth = 0;

        for handle in self.objects.handles() {
            if let Ok(obj) = self.objects.get(handle) {
                match obj.object_type() {
                    ObjectType::Workspace => workspaces += 1,
                    ObjectType::Model => models += 1,
                    ObjectType::Part => parts += 1,
                }
                total_children += obj.data().children.len();

                // Calculate depth
                if let Ok(depth) = self.get_depth(obj.id()) {
                    max_depth = max_depth.max(depth);
                }
            }
        }

        RegistryStatistics {
            total_objects: workspaces + models + parts,
            workspaces,
            models,
            parts,
            total_children,
            max_depth,
        }
    }

    /// Find objects by name
    pub fn find_by_name(&self, name: &str) -> Vec<Object> {
        let mut results = Vec::new();
        for handle in self.objects.handles() {
            if let Ok(obj) = self.objects.get(handle) {
                if obj.data().name == name {
                    results.push(obj.clone());
                }
            }
        }
        results
    }

    /// Find objects by type
    pub fn find_by_type(&self, object_type: ObjectType) -> Vec<Object> {
        let mut results = Vec::new();
        for handle in self.objects.handles() {
            if let Ok(obj) = self.objects.get(handle) {
                if obj.object_type() == object_type {
                    results.push(obj.clone());
                }
            }
        }
        results
    }

    /// Get all workspaces
    pub fn get_workspaces(&self) -> Vec<Workspace> {
        self.find_by_type(ObjectType::Workspace)
            .into_iter()
            .filter_map(|obj| {
                if let Object::Workspace(w) = obj {
                    Some(w)
                } else {
                    None
                }
            })
            .collect()
    }

    /// Batch create models under a workspace
    pub fn create_models_batch(
        &mut self,
        parent: Id,
        names: Vec<String>,
    ) -> Result<Vec<Model>, ObjectModelError> {
        let mut models = Vec::new();
        for name in names {
            let model = self.create_model(parent, name)?;
            models.push(model);
        }
        Ok(models)
    }

    /// Batch create parts under a parent
    pub fn create_parts_batch(
        &mut self,
        parent: Id,
        names: Vec<String>,
    ) -> Result<Vec<Part>, ObjectModelError> {
        let mut parts = Vec::new();
        for name in names {
            let part = self.create_part(parent, name)?;
            parts.push(part);
        }
        Ok(parts)
    }

    /// Clear all objects from the registry
    pub fn clear(&mut self) {
        self.objects = HandleRegistry::new();
        self.id_to_handle.clear();
        self.event_listeners.clear();
    }

    /// Check if an object exists
    pub fn exists(&self, id: Id) -> bool {
        self.id_to_handle.contains_key(&id)
    }

    /// Get object count by type
    pub fn count_by_type(&self, object_type: ObjectType) -> usize {
        self.find_by_type(object_type).len()
    }

    /// Set metadata for an object
    pub fn set_metadata(
        &mut self,
        id: Id,
        key: String,
        value: String,
    ) -> Result<(), ObjectModelError> {
        let handle = self
            .id_to_handle
            .get(&id)
            .ok_or(ObjectModelError::ObjectNotFound(id))?
            .clone();

        if let Some(obj) = self.objects.get_mut(handle).ok() {
            obj.data_mut().metadata.insert(key, value);
        }

        Ok(())
    }

    /// Get metadata for an object
    pub fn get_metadata(&self, id: Id, key: &str) -> Result<Option<String>, ObjectModelError> {
        let handle = self
            .id_to_handle
            .get(&id)
            .ok_or(ObjectModelError::ObjectNotFound(id))?;

        let obj = self.objects.get(*handle)?;
        Ok(obj.data().metadata.get(key).cloned())
    }

    /// Remove metadata from an object
    pub fn remove_metadata(
        &mut self,
        id: Id,
        key: &str,
    ) -> Result<Option<String>, ObjectModelError> {
        let handle = self
            .id_to_handle
            .get(&id)
            .ok_or(ObjectModelError::ObjectNotFound(id))?
            .clone();

        if let Some(obj) = self.objects.get_mut(handle).ok() {
            Ok(obj.data_mut().metadata.remove(key))
        } else {
            Ok(None)
        }
    }

    /// Get all metadata for an object
    pub fn get_all_metadata(&self, id: Id) -> Result<HashMap<String, String>, ObjectModelError> {
        let handle = self
            .id_to_handle
            .get(&id)
            .ok_or(ObjectModelError::ObjectNotFound(id))?;

        let obj = self.objects.get(*handle)?;
        Ok(obj.data().metadata.clone())
    }

    /// Clone an object (creates a new object with same properties but new ID)
    pub fn clone_object(
        &mut self,
        id: Id,
        new_name: Option<String>,
    ) -> Result<Id, ObjectModelError> {
        let obj = self.get_object(id)?;
        let data = obj.data();

        let new_id = match obj.object_type() {
            ObjectType::Workspace => {
                let workspace = self.create_workspace(
                    new_name.unwrap_or_else(|| format!("{} (Copy)", data.name)),
                )?;
                workspace.data.id
            }
            ObjectType::Model => {
                if let Some(parent_id) = data.parent {
                    let model = self.create_model(
                        parent_id,
                        new_name.unwrap_or_else(|| format!("{} (Copy)", data.name)),
                    )?;
                    model.data.id
                } else {
                    return Err(ObjectModelError::InvalidOperation(
                        "Cannot clone model without parent".to_string(),
                    ));
                }
            }
            ObjectType::Part => {
                if let Some(parent_id) = data.parent {
                    let part = self.create_part(
                        parent_id,
                        new_name.unwrap_or_else(|| format!("{} (Copy)", data.name)),
                    )?;
                    part.data.id
                } else {
                    return Err(ObjectModelError::InvalidOperation(
                        "Cannot clone part without parent".to_string(),
                    ));
                }
            }
        };

        // Copy metadata
        for (key, value) in &data.metadata {
            self.set_metadata(new_id, key.clone(), value.clone())?;
        }

        Ok(new_id)
    }

    /// Clone an object with its descendants
    pub fn clone_subtree(
        &mut self,
        id: Id,
        new_name: Option<String>,
    ) -> Result<Id, ObjectModelError> {
        let mut old_to_new_id: HashMap<Id, Id> = HashMap::new();

        // First pass: clone all objects in the subtree
        let mut to_clone = vec![id];
        while let Some(current_id) = to_clone.pop() {
            let obj = self.get_object(current_id)?;
            let data = obj.data();

            let new_id = self.clone_object(current_id, new_name.clone())?;
            old_to_new_id.insert(current_id, new_id);

            // Add children to clone queue
            for child_id in &data.children {
                to_clone.push(*child_id);
            }
        }

        // Second pass: re-establish parent-child relationships among cloned objects
        for (old_id, new_id) in &old_to_new_id {
            let old_obj = self.get_object(*old_id)?;
            let old_data = old_obj.data();

            if let Some(old_parent_id) = old_data.parent {
                if let Some(new_parent_id) = old_to_new_id.get(&old_parent_id) {
                    self.reparent(*new_id, Some(*new_parent_id))?;
                }
            }
        }

        Ok(*old_to_new_id.get(&id).unwrap())
    }

    /// Create a snapshot of the current registry state
    pub fn snapshot(&self) -> String {
        self.serialize_hierarchy().unwrap()
    }

    /// Restore from a snapshot
    pub fn restore(&mut self, snapshot: &str) -> Result<(), ObjectModelError> {
        self.clear();
        self.deserialize_hierarchy(snapshot)
    }
}

impl Default for ObjectRegistry {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_create_workspace() {
        let mut registry = ObjectRegistry::new();
        let workspace = registry
            .create_workspace("TestWorkspace".to_string())
            .unwrap();

        assert_eq!(workspace.data.name, "TestWorkspace");
        assert!(!workspace.data.has_parent());
        assert!(!workspace.data.has_children());
    }

    #[test]
    fn test_create_model() {
        let mut registry = ObjectRegistry::new();
        let workspace = registry
            .create_workspace("TestWorkspace".to_string())
            .unwrap();
        let model = registry
            .create_model(workspace.data.id, "TestModel".to_string())
            .unwrap();

        assert_eq!(model.data.name, "TestModel");
        assert!(model.data.has_parent());
        assert_eq!(model.data.parent, Some(workspace.data.id));
    }

    #[test]
    fn test_create_part() {
        let mut registry = ObjectRegistry::new();
        let workspace = registry
            .create_workspace("TestWorkspace".to_string())
            .unwrap();
        let model = registry
            .create_model(workspace.data.id, "TestModel".to_string())
            .unwrap();
        let part = registry
            .create_part(model.data.id, "TestPart".to_string())
            .unwrap();

        assert_eq!(part.data.name, "TestPart");
        assert!(part.data.has_parent());
        assert_eq!(part.data.parent, Some(model.data.id));
    }

    #[test]
    fn test_get_children() {
        let mut registry = ObjectRegistry::new();
        let workspace = registry
            .create_workspace("TestWorkspace".to_string())
            .unwrap();
        let _model1 = registry
            .create_model(workspace.data.id, "Model1".to_string())
            .unwrap();
        let _model2 = registry
            .create_model(workspace.data.id, "Model2".to_string())
            .unwrap();

        let children = registry.get_children(workspace.data.id).unwrap();
        assert_eq!(children.len(), 2);
    }

    #[test]
    fn test_reparent() {
        let mut registry = ObjectRegistry::new();
        let workspace = registry
            .create_workspace("TestWorkspace".to_string())
            .unwrap();
        let model1 = registry
            .create_model(workspace.data.id, "Model1".to_string())
            .unwrap();
        let model2 = registry
            .create_model(workspace.data.id, "Model2".to_string())
            .unwrap();

        // Reparent model2 under model1
        registry
            .reparent(model2.data.id, Some(model1.data.id))
            .unwrap();

        let model2_updated = registry.get_object(model2.data.id).unwrap();
        assert_eq!(model2_updated.data().parent, Some(model1.data.id));
    }

    #[test]
    fn test_cycle_detection() {
        let mut registry = ObjectRegistry::new();
        let workspace = registry
            .create_workspace("TestWorkspace".to_string())
            .unwrap();
        let model1 = registry
            .create_model(workspace.data.id, "Model1".to_string())
            .unwrap();
        let model2 = registry
            .create_model(model1.data.id, "Model2".to_string())
            .unwrap();

        // Try to create a cycle: model1 -> model2 -> model1
        let result = registry.reparent(model1.data.id, Some(model2.data.id));
        assert!(matches!(result, Err(ObjectModelError::CycleDetected(_))));
    }

    #[test]
    fn test_destroy_cascade() {
        let mut registry = ObjectRegistry::new();
        let workspace = registry
            .create_workspace("TestWorkspace".to_string())
            .unwrap();
        let model = registry
            .create_model(workspace.data.id, "TestModel".to_string())
            .unwrap();
        let part1 = registry
            .create_part(model.data.id, "Part1".to_string())
            .unwrap();
        let part2 = registry
            .create_part(model.data.id, "Part2".to_string())
            .unwrap();

        // Destroy model should destroy parts
        registry.destroy(model.data.id).unwrap();

        // Parts should be destroyed
        assert!(registry.get_object(part1.data.id).is_err());
        assert!(registry.get_object(part2.data.id).is_err());
    }

    #[test]
    fn test_lifecycle_events() {
        use std::sync::{Arc, Mutex};

        let mut registry = ObjectRegistry::new();
        let events = Arc::new(Mutex::new(Vec::new()));
        let events_clone = Arc::clone(&events);

        registry.add_event_listener(move |event| {
            events_clone.lock().unwrap().push(event);
        });

        let _workspace = registry
            .create_workspace("TestWorkspace".to_string())
            .unwrap();

        let events = events.lock().unwrap();
        assert_eq!(events.len(), 1);
        assert!(matches!(events[0], LifecycleEvent::Created { .. }));
    }

    #[test]
    fn test_get_depth() {
        let mut registry = ObjectRegistry::new();
        let workspace = registry
            .create_workspace("TestWorkspace".to_string())
            .unwrap();
        let model = registry
            .create_model(workspace.data.id, "TestModel".to_string())
            .unwrap();
        let part = registry
            .create_part(model.data.id, "TestPart".to_string())
            .unwrap();

        let depth = registry.get_depth(part.data.id).unwrap();
        assert_eq!(depth, 2);
    }

    #[test]
    fn test_get_path() {
        let mut registry = ObjectRegistry::new();
        let workspace = registry
            .create_workspace("TestWorkspace".to_string())
            .unwrap();
        let model = registry
            .create_model(workspace.data.id, "TestModel".to_string())
            .unwrap();
        let part = registry
            .create_part(model.data.id, "TestPart".to_string())
            .unwrap();

        let path = registry.get_path(part.data.id).unwrap();
        assert_eq!(path.len(), 3);
        assert_eq!(path[0], workspace.data.id);
        assert_eq!(path[1], model.data.id);
        assert_eq!(path[2], part.data.id);
    }

    #[test]
    fn test_get_descendants() {
        let mut registry = ObjectRegistry::new();
        let workspace = registry
            .create_workspace("TestWorkspace".to_string())
            .unwrap();
        let model = registry
            .create_model(workspace.data.id, "TestModel".to_string())
            .unwrap();
        let _part1 = registry
            .create_part(model.data.id, "Part1".to_string())
            .unwrap();
        let _part2 = registry
            .create_part(model.data.id, "Part2".to_string())
            .unwrap();

        let descendants = registry.get_descendants(model.data.id).unwrap();
        assert_eq!(descendants.len(), 2);
    }

    #[test]
    fn test_get_ancestors() {
        let mut registry = ObjectRegistry::new();
        let workspace = registry
            .create_workspace("TestWorkspace".to_string())
            .unwrap();
        let model = registry
            .create_model(workspace.data.id, "TestModel".to_string())
            .unwrap();
        let part = registry
            .create_part(model.data.id, "TestPart".to_string())
            .unwrap();

        let ancestors = registry.get_ancestors(part.data.id).unwrap();
        assert_eq!(ancestors.len(), 2);
        assert_eq!(ancestors[0].id(), workspace.data.id);
        assert_eq!(ancestors[1].id(), model.data.id);
    }

    #[test]
    fn test_validate_hierarchy() {
        let mut registry = ObjectRegistry::new();
        let workspace = registry
            .create_workspace("TestWorkspace".to_string())
            .unwrap();
        let _model = registry
            .create_model(workspace.data.id, "TestModel".to_string())
            .unwrap();

        let errors = registry.validate_hierarchy().unwrap();
        assert!(errors.is_empty());
    }

    #[test]
    fn test_rename() {
        use std::sync::{Arc, Mutex};

        let mut registry = ObjectRegistry::new();
        let events = Arc::new(Mutex::new(Vec::new()));
        let events_clone = Arc::clone(&events);

        registry.add_event_listener(move |event| {
            events_clone.lock().unwrap().push(event);
        });

        let workspace = registry
            .create_workspace("TestWorkspace".to_string())
            .unwrap();
        registry
            .rename(workspace.data.id, "NewName".to_string())
            .unwrap();

        let events = events.lock().unwrap();
        assert_eq!(events.len(), 2);
        assert!(matches!(events[1], LifecycleEvent::Renamed { .. }));

        let workspace_updated = registry.get_object(workspace.data.id).unwrap();
        assert_eq!(workspace_updated.data().name, "NewName");
    }

    #[test]
    fn test_traverse_depth_first() {
        let mut registry = ObjectRegistry::new();
        let workspace = registry
            .create_workspace("TestWorkspace".to_string())
            .unwrap();
        let model = registry
            .create_model(workspace.data.id, "TestModel".to_string())
            .unwrap();
        let _part1 = registry
            .create_part(model.data.id, "Part1".to_string())
            .unwrap();
        let _part2 = registry
            .create_part(model.data.id, "Part2".to_string())
            .unwrap();

        let mut visited = Vec::new();
        registry
            .traverse_depth_first(workspace.data.id, |obj| {
                visited.push(obj.id());
                true
            })
            .unwrap();

        assert!(visited.contains(&workspace.data.id));
        assert!(visited.contains(&model.data.id));
    }

    #[test]
    fn test_traverse_breadth_first() {
        let mut registry = ObjectRegistry::new();
        let workspace = registry
            .create_workspace("TestWorkspace".to_string())
            .unwrap();
        let model1 = registry
            .create_model(workspace.data.id, "Model1".to_string())
            .unwrap();
        let model2 = registry
            .create_model(workspace.data.id, "Model2".to_string())
            .unwrap();
        let _part1 = registry
            .create_part(model1.data.id, "Part1".to_string())
            .unwrap();

        let mut visited = Vec::new();
        registry
            .traverse_breadth_first(workspace.data.id, |obj| {
                visited.push(obj.id());
                true
            })
            .unwrap();

        assert!(visited.contains(&workspace.data.id));
        assert!(visited.contains(&model1.data.id));
        assert!(visited.contains(&model2.data.id));
    }

    #[test]
    fn test_serialize_deserialize_hierarchy() {
        let mut registry1 = ObjectRegistry::new();
        let workspace = registry1
            .create_workspace("TestWorkspace".to_string())
            .unwrap();
        let model = registry1
            .create_model(workspace.data.id, "TestModel".to_string())
            .unwrap();
        let _part = registry1
            .create_part(model.data.id, "TestPart".to_string())
            .unwrap();

        let json = registry1.serialize_hierarchy().unwrap();
        assert!(!json.is_empty());

        let mut registry2 = ObjectRegistry::new();
        registry2.deserialize_hierarchy(&json).unwrap();

        // Verify the workspace was deserialized
        let deserialized_workspace = registry2.get_object(workspace.data.id).unwrap();
        assert_eq!(deserialized_workspace.data().name, "TestWorkspace");

        // Verify the model was deserialized
        let deserialized_model = registry2.get_object(model.data.id).unwrap();
        assert_eq!(deserialized_model.data().name, "TestModel");
    }

    #[test]
    fn test_deserialize_invalid_json() {
        let mut registry = ObjectRegistry::new();
        let result = registry.deserialize_hierarchy("invalid json");
        assert!(result.is_err());
    }

    #[test]
    fn test_get_stats() {
        let mut registry = ObjectRegistry::new();
        let workspace = registry
            .create_workspace("TestWorkspace".to_string())
            .unwrap();
        let model = registry
            .create_model(workspace.data.id, "TestModel".to_string())
            .unwrap();
        let _part = registry
            .create_part(model.data.id, "TestPart".to_string())
            .unwrap();

        let stats = registry.get_stats();
        assert_eq!(stats.total_objects, 3);
        assert_eq!(stats.workspaces, 1);
        assert_eq!(stats.models, 1);
        assert_eq!(stats.parts, 1);
        assert_eq!(stats.total_children, 2); // workspace has 1 child (model), model has 1 child (part)
        assert_eq!(stats.max_depth, 2);
    }

    #[test]
    fn test_find_by_name() {
        let mut registry = ObjectRegistry::new();
        let workspace = registry
            .create_workspace("MyWorkspace".to_string())
            .unwrap();
        let model = registry
            .create_model(workspace.data.id, "MyModel".to_string())
            .unwrap();

        let results = registry.find_by_name("MyModel");
        assert_eq!(results.len(), 1);
        assert_eq!(results[0].id(), model.data.id);

        let empty = registry.find_by_name("NonExistent");
        assert!(empty.is_empty());
    }

    #[test]
    fn test_find_by_type() {
        let mut registry = ObjectRegistry::new();
        let workspace = registry
            .create_workspace("TestWorkspace".to_string())
            .unwrap();
        let model = registry
            .create_model(workspace.data.id, "TestModel".to_string())
            .unwrap();
        let _part = registry
            .create_part(model.data.id, "TestPart".to_string())
            .unwrap();

        let models = registry.find_by_type(ObjectType::Model);
        assert_eq!(models.len(), 1);

        let parts = registry.find_by_type(ObjectType::Part);
        assert_eq!(parts.len(), 1);
    }

    #[test]
    fn test_get_workspaces() {
        let mut registry = ObjectRegistry::new();
        let workspace1 = registry.create_workspace("Workspace1".to_string()).unwrap();
        let workspace2 = registry.create_workspace("Workspace2".to_string()).unwrap();

        let workspaces = registry.get_workspaces();
        assert_eq!(workspaces.len(), 2);
        assert!(workspaces.iter().any(|w| w.data.id == workspace1.data.id));
        assert!(workspaces.iter().any(|w| w.data.id == workspace2.data.id));
    }

    #[test]
    fn test_create_models_batch() {
        let mut registry = ObjectRegistry::new();
        let workspace = registry
            .create_workspace("TestWorkspace".to_string())
            .unwrap();

        let names = vec![
            "Model1".to_string(),
            "Model2".to_string(),
            "Model3".to_string(),
        ];
        let models = registry
            .create_models_batch(workspace.data.id, names)
            .unwrap();

        assert_eq!(models.len(), 3);
        assert_eq!(registry.count_by_type(ObjectType::Model), 3);
    }

    #[test]
    fn test_create_parts_batch() {
        let mut registry = ObjectRegistry::new();
        let workspace = registry
            .create_workspace("TestWorkspace".to_string())
            .unwrap();
        let model = registry
            .create_model(workspace.data.id, "TestModel".to_string())
            .unwrap();

        let names = vec!["Part1".to_string(), "Part2".to_string()];
        let parts = registry.create_parts_batch(model.data.id, names).unwrap();

        assert_eq!(parts.len(), 2);
        assert_eq!(registry.count_by_type(ObjectType::Part), 2);
    }

    #[test]
    fn test_clear() {
        let mut registry = ObjectRegistry::new();
        let workspace = registry
            .create_workspace("TestWorkspace".to_string())
            .unwrap();
        let _model = registry
            .create_model(workspace.data.id, "TestModel".to_string())
            .unwrap();

        assert!(registry.exists(workspace.data.id));
        registry.clear();
        assert!(!registry.exists(workspace.data.id));

        let stats = registry.get_stats();
        assert_eq!(stats.total_objects, 0);
    }

    #[test]
    fn test_exists() {
        let mut registry = ObjectRegistry::new();
        let workspace = registry
            .create_workspace("TestWorkspace".to_string())
            .unwrap();

        assert!(registry.exists(workspace.data.id));
        assert!(!registry.exists(Id::random()));
    }

    #[test]
    fn test_count_by_type() {
        let mut registry = ObjectRegistry::new();
        let workspace = registry
            .create_workspace("TestWorkspace".to_string())
            .unwrap();
        let model = registry
            .create_model(workspace.data.id, "TestModel".to_string())
            .unwrap();
        let _part = registry
            .create_part(model.data.id, "TestPart".to_string())
            .unwrap();

        assert_eq!(registry.count_by_type(ObjectType::Workspace), 1);
        assert_eq!(registry.count_by_type(ObjectType::Model), 1);
        assert_eq!(registry.count_by_type(ObjectType::Part), 1);
    }

    #[test]
    fn test_metadata_operations() {
        let mut registry = ObjectRegistry::new();
        let workspace = registry
            .create_workspace("TestWorkspace".to_string())
            .unwrap();

        // Set metadata
        registry
            .set_metadata(workspace.data.id, "author".to_string(), "test".to_string())
            .unwrap();
        registry
            .set_metadata(workspace.data.id, "version".to_string(), "1.0".to_string())
            .unwrap();

        // Get metadata
        let author = registry.get_metadata(workspace.data.id, "author").unwrap();
        assert_eq!(author, Some("test".to_string()));

        let version = registry.get_metadata(workspace.data.id, "version").unwrap();
        assert_eq!(version, Some("1.0".to_string()));

        // Get non-existent metadata
        let missing = registry.get_metadata(workspace.data.id, "missing").unwrap();
        assert!(missing.is_none());

        // Get all metadata
        let all = registry.get_all_metadata(workspace.data.id).unwrap();
        assert_eq!(all.len(), 2);

        // Remove metadata
        let removed = registry
            .remove_metadata(workspace.data.id, "author")
            .unwrap();
        assert_eq!(removed, Some("test".to_string()));

        let after_remove = registry.get_metadata(workspace.data.id, "author").unwrap();
        assert!(after_remove.is_none());
    }

    #[test]
    fn test_clone_object() {
        let mut registry = ObjectRegistry::new();
        let workspace = registry
            .create_workspace("TestWorkspace".to_string())
            .unwrap();
        let model = registry
            .create_model(workspace.data.id, "TestModel".to_string())
            .unwrap();

        // Add metadata to original
        registry
            .set_metadata(model.data.id, "tag".to_string(), "original".to_string())
            .unwrap();

        // Clone the model
        let cloned_id = registry
            .clone_object(model.data.id, Some("ClonedModel".to_string()))
            .unwrap();

        // Verify clone exists and has different ID
        assert!(registry.exists(cloned_id));
        assert_ne!(cloned_id, model.data.id);

        // Verify clone has metadata
        let tag = registry.get_metadata(cloned_id, "tag").unwrap();
        assert_eq!(tag, Some("original".to_string()));

        // Verify clone name
        let cloned_obj = registry.get_object(cloned_id).unwrap();
        assert_eq!(cloned_obj.data().name, "ClonedModel");
    }

    #[test]
    fn test_clone_subtree() {
        let mut registry = ObjectRegistry::new();
        let workspace = registry
            .create_workspace("TestWorkspace".to_string())
            .unwrap();
        let model = registry
            .create_model(workspace.data.id, "TestModel".to_string())
            .unwrap();
        let part1 = registry
            .create_part(model.data.id, "Part1".to_string())
            .unwrap();
        let part2 = registry
            .create_part(model.data.id, "Part2".to_string())
            .unwrap();

        // Clone the model subtree
        let cloned_model_id = registry
            .clone_subtree(model.data.id, Some("ClonedModel".to_string()))
            .unwrap();

        // Verify clone exists
        assert!(registry.exists(cloned_model_id));
        assert_ne!(cloned_model_id, model.data.id);

        // Verify cloned parts exist
        let cloned_model = registry.get_object(cloned_model_id).unwrap();
        assert_eq!(cloned_model.data().children.len(), 2);

        // Verify original parts still exist
        assert!(registry.exists(part1.data.id));
        assert!(registry.exists(part2.data.id));
    }

    #[test]
    fn test_snapshot_restore() {
        let mut registry1 = ObjectRegistry::new();
        let workspace = registry1
            .create_workspace("TestWorkspace".to_string())
            .unwrap();
        let model = registry1
            .create_model(workspace.data.id, "TestModel".to_string())
            .unwrap();

        // Add metadata
        registry1
            .set_metadata(model.data.id, "tag".to_string(), "snapshot".to_string())
            .unwrap();

        // Create snapshot
        let snapshot = registry1.snapshot();
        assert!(!snapshot.is_empty());

        // Create new registry and restore
        let mut registry2 = ObjectRegistry::new();
        registry2.restore(&snapshot).unwrap();

        // Verify restored state
        let restored_workspace = registry2.get_object(workspace.data.id).unwrap();
        assert_eq!(restored_workspace.data().name, "TestWorkspace");

        let restored_model = registry2.get_object(model.data.id).unwrap();
        assert_eq!(restored_model.data().name, "TestModel");

        // Verify metadata was restored
        let tag = registry2.get_metadata(model.data.id, "tag").unwrap();
        assert_eq!(tag, Some("snapshot".to_string()));
    }
}
