# Rendering and Platform

## Diligent-backed renderer sequence
- [ ] Inventory existing integration and actual backend support per target.
- [ ] Minimal Windows device/window/swapchain smoke test.
- [ ] Poko-owned interfaces for buffers, textures, pipelines, materials and commands.
- [ ] Resource lifetime and device-loss recovery.
- [ ] Baseline forward renderer: depth, material, directional light, shadow.
- [ ] Visibility/culling, instancing, LOD after profiler baselines.
- [ ] Post-processing, terrain, water and VFX as separate milestones.
- [ ] Prove Android graphics path independently.

## Quality modes
- [ ] Define scalable presets and per-feature overrides.
- [ ] Adapt quality only from measured pressure; avoid oscillation.
- [ ] Expose active settings and diagnostics.
- [ ] Benchmark representative low/mid/high supported devices.

## Platform layer
- [ ] Window/input, filesystem, clocks, threads, synchronization.
- [ ] Device capability detection.
- [ ] Android lifecycle, touch, pause/resume, thermal and memory pressure.
- [ ] Crash diagnostics and resource cleanup.

## Gate
Sample scene renders; resources release on unload/shutdown; presets have measured CPU/GPU/memory results by device. Windows success does not prove Android compatibility.
