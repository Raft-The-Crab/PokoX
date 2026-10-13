# Architecture Decisions

## Current direction
- PokoX: Windows 10+ and Android 11+.
- Windows application UI (Studio, moderation, PokoX menus): Rust + EGUI for internal tools and editors.
- Android application UI (PokoX menus): Native Kotlin for native feel and reliability.
- Game UI (in-experience): Custom Rust UI engine for tight engine integration and performance.
- Backend: Rust on Huggingface CPU (2 VCPUs, 16GB RAM each).
- Game handler/authority: Rust on Huggingface CPU (separate instance from main backend).
- Studio/basic APIs: TypeScript Cloudflare Workers + Rust WASM (100k req/day, fallback to Huggingface).
- Edge/CDN: TypeScript Cloudflare Workers + Rust WASM where justified.
- Mute compiler and VM: C.
- Doxygen for C/C++.
- PostgreSQL: Aiven (3GB) + Layerbase (5GB) as durable source of truth.
- Cache/session: Aiven Valkey (3GB) for transient workloads.
- Object storage: Cloudflare R2 for asset/package bytes.
- Edge database: Cloudflare D1 (SQLite) for edge-local data.
- Poko Physics owns its public API. Jolt may be a temporary backend/reference until explicit correctness/performance gates pass.
- Diligent remains the initial renderer integration; native adapters are acceptable.
- Proprietary Poko code; review every dependency license separately.

## Rust engine decision is provisional
Rust is being evaluated for engine core because of memory safety and AI-assisted development ergonomics. Windows application UI (Studio, moderation, PokoX menus) will use Rust + EGUI via eframe. Android application UI will use native Kotlin with JNI bridge to Rust engine. First prove a minimal Rust library + EGUI + eframe for Windows, and Kotlin + JNI integration for Android. Migrate subsystem-by-subsystem only if the experiment supports it. Do not rewrite everything in one generated change.

## Backend language decision
**Context:** Need backend services for auth, session, publishing, economy, moderation. Targeting 700+ CCU with limited hosting resources.

**Options:**
1. Go modular monolith (original plan)
2. Rust backend + Rust Game Handler/Authority on Huggingface CPU
3. Go backend + Rust Game Handler/Authority on Huggingface CPU

**Decision:** Option 2 - Rust backend + Rust Game Handler/Authority on Huggingface CPU

**Trade-offs:**
- Rust pros: single language stack with engine, memory safety, WASM compatibility, better performance
- Rust cons: ecosystem smaller than Go, more complex async runtime
- Go pros: mature ecosystem, simpler concurrency, battle-tested for backends
- Go cons: different language from engine, no native WASM support
- Cloudflare Workers: Used for Studio/basic APIs (100k req/day limit), fallback to Huggingface CPU if exceeded

**Test/benchmark:** Build minimal Rust backend service on Huggingface CPU, validate Cloudflare Workers for Studio APIs with fallback logic

**Revisit trigger:** If Rust backend proves too complex, or if Cloudflare Workers fallback causes issues

## Physics policy
Target Indie-A and AA requirements first; add selected advanced features only for demonstrated use cases. Feature percentages are internal estimates, not objective AAA parity.

## Decision record template
1. Context/problem
2. Options
3. Decision/owner
4. Trade-offs
5. Test or benchmark
6. Revisit trigger

## Still provisional
Exact Rust workspace, final renderer adapter, physics cutover, hosting mix, current quotas, economy rules, payment provider, and external VPS support.

## UI framework decision
**Context:** Need application UI for Studio, moderation tools, and PokoX menus. Need game UI for in-experience developer-created UI.

**Options:**
1. C# + Avalonia (Windows) + Kotlin (Android) - different languages per platform
2. Rust + EGUI for all application UI, custom Rust UI for game UI
3. Custom Rust UI for everything
4. Rust + EGUI (Windows), Native Kotlin (Android), custom Rust UI for game UI

**Decision:** Option 4 - Rust + EGUI for Windows application UI, Native Kotlin for Android application UI, custom Rust UI for game UI

**Trade-offs:**
- EGUI (Windows) pros: cross-platform, easy to use, integrates with any engine, immediate-mode simplicity
- EGUI (Windows) cons: 1-2ms per-frame overhead, breaking changes, not native-looking
- Kotlin (Android) pros: native feel, mature ecosystem, reliable platform integration, better accessibility
- Kotlin (Android) cons: different language from Windows UI, requires JNI bridge to engine
- Custom UI pros: full control, optimized for Poko, tight engine integration
- Custom UI cons: significant development effort, must implement everything from scratch

**Test/benchmark:** Phase 1 experiment - build minimal EGUI + eframe app for Windows, validate Kotlin + JNI engine integration for Android

**Revisit trigger:** If EGUI overhead is unacceptable on Windows, or if Kotlin integration proves too complex
