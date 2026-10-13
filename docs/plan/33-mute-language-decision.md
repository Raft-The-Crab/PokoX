# Mute Language: C vs Rust Decision

## Context
Mute is Poko's custom scripting language for game logic. Originally planned as C implementation. Question: Should Mute be implemented in C or Rust?

## Current Direction
**Decision: Keep Mute in C with C ABI for Rust integration**

## Evaluated Options

### Option 1: Mute in C (Selected)

**Pros:**
- **Simpler compilation**: C compiler is simpler, faster compile times
- **Predictable performance**: No hidden allocations, easier to reason about
- **Mature ecosystem**: Battle-tested for language implementations
- **Smaller binary**: C binaries are typically smaller than Rust
- **Simpler FFI**: C ABI is the lingua franca, easier to integrate with anything
- **Existing investment**: Original plan was C, may have existing code/prototypes
- **Easier to port**: C is easier to port to constrained platforms if needed
- **Tooling maturity**: Debuggers, profilers, static analyzers are more mature for C

**Cons:**
- **Memory safety**: Manual memory management, risk of bugs
- **Different from engine**: Engine is Rust, Mute is C = two languages
- **Less ergonomic**: More boilerplate, less developer-friendly
- **Standard library**: C standard library is smaller, may need to implement more
- **Modern features**: No borrow checker, no pattern matching, etc.

### Option 2: Mute in Rust

**Pros:**
- **Single language stack**: Engine + Mute both in Rust
- **Memory safety**: Borrow checker prevents memory bugs
- **Better ergonomics**: More expressive, less boilerplate
- **Modern tooling**: Cargo, rustfmt, clippy
- **Rich ecosystem**: More libraries available
- **Easier maintenance**: Fewer memory bugs, easier to refactor
- **Pattern matching**: Excellent for AST processing
- **Zero-cost abstractions**: Can write high-level code with low-level performance

**Cons:**
- **Compilation time**: Rust compile times are slower
- **Binary size**: Rust binaries are typically larger
- **Learning curve**: Team may not know Rust as well as C
- **FFI complexity**: Calling Rust from C is harder than C from Rust
- **Async complexity**: If Mute needs async, Rust async is complex
- **Monomorphization**: Code bloat from generics
- **Less control**: Borrow checker can be restrictive for language implementations

## Decision Analysis

### The Real Trade-off

**C is better for language implementation** because:
- Language VMs need fine-grained control over memory
- Predictable performance is critical for game scripting
- Simpler compilation = faster iteration
- C ABI makes integration trivial with Rust engine
- Smaller binary size matters for distribution

**Rust is better for general application development** because:
- Memory safety prevents bugs
- Better ergonomics = faster development
- Modern tooling = better developer experience
- But for a language VM, these benefits are less relevant

### Specific Considerations for Mute

#### Memory Management
A language VM needs custom memory management anyway:
- Arena allocators for bytecode
- Object pools for VM objects
- Custom garbage collection or reference counting
- **Conclusion**: Manual memory management in C is not a disadvantage here

#### Performance
Mute needs to be fast:
- Direct memory access
- No hidden allocations
- Predictable performance
- **Conclusion**: C gives more control and predictability

