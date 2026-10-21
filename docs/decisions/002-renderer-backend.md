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

Use Vulkan as the primary renderer backend with OpenGL as a fallback. D3D11/D3D12 are not available with MinGW due to missing ATL headers (atlbase.h). Vulkan is supported and provides modern cross-platform graphics. OpenGL provides a legacy fallback. Both backends are available through Diligent's abstraction layer with MinGW.

## Alternatives considered

### Option A: Vulkan as primary with OpenGL fallback (ACCEPTED)
- Pros: Modern cross-platform API; MinGW supports Vulkan backend; available with Diligent CMake; future-proof for Linux/Android; OpenGL provides fallback
- Cons: Requires Vulkan SDK installation; GCC 16.2.0 optimization downgrade warning (O2/O3 → O1)

### Option B: D3D11 primary with Vulkan/OpenGL support
- Pros: Broad Windows compatibility; user confirmed direct linking works
- Cons: Not available with MinGW Diligent build (missing atlbase.h/ATL); requires Visual Studio; violates ADR-001 decision to use MinGW

### Option C: OpenGL only
- Pros: Available with MinGW; broad compatibility
- Cons: Legacy API; inconsistent driver quality; not recommended for new projects

### Option D: Switch to Visual Studio for D3D11
- Pros: Full Diligent support; recommended toolchain
- Cons: Violates ADR-001 decision; requires VS installation

## Consequences

### Positive
- Vulkan provides modern cross-platform graphics API
- Future-proof for Linux and Android support
- MinGW/Diligent CMake supports Vulkan backend
- OpenGL provides fallback for systems without Vulkan
- Single toolchain (MinGW) maintained

### Negative / tradeoffs
- Requires Vulkan SDK installation (not yet installed)
- GCC 16.2.0 optimization downgrade (O2/O3 → O1) may impact performance
- D3D11/D3D12 not available with MinGW (could add later with VS2022 if needed)
- Vulkan driver support varies by hardware

## Security and compatibility impact

- No security impact: backend choice affects performance and features, not security
- Compatibility: Vulkan supported on Windows 10+; OpenGL provides broader legacy support
- Future: Can add D3D11/D3D12 later by switching to VS2022 if needed

## Validation

- Install Vulkan SDK
- Build Diligent Engine with Vulkan support enabled (already configured successfully)
- Build Diligent Engine with OpenGL support enabled (already configured successfully)
- Run minimal Vulkan sample (clear/present)
- Run minimal OpenGL sample (clear/present)
- Verify compatibility with target Windows 11 hardware

## Revisit when

- If Vulkan SDK installation or driver issues arise
- If GCC 16.2.0 optimization downgrade causes performance problems
- If D3D11/D3D12 features are needed and Visual Studio 2022 becomes available
- If Vulkan driver support proves insufficient on target hardware
