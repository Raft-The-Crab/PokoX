# PokoX Client (Rust UI Layer)

Windows and Android player client UI and application logic.

## Components

- **ui**: Rust + egui (Windows) / Kotlin (Android) application shell
- **library**: Game library, installed games, update management
- **downloads**: Download manager, resumable downloads, chunk cache
- **offline**: Offline runtime, local saves, cloud sync
- **network**: Session discovery, join tickets, real-time connection

## Architecture

- UI shell calls shared C++ engine through stable C ABI
- Downloads use signed R2 URLs with chunk deduplication
- Local saves are untrusted (device owner can modify)
- Offline modes: OFFLINE_READY, HYBRID, ONLINE_REQUIRED
- Pokoins cannot be earned/spent offline

## Features

- Account login and profile
- Game discovery and store pages
- Download/install/update/verify/uninstall/repair
- Game launch and runtime lifecycle
- Session discovery/join
- Friends/party UI (as released)
- Local saves and offline runtime
- Report/block/mute features
- Download storage settings and parental controls

## Platform Differences

- Windows: Rust + egui shell
- Android: Kotlin shell with JNI bridge to C++ engine
- Both use the same C++ engine core
