# M0-E — Android Feasibility Assessment

**Status:** assessment only. No Android build was performed in M0; the milestone
requires an identified path and early warning of incompatibilities, not a client.

**Verdict:** no architectural blocker was found. Two items must be validated
before Android work is scheduled seriously — **EGL/GL context ownership** (§4)
and the **NDK version** (§2.2) — and one honest limitation: **no device or
emulator was connected**, so nothing below is a runtime result.

---

## 1. Target

- Android 11+ → `minSdkVersion = 30`.
- ABI: `arm64-v8a` primary; `x86_64` only for emulator testing.
- Renderer: OpenGL ES 3.0 baseline (Diligent GLES backend), decided at runtime
  by feature detection, never by OS version alone.
- UI: Kotlin owns navigation, permissions and app lifecycle; C++ owns the game
  runtime and renderer; the boundary is JNI over the same C ABI proven in M0-D.

---

## 2. Local Android toolchain — measured

| Component | Present | Version | Assessment |
|---|---|---|---|
| SDK root | `C:\Android` | — | usable |
| Platforms | `android-35` | API 35 | fine (target ≥ 34 required by Play) |
| Build tools | `35.0.0` | — | fine |
| NDK | `C:\Android\ndk-bundle` | **22.1.7171670 (r22b)** | **too old** — see §2.2 |
| Gradle | `C:\Android\Gradle` | **9.7.1** (Kotlin 2.4.0) | fine, requires AGP 8.x+ |
| JDK | `C:\Program Files\Java\jdk-21.0.12` | 21 LTS | fine |
| `adb` | `C:\Android\platform-tools` | 37.0.1 | fine |
| Devices | — | **none attached** | blocks any runtime claim |

### 2.1 JDK prerequisite

Gradle 9.7.1 runs on JDK 21, confirmed by `gradle --version`. The shell default
`java` is Zulu 11, which is too old for AGP 8.x; Android builds must set
`JAVA_HOME` to the JDK 21 installation explicitly. This is a build-script
concern, not an engine concern.

### 2.2 NDK r22b is a real problem

NDK 22.1 (2021) ships Clang 12-era libc++ with incomplete C++20 library support.
The PokoX engine is C++20 — `concepts`, `std::span`, `std::bit_cast`,
`std::endian` (already used in the M0 smoke test). Expect missing library
features and obscure template failures rather than clean diagnostics.

**Recommendation:** install **NDK r27 or newer** before any Android C++ work, and
pin it in `cmake/toolchains/android/ndk`. Do not attempt to work around this by
lowering the engine's language level; the Windows toolchain is already C++20 and
divergent language levels per platform is a long-term maintenance hazard.

---

## 3. Required components and what each one implies

| Requirement | Approach | Risk |
|---|---|---|
| Android NDK | r27+, pinned; `android.toolchain.cmake` | low, once upgraded |
| Kotlin/JNI | thin JNI layer over the M0-D C ABI; no C++ types cross it | low — ABI already proven on Windows |
| SDL2 on Android | build from pinned source, ship `SDLActivity` Java classes in the Gradle project | medium — see §4 |
| Diligent GLES | `GraphicsEngineOpenGL` with `GLES_SUPPORTED`, EGL surface from the native window | medium — see §4 |
| App lifecycle | `surfaceCreated`/`surfaceDestroyed`, `onPause`/`onResume`, `onLowMemory`, configuration change | low, but must be designed now (§5) |
| CMake | NDK toolchain file, **separate build directory**, never the MSYS2 one | low |

`docs/26` §6 already states that the Windows generator must not be reused for
Android. That is correct and is reinforced here: MSYS2 GCC cannot produce
Android binaries at all, so cross-contamination would fail at the first link
rather than silently.

---

## 4. The real architectural risk: who owns EGL

On Windows, M0-C had a clean answer: **Diligent owns the GL context**, created
from an SDL-created `HWND`. SDL never touches GL. One owner, no competing
context.

On Android the situation inverts, and this is the incompatibility to surface
early:

- SDL2's Android backend (`SDLActivity`) receives `surfaceCreated` /
  `surfaceDestroyed` and manages an `EGLSurface` bound to the `ANativeWindow`
  that Android gives it. It exposes the native window through
  `SDL_WINDOWEVENT`/`SDL_AndroidGetWindow`-style access.
- Diligent's GL/GLES factory can create its own EGL surface, or attach to a
  context that already exists (`AttachToActiveGLContext`).

If both create a surface, there are two `EGLSurface`s on one window — the exact
"competing contexts" failure the requirements forbid, and it typically manifests
as a black screen or `EGL_BAD_SURFACE` rather than a clear error.

**Proposed per-platform ownership, to be recorded in an ADR and validated with a
device:**

| Platform | Window owner | GL context owner | Diligent entry point |
|---|---|---|---|
| Windows | SDL2 | **Diligent** | `CreateDeviceAndSwapChainGL` with the `HWND` |
| Android | SDL2 (`SDLActivity`) | **SDL2** (`SDL_GL_CreateContext`) | `AttachToActiveGLContext` |

Consequences that must exist in the platform layer from M1, not be retrofitted:

1. The C ABI needs an explicit "the surface is gone" notification, and device
   re-creation must not depend on a context that died with it.
2. Shaders, pipeline states and GPU buffers are destroyed on context loss and
   recreated from CPU-side asset references — the asset cache is therefore a
   correctness requirement, not an optimisation.
3. The render thread must be the thread that owns the EGL context, while surface
   events arrive on the Android UI thread. This handoff needs one explicit,
   documented queue; it is the most common source of Android GL bugs.

---

## 5. Lifecycle requirements to design for now

- `onPause`: stop rendering before the surface is invalidated; checkpoint saves.
- `surfaceDestroyed`: release swap chain and GL objects; assume they are gone.
- `surfaceCreated`: rebuild the swap chain for the new `ANativeWindow` and its
  possibly different size.
- `onLowMemory`: drop caches, not correctness-critical state.
- Configuration change (rotation, DPI, dark mode): either handle in place or let
  the activity recreate and restore through the C ABI, never through a static
  singleton in the C++ layer.
- Safe areas/insets: UI insets are a Kotlin concern; the renderer receives a
  viewport rectangle.

The engine lifecycle states already specified in
`docs/03_ENGINE_ARCHITECTURE.md` §6 (`Suspended`, `Recovering`, `Failed`) exist
for exactly these transitions and should be implemented in M1 on Windows with the
Android transitions in mind.

---

## 6. Packaging and storage

- Game packages (`*.pkx`) live in app-private storage; there is no arbitrary
  filesystem access on Android, which aligns with the MUTE sandbox goals.
- Large packages: `allowBackup=false` for package caches, `android:extractNativeLibs`
  handled per AGP defaults, and download staging on internal storage with a
  fallback to app-specific external storage.
- ABI splits (`arm64-v8a`, `x86_64`) keep APK size honest; Play requires 64-bit.
- Down-scale path: GLES 3.0 feature detection at runtime, with a clear
  "unsupported device" screen rather than a crash.

---

## 7. What M0 does *not* claim

- No APK was built, no emulator was started, no device was connected.
- Diligent's GLES backend was **not** compiled for Android.
- SDL2 was **not** built for Android.
- The EGL ownership proposal in §4 is a design decision, not a measured result.

Validating §4 with a minimal `surfaceCreated → triangle → surfaceDestroyed →
surfaceCreated` loop is the single highest-value Android task, and it should be
done before any Android UI work is scheduled.
