# Poko Engine Core

Shared C++ engine modules used by PokoX Client, Poko Studio, Poko Mod Review Runtime, and the headless Game Authority.

## Module Structure

- **core**: Base infrastructure (logging, assertions, configuration, allocators, IDs)
- **math**: Vector math, matrices, quaternions, bounds, transforms
- **platform**: OS abstraction, filesystem, threading, lifecycle hooks
- **input**: Input abstraction and device management (SDL2 integration)
- **world**: ECS storage, scenes, hierarchy, object handles, serialization
- **renderer**: Render graph, materials, lighting, camera, GPU resources (Diligent Core)
- **physics**: Custom collision and simulation (incremental, replaceable interface)
- **assets**: Virtual filesystem, package resolution, async loading
- **animation**: Skeletons, clips, blend evaluation, animation events
- **audio**: Audio abstraction
- **ui_runtime**: Runtime UI document parsing, layout, events
- **mute**: MUTE runtime integration and host capability bridge
- **network**: Client transport, Bitstream serialization, session messages
- **diagnostics**: Profiler hooks, crash breadcrumbs, structured logs

## Dependencies

- Diligent Core (graphics abstraction)
- SDL2 (input/window/events)
- Custom physics implementation

## ABI Boundary

All engine modules expose a stable C ABI for Rust (Windows) and Kotlin (Android via JNI) UI layers.
No C++ STL, exceptions, or compiler-specific types cross the ABI boundary.
