---
id: "docs-decisions-002-renderer-backend-md"
title: "ADR-002: Initial Renderer Backend Selection"
kind: "implementation-contract"
execution_phase: "engine-foundation"
status: "accepted"
source_of_truth: "This file plus ../02-architecture/module-boundaries.md and ../11-engineering/testing-and-ci.md"
---

# ADR-002: Initial Renderer Backend Selection

## Status
Accepted

## Date
2026-10-21

## Context

PKX-010 requires selecting a renderer backend for initial integration. Diligent Engine provides multiple backend options on Windows:
- Direct3D 11 (D3D11)
- Direct3D 12 (D3D12)
- Vulkan
- OpenGL
- WebGPU (new in v2.5.6)

**Backend characteristics:**
- D3D11: Works on Windows 7+, broad hardware support, mature API, simpler than D3D12
- D3D12: Windows 10+ only, low-level explicit control, higher complexity, better performance on modern hardware
- Vulkan: Cross-platform, explicit control, requires driver support, higher complexity
- OpenGL: Legacy on Windows, inconsistent driver quality, not recommended for new projects
- WebGPU: Very new (2024), browser-first, desktop support still emerging

**Project requirements:**
- Windows 11 x64 target (docs/10-platforms/windows-and-android.md)
- "Diligent renderer" specified but backend not pinned
- First milestone: build and test basic rendering (clear, present, triangle)

**Build environment reality (ADR-001):**
- Using MinGW/GCC 16.2.0 (MSYS2) based on user confirmation
- CMake configuration of Diligent Core shows:
  - D3D11_SUPPORTED: FALSE (atlbase.h not found - requires Visual Studio ATL)
  - D3D12_SUPPORTED: FALSE (same issue)
  - GL_SUPPORTED: TRUE
  - VULKAN_SUPPORTED: TRUE
- GCC 16.2.0 warning: "gcc 9.0 and above seemingly produce invalid binary code with O2 and O3 optimization levels. Optimization in release configurations will be downgraded to O1. It is strongly recommended to use clang."
- User confirmed D3D11 linking works (`-ld3d11 -ldxgi -ld3dcompiler`) but Diligent's CMake cannot build D3D11 backends without ATL headers

## Decision

Use OpenGL as the primary renderer backend. Vulkan SDK installed successfully, but Diligent Vulkan backend linking fails with MinGW (linker error at DLL creation). OpenGL provides working MinGW compatibility through GLEW. Vulkan can be added later if Visual Studio 2022 becomes available or if MinGW Vulkan linking issues are resolved.

## Alternatives considered

### Option A: OpenGL as primary (ACCEPTED)
- Pros: Available with MinGW; builds successfully with Diligent; broad compatibility; immediate path to rendering milestone
- Cons: Legacy API; inconsistent driver quality; not recommended for new projects

### Option B: Vulkan as primary
- Pros: Modern cross-platform API; future-proof for Linux/Android
- Cons: Linking fails with MinGW (linker error at DLL creation despite SDK installation); GCC 16.2.0 optimization downgrade warning

### Option C: D3D11 primary
- Pros: Broad Windows compatibility; user confirmed direct linking works
- Cons: Not available with MinGW Diligent build (missing atlbase.h/ATL); requires Visual Studio

### Option D: Switch to Visual Studio for D3D11 or Vulkan
- Pros: Full Diligent support; recommended toolchain
- Cons: Violates ADR-001 decision; requires VS installation

## Consequences

### Positive
- OpenGL provides immediate working path with MinGW
- Broad compatibility across hardware
- No additional SDK requirements beyond MinGW
- Single toolchain (MinGW) maintained
- Diligent OpenGL backend builds successfully

### Negative / tradeoffs
- Legacy API with inconsistent driver quality
- Not recommended for new projects by Diligent
- Will need to switch to Vulkan or D3D11 for production-quality rendering
- Less modern features than Vulkan/D3D12

## Security and compatibility impact

- No security impact: backend choice affects performance and features, not security
- Compatibility: OpenGL provides broadest support across Windows versions
- Future: Can switch to Vulkan (with VS2022 or MinGW fix) or D3D11 (with VS2022) later

## Validation

- Build Diligent Engine with OpenGL support enabled (builds successfully)
- Build Diligent Engine with Vulkan disabled (linking fails, confirming blocker)
- Run minimal OpenGL sample (clear/present)
- Verify compatibility with target Windows 11 hardware

## Revisit when

- If OpenGL driver quality proves insufficient
- If MinGW Vulkan linking issues are resolved or Visual Studio 2022 becomes available
- When moving to production-quality rendering requiring modern GPU features
