# 30 — Hugging Face Two-Account Deployment Plan

**Status:** Conditional deployment topology; not part of guaranteed $0 capacity.  
**Intended split:** Account A = Go platform backend; Account B = C++ game authority.

## 1. Why two accounts

The project owner wants separate Hugging Face accounts for the main platform server and game-authority server. Retain that separation as an operational design:

- Account A owns the Go API deployment, API secrets and deployment repository.
- Account B owns the C++ game-authority deployment, runtime secrets and deployment repository.
- No shared personal access token. Credentials are separately scoped and rotated.
- Both deployments use external persistent PostgreSQL/R2; Space disk is never authoritative.

Account separation reduces accidental coupling and blast radius. It must not be used to evade per-account plan restrictions or provider quotas.

## 2. Current compute eligibility gate

Official Hugging Face Spaces documentation checked 2026-11-03:

- CPU Basic is listed as 2 vCPU, 16 GB RAM and 50 GB disk with no hourly hardware charge.
- The same documentation says compute-backed Gradio and Docker Spaces require PRO for personal accounts or Team/Enterprise for organizations.
- The free personal exception is limited to up to two Gradio Spaces running on ZeroGPU; this is not a general-purpose Go/C++ Docker entitlement.
- Free Spaces sleep after a period of inactivity.
- Space disk is not persistent.

Therefore, two ordinary free personal accounts do not by themselves grant two free Docker Spaces. The two-account plan is viable only if each account independently has appropriate Docker compute eligibility (PRO, eligible organization plan, a grant, or an updated documented entitlement). Do not assume that 2 x free CPU Basic hardware means 2 x free Docker compute.

## 3. Workload assignment

| Account | Workload | Requirements |
|---|---|---|
| HF Account A | Go platform API/control plane | HTTP/HTTPS, stateless processes, external PostgreSQL, external R2, health check |
| HF Account B | C++ headless authority prototype | Tested inbound transport, external state, session recovery, no reliance on local disk |

## 4. Transport limitation

The platform API is naturally HTTP/HTTPS. The game authority may need low-latency real-time transport. Do not assume Spaces supports arbitrary raw UDP or arbitrary inbound TCP. First prototype the authority using the intended supported public ingress (possibly WebSocket over HTTPS) and measure latency, disconnect/reconnect, proxy timeouts, idle behavior and port restrictions. If the game protocol requires UDP or unsupported ingress, Hugging Face Spaces is not the authority host for that build; use community hosts or a different eligible provider.

## 5. Lifecycle and persistence

- Free Spaces sleep after inactivity; assume restart and cold start.
- 50 GB disk is not persistent. Never store user accounts, game saves, Pokoins, packages or authority state solely on Space disk.
- Externalize PostgreSQL, R2 and session recovery state.
- Make process restart safe and idempotent.
- Health checks must report readiness only after required external dependencies are reachable.
- Do not use an in-memory session as the sole copy of durable player progress.

## 6. Deployment sequence

1. Verify account A's Docker eligibility and record the plan/entitlement evidence.
2. Verify account B's Docker eligibility separately.
3. Build a minimal Go health API image and deploy to Account A.
4. Build a minimal C++ authority image and deploy to Account B.
5. Test process restart, sleep, cold start and external DB reconnect.
6. Test actual inbound transport from Windows and Android test clients.
7. Verify no secrets are shared in logs/repositories.
8. Run small load tests; record CPU, RAM, cold-start and latency.
9. Decide whether the environment is suitable for development, closed alpha or not at all.

## 7. Fallback

If either account lacks Docker compute eligibility, run the respective service locally for development. For multiplayer testing, use a creator-hosted server. Do not claim a public cloud backend is available until the Go API has a reliable reachable deployment. Cloudflare Workers can provide lightweight edge functions, but they do not automatically replace a general Go server.

## 8. Acceptance criteria

- Each account's compute eligibility is independently verified.
- Each service is deployable from a reproducible container image.
- Account A cannot access Account B deployment secrets and vice versa.
- Server restart loses no authoritative permanent data.
- API responds after cold start within a measured target.
- Authority transport works from target clients and is benchmarked.
- No unsupported UDP assumption remains.
- Capacity and uptime are described as best-effort, not guaranteed.

## 9. Official source

https://huggingface.co/docs/hub/spaces-overview