#### Integration
Mute needs to integrate with Rust engine:
- C ABI is the standard for FFI
- Rust can call C easily via `extern "C"`
- Rust calling Rust requires stable ABI (Rust doesn't have stable ABI yet)
- **Conclusion**: C → Rust integration is simpler and more stable

#### Team Expertise
- If team knows C better than Rust: C is better
- If team knows Rust better than C: Rust is better
- **Assumption**: Language implementation typically requires C expertise anyway

#### Codebase Size
- Mute is a focused language implementation (not a huge codebase)
- Maintenance burden of C is manageable for this scope
- **Conclusion**: Language size doesn't justify Rust's benefits

## Decision: Keep Mute in C

### Rationale

1. **C is the right tool for language VMs**
   - LLVM, Python, Lua, JavaScript engines are all C/C++
   - Fine-grained memory control is essential
   - Predictable performance is critical

2. **Integration is simpler**
   - C ABI is standard
   - Rust can call C trivially
   - No stable ABI concerns

3. **Performance is more predictable**
   - No hidden allocations
   - No borrow checker surprises
   - Easier to optimize hot paths

4. **Compilation is faster**
   - Language implementation requires frequent iteration
   - C compiles faster than Rust
   - Faster feedback loop

5. **Binary size matters**
   - Mute VM may be distributed with games
   - Smaller binary = faster download
   - Less memory overhead

6. **Domain-specific expertise**
   - Language implementation requires C expertise anyway
   - Team likely needs C knowledge for other parts (physics adapters)
   - Two languages (C + Rust) is acceptable

### Implementation Strategy

#### C ABI for Rust Integration
```c
// mute.h
#ifdef __cplusplus
extern "C" {
#endif

typedef struct MuteVM MuteVM;
typedef struct MuteScript MuteScript;

// VM lifecycle
MuteVM* mute_vm_create(void);
void mute_vm_destroy(MuteVM* vm);

// Script lifecycle
MuteScript* mute_script_compile(const char* source, char** error);
void mute_script_destroy(MuteScript* script);

// Execution
int mute_script_execute(MuteVM* vm, MuteScript* script, char** error);

// Engine bindings
void mute_register_function(MuteVM* vm, const char* name, void* func);
void mute_set_userdata(MuteVM* vm, void* data);
void* mute_get_userdata(MuteVM* vm);

#ifdef __cplusplus
}
#endif
```

```rust
// Rust integration
use std::ffi::{CString, CStr};

#[repr(C)]
pub struct MuteVM {
    _private: [u8; 0],
}

extern "C" {
    fn mute_vm_create() -> *mut MuteVM;
    fn mute_vm_destroy(vm: *mut MuteVM);
    fn mute_script_compile(source: *const i8, error: *mut *mut i8) -> *mut MuteScript;
    fn mute_script_execute(vm: *mut MuteVM, script: *mut MuteScript, error: *mut *mut i8) -> i32;
}

pub struct Mute {
    vm: *mut MuteVM,
}

impl Mute {
    pub fn new() -> Self {
        unsafe {
            let vm = mute_vm_create();
            Mute { vm }
        }
    }

    pub fn compile(&self, source: &str) -> Result<MuteScript, String> {
        unsafe {
            let c_source = CString::new(source).unwrap();
            let mut error: *mut i8 = std::ptr::null_mut();
            let script = mute_script_compile(c_source.as_ptr(), &mut error);

            if script.is_null() {
                let error_msg = CStr::from_ptr(error).to_string_lossy().into_owned();
                libc::free(error as *mut libc::c_void);
                Err(error_msg)
            } else {
                Ok(MuteScript { script })
            }
        }
    }
}

impl Drop for Mute {
    fn drop(&mut self) {
        unsafe {
            mute_vm_destroy(self.vm);
        }
    }
}
```

#### Documentation
- Use Doxygen for C API documentation
- Generate Rust docs from C API
- Keep C and Rust documentation in sync

#### Testing
- Unit tests in C for VM internals
- Integration tests in Rust for C ABI
- Fuzz testing for both C and Rust layers

### When to Reconsider

**Switch to Rust if:**
- Team lacks C expertise and Rust expertise is strong
- Memory bugs in C become frequent
- Compile times become bottleneck
- Binary size is not a concern
- Stable Rust ABI becomes available

**Keep C if:**
- Team has C expertise
- Performance requirements are strict
- Binary size matters
- Fast compilation is important
- Integration simplicity is valued

## Mute Architecture (C Implementation)

### Components
- **Lexer**: Tokenization
- **Parser**: AST generation
- **Resolver**: Symbol resolution
- **Compiler**: Bytecode generation
- **Verifier**: Bytecode validation
- **VM**: Execution engine
- **Runtime**: Built-in functions and engine bindings

### Memory Management
- Arena allocator for AST and bytecode
- Object pool for VM objects
- Reference counting for script objects
- Explicit cleanup for VM lifecycle

### Safety Measures
- Bounds checking on array access
- Null pointer checks
- Input validation
- Fuzz testing for parser and verifier
- Memory sanitizers (ASan, Valgrind)

### Performance Optimizations
- Bytecode interpreter (not JIT initially)
- Inline caching for hot paths
- SIMD for vector operations (if needed)
- Profile-guided optimization

## Integration with Rust Engine

### Architecture
```
Rust Engine → C ABI → Mute VM (C) → Engine Bindings → Rust Engine
```

### Data Flow
1. Rust engine loads Mute script
2. Rust calls C `mute_script_compile()`
3. C compiles to bytecode
4. Rust calls C `mute_script_execute()`
5. Mute VM executes bytecode
6. Mute calls Rust engine functions via registered bindings
7. Rust returns results to Mute
8. Mute returns results to Rust engine

### Error Handling
- C returns error codes and error strings
- Rust converts to Result types
- Stack traces from C (if possible)
- Source location tracking in bytecode

## Alternatives Considered

### Option 3: Hybrid Approach
- Core VM in C
- Compiler/optimizer in Rust
- Best of both worlds?
- **Rejected**: Too complex, not worth the complexity

### Option 4: WebAssembly VM
- Compile Mute to WebAssembly
- Run in WASM runtime
- **Rejected**: Adds complexity, performance overhead

## Implementation Order

1. **Phase 1**: C lexer and parser
2. **Phase 2**: C bytecode compiler
3. **Phase 3**: C bytecode verifier
4. **Phase 4**: C VM interpreter
5. **Phase 5**: C engine bindings skeleton
6. **Phase 6**: Rust FFI integration
7. **Phase 7**: End-to-end testing
8. **Phase 8**: Performance optimization
9. **Phase 9**: Documentation
10. **Phase 10**: Tooling (language server, debugger)

## Revisit Triggers
- Memory bugs become frequent
- Performance is inadequate
- Compile times become bottleneck
- Team expertise shifts to Rust
- New requirements require Rust features

## References
- Lua implementation (C)
- Python implementation (C)
- C FFI with Rust
- Doxygen documentation
- Memory sanitizers
