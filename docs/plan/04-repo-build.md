# Repository and Build Standards

## Suggested organization (adapt after audit)
```text
poko/
  docs/plan/
  apps/pokox-windows/      # Rust + EGUI application UI
  apps/pokox-android/      # Kotlin native UI
  apps/poko-studio/        # Rust + EGUI editor
  apps/poko-moderation/    # Rust + EGUI moderation tools
  engine/                  # Rust engine core
  native-adapters/         # C/C++ dependency adapters
  include/poko/            # stable C ABI
  mute/                    # C compiler, VM, tests, tooling
  backend/                 # Rust backend
  game-handler/            # Rust game handler/authority
  edge/                    # TypeScript Cloudflare Workers + Rust WASM (Studio APIs)
  scripts/
```
Keep current repository boundaries until audit shows a concrete reason to move files.

## Build tasks
- [ ] Pin compiler/toolchain versions.
- [ ] Document clean Windows build.
- [ ] CI build/tests for every supported target.
- [ ] Reproducible generated artifacts; no machine-specific paths.
- [ ] Debug symbols and useful diagnostics in development.
- [ ] Release artifact metadata: commit, toolchain, dependencies.
- [ ] Dependency/license inventory and vulnerability scan.

## C/C++ rules
- Doxygen public interfaces and non-obvious algorithms.
- `.h` for declarations; `.cpp` for substantial implementation; `.inl` mainly for templates/header-visible code, not by habit.
- Check allocations, lengths, conversions, and external input.
- Small translation units are fine when ownership is clear; avoid meaningless fragmentation.

## Rust rules
- Keep `unsafe` small and justified.
- C ABI uses opaque handles and stable primitive layouts; never expose Rust-native types directly.
- Document who allocates/frees and how handles expire.
- ABI entry points must return errors rather than leak panics across the boundary.
- Format, lint, unit-test, and review every FFI change.

## Definition of done
Acceptance criteria and tests pass; docs/contracts updated; affected targets build; logs do not leak secrets; persistent changes have migration/rollback plans.
