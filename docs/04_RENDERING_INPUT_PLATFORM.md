# 04 — Rendering, Input, and Platform Integration

**Status:** Proposed baseline  
**Graphics foundation:** Diligent Core  
**Initial rendering backends:** OpenGL on Windows; OpenGL ES on Android  
**Input/event library:** SDL2.

## 1. Renderer responsibilities

Diligent Core provides low-level graphics API interfaces and backend implementations. PokoX must still implement the engine renderer above it: render graph, scene visibility, material model, lighting, camera, batching, shader management, asset-to-GPU upload, post-processing, render profiling and scalability settings.

The world/ECS layer must not include Diligent headers. Render data should be extracted into render snapshots or command structures that the renderer consumes.

## 2. Initial rendering target

- Windows 11: desktop OpenGL baseline selected after hardware survey; proposed minimum OpenGL 4.2, subject to Diligent backend and driver validation.
- Android 11+: OpenGL ES 3.x baseline; proposed minimum ES 3.0, subject to target-device tests and Diligent support.
- Feature detection at runtime is mandatory. Never assume that an OS version guarantees a GPU feature.
- Shader code must use a tested common feature subset across desktop GL and GLES. Define shader language and cross-compilation pipeline in an ADR before shader library grows.

## 3. Render pipeline proposal

Initial pipeline:

1. Frame begin and surface validation.
2. Gather visible world render components.
3. Frustum culling and basic sorting.
4. Opaque geometry pass.
5. Sky/environment pass.
6. Transparent geometry pass with explicit ordering policy.
7. UI pass.
8. Debug/profiling overlays.
9. Present.

Later: shadow maps, instancing, GPU-driven visibility, post-processing, HDR, temporal effects, clustered lighting and advanced material variants. These are not MVP blockers.

## 4. Device capability profile

At startup collect: API version, renderer/vendor/device identifiers as permitted, max texture dimensions, uniform/buffer limits, shader features, texture compression support, depth formats, MSAA modes and relevant alignment constraints. Use a capability object to choose features. Do not send unique hardware fingerprints to analytics unless necessary, disclosed, and privacy-reviewed.

## 5. Quality profiles

- Low: reduced shadows/effects, smaller texture mip bias, lower render scale where needed, limited particle count.
- Medium: balanced defaults.
- High: higher resolution/shadow/effects where supported.

Profiles must not change authoritative gameplay rules. Games can request quality profiles but cannot demand unsupported GPU features.

## 6. SDL2 responsibilities

SDL2 is used for window creation/events and keyboard, mouse, controller, joystick and touch input where supported. PokoX owns a platform-independent input action layer: physical events → device abstraction → action mapping → game input state. MUTE game APIs receive normalized actions rather than raw unbounded SDL event objects.

The application must decide one owner for OpenGL context creation and lifetime. Prototype SDL-created window/context interoperability with Diligent early. Do not accidentally create competing GL contexts or switch contexts from the wrong thread.

## 7. Android lifecycle

Handle pause/resume, surface created/destroyed, low-memory events, safe areas/insets, rotation, touch gestures, activity recreation, audio focus and back navigation. On background transition, stop rendering against an invalid surface and checkpoint local saves. If the graphics device/context is lost, recreate resources from CPU-side asset references and the asset cache.

Kotlin owns Android app-level navigation and permission flows. Native C++ owns the game runtime and renderer. JNI calls must be bounded and must not cross threads without an explicit contract.

## 8. Windows platform

Handle window resize, DPI scaling, high-DPI display, keyboard layouts, mouse capture/release, controller hotplug, suspend/resume, file associations for `.pk`/`.pkx`, and user-selected game library locations.

## 9. Asset and shader policy

Use glTF 2.0/GLB as the initial 3D interchange path. Convert assets into optimized Poko runtime representations. Store source/authoring asset separately from optimized runtime asset when licensing permits. Shader compilation should happen during build/import or controlled pipeline stages; player package loading must validate shader format and resource requirements before GPU use. Avoid arbitrary executable shader extensions supplied as native code.

## 10. Acceptance tests

- Windows UCRT64 build creates an SDL2 window and Diligent OpenGL render device.
- Draw a known triangle and a textured mesh.
- Resize and DPI change do not corrupt rendering.
- Android test app initializes GLES, handles pause/resume and surface recreation.
- Touch, keyboard, mouse and controller map to the same logical action interface.
- Renderer falls back or gives a clear compatibility error on unsupported hardware.
- Shader/resource validation rejects malformed or unsupported package content safely.

## 11. References

- [Diligent Core](https://github.com/DiligentGraphics/DiligentCore)
- [Diligent getting started](https://diligentgraphics.com/diligent-engine/getting-started/)
- [SDL2 Introduction](https://wiki.libsdl.org/SDL2/Introduction)
- [SDL Android lifecycle notes](https://github.com/libsdl-org/SDL/blob/main/docs/README-android.md)
