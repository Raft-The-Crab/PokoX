# Product Scope

## Product goal
Build a practical, independent UGC game platform. Poko is Roblox-like in product category, not a promise to outperform Roblox or AAA engines.

## Platforms and languages
| Component | Direction |
|---|---|
| PokoX | Windows 10+, Android 11+ |
| Windows application UI (Studio, moderation, PokoX menus) | Rust + EGUI |
| Android application UI (PokoX menus) | Native Kotlin |
| Game UI (in-experience) | Custom Rust UI engine |
| Engine core | Rust |
| Native integrations | C/C++ adapters where required |
| Mute compiler/VM | C (with C ABI for Rust integration) |
| Main backend | Rust (Huggingface CPU) |
| Game handler/authority | Rust (Huggingface CPU) |
| Studio/basic APIs | TypeScript Cloudflare Workers + Rust WASM (100k req/day, fallback to Huggingface) |
| Object storage | Cloudflare R2 (raw assets) + Cloudinary (processed images) |
| Database | Aiven PostgreSQL (3GB) + Layerbase PostgreSQL (5GB) |
| Cache/session | Aiven Valkey (3GB) |
| Edge database | Cloudflare D1 (SQLite) |
| C/C++ docs | Doxygen |

## Principles
- Cheap by default, powerful when needed.
- Poko-owned APIs around third-party libraries.
- Validate consequential actions on trusted services.
- Deterministic import/package pipeline.
- One source of truth for identity, transforms, economy, permissions, and versions.
- Measure before optimizing or claiming performance.
- Basic game creation should be approachable; advanced features must not burden every creator.

## Non-goals for the first release
- No globally distributed physics simulation.
- No user scripts running on the main platform backend.
- Durable Objects are not the game authority.
- No remote AI requirement for autocomplete.
- No claim of full AAA parity.
- No public plugin marketplace before publishing, moderation, and safety foundations.

## Launch assumption
APAC-first. A 5,000 concurrent-user load test is a target, not a verified capacity claim. More regions and expensive features require measured justification.
