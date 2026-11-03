# Poko Studio (Creator Application)

Windows and Android creation environment for building PokoX games.

## Workspaces

- **project**: Project manager, create/open/backup/recover .pk projects
- **world_editor**: Scenes, objects, hierarchy, terrain/geometry tools, transform gizmos
- **ui_maker**: Responsive canvas, hierarchy, inspector, themes, components, data binding
- **mute_editor**: Syntax highlighting, completion, diagnostics, formatting, API help, debugger
- **asset_manager**: Import, preview, organize, optimize, and manage dependencies
- **test_lab**: Local play, multiplayer simulation, network fault simulation, device profiles
- **server_manager**: Server config, package export, registration, health and logs
- **publishing**: Build artifacts, compare releases, submit review, inspect status

## Project Model

Shared .pk project structure:
- manifest
- settings
- scenes/
- ui/
- scripts/client/
- scripts/server/
- scripts/shared/
- assets/
- materials/
- animations/
- dependencies.lock
- server-config/
- publishing/

## Platform Differences

- Windows: Rust + egui with full editor suite
- Android: Kotlin with touch-first UI, collapsible panels, simplified toolbar
- Both open the same project model and produce compatible .pk/.pkx/.pkB outputs

## Key Features

- Beginner-friendly by default, advanced on demand
- Build .pkx and .pkB from reproducible project snapshots
- Technical validation before upload
- Human review integration
- Offline save and autosave
