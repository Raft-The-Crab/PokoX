# Engine Core

## IDs and handles
- [ ] Define stable serialized IDs separately from process-local handles.
- [ ] Use generation-safe handles to reject stale references.
- [ ] Test destruction, reuse, overflow, invalid handles, and cross-world access.

## Object model
- [ ] Implement creator-facing `Workspace → Model → Part` semantics.
- [ ] Define parenting, cycle prevention, child enumeration, destruction, and lifecycle events.
- [ ] Separate public instance model from internal component/ECS storage.
- [ ] Test reparenting, recursive destroy, and partial initialization cleanup.

## Metadata registry
- [ ] Canonical type/property/function/event metadata.
- [ ] Defaults, validators, replication/serialization policy, Mute visibility, inspector display, docs.
- [ ] Generate/derive bindings, autocomplete, inspector and serialization from shared metadata.
- [ ] Validate duplicate IDs and invalid metadata at build/startup.

## Transform ownership
- [ ] One authoritative transform state per object.
- [ ] Local/world transform and invalidation policy.
- [ ] Parent propagation, reparenting and numerical edge tests.
- [ ] Explicit policy for non-uniform scale and physics transforms.

## Jobs, memory and lifecycle
- [ ] Establish frame/time-step policy and fixed-step accumulator.
- [ ] Define job dependencies, cancellation, thread ownership and shutdown.
- [ ] Measure allocations before adding custom allocators.
- [ ] Add arenas/pools only for measured hot paths.
- [ ] Define device-loss and partial-init teardown behavior.

## Gate
A test scene can create, parent, transform, enumerate, serialize and destroy objects without stale handles or duplicate state. Mute/Studio consume the canonical metadata.
