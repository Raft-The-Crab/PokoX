# Changelog

All notable changes to the Poko project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Comprehensive project folder structure (2,653 folders)
  - apps/: PokoX (Windows/Android), Studio, Moderation desktop applications
  - engine/: Core, rendering, physics, audio, assets, UI subsystems
  - mute/: Compiler, VM, bindings, tooling for Mute language
  - backend/: Main backend and game handler services
  - api/: Cloudflare Workers and public API layers
  - networking/: Protocol, transport, serialization, compression, encryption
  - storage/: R2 and D1 storage adapters
  - database/: PostgreSQL and Valkey schemas, migrations, backups
  - integration/: Native adapters (Diligent, physics, audio) and JNI bridge
  - testing/: Unit, integration, E2E, performance testing infrastructure
  - ci/: Continuous integration scripts, configs, dockerfiles, workflows
  - deployment/: Terraform, Docker, Kubernetes, monitoring, alerts
  - docs/: API documentation, guides, tutorials, references, specs
  - tools/: Build system, linters, formatters, coverage tools
  - security/: Audits, policies, scanners, reviews
  - monitoring/: Metrics, logging, tracing infrastructure
  - config/: Environment configs, feature toggles, experiments
  - assets/: Sample assets and templates
  - shared/: Shared types, utilities, ABI libraries
  - platform/: Windows and Android platform-specific code
- Proprietary LICENSE file
- Organized asset files:
  - Character.png → assets/samples/models/characters/
  - PokoX.png → apps/pokox/windows/resources/icons/128x128/
  - PokoX.svg → apps/pokox/windows/resources/icons/
  - Studio.png → apps/studio/windows/resources/icons/128x128/
  - Studio.svg → apps/studio/windows/resources/icons/
- Planning documentation (33 documents)
- Rust + EGUI for Windows application UI
- Native Kotlin for Android application UI
- Custom Rust UI for in-game UI
- Rust backend architecture
- Rust game handler/authority
- Cloudflare Workers for Studio APIs
- Hugging Face CPU deployment strategy
- Diligent rendering integration plan
- Mute language (C implementation) architecture
- Proprietary physics engine plan
- Capacity planning for 700 CCU target
- Session management architecture
- Deployment pipeline strategy
- Monitoring and observability plan
- Error handling and resilience strategy
- Performance budgets and SLAs
- Cloudinary integration for image processing

### Changed
- Removed C++ WebSocket authority (replaced with Rust on Hugging Face)
- Removed Jolt physics (building proprietary physics engine)
- Moved from Go backend to Rust backend
- Simplified Cloudflare Workers usage to Studio/basic APIs only

### Infrastructure
- Cloudflare R2, Workers, D1
- Firebase Spark Plan
- Aiven PostgreSQL (3 accounts × 1GB)
- Aiven Valkey (3 accounts × 1GB)
- Layerbase (5GB usage-based)
- Hugging Face CPU (2 instances × 2 vCPUs, 16GB RAM)
- Cloudinary (25 credits/month)

## [0.0.1] - TBD

### Planned
- Phase 0: Audit and baseline
- Phase 1: Rust + EGUI + Kotlin integration validation
- Phase 2: Engine vertical slice
- Phase 3: Mute compiler and VM
- Phase 4: Poko Studio creator loop
- Phase 5: Multiplayer and auth
- Phase 6: Public platform launch
- Phase 7: Expansion and marketplace
