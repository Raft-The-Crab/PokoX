# Mute Tooling and Language Server

## Local-first tooling
- [ ] Document lifecycle and incremental edits.
- [ ] Parse unsaved buffers without requiring full successful compile.
- [ ] Diagnostics with accurate source ranges.
- [ ] Autocomplete keywords, locals, functions, modules, engine instances, properties, methods and events.
- [ ] Hover docs/types, go-to-definition and references where supported.
- [ ] Cancellation and resource limits for huge/malformed files.
- [ ] Test CRLF/LF, Unicode, partial edits, multiple documents and unsaved buffers.

## Shared metadata
Use the same canonical registry as runtime bindings and Studio. Show capability/authority warnings. Respect plugin visibility. No remote AI/API dependency for normal autocomplete.

## Debugger roadmap
- [ ] Bytecode-to-source maps.
- [ ] Breakpoints at safe instruction boundaries.
- [ ] Pause/resume for local/debug sessions.
- [ ] Locals/stack inspection without exposing host pointers.
- [ ] Step over/into/out where VM architecture permits.
- [ ] Remote debugging authenticated, opt-in, and off by default in public sessions.

## Gate
Malformed files never crash the language server; autocomplete follows current metadata and permissions; benchmark latency and memory on a fixed fixture.
