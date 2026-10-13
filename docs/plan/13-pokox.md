# PokoX Client

## Core flows
- [ ] First launch, health/error states, sign-in/out, session renewal.
- [ ] Home/discovery, search, experience detail and join.
- [ ] Reconnect and clean leave behavior.
- [ ] Profile, avatar, inventory, favorites and history.
- [ ] Graphics/audio/controls/accessibility/privacy settings.
- [ ] Report/block/mute controls in relevant contexts.
- [ ] Asset cache integrity and storage limits.
- [ ] Update/recovery policy.

## Platform UI
**Windows:** Rust + EGUI application UI via eframe. Integrate engine via stable native API. Test window/render-surface integration and crash handling.

**Android:** Native Kotlin UI. Define JNI boundary to Rust engine with stable C ABI. Handle pause/resume, network changes, memory pressure, touch, safe areas and permissions. Test engine integration via JNI thoroughly.

## Discovery
- [ ] Genre/category filters and search.
- [ ] Similar experience suggestions using metadata and permitted activity signals.
- [ ] Example: playing a BedWars-like game can suggest related PvP/team-combat genres.
- [ ] User controls for history/personalization.
- [ ] Do not make region-based discovery a prerequisite for an APAC-first launch.

## Gate
New user authenticates, finds a test experience, joins/leaves, and reports abusive content.
