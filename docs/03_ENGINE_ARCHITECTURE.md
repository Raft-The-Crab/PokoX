# 03 — Shared C++ Engine Architecture

**Status:** Proposed baseline  
**Primary implementation:** C++  
**Build system:** CMake + Ninja  
**First desktop environment:** MSYS2 UCRT64  
**Application boundary:** Stable C ABI; JNI bridge for Android.

## 1. Goals

The engine is shared by PokoX Client, Poko Studio, Poko Mod Review Runtime, and the headless Game Authority runtime where appropriate. Sharing means common libraries and contracts, not one executable or one privilege domain.

Goals:

- One asset/world/runtime implementation across applications.
- Windows-first development without blocking Android support.
- Diligent Core as the graphics API foundation.
- OpenGL desktop backend first; OpenGL ES on Android.
- SDL2 for input, window events and platform event handling where appropriate.
- Custom physics developed incrementally behind a replaceable interface.
- ECS plus hierarchy and stable object handles.
- MUTE runtime isolated from platform control code.
- Headless server build without renderer/audio/UI dependencies.

Non-goals for the initial engine: general-purpose AAA engine, full editor framework in C++, arbitrary native game plugins, complete physical simulation of every possible object, or support for every graphics backend on day one.

## 2. Module layers

### Foundation

`poko_core`: assertions, result/error types, logging interfaces, configuration, allocators, job abstraction, time, IDs, build/version metadata.

`poko_math`: vectors, matrices, quaternions, bounds, rays, transforms, numerical helpers. Define coordinate system, handedness, unit scale, angle units, and matrix convention in an ADR before content is created.

`poko_platform`: OS abstraction, filesystem sandbox roots, monotonic clock, thread primitives, lifecycle hooks, surface handles, app events.

`poko_input`: normalized input actions, physical input devices, rebinding, touch gesture mapping, controller abstraction. SDL2 is the initial event/device integration, not the public game API.

### Runtime

`poko_world`: scenes, ECS storage, object handles, hierarchy, transform propagation, serialization boundaries.

`poko_renderer`: render graph/pass orchestration, materials, camera, lighting, GPU-resource lifetime, shader variants, visibility and draw submission. Diligent Core is underneath this layer.

`poko_physics`: custom collision and simulation implementation behind `IPhysicsWorld`.

`poko_assets`: virtual asset filesystem, package resolution, cache, resource state, asynchronous loading.

`poko_animation`: skeletons, clips, blend evaluation, animation events, retargeting later.

`poko_audio`: audio abstraction; miniaudio is a candidate dependency and must pass licensing/platform tests before adoption.

`poko_ui_runtime`: runtime document parsing, layout, text, image, input, accessibility semantics, animation and event bindings.

`poko_mute`: parser/runtime integration and host capability bridge. Keep this separate from trusted C++ platform code.

`poko_network`: client transport adapter, Bitstream serialization, session messages, replication codecs. Do not put platform REST API logic in the real-time packet module.

`poko_diagnostics`: profiler hooks, crash breadcrumbs, counters and structured logs.

### Application hosts

- `pokox_client_host`: player library, login, install, runtime launch, social and session UX.
- `poko_studio_host`: project manager, editors, local test runner, package builder, publishing UX.
- `poko_mod_host`: moderator UI and review orchestration; no creator script execution in this process.
- `poko_review_host`: separate restricted process that loads exact submitted content using shared runtime libraries.
- `poko_authority_host`: headless C++ server process; does not link renderer or desktop UI.

## 3. Dependency direction

Allowed direction:

`core → math/platform` (or shared low-level dependencies) → world/assets/input → physics/renderer/audio/UI → application hosts.

Avoid cycles. Renderer may read world render components through a stable snapshot/query interface, but the world module should not depend on Diligent types. Physics may consume transforms/colliders through defined interfaces, but should not own scene hierarchy. MUTE receives host capabilities; it must not receive raw internal engine pointers or unrestricted native function pointers.

## 4. Diligent integration

Use Diligent Core as the lower-level graphics abstraction. Poko's renderer owns higher-level concepts: render passes, material system, scene visibility, shader variant selection, render queues, lighting policy, post-processing, profiling and device capability policy.

