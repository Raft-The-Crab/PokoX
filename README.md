# Poko

Poko is a practical independent UGC (User-Generated Content) game platform with Roblox-like product characteristics. It enables creators to build, share, and monetize games while providing players with a diverse gaming experience.

## Platform

- **PokoX**: Player client for Windows and Android
- **Poko Studio**: Creator tooling for building games
- **Mute**: Custom scripting language for game logic

## Architecture

- **Engine Core**: Rust-based with native adapters
- **Physics**: Proprietary Poko physics engine
- **Rendering**: Diligent integration (not rebuilt)
- **Scripting**: Mute language (C implementation with C ABI)
- **Backend**: Rust services on Hugging Face CPU
- **Edge**: Cloudflare Workers for Studio APIs
- **Storage**: Cloudflare R2, PostgreSQL, Valkey

## Platforms

- Windows 10+
- Android 11+

## Tech Stack

### Applications
- **Windows UI**: Rust + EGUI
- **Android UI**: Native Kotlin
- **In-Game UI**: Custom Rust UI system

### Engine
- **Core**: Rust
- **Physics**: Rust (proprietary)
- **Rendering**: Diligent (C++) via C adapter
- **Audio**: miniaudio via C adapter
- **Fonts**: freetype via C adapter

### Backend
- **Main Backend**: Rust on Hugging Face CPU
- **Game Handler**: Rust on Hugging Face CPU
- **Edge APIs**: TypeScript + Rust WASM on Cloudflare Workers

### Scripting
- **Mute**: C-based compiler and VM with C ABI

## Getting Started

### Prerequisites
- Rust 1.70+
- CMake 3.20+
- Android SDK (for Android builds)
- Node.js 18+ (for Workers)

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/Raft-The-Crab/PokoX.git
cd PokoX

# Build desktop PokoX
cargo build --release

# Build Poko Studio
cargo build --release -p poko-studio

# Build Android APK
./scripts/build-android.sh
```

## Documentation

Planning and architecture documentation is available in the `docs/` directory.

## License

Proprietary - All rights reserved. See LICENSE file for details.

## Status

Early development phase. Not yet ready for public use.
