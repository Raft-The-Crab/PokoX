# Open Questions

Resolve with an experiment or explicit product decision; do not silently assume answers.

## Engine
- [ ] Which subsystem migrates to Rust first after the UI experiment?
- [ ] Stable C ABI needed by C Mute and C++ adapters?
- [ ] Which Diligent backends build on each target?
- [ ] Which modules should remain C++ based on dependency/performance evidence?

## UI
- [ ] Kotlin + JNI bridge complexity and performance.
- [ ] Custom game UI architecture: retained-mode vs hybrid approach.
- [ ] EGUI breaking change impact and version pinning strategy.
- [ ] Performance baseline for EGUI overhead on Windows devices.

## Backend
- [ ] Rust backend complexity vs Go for platform APIs.
- [ ] Cloudflare Workers fallback logic and performance when 100k req/day limit exceeded.
- [ ] Rust game handler/authority capacity on Huggingface CPU (2 VCPUs, 16GB).
- [ ] Rust async runtime (Tokio) complexity vs Go goroutines.
- [ ] WebSocket performance on Huggingface CPU for real-time game traffic.

## Mute
- [ ] Type system and class milestone.
- [ ] Stack vs register VM based on measurements.
- [ ] Closures, coroutines/async and module semantics.
- [ ] Bytecode/language compatibility and deprecation policy.
- [ ] Compile-time target after benchmark fixture and machine are defined.
- [ ] C ABI stability and Rust integration patterns.

## Physics
- [ ] Required determinism for network/replay.
- [ ] Character controller/joint scope.
- [ ] Conditions to retire Jolt as backend/reference.
- [ ] Mobile performance budgets.

## Platform
- [ ] Current provider plans, quotas, overages and backup features.
- [ ] Payment provider, currency issuance/sinks and fraud policy.
- [ ] Safety requirements for launch markets/age groups.
- [ ] External VPS hosting as supported product vs developer-operated path.
- [ ] Recommendation data collection and user controls.

## Release
- [ ] Numeric version encoding and overflow policy.
- [ ] Package signing/key rotation.
- [ ] Incident-response owner.
- [ ] Device performance targets based on measurements.
