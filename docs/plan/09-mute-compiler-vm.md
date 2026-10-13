# Mute Language, Compiler, and VM

Mute remains implemented in C for now, regardless of engine language.

## Pipeline
Source manager/diagnostics → lexer → parser/AST → resolver → semantic/type checks → capability/authority analysis → lowered IR → optimizers → bytecode emitter → verifier → VM.

Keep stages only when they earn their complexity; benchmark and document simplifications.

## Language design
- [ ] Freeze grammar and error-recovery strategy.
- [ ] Define literals, comments, operators, precedence, blocks, functions, modules, arrays/tables.
- [ ] Add source spans to tokens/AST nodes.
- [ ] Define scope, imports, functions, return behavior, type/conversion rules.
- [ ] Keep simple games easy; classes/advanced abstractions are separate milestones.
- [ ] Define deterministic value serialization across engine boundaries.

## Bytecode/VM
- [ ] Version bytecode independently from language version.
- [ ] Choose stack/register model based on simplicity and benchmark.
- [ ] Verify bytecode before execution.
- [ ] Instruction budget, memory/object limit, recursion/stack limit, cancellation.
- [ ] Capability domains: `CLIENT`, `SERVER`, `SHARED`, `STUDIO`, `PLUGIN`.
- [ ] Safe engine calls from metadata; no raw native pointers.
- [ ] Useful source-located errors and stack traces.
- [ ] Fuzz invalid indexes, jumps, constants, oversized payloads, malformed files.

## Optimization
- [ ] Establish compile/runtime benchmark fixtures first.
- [ ] Add constant folding/dead-code elimination with differential tests.
- [ ] Profile dispatch, allocations, calls and table access.
- [ ] Keep optimized/unoptimized semantics equivalent.
- [ ] “1,500 lines under 3 seconds” is a benchmark target to measure on named hardware, not a promise.

## Gate
A script operates on a sample world; unsafe capabilities/over-budget execution are rejected; malformed source/bytecode produces diagnostics without crashing.
