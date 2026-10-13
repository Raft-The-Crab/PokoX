# UI Framework Research and Decision

## Context
Poko needs two distinct UI systems:
1. **Application UI**: Studio editor, moderation tools, PokoX menus/discovery
2. **Game UI**: Developer-created UI objects rendered inside experiences

## Evaluated Options

### Option 1: C# + Avalonia (Windows) + Kotlin (Android)
**Pros:**
- Mature, native-like frameworks
- Strong ecosystem and tooling
- Platform-specific idioms

**Cons:**
- Different languages per platform = duplicate logic
- JNI bridge complexity for Android
- Engine integration requires native interop
- Not aligned with Rust engine direction

### Option 2: Rust + EGUI for everything
**Pros:**
- Single language across platforms
- Cross-platform via eframe (Windows, Linux, Mac, Android, Web)
- Easy to integrate into game engine
- Immediate-mode simplicity
- Active development

**Cons:**
- Immediate mode = rebuilds UI every frame (1-2ms overhead)
- Breaking changes between versions
- Experimental Android support (eframe or egui-android)
- Web/mobile limitations: poor accessibility, text editing issues
- Not native-looking
- Better suited for tools than consumer apps

### Option 3: Custom Rust UI for everything
**Pros:**
- Full control over rendering and behavior
- Optimized specifically for Poko's needs
- Tight engine integration
- No external dependency churn

**Cons:**
- Significant development effort
- Must implement layouts, input, accessibility, etc.
- Testing and maintenance burden
- Risk of reinventing wheels poorly

### Option 4: Rust + EGUI (Windows), Native Kotlin (Android), custom Rust UI for game UI
**Pros:**
- EGUI excels at internal tools and editors on Windows
- Kotlin provides native feel and reliability on Android
- Custom game UI optimized for performance and game features
- Leverages platform strengths where they fit
- Avoids EGUI limitations on mobile (accessibility, text editing)

**Cons:**
- Three UI systems to maintain (EGUI, Kotlin, custom Rust)
- EGUI dependency management (breaking changes)
- JNI bridge complexity for Android engine integration
- Different languages across platforms

## Decision: Option 4

**Windows Application UI (Studio, moderation, PokoX menus): Rust + EGUI**
- EGUI is well-suited for internal tools and editors
- Immediate-mode simplicity matches rapid iteration needs
- Custom rendering callbacks support 3D viewports in Studio
- Accepts EGUI trade-offs for tools (overhead, non-native look)

**Android Application UI (PokoX menus): Native Kotlin**
- Native feel and reliable platform integration
- Mature ecosystem and tooling
- Better accessibility and text editing than EGUI on mobile
- Requires JNI bridge to Rust engine

**Game UI (in-experience): Custom Rust UI engine**
- Needs tight integration with engine rendering pipeline
- Performance-critical in game loop
- Requires game-specific features (layout virtualization, complex animations)
- Avoids EGUI limitations for consumer-facing experiences

## EGUI Research Findings (Windows)

### Capabilities
- Cross-platform: Windows, Linux, Mac, Web via eframe
- Widgets: labels, buttons, checkboxes, sliders, text editing, color pickers, images
- Layouts: horizontal, vertical, columns, wrapping, panels, scrolling
- Text editing: multiline, copy/paste, undo, emoji support
- Windows: move, resize, minimize, close
- Rendering: anti-aliased lines, circles, text, polygons
- Custom rendering callbacks for 3D viewports
- Accessibility via AccessKit

### Limitations
- **Immediate mode overhead**: 1-2ms per frame to rebuild layout
- **Breaking changes**: API still in flux, interfaces change between versions
- **Not native-looking**: Custom widget appearance
- Better suited for tools than consumer apps

## Kotlin + JNI Research Findings (Android)

### Capabilities
- Native Android UI with Jetpack Compose or traditional XML layouts
- Mature ecosystem and tooling
- Reliable platform integration (lifecycle, permissions, safe areas)
- Better accessibility support than EGUI on mobile
- Excellent text editing and input handling
- Native feel and performance

### JNI Bridge Considerations
- JNI provides C-compatible interface to Rust engine
- Need stable C ABI for Rust → Kotlin communication
- Complexity in passing complex data structures across boundary
- Performance overhead for frequent calls (measure and optimize)
- Memory management across language boundary (who allocates/frees)

## Implementation Plan

### Phase 1 Experiment
- Build minimal EGUI + eframe application for Windows
- Build minimal Kotlin + JNI bridge to Rust engine for Android
- Measure EGUI overhead on representative Windows hardware
- Test JNI bridge performance and complexity
- Test EGUI custom rendering callbacks for 3D viewport
- Document EGUI version and feature constraints
- Document JNI boundary and data passing patterns

### Phase 2 Application UI
- Implement Studio shell with EGUI (Windows)
- Implement moderation tools with EGUI (Windows)
- Implement PokoX menus/discovery with EGUI (Windows)
- Implement PokoX menus/discovery with Kotlin (Android)
- Track EGUI version changes and breaking updates
- Validate JNI bridge with real engine calls

### Phase 3 Game UI
- Design custom Rust UI architecture (retained-mode or hybrid)
- Implement core widgets: frames, text, images, buttons, input fields
- Implement layout system: anchoring, scrolling, lists/grids
- Integrate with engine rendering pipeline
- Add accessibility support

## Revisit Triggers
- EGUI overhead exceeds acceptable thresholds on Windows devices
- EGUI breaking changes cause excessive maintenance burden
- JNI bridge proves too complex or slow for Android
- Custom game UI development proves infeasible within resources

## References
- EGUI: https://github.com/emilk/egui
- eframe: https://github.com/emilk/egui/tree/main/crates/eframe
- egui-android: https://github.com/sagebind/egui-android
- Rust GUI Architecture Models: https://rust.codeguides.io/desktop-gui/rust-gui-architecture-models/
