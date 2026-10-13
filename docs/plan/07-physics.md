# Poko Physics

## Target and rules
Build a Poko-owned API for Indie-A and AA gameplay first. Keep physics light by default and allow higher-quality options. Use Jolt temporarily/reference-only until explicit gates pass. Avoid adding CCD, Jacobian solvers, SIMD, or joints without a concrete use case and test.

## P0 — Specification and harness
- [ ] Units, up axis, handedness, timestep, gravity, tolerances, body types.
- [ ] Headless deterministic test harness.
- [ ] Vector/quaternion/transform/AABB tests.
- [ ] Shape-pair coverage matrix; unsupported pairs explicit.

## P1 — World and state
- [ ] Body/shape/material handles and lifecycle.
- [ ] Static/dynamic/kinematic semantics.
- [ ] Gravity, force, impulse, damping and mass.
- [ ] Correct inertia and angular integration.
- [ ] Broadphase proxy create/update/remove ownership.
- [ ] Raycast/query API with correct point, normal, distance and filtering.

## P2 — Collision/contact
- [ ] Sphere/sphere, sphere/plane, sphere/box, capsule/plane.
- [ ] Box/box SAT with stable axis and penetration results.
- [ ] Capsule/sphere and capsule/capsule.
- [ ] Box/capsule only after robust tests.
- [ ] Contact manifold persistence/invalidation policy.
- [ ] Collision filtering, trigger events, sleep/wake, body removal.

## P3 — Solver
- [ ] Define contact constraint units/model.
- [ ] Position correction or documented stabilization.
- [ ] Velocity response, restitution, friction and stacking tests.
- [ ] Angular response after linear response is stable.
- [ ] Implement a joint only when its full constraint data is mapped and tested.
- [ ] No silent no-op solver methods.

## P4 — Integration and profiling
- [ ] One authoritative transform path.
- [ ] Broadphase lifecycle integration.
- [ ] Trigger enter/stay/exit and destruction tests.
- [ ] Accelerated queries when benchmarks justify them.
- [ ] Debug draw, counters, contact visualization, Tracy zones.
- [ ] Compare fixed scenarios against Jolt/reference backend.

## P5 — Optional advanced features
- [ ] CCD for reproduced tunneling cases.
- [ ] Character controller for documented movement needs.
- [ ] SIMD only after scalar correctness and profiling.
- [ ] Determinism only to the level required by network/replay needs.
- [ ] Defer soft bodies/destruction/advanced vehicles until core is stable.

## Required edge tests
Degenerate shapes/directions, extreme allowed scale, NaN/Inf rejection, fast bodies, stacks, repeated create/destroy, filter changes, triggers, pause/resume, many sleeping bodies.

## Gate
Broadphase/manifold ownership is tested; solver has no core no-op; raycast results are verified; physics is integrated and benchmarked on fixed scenes.
