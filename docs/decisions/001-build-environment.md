---
id: "docs-decisions-001-build-environment-md"
title: "ADR-001: Build Environment for Diligent Engine Integration"
kind: "implementation-contract"
execution_phase: "engine-foundation"
status: "accepted"
source_of_truth: "This file plus ../02-architecture/module-boundaries.md and ../11-engineering/testing-and-ci.md"
---

# ADR-001: Build Environment for Diligent Engine Integration

## Status
Accepted

## Date
2026-10-21

## Context

PKX-010 requires pinning and integrating Diligent Engine as the renderer dependency. The project documentation specifies Visual Studio 2022 as the Windows build toolchain. However, the current local development environment uses MinGW/GCC 16.2.0 (MSYS2).

**Diligent Engine requirements and compatibility:**
- Latest stable release: v2.5.6 (September 2024)
- License: Apache 2.0 (permissive, suitable for commercial use)
- Recommended Windows build: Visual Studio with Windows SDK 10.0.17763.0 or later
- MinGW support: Possible with caveats
  - GCC 9, 10, 11 produce invalid code in release configurations that causes crash on startup
  - Recommended: GCC 7 or 8, or Clang
  - Historical MinGW build issues (GLEW header conflicts, PCH errors)
  - CI validates Visual Studio and Ninja builds, not MinGW
  - Direct D3D11 linking with MinGW works: `g++ -ld3d11 -ldxgi -ld3dcompiler`

**Current environment:**
- Compiler: GCC 16.2.0 (MSYS2 UCRT64)
- CMake: 4.4.2
- Platform: Windows 11 x64
- This is a newer GCC version than Diligent recommends for MinGW builds

**Project requirements:**
- "Windows 11 x64, C++20 engine, CMake, Visual Studio 2022" from docs/10-platforms/windows-and-android.md
- User confirmation: MinGW works with D3D11, Vulkan, and OpenGL libraries

## Decision

Proceed with MinGW/GCC 16.2.0 for Diligent Engine integration. The historical MinGW issues appear to be related to Diligent's complex build system and GLEW integration. Direct library linking with MinGW works (`-ld3d11 -ldxgi -ld3dcompiler` for D3D11, `-lvulkan` for Vulkan, `-lopengl32` for OpenGL). We will attempt a minimal Diligent integration focusing on the core renderer without full sample build system complexity.

## Alternatives considered

### Option A: Use MinGW/GCC 16.2.0 (ACCEPTED)
- Pros: No toolchain change, continues using current setup; user confirmed D3D11/Vulkan/OpenGL linking works with MinGW
- Cons: GCC 16.2.0 is outside Diligent's recommended range (7, 8, or Clang); historical MinGW issues with Diligent's build system; release build crash risk
- Mitigation: Focus on minimal integration without full Diligent build system; test thoroughly; fall back to VS2022 if issues arise

### Option B: Use Visual Studio 2022
- Pros: Recommended by Diligent docs; CI-validated; full C++20 support; matches project spec
- Cons: Requires installing Visual Studio 2022; toolchain divergence from current setup

### Option C: Downgrade to GCC 7 or 8 for MinGW
- Pros: Within Diligent's supported range
- Cons: Requires changing MinGW installation; older GCC may lack C++20 features; still not the recommended Windows toolchain

## Consequences

### Positive
- Single toolchain (MinGW) for all PokoX development
- No Visual Studio installation required
- Faster iteration with current environment
- User-confirmed D3D11/Vulkan/OpenGL library linking works

### Negative / tradeoffs
- GCC 16.2.0 is outside Diligent's recommended compiler range
- Potential release build issues (GCC 9, 10, 11 have known crashes)
- Historical MinGW issues with Diligent's build system (GLEW, PCH)
- May need to work around Diligent's full build system complexity
- Fallback to VS2022 if MinGW integration proves unstable

## Security and compatibility impact

- No security impact: toolchain choice affects build, not runtime behavior
- Binary compatibility: All components built with MinGW; consistent linking

## Validation

- Clone Diligent Engine repository
- Attempt minimal CMake build with MinGW (not full sample build system)
- Test D3D11 linking: `-ld3d11 -ldxgi -ld3dcompiler`
- Test Vulkan linking: `-lvulkan`
- Test OpenGL linking: `-lopengl32`
- If MinGW integration fails, revisit decision and switch to VS2022

## Revisit when

- If MinGW integration proves unstable or has runtime issues
- If release builds crash due to GCC 16.2.0 issues
- If Diligent adds official MinGW CI validation
