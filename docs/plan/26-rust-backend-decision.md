# Rust Backend and Game Handler Decision

## Context
Original plan used Go for backend and C++ for Game Authority WebSocket service. New proposal shifts to Rust for backend and Rust for Game Handler/Authority, both running on Huggingface CPU. Cloudflare Workers is used for Studio/basic APIs with 100k req/day limit and fallback to Huggingface CPU.

## Evaluated Options

### Option 1: Go Backend + C++ WebSocket Game Authority (Original)
**Pros:**
- Go is battle-tested for backends, mature ecosystem
- Simple concurrency model (goroutines)
- C++ WebSocket service provides dedicated game authority infrastructure
- Clear separation of concerns

**Cons:**
- Three languages (Go, C++, Rust engine) = complexity
- C++ WebSocket service requires infrastructure management
- Different language from engine

### Option 2: Go Backend + Rust Game Handler/Authority
**Pros:**
- Go backend remains simple and mature
- Rust game handler shares language with engine
- Two languages (Go, Rust)

**Cons:**
- Still two languages
- Game handler logic in Rust, backend in Go

### Option 3: Rust Backend + Rust Game Handler/Authority (Selected)
**Pros:**
- Single language stack (Rust) for engine, backend, game handler
- Shared code between backend and game handler where appropriate
- Memory safety across entire stack
- Better performance characteristics
- Rust async runtime (Tokio) is production-grade

**Cons:**
- Rust backend ecosystem smaller than Go
- Tokio async runtime more complex than Go goroutines
- Less battle-tested for large-scale backends
- Steeper learning curve for backend patterns

## Decision: Option 3 - Rust Backend + Rust Game Handler/Authority

**Rationale:**
1. **Language consistency**: Single Rust stack reduces context switching and improves code sharing
2. **Performance**: Rust's performance characteristics align with game platform needs
3. **Memory safety**: Critical for a platform handling user data and currency
4. **Future-proof**: Rust ecosystem is growing rapidly; early adoption has long-term benefits
5. **Cloudflare Workers**: Rust compiles to WASM for Studio/basic APIs on edge

## Cloudflare Workers for Studio/basic APIs

### Architecture
- TypeScript + Rust WASM on Cloudflare Workers
- Used for Poko Studio and basic platform APIs (asset metadata, discovery feeds, etc.)
- 100,000 requests/day limit on free tier
- Automatic fallback to Huggingface CPU when limit exceeded
- Provides edge caching and low latency for Studio workflows

### Advantages
- **Edge deployment**: Low latency for Studio users worldwide
- **Auto-scaling**: Handles load spikes automatically
- **Cost-effective**: Free tier covers Studio/basic API usage
- **Caching**: Edge caching for static metadata
- **Fallback**: Seamlessly routes to Huggingface CPU when limit exceeded

### Constraints and Mitigations
| Constraint | Impact | Mitigation |
|---|---|---|
| 100k req/day limit | High-volume game traffic can't use Workers | Only use for Studio/basic APIs, fallback to Huggingface |
| CPU time limit | Complex processing may timeout | Keep Workers logic simple, heavy processing on Huggingface |
| Cold starts | First request latency | Acceptable for Studio workflows |
| WebSocket support | Not suitable for real-time game traffic | Game traffic goes to Huggingface CPU |

## Rust Backend on Huggingface CPU

### Architecture
- Main backend instance (separate from game handler)
- 2 VCPUs, 16GB RAM
- Handles durable APIs, session management, economy, moderation
- Also serves as fallback for Cloudflare Workers when 100k req/day limit exceeded
- Connects to PostgreSQL, Valkey, R2

### Advantages
- **Consistent with engine**: Same language, shared types where appropriate
- **Performance**: Better resource efficiency than Go for some workloads
- **Memory safety**: Critical for financial transactions and moderation
- **Workers fallback**: Handles traffic when Cloudflare Workers limit exceeded

### Concerns and Mitigations
| Concern | Impact | Mitigation |
|---|---|---|
| Smaller ecosystem than Go | Fewer battle-tested libraries | Use proven crates (axum, sqlx, tokio), measure carefully |
| Tokio complexity | Harder to reason about async | Start simple, add complexity only when needed |
| Less backend experience | Unknown failure modes | Extensive testing, monitoring, gradual rollout |
| Compilation time | Slower iteration | Use cargo-watch, separate binary targets |

## Rust Game Handler/Authority on Huggingface CPU

### Architecture
- Runs on separate Huggingface CPU instance from main backend
- 2 VCPUs, 16GB RAM
- Handles game session coordination and request validation
- Validates game requests (ownership, permissions, invariants)
- Does not simulate every frame (request-level validation only)
- Connects to PostgreSQL, Valkey for authoritative state

