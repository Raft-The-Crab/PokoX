# System Architecture

```text
PokoX / Poko Studio / Poko Moderation
                 |
       Public API + Identity
                 |
        Rust backend (Huggingface CPU)
       /       |        \
 PostgreSQL  Valkey     Cloudflare R2
 source of   session/  asset/package
 truth       transient bytes
                 |
      Cloudflare Workers + Rust WASM
       Studio/basic APIs (100k req/day)
       fallback to Huggingface if exceeded

PokoX -- session protocol --> Rust Game Handler/Authority (Huggingface CPU)
                             validates requests; does not simulate every frame

Engine runtime: Rust + Poko APIs + native adapters
  Diligent renderer | Poko Physics | audio | assets | Mute C VM
```

## Trust boundaries
- Client state is untrusted for currency, inventory, purchases, bans, permissions, and rewards.
- Studio publishing requires server-side ownership and permission checks.
- Mute scripts are untrusted; enforce capabilities, bytecode validation, CPU/memory limits, and cancellation.
- Rust WebAssembly Game Authority owns session-level validation, not account/economy truth.
- Moderator actions require server-side roles and immutable audit events.
- Package storage contains bytes; database records control ownership and published pointers.

## Data ownership
| Data | Owner |
|---|---|
| Accounts, roles, ban state | Backend database |
| Currency, purchases, entitlements | Transactional economy ledger |
| Experience metadata/version pointer | Publishing subsystem |
| Asset/package bytes | Object storage |
| Chat connection state | Chat edge/session layer |
| Live game request validation | Game Authority |
| Unsaved scene state | Studio |
| Frame presentation | Client engine |

## Contract rules
Public APIs and protocols are versioned. Rust/C/C++ boundaries use explicit C-compatible interfaces where practical. Mute bindings, Studio properties, autocomplete, serialization, and docs should derive from one metadata registry.
