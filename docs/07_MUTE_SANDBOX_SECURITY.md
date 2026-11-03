# 07 — MUTE Sandbox and Script Security

**Status:** Security-critical proposed baseline.  
**Principle:** Rich language capabilities do not imply host privileges.

## 1. Threat model

Treat every creator script and marketplace library as hostile input. A script may be buggy, malicious, intentionally resource-exhausting, or written to exploit a runtime defect. Client scripts are additionally controlled by the player and can be modified after delivery.

Security goals: contain scripts, prevent privilege escalation, cap resource use, isolate game tenants, protect platform services, and produce useful audit/diagnostic information.

## 2. Capability model

Scripts receive explicit capabilities from a trusted host manifest and runtime policy. Examples:

- `Players.Read`
- `Players.StateWrite` (server realm only)
- `World.Read`
- `World.Modify`
- `Physics.Query`
- `UI.Create`
- `UI.Modify`
- `Events.Subscribe`
- `Events.Publish`
- `DataStore.Read`
- `DataStore.Write`
- `Matchmaking.Use`
- `Chat.Use`
- `Analytics.Write`

Never provide capabilities such as `Platform.Account.Admin`, `Pokoins.Grant`, `Moderation.Admin`, `Filesystem.Root`, `Process.Execute`, `Native.Load`, `RawSocket.Open`, or `OtherGame.Read` to ordinary game scripts.

The manifest is a request/description. The platform/server operator grants the effective capability set. A script cannot grant itself a permission. Every host API call checks the current script context and capability, not merely the editor UI.

## 3. Realm separation

Client, server, shared, and Studio scripts use separate environments. Prefer separate VM instances or similarly strong per-realm/per-tenant isolation for hostile code. Shared libraries must be immutable or copied into a controlled environment. Avoid shared mutable global state across untrusted scripts.

A server-hosted process may run multiple scripts, but a game bug must not expose another game's environment. The initial creator server runtime should use one isolated game-server process per instance; multi-tenant multiplexing is deferred until measured and security-reviewed.

## 4. Forbidden host access

Initial MUTE must not provide unrestricted filesystem, process execution, environment variables, native dynamic loading, raw memory access, arbitrary sockets, arbitrary database connections, platform secrets, or unrestricted reflection into VM/host internals. HTTP access, if later supported, must use an allowlisted broker with destination policy, timeouts, response-size caps, and SSRF protections.

## 5. Resource budgets

Each script/realm receives quotas:

- Instruction or operation budget per scheduling slice.
- Memory allocation budget.
- Maximum stack/call depth.
- Maximum active tasks/coroutines.
- Event subscription and dispatch budget.
- Network message count and byte budget.
- DataStore request/byte budget.
- Execution deadline/cancellation.

Budgets are configurable by trusted runtime policy and game tier. A developer cannot increase platform-level quotas by script code. Exceeding a budget yields a controlled error, task termination or server policy response; it must not crash the runtime.

## 6. Host API design

- Pass narrow handles, not raw pointers.
- Validate object ownership and realm on every call.
- Check game/tenant ID on every service request.
- Use immutable snapshots for read-heavy data.
- Avoid reentrant arbitrary callbacks across sensitive C++ internals.
- Validate numeric ranges and reject NaN/infinity where inappropriate.
- Bound collections, string lengths, nesting and serialization sizes.

## 7. Libraries and supply chain

Every MUTE library has creator identity, version, dependency manifest, hash, license, permission declaration and moderation status. Lock dependency versions in published packages. Do not load unpinned latest dependencies at runtime. Static analysis assists review but does not replace human review or sandboxing. Revoked libraries must be blocked from new installs/updates; already installed offline packages cannot learn about revocation until they reconnect.

## 8. Debugging policy

Studio can expose source maps, stack traces and profiler output for the creator's own project. Player builds need not expose source code to other players. Obfuscation is optional friction only and is never a security boundary. Server scripts remain out of `.pkx`.

## 9. Failure containment

- Script error: terminate or disable the affected script according to policy.
- VM crash: terminate the isolated process; preserve host service integrity.
- Budget exhaustion: controlled stop and diagnostic.
- Invalid bytecode: reject before execution.
- Host API violation: deny, log security event, apply rate/risk policy.

## 10. Required security tests

Attempt filesystem/process/socket access, privilege escalation, cross-script environment access, cross-game DataStore access, oversized allocations, infinite loops, recursion exhaustion, event flooding, malformed bytecode, hostile serialization, native module loading, and reentrant host calls. Fuzz parser, bytecode verifier, serializer, and host API decoders.

## 11. Reference

Use the [OWASP Game Security Framework](https://owasp.org/projects/gamesec-framework) as a living checklist. Its scope includes game clients, servers, anti-cheat, identity, virtual economies and connected services.
