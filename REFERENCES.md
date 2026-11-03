# References and Verification Notes

**Verified:** 2026-11-03. Provider plans and documentation may change. Re-check before creating accounts, deploying workloads or accepting public user data. These sources are technical planning references, not legal advice.

## Engine and toolchain

- Diligent Core: https://github.com/DiligentGraphics/DiligentCore
- Diligent getting started/build notes: https://diligentgraphics.com/diligent-engine/getting-started/
- Diligent Win32/OpenGL and backend notes: https://github.com/DiligentGraphics/DiligentCore
- SDL2 introduction: https://wiki.libsdl.org/SDL2/Introduction
- SDL2 Android: https://wiki.libsdl.org/SDL2/Android
- MSYS2 environments (UCRT64 guidance): https://www.msys2.org/docs/environments/

## Cloudflare

- Workers limits: https://developers.cloudflare.com/workers/platform/limits/
- D1 limits: https://developers.cloudflare.com/d1/platform/limits/
- R2 pricing/free allowance: https://developers.cloudflare.com/r2/pricing/
- R2 S3 compatibility: https://developers.cloudflare.com/r2/api/s3/
- R2 temporary credentials: https://developers.cloudflare.com/r2/api/s3/temporary-credentials/

## Rilavek

- Rilavek official product and pricing: https://rilavek.com/
- Rilavek docs: https://rilavek.com/docs
- HTTP/TUS source and temporary upload token guidance: https://rilavek.com/docs/sources/http
- Sender identity and scoped token handling: https://rilavek.com/docs/senders
- Webhook HMAC-SHA256 verification and idempotency: https://rilavek.com/docs/webhooks

## Other service providers

- Firebase Authentication limits: https://firebase.google.com/docs/auth/limits
- Aiven PostgreSQL free tier: https://aiven.io/docs/products/postgresql/concepts/pg-free-tier
- Aiven Valkey free tier: https://aiven.io/docs/products/valkey/concepts/valkey-free-tier
- Layerbase pricing: https://layerbase.com/pricing
- Cloudinary billing and plans: https://cloudinary.com/documentation/billing_and_plans
- Hugging Face Spaces overview, Docker/Gradio eligibility, hardware, networking and sleep: https://huggingface.co/docs/hub/spaces-overview
- Hugging Face Spaces Docker guide: https://huggingface.co/docs/hub/spaces-sdks-docker

## Security

- OWASP Game Security Framework: https://owasp.org/www-project-gamesec-framework/
- OWASP API Security Top 10: https://owasp.org/www-project-api-security/
- OWASP Application Security Verification Standard: https://owasp.org/www-project-application-security-verification-standard/
- OWASP MASVS: https://mas.owasp.org/MASVS/

## Policy/legal starting points — obtain counsel review

- Philippines RA 11930: https://lawphil.net/statutes/repacts/ra2022/ra_11930_2022.html
- Philippines Data Privacy Act (RA 10173): https://lawphil.net/statutes/repacts/ra2012/ra_10173_2012.html
- National Privacy Commission: https://privacy.gov.ph/
- UK Ofcom Online Safety guidance: https://www.ofcom.org.uk/online-safety/
- ESRB rating guide: https://www.esrb.org/ratings-guide/
- PEGI ratings: https://pegi.info/what-do-the-labels-mean

## Important interpretation notes

### Cloudflare R2

Current Free allowance: 10 GB-month Standard storage per Cloudflare account, 1 million Class A operations/month, 10 million Class B operations/month, internet egress free. This is account-wide storage, not 10 GB per bucket. See official R2 pricing. R2 bucket count does not multiply the free storage allowance.

### Rilavek

Rilavek is an ingress/transfer bridge that streams files into an S3-compatible destination such as R2; it is not the object store. Its current free page advertises 10 GB monthly transfer and 7 days of transfer logs. Its TUS docs say a backend-held private Sender API token creates temporary upload tokens for the client. Its webhook docs specify `X-Rilavek-Signature` as HMAC-SHA256 over the raw request body and recommend idempotent processing using `file_id`. Use it only as an optional ingestion path after Terms/Privacy/vendor review. The 10 GB transfer allowance is separate from, and does not increase, R2 storage.

### Hugging Face

Official Spaces documentation currently lists CPU Basic at 2 vCPU, 16 GB RAM and 50 GB non-persistent disk, with no hourly hardware charge. But it states compute-backed Gradio and Docker Spaces require PRO for personal accounts or Team/Enterprise for organizations. The free personal exception is up to two Gradio Spaces on ZeroGPU, not general-purpose Docker compute. Free Spaces sleep after inactivity. Therefore a two-account Go/C++ Docker deployment is conditional on both accounts independently having compute eligibility; two ordinary free accounts do not bypass the requirement. Test networking before relying on a game-authority workload; do not assume arbitrary UDP/TCP access.

### Other free allowances

- Cloudinary Free is 25 monthly credits, not 25 GB. Credits are shared between transformations, storage and bandwidth.
- Cloudflare Workers Free quota and D1 limits must be checked directly before deployment; thresholds and account-level details can change.
- Aiven and Layerbase eligibility, sleep behavior, connection/storage quotas and terms must be rechecked when provisioning. Avoid dual-write or splitting authoritative transactional data across providers.
- Never use extra accounts to circumvent provider restrictions or quotas.

---

The final system design uses providers behind adapter interfaces where practical. Provider changes should not alter `.pkx`, MUTE semantics, Pokoins ledger semantics or public server protocol contracts.
