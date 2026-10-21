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
- MinGW support: Possible but with known issues
  - GCC 9, 10, 11 produce invalid code in release configurations that causes crash on startup
  - Recommended: GCC 7 or 8, or Clang
  - Historical MinGW build issues (GLEW header conflicts, PCH errors)
  - CI validates Visual Studio and Ninja builds, not MinGW

**Current environment:**
- Compiler: GCC 16.2.0 (MSYS2 UCRT64)
- CMake: 4.4.2
- Platform: Windows 11 x64
- This is a newer GCC version than Diligent recommends for MinGW builds

**Project requirements:**
- "Windows 11 x64, C++20 engine, CMake, Visual Studio 2022" from docs/10-platforms/windows-and-android.md
- "When building on Windows, generating Visual Studio project files is the recommended way" from Diligent docs

## Decision

Build Diligent Engine and integrate it using Visual Studio 2022 rather than MinGW/GCC. MinGW/GCC will remain available for early core library development (PokoCore, PokoPlatform) but renderer integration requires the supported Windows toolchain.

## Alternatives considered

### Option A: Use MinGW/GCC 16.2.0 (current environment)
- Pros: No toolchain change, continues using current setup
- Cons: GCC 16.2.0 is outside Diligent's recommended range (7, 8, or Clang); high risk of release build crashes; unsupported configuration; historical MinGW issues with Diligent

### Option B: Downgrade to GCC 7 or 8 for MinGW
- Pros: Within Diligent's supported range
- Cons: Requires changing MinGW installation; older GCC may lack C++20 features needed by PokoX; still not the recommended Windows toolchain

### Option C: Use Visual Studio 2022 (ACCEPTED)
- Pros: Recommended by Diligent docs; CI-validated; full C++20 support; matches project spec; industry standard for Windows game development
- Cons: Requires installing Visual Studio 2022; dual toolchain maintenance (MinGW for early work, VS for renderer)

## Consequences

### Positive
- Diligent Engine builds with a supported, validated configuration
- Matches project specification for Windows development
- CI workflows can follow Diligent's examples
- Industry-standard toolchain for Windows game development

### Negative / tradeoffs
- Requires Visual Studio 2022 installation (free Community edition available)
- Dual toolchain: MinGW for early core work, VS for renderer integration
- Build system must handle both toolchains
- Developer onboarding requires VS installation

## Security and compatibility impact

- No security impact: toolchain change only affects build, not runtime behavior
- Binary compatibility: Diligent libraries built with VS will link against VS-built PokoX; MinGW-built PokoCore/PokoPlatform may need VS rebuild for final production

## Validation

- Install Visual Studio 2022 Community with C++ build tools and Windows SDK
- Build Diligent Engine minimal sample using VS
- Verify sample runs correctly
- Attempt to link Diligent library against PokoX (mixed toolchain may require adjustments)

## Revisit when

- If MinGW/GCC becomes officially supported by Diligent Engine with CI validation
- If PokoX decides to standardize on a single cross-platform toolchain (e.g., LLVM/Clang everywhere)
