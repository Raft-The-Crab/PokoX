# MUTE Language Implementation

MUTE is the official gameplay and UI scripting language for PokoX.

## Design

- Lua-like block syntax with `end`
- Dynamic runtime with optional static type checking
- One-based array indexing
- Interpreter first; bytecode VM follows after semantics stabilize
- No native library loading in initial release

## Realms

- **Client**: Local input, camera, UI, presentation, local prediction
- **Server**: Authoritative game rules, player state, validation, inventory
- **Shared**: Non-secret constants, type/schema definitions, deterministic utilities
- **Studio**: Editor automation and tooling APIs

## Security

- Capability-based sandbox
- Client, server, shared, and Studio environments are isolated
- Resource budgets: instruction count, memory, call depth, event subscription, network messages
- No unrestricted filesystem, process, socket, or OS access

## Components

- **lexer**: Tokenization with source locations
- **parser**: AST generation with error recovery
- **ast**: Abstract syntax tree definitions
- **interpreter**: Interpreter with explicit value model and garbage collection
- **runtime**: Host capability bridge and realm management
- **vm**: Bytecode VM (deferred until interpreter is stable)
- **libraries**: Standard libraries (math, string, table helpers, etc.)
- **tools**: Formatters, test runners, source map generators
