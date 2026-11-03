# ADR-0001 — Shared C++ Engine Across Client, Studio and Mod

**Status:** Accepted baseline  
**Date:** 2026-11-03

## Context
PokoX Client, Poko Studio and Poko Mod need consistent package loading, rendering, world and runtime behavior. Separate engines would increase workload and allow mismatches.

## Decision
Use a shared C++ engine core. Windows UI uses Rust/egui; Android UI uses Kotlin. Both call a versioned C ABI (Android through JNI). Poko Mod uses the same engine libraries in a separate restricted Review Runtime process, not in the privileged moderator process.

## Consequences
- Engine components are reusable and tested once.
- Application privilege boundaries remain separate.
- ABI compatibility and engine versioning become important.
- Untrusted review content stays out of the dashboard process.
