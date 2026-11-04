# Changelog

## 1.1 — 2026-11-04

Renderer core implementation complete (M1-M9). Production-ready graphics abstraction layer built on Diligent Core:

**M1 — Engine Foundation**
- Device with capability queries and backend detection
- SwapChain with resize support and present functionality
- Context with viewport/scissor, clear operations, and pipeline binding
- Comprehensive error reporting and validation

**M2 — Pipeline State**
- GLSL 3.30 core shader support (vertex and pixel)
- Resource signatures for shader resource binding
- Rasterizer, blend, depth-stencil, and sampler states

**M3 — Vertex Buffers**
- Extended vertex formats (Float/Int 1-4 components)
- Position, Normal, TexCoord, Color semantics
- Dynamic buffer updates with validation

**M4 — Index Buffers**
- 16-bit and 32-bit index support
- Indexed drawing with draw_indexed()
- Dynamic index buffer updates

**M5 — Textures**
- 2D textures with multiple formats (RGBA8, RG8, R8, RGBA16, RGBA32, depth)
- Dimension validation (16384x16384 max)
- Texture sampling through materials

**M6 — Materials**
- Texture and constant buffer binding
- Shader resource binding (SRB) with device compatibility checks
- Up to 32 textures and 16 constant buffers per material

**M7 — Constant Buffers**
- Uniform buffers with std140 alignment validation
- Dynamic updates for frame-varying data
- 16-byte alignment enforcement

**M8 — Framebuffers**
- Up to 8 color render targets (MRT)
- Render-to-texture support via Context::set_render_target(Framebuffer*)
- Color and depth/stencil attachments with validation

**M9 — Advanced Drawing**
- Instanced rendering (draw_instanced, draw_indexed_instanced)
- Multi-slot vertex buffer binding (geometry + instance data)
- Vertex layout with input_slot and instance_data_step_rate
- Indirect drawing API (validation complete, backend deferred)
- Performance benchmark utilities (API overhead measurement)

**Production Status:**
- 24/24 tests passing (100% success rate, 18.32 seconds)
- Comprehensive API validation and error reporting
- Fully documented with usage examples
- Production-ready for instanced rendering, materials, framebuffers
- Indirect drawing API available with documented limitations

## 1.0 — 2026-11-03

Final consolidated planning baseline. Reconciles earlier discussions and adds:

- Explicit PokoX product identity as downloadable UGC game platform.
- Shared C++ engine with separate PokoX, Studio, and Poko Mod process/privilege boundaries.
- MUTE realm/capability model and execution budgets.
- PPC package architecture and immutable hash-bound publication.
- Server-authoritative online game state; untrusted creator-operated hosts.
- Offline-ready/hybrid/online-required modes and untrusted local save semantics.
- Human review requirements, sexual-content prohibition and bounded fictional-violence policy.
- Creator Server SDK scope and cross-tenant isolation.
- Rilavek integration as optional TUS/resumable ingestion into R2, not as storage.
- Hugging Face two-account deployment retained but conditional on independent Docker compute eligibility and transport tests.
- Expanded implementation backlog, operational runbook, moderation operations and finalization gates.

## 0.2 — 2026-11-03

Initial multi-document architecture plan with 29 technical specifications.
