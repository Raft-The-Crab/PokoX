# 13 — Poko Studio Creator Application Specification

**Status:** Proposed baseline  
**Targets:** Windows 11+ and Android 11+  
**Frontends:** Rust + egui on Windows; Kotlin on Android; shared C++ engine/services through C ABI/JNI where applicable.

## 1. Product goals

Poko Studio should be beginner-friendly by default and advanced on demand. Windows and Android must open the same project model. Android UI should be touch-first rather than a compressed desktop layout.

## 2. Workspaces

- Project Manager: create/open/backup/recover projects.
- World Editor: scenes, objects, hierarchy, terrain/geometry tools, transform gizmos and environment.
- UI Maker: responsive canvas, hierarchy, inspector, themes, components, data binding and animation.
- MUTE Editor: completion, diagnostics, formatting, API help, debugger and profiler.
- Asset Manager: import, preview, organize, optimize and manage dependencies.
- Test Lab: local play, multiplayer simulation, network fault simulation, device profiles and offline tests.
- Server Manager: server config, package export, registration, health and logs.
- Publishing: build artifacts, compare releases, submit review and inspect status.

## 3. Shared project model

```text
PokoProject
  manifest
  settings
  scenes/
  ui/
  scripts/client/
  scripts/server/
  scripts/shared/
  assets/
  materials/
  animations/
  dependencies.lock
  server-config/
  publishing/
```

The project service is shared by all workspaces. Do not let editors maintain independent copies of project data.

## 4. World Editor

MVP: scene hierarchy, selection, transform tools, object duplication, grouping, property inspector, material assignment, asset browser, undo/redo, save/reopen, grid/snapping, play preview and validation warnings. Later: terrain sculpting, advanced lighting, procedural tools, collaboration and complex prefab workflows.

## 5. UI Maker

One UI document supports beginner and advanced modes. Beginner mode offers drag/drop, templates, visual properties, auto-layout, live preview and simple animation. Advanced mode exposes anchors, constraints, nested layouts, components, variants, binding, event connections, timeline, themes and profiling.

Required runtime layout primitives: stack/flow, grid, anchors, constraints, alignment, min/max sizing, safe-area support and scaling. Preview Android phone/tablet and Windows aspect ratios. UI documents compile into a runtime representation; no separate incompatible visual scripting runtime.

## 6. MUTE editor

Syntax highlighting, formatting, type diagnostics, auto-completion, hover docs, go-to-definition, find references, breakpoints where supported, call stack, variables, execution budget display, realm/capability warnings, test runner and source mapping.

## 7. Asset Manager

Initial import targets: glTF 2.0/GLB models, PNG/JPEG images, selected audio formats, MUTE source, UI documents and project templates. Validate file signatures, dimensions, duration, mesh complexity, path structure and metadata. Provide preview before importing. Convert into optimized runtime formats. Record source creator/license/version/hash.

## 8. Edit history and autosave

Use command-based edits with reversible commands for create/delete/change transform/change property/edit UI. Autosave locally and atomically. Local autosave must not depend on network availability. Project backups are snapshots; Git integration is optional and later. Do not put large generated binaries into ordinary Git by default; use R2/reference storage or an agreed binary strategy.

## 9. Android Studio adaptation

- Collapsible panels and contextual inspector.
- Touch-safe gizmos and selection.
- Simplified toolbar with advanced tools discoverable.
- Safe-area aware previews.
- Background/foreground lifecycle handling.
- Save checkpoint before memory pressure/termination.
- Heavy builds may be delegated to a supported build service later; initial version may build locally if device resources permit.

## 10. Publishing workflow

Create immutable build snapshot → validate dependencies → compile realm-specific scripts → generate `.pkx` and `.pkB` → generate content hashes/manifest → technical checks → private upload → human review. If project changes during upload, publish from snapshot rather than reading mutable project files.

## 11. GitHub backup

Deferred after local backup is reliable. Prefer least-privilege GitHub App/repository scope. Store source project files and metadata; manage large assets separately or with explicit LFS/binary strategy. Never store signing keys, server private keys or platform credentials in repositories.

## 12. Acceptance tests

- Windows and Android project files round-trip.
- Switching UI Maker modes preserves advanced properties.
- Undo/redo restores exact previous state.
- Autosave recovers after forced close.
- Project builds deterministic content hashes for unchanged assets.
- Android UI is usable on narrow screens and handles lifecycle changes.
- Invalid assets are rejected safely.
- Published output excludes server-only files from `.pkx`.
