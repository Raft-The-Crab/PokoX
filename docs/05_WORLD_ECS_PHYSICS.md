# 05 — World, ECS, Hierarchy, and Custom Physics

**Status:** Proposed baseline  
**World model:** Hybrid ECS with hierarchy and stable object handles  
**Physics:** Custom C++ implementation behind replaceable interfaces.

## 1. World model

A `World` owns one or more scenes. Entities are lightweight identities. Components own typed data. Systems process component sets. A separate hierarchy stores parent/child relationships and is not the sole source of component data.

Required concepts:

- `WorldID`, `SceneID`, `EntityID`, `ComponentTypeID`.
- Generation-counted object handles to detect stale references after deletion/reuse.
- Stable serialized IDs distinct from ephemeral runtime handles.
- Transform local/world data and dirty propagation.
- Parent-cycle prevention.
- Deterministic serialization order where needed.
- Explicit authority/replication metadata for networked entities.

## 2. Core components

Initial component set: Transform, Name/Tag, MeshRenderer, Camera, Light, MaterialReference, Collider, RigidBody, CharacterController, AudioSource, AnimationPlayer, ScriptComponent, UIReference, NetworkIdentity, Ownership/AuthorityMetadata, and PrefabReference.

Keep components data-oriented and avoid embedding arbitrary C++ virtual behavior in every entity. Script behavior is handled by MUTE host objects and event dispatch.

## 3. Hierarchy rules

- An entity has at most one parent per scene.
- Parent-child cycles are invalid.
- Destroy semantics must be explicit: destroy subtree, detach children, or reparent. Do not leave it implicit.
- Transform propagation must avoid recomputing unchanged subtrees.
- Runtime handles are not network IDs and must not be trusted from clients.
- Cross-scene references require explicit asset/entity reference policy.

## 4. Serialization

Project serialization stores stable IDs, component versions, hierarchy edges, and asset references. Do not serialize raw memory layouts or pointers. Use schema versions and migration functions. Unknown optional components may be preserved or skipped according to schema policy; unknown required components must produce a clear compatibility error.

## 5. Physics scope

The custom physics plan is incremental. Do not attempt a complete general-purpose physics engine before the first playable prototype.

### Physics v0.1

- Vector math and AABB/sphere/plane/capsule primitives.
- Static colliders.
- Raycasts and overlap queries.
- Collision filtering/layers.
- Character capsule movement with gravity, grounding, jump and swept collision.

### v0.2

- Dynamic rigid body basics: mass, linear/angular velocity, impulses, contacts, friction and position correction.
- Broadphase acceleration (initial simple spatial hash or sweep-and-prune; benchmark before choosing).

### v0.3+

- Continuous collision detection for selected fast bodies.
- Sleeping.
- Joints/constraints.
- Ragdoll and vehicle support only when product need is demonstrated.

## 6. Fixed timestep

Use a fixed simulation step for gameplay/authority. Rendering may interpolate between simulation states. The fixed tick value is OPEN and must be benchmarked; do not assume 30 or 60 Hz universally. Clamp excessive accumulated frame time and define catch-up limits to avoid a spiral of death.

## 7. Client/server physics

Client physics is predictive and may be modified. Server physics/state is authoritative for online interactions. To keep behavior aligned, share deterministic math and collision definitions where possible, but do not assume floating-point determinism across all devices. Server collision proxies can be simpler than visual geometry. Visual mesh complexity must not dictate server collision complexity.

## 8. Character controller

The controller must support capsule dimensions, step/ground policy, jump eligibility, slope limit, air control, acceleration, maximum speed, collision filters and movement modes. These values are game-authority rules, not trusted client claims. Game developers can configure allowed movement modes through server-side rules.

## 9. Physics security

- Validate finite values; reject NaN/infinity.
- Bound coordinate magnitudes, velocities, impulses and collider dimensions.
- Limit collider/entity counts and query frequency per game.
- Treat client-provided collision claims as hints only.
- Fuzz collision and serialization entry points.

## 10. Acceptance tests

- A capsule character walks on a plane, collides with a wall, steps/jumps per configured rules, and cannot tunnel through test geometry at supported speed.
- Raycast and overlap results are stable for defined test scenes.
- Stale handles are rejected.
- Hierarchy cycles are rejected.
- Serialization round-trips stable IDs and handles are recreated safely.
- Server rejects impossible movement beyond game-configured limits.
- Fuzzing finds no unhandled crash in collider creation, raycast or world deserialization.
