# Android Implementation

Android 11+ native implementation for PokoX Client and Poko Studio.

## Structure

- **app**: Kotlin application shell, activities, and Android UI
- **native**: C++ engine integration via JNI
- **res**: Android resources (layouts, strings, drawables)
- **assets**: Bundled assets and resources

## Architecture

- Kotlin app owns Android lifecycle (activity, permissions, navigation)
- C++ engine owns game runtime and renderer
- JNI bridge calls C ABI (same interface as Windows Rust layer)
- OpenGL ES backend via Diligent Core
- SDL2 for input and touch events

## Key Considerations

- Handle pause/resume, surface created/destroyed, low-memory events
- Safe areas/insets for different screen sizes
- Rotation and configuration changes
- Audio focus and back navigation
- Touch gestures and controller support
- Storage and background lifecycle
- Performance budgets on representative devices

## Build System

- Android Studio + NDK
- CMake cross-compilation for supported ABIs
- Separate toolchain from MSYS2
- Build SDL2 for Android
- Verify GLES device creation and surface recreation