### Advantages
- **Dedicated compute**: Predictable performance, no Worker limits
- **Full WebSocket support**: Native WebSocket for real-time game traffic
- **Consistent with engine**: Same language, shared types where appropriate
- **Memory safety**: Critical for game validation logic
- **No cold starts**: Always available for game sessions

### Concerns and Mitigations
| Concern | Impact | Mitigation |
|---|---|---|
| Single instance = SPOF | If instance fails, game sessions disrupted | Add health checks, auto-restart, eventual redundancy |
| Capacity limits | 2 VCPUs may not handle high CCU | Load test to validate, add instances if needed |
| No auto-scaling | Manual scaling required | Monitor capacity, plan scaling triggers |

## Hosting Provider Strategy

### Huggingface CPU (2 instances)
- **Main backend**: Auth, session, publishing, economy, moderation
- **Game handler**: Session coordination, match-making, state persistence
- **Resources**: 2 VCPUs, 16GB RAM per instance
- **Advantage**: Dedicated compute, predictable performance
- **Risk**: Single point of failure per instance (need redundancy plan)

### Cloudflare Workers + Rust WASM
- **Game Authority**: Request validation at edge
- **Edge functions**: Cached API responses, simple transformations
- **Advantage**: Global edge deployment, auto-scaling
- **Risk**: CPU/memory/time limits, cold starts

### Database and Storage
- **Aiven PostgreSQL (3GB)**: Durable source of truth
- **Aiven Valkey (3GB)**: Cache, sessions, rate limits
- **Layerbase (5GB)**: Additional PostgreSQL or Redis as needed
- **Cloudflare R2**: Asset and package storage
- **Cloudflare D1**: Edge-local SQLite for specific use cases

### Firebase Spark Plan
- **Authentication**: Optional auth provider
- **Real-time database**: Chat or edge data (if needed)
- **Hosting**: Static assets (optional)

## Rust Backend vs Go: Honest Assessment

### Go Advantages Over Rust
- **Simpler concurrency**: Goroutines are easier than Tokio
- **Mature ecosystem**: More battle-tested backend libraries
- **Faster compilation**: Shorter iteration cycles
- **Easier hiring**: More Go backend developers
- **Standard library**: Comprehensive, well-documented

### Rust Advantages Over Go
- **Memory safety**: No data races, no null pointer dereferences
- **WASM compatibility**: Native compilation to WebAssembly
- **Performance**: Zero-cost abstractions, better CPU efficiency
- **Type system**: More expressive, catches more errors at compile time
- **Single stack**: Same language as engine, game authority

### The Real Trade-off
Go is the safer, pragmatic choice for a backend today. Rust is the ambitious, future-proof choice that aligns with the rest of the stack.

**If Poko were a traditional web app**: Go would be the clear winner.
**Since Poko is a game platform with Rust engine**: Rust backend makes sense for consistency.

## Implementation Plan

### Phase 1: Rust Backend Experiment
- Build minimal Rust backend service on Huggingface CPU
- Implement basic auth/session API
- Test PostgreSQL and Valkey integration
- Measure compilation time, memory usage, request latency
- Compare with equivalent Go prototype (optional)

### Phase 2: Cloudflare Workers Experiment
- Build minimal TypeScript + Rust WASM for Studio APIs
- Deploy to Cloudflare Workers
- Test edge caching and latency
- Implement fallback logic to Huggingface CPU
- Monitor 100k req/day limit and trigger fallback

### Phase 3: Rust Game Handler/Authority Experiment
- Build minimal Rust game handler/authority on Huggingface CPU
- Test WebSocket connections and request validation
- Measure CPU, memory, and latency under load
- Validate session coordination logic

### Phase 4: Go/No-Go Decision
- If Rust backend proves too complex: fallback to Go backend
- If Cloudflare Workers fallback causes issues: route all traffic to Huggingface
- Document performance measurements and decision rationale

### Phase 5: Full Implementation
- Implement remaining backend modules in Rust
- Implement game handler/authority on Huggingface CPU
- Implement Studio/basic APIs on Cloudflare Workers with fallback
- Add monitoring, metrics, and alerting
- Run load tests with target CCU

## Revisit Triggers
- Rust backend complexity exceeds Go by significant margin
- Cloudflare Workers fallback causes unacceptable latency or errors
- Game handler/authority on Huggingface CPU doesn't meet performance targets
- Compilation time becomes bottleneck for development
- Performance measurements show Rust worse than Go
- Team productivity suffers due to Rust learning curve

## References
- Rust async book: https://rust-lang.github.io/async-book/
- Axum web framework: https://github.com/tokio-rs/axum
- Cloudflare Workers TypeScript: https://developers.cloudflare.com/workers/
- Cloudflare Workers Rust WASM: https://developers.cloudflare.com/workers/languages/rust/
- Tokio runtime: https://tokio.rs/
