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

## Decision

Use Direct3D 11 as the initial renderer backend for Windows. This provides the broadest compatibility and simplest integration path for the first rendering milestone.

## Alternatives considered

### Option A: Direct3D 11 (ACCEPTED)
- Pros: Broad Windows support (7+), mature and stable, simpler API than D3D12/Vulkan, minimal driver issues, Diligent has extensive D3D11 samples
- Cons: Less modern than D3D12/Vulkan, some performance overhead compared to low-level APIs

### Option B: Direct3D 12
- Pros: Modern low-level API, better performance on modern hardware, explicit control
- Cons: Windows 10+ only (though project targets Windows 11), higher complexity, steeper learning curve, more error-prone

### Option C: Vulkan
- Pros: Cross-platform, modern, explicit control
- Cons: Requires driver support validation, higher complexity, Windows Vulkan drivers vary in quality

### Option D: WebGPU
- Pros: Modern API, cross-platform future
- Cons: Very new (added to Diligent in 2024), desktop WebGPU still emerging, less proven for production games

## Consequences

### Positive
- Fastest path to first rendering milestone
- Minimal driver compatibility issues
- Simplest error handling and debugging
- Matches Diligent's sample patterns
- Can switch to D3D12 or Vulkan later if needed (Diligent makes backend switching relatively painless)

### Negative / tradeoffs
- Not the most modern or performant API
- May need to add D3D12 or Vulkan later for advanced features (ray tracing, compute shaders)
- Legacy API (though still widely used and supported)

## Security and compatibility impact

- No security impact: backend choice affects performance and features, not security
- Compatibility: D3D11 works on all Windows 11 systems
- Future: Backend switching requires render API changes but Diligent's abstraction layer reduces impact

## Validation

- Build Diligent Engine with D3D11 support enabled
- Run minimal D3D11 sample (clear/present)
- Verify compatibility with target Windows 11 hardware
- Test on at least one Windows 11 machine

## Revisit when

- When performance profiling shows D3D11 as a bottleneck
- When D3D12 or Vulkan features are needed (ray tracing, advanced compute)
- When Windows 7/8 support is dropped from requirements
- When WebGPU desktop support matures and becomes the preferred cross-platform backend
