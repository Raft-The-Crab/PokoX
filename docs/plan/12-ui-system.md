# UI System

## Two-tier UI architecture
1. **Windows Application UI (Studio, moderation, PokoX menus):** Rust + EGUI
   - Immediate-mode GUI well-suited for internal tools and editors
   - Easy integration and rapid iteration
   - Accepts immediate-mode trade-offs (1-2ms overhead, non-native look)
   - Leverage EGUI's custom rendering callbacks for 3D viewports in Studio

2. **Android Application UI (PokoX menus):** Native Kotlin
   - Native feel and reliable platform integration
   - Mature ecosystem and tooling
   - Better accessibility and text editing than EGUI on mobile
   - Requires JNI bridge to Rust engine

3. **Game UI (in-experience UI):** Custom Rust UI engine
   - Tight integration with engine rendering pipeline
   - Optimized for performance and game-specific features
   - Retained-mode or hybrid approach for complex layouts
   - Full control over behavior, input, and accessibility
   - Avoids EGUI limitations for consumer-facing experiences

## EGUI for Windows application UI
- Use EGUI for Studio editor shell, moderation tools, and PokoX application menus on Windows
- Accept breaking changes and API evolution (track EGUI version)
- Measure overhead on target devices; optimize hot paths if needed

## Kotlin for Android application UI
- Use native Kotlin for PokoX menus and discovery on Android
- Define JNI boundary to Rust engine with stable C ABI
- Handle Android lifecycle, permissions, and platform-specific behavior
- Test engine integration via JNI bridge thoroughly

## Game UI goals
Provide frames/containers, text, images, buttons, scrolling, lists/grids, input fields, visibility, anchoring, scale/offset, z-order and focus. Keep ordinary layouts simple; advanced layout primitives should solve demonstrated needs.

## Runtime tasks
- [ ] UI tree and lifetime.
- [ ] Layout pass and invalidation.
- [ ] Coordinate units, scale, clipping, hit-testing and z-order.
- [ ] Event routing and input capture.
- [ ] Text shaping/font fallback.
- [ ] Render batching and profiling.
- [ ] Mouse, keyboard and touch behavior.

## Studio tasks
- [ ] UI canvas/preview and hierarchy.
- [ ] Inspector, drag/drop, alignment guides and responsive previews.
- [ ] Property panel and code-like property edits stay synchronized.
- [ ] Undo/redo and copy/paste.
- [ ] Bind events to Mute through metadata.

## Gate
Creator can make a responsive menu with text/images/buttons, bind actions, edit properties and preview desktop/mobile dimensions.
