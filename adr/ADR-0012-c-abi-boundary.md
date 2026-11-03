# ADR-0012 — C ABI: opaque handles, versioned structs, no exceptions

**Status:** Accepted (survives into M1; the M0 header is a reduced surface)
**Date:** 2026-11-03

## Context

The Windows UI is Rust/egui and the Android UI is Kotlin/JNI, both consuming the
C++ engine. Rust and Kotlin do not share C++'s object model, allocator or
exception mechanism. Every decision at this boundary is expensive to reverse,
because it is compiled into two other languages.

The failure modes that matter are not "does it link" but:

- a caller built against an older header passing a smaller struct, so the library
  reads past the end of the caller's stack frame;
- a C++ exception unwinding into a Rust or JNI frame, which is undefined
  behaviour and in practice a process abort with an unhelpful stack;
- a caller ignoring a status code and using an uninitialised handle.

## Decision

1. **Opaque handles only.** `poko_engine` is forward-declared in the C header and
   defined only inside the implementation. A caller cannot depend on its layout,
   its size, or its allocator.

2. **Every entry point returns an integer status.** `poko_status` is a fixed
   enumeration; `0` is success, negative values are errors. No function returns a
   pointer that can also be an error.

3. **Every extensible struct carries `struct_size` and `api_version`.**
   - `struct_size` smaller than the library's minimum → rejected, never read.
   - `struct_size` larger than the library's own → accepted; the extra bytes
     belong to a newer caller and are ignored. This is what allows a newer client
     to run against an older engine during a staged update.
   - `api_version` mismatch → distinct error code (`POKO_STATUS_ABI_MISMATCH`),
     never a silent reinterpretation.

4. **Unknown inputs are rejected, not ignored.** Unknown flag bits and non-zero
   reserved fields fail. Silently dropping a flag is how a security or behaviour
   feature disappears between two versions without anyone noticing.

5. **Out-parameters are cleared before any work.** On failure the caller receives
   `NULL`, so a caller that ignores the status code still cannot use garbage.

6. **No exception crosses the boundary.** Each entry point wraps its body in
   `try { ... } catch (...) { return POKO_STATUS_INTERNAL_ERROR; }`. Destructors
   contain their own exceptions. The ABI is therefore safe to call from a
   `noexcept` caller in Rust or from JNI.

7. **Numeric inputs are validated.** `poko_engine_update()` rejects `NaN`,
   infinity and negative time steps, because a `NaN` that reaches simulation
   state is extremely hard to trace back to its source.

8. **The header must compile as C11.** The M0 caller is a `.c` file compiled as C,
   so any leaked C++ construct breaks the build rather than the runtime. Static
   assertions in the caller pin the struct layout (`sizeof` and every `offsetof`),
   turning a silent ABI change into a compile error.

## Consequences

- Rust and Kotlin bindings are mechanical and safe to generate.
- Adding a field to a struct is a compatible change if it is appended and
  `struct_size` is used by the implementation to decide whether to read it.
- Reordering or shrinking a field is a breaking change and requires a new
  `POKO_ABI_VERSION`.
- The implementation always pays a `try/catch` at every entry point. Negligible
  against the cost of a crash on the UI thread.

## Verification performed in M0

`m0/abi/caller/main.c` (C11) exercises: happy path; null config; null
out-parameter; undersized `struct_size`; ABI mismatch; unknown flags; non-zero
reserved field; **oversized struct from a newer caller**; null handles on update
and query; `destroy(NULL)`; `NaN`, `infinity` and negative time steps; unknown
status strings. All 26 checks pass.

## Deferred to M1

Surface creation and recreation, lifecycle notification, input submission, render
invocation, package loading, diagnostics, and explicit thread-affinity rules. The
versioning mechanism above is what those additions will reuse; none of them
requires changing this decision.
