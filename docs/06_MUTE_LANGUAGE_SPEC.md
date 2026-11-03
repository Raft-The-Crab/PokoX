# 06 — MUTE Language Specification v0.1

**Status:** Baseline language direction; grammar details remain implementation work.  
**Style:** Lua-like blocks with `end`; approachable syntax; feature-rich language with restricted host capabilities.

## 1. Goals

MUTE is the official gameplay and UI scripting language for PokoX. It should be familiar to Lua/Luau users while improving diagnostics, discoverability, tooling, and beginner experience. The language should be powerful in expression but not powerful in operating-system privileges.

## 2. v0.1 decisions

- Lua-style block syntax using `end`.
- Dynamic runtime with optional static type checking.
- One-based array indexing.
- Interpreter first; bytecode VM follows after semantics and tests stabilize.
- No native libraries/extensions in initial release.
- Schema-defined Bitstream is the PokoX network serialization model.
- Client, server, shared, and Studio/editor execution contexts are distinct.

## 3. Proposed language feature set

### Core syntax

- `local`, constants/immutable bindings, assignment.
- `if` / `elseif` / `else` / `end`.
- `while` / `for` / `repeat` / `until`.
- Functions, closures, anonymous functions and callbacks.
- Tables/maps/records and arrays.
- Modules with explicit imports/exports.
- Structured errors and protected calls.
- Coroutines/tasks with cooperative yield.
- Events/signals.

### Type features

- Dynamic values at runtime.
- Optional annotations and inference.
- Primitive types: nil, boolean, number, integer (where supported), string.
- Arrays, maps, records, unions, optional fields, function types.
- Interfaces/structural contracts for APIs.
- Generics only if the v0.1 parser/type checker can support them without excessive complexity; otherwise defer to v0.2.
- Static checking is advisory/build validation; runtime boundaries still validate values.

### Standard libraries

Math/vector/quaternion helpers, string utilities, table/collection helpers, deterministic time/random APIs where appropriate, safe formatting, basic serialization, and test assertions. No standard `io`, `os`, arbitrary `package.loadlib`, unrestricted `debug`, raw process APIs, or arbitrary socket/file APIs.

## 4. Example syntax

```lua
local speed: number = 8
local isReady = true

function calculateReward(level)
    if level <= 0 then
        return 0
    elseif level < 10 then
        return 25
    else
        return 100
    end
end

local inventory = {
    Items = {},
    Capacity = 20
}
```

The final language spec must define truthiness, equality, numeric coercion, integer overflow policy, iteration order, table key rules, closure capture, error propagation, task scheduling, cancellation, and module initialization order. These are not safe to leave as undocumented Lua assumptions.

## 5. Realms

- **Client:** local input, camera, UI, presentation, local prediction, audio/visual effects.
- **Server:** authoritative game-specific rules, player state, validation, game inventory, NPC state, session events.
- **Shared:** non-secret constants, type/schema definitions, deterministic utility code. Shared source is not secret and may be distributed to clients.
- **Studio:** editor automation and tooling APIs; no unrestricted host process access by default.

The host chooses the realm. A script cannot change its own realm or elevate itself by changing a variable or package field.

## 6. API exposure

MUTE accesses engine/platform functionality through host-provided capability objects. Do not expose raw C++ pointers, arbitrary function pointers, Diligent objects, OS handles, or direct database connections. Every API argument and return value crossing the host boundary must be validated and budgeted.

## 7. Networking

MUTE networking APIs produce typed messages defined by registered schemas. They do not expose raw arbitrary socket access. Client-to-server requests are requests, not authoritative claims. Server-to-client messages must not contain private server secrets.

## 8. Execution and compilation roadmap

1. Lexer with source locations.
2. Parser and AST.
3. Semantic checks and diagnostics.
4. Interpreter with explicit value model and garbage/memory policy.
5. Host API bindings by realm/capability.
6. Static checker.
7. Test runner and formatter.
8. Bytecode compiler/VM only after interpreter semantics are stable.
9. Source maps and debug metadata for Studio.

## 9. Versioning

MUTE language version, standard-library API version, bytecode format version, and engine API version must be separate. A script package declares the versions it targets. Runtime must reject unsupported bytecode or clearly report compatibility failure; never attempt to execute unknown bytecode formats.

## 10. Acceptance tests

- Lexer/parser tests for valid and malformed source.
- Golden tests for operator precedence and control flow.
- One-based array behavior verified.
- Optional type checker catches declared type errors.
- Script realm APIs differ as intended.
- Runtime errors include file/line/stack trace.
- Infinite loop is interruptible by instruction/time budget.
- Memory exhaustion is bounded.
- Client script cannot call server-only API.

See `07_MUTE_SANDBOX_SECURITY.md` for the security contract. See `08_NETWORK_BITSTREAM_PROTOCOL.md` for network schemas.