Do not expose Diligent interfaces through the C ABI. Keep Diligent types inside the C++ renderer module. Initial backend targets are desktop OpenGL on Windows and OpenGL ES on Android. Vulkan or Direct3D are future options, not current requirements.

## 5. C ABI boundary

Rust egui and Kotlin must not depend on C++ ABI details. Export opaque handles and fixed-layout structs with explicit sizes/version fields. Use integer error codes or tagged result structs. Catch C++ exceptions before crossing the ABI. Never pass STL containers, C++ classes, references, exceptions, or compiler-specific types across the boundary.

Example sketch:

```c
typedef struct PokoEngine PokoEngine;

typedef struct PokoEngineConfig {
    uint32_t struct_size;
    uint32_t api_version;
    uint32_t flags;
} PokoEngineConfig;

int32_t poko_engine_create(
    const PokoEngineConfig* config,
    PokoEngine** out_engine);

int32_t poko_engine_update(PokoEngine* engine, double elapsed_seconds);
void poko_engine_destroy(PokoEngine* engine);
```

The production ABI also needs surface creation/recreation, lifecycle notification, input submission, render invocation, package loading, diagnostics and thread-affinity rules.

## 6. Lifecycle

Define explicit states: `Uninitialized`, `Initializing`, `Ready`, `Running`, `Suspended`, `Recovering`, `Stopping`, `Stopped`, `Failed`. Android surface loss and low-memory notifications must be handled; do not assume the graphics context survives backgrounding. On Windows, handle resize, DPI changes, focus changes and device/context loss where possible.

The client host owns application lifecycle. Engine modules receive explicit callbacks and do not independently run unbounded loops.

## 7. Thread model

Proposed initial model:

- Main/application thread: UI and platform event dispatch.
- Render thread: graphics API/context ownership; exact choice is validated in prototype.
- Simulation thread: fixed-step world/physics update where worthwhile.
- Worker pool: asset decode, package hashing, compression, non-GPU preprocessing.

Do not create a thread per entity, script, or asset. MUTE tasks are cooperative and budgeted. Define which APIs are main-thread-only and enforce through assertions.

## 8. CMake target plan

```text
poko_core
poko_math
poko_platform
poko_input
poko_assets
poko_world
poko_physics
poko_renderer
poko_animation
poko_audio
poko_ui_runtime
poko_mute
poko_network
poko_diagnostics
poko_client_host
poko_studio_host
poko_mod_host
poko_review_host
poko_authority_host
poko_tests
```

Create imported/interface targets for pinned third-party dependencies. Do not leak dependency include paths globally. Use `target_link_libraries` and `target_compile_features` per target. Separate desktop and Android toolchain configurations. Keep headless target free of SDL video and Diligent renderer linkage.

## 9. Error handling and logging

- Use structured error codes for expected failures.
- Use exceptions only internally if selected by project convention; never across ABI, network, or scripting boundaries.
- Log package/game/server/submission IDs where relevant, but redact authentication tokens, private messages, email addresses and secrets.
- Crash reporting must be opt-in/transparent as required by platform policy and privacy rules.

## 10. Acceptance tests

- Every module builds independently where designed to do so.
- Headless authority links no graphics backend.
- Engine C ABI can be called from a small Rust test host.
- Android JNI bridge can create/destroy the engine and handle suspend/resume.
- Renderer can create a device/context and draw a known test scene.
- Package loading and rendering use identical runtime code in client and review host.
- A malformed asset cannot crash the trusted Studio/Mod process.
- Engine can shut down cleanly and release GPU resources.

## 11. Dependencies and source references

Diligent Core supports OpenGL and OpenGL ES backends and documents Win32 and Android initialization; see [DiligentCore](https://github.com/DiligentGraphics/DiligentCore) and [Diligent getting started](https://diligentgraphics.com/diligent-engine/getting-started/). SDL2 supports Windows and Android and provides input/event abstractions; see [SDL2 Introduction](https://wiki.libsdl.org/SDL2/Introduction). Actual MSYS2 compatibility must be proven in the feasibility milestone, not assumed from upstream support alone.
