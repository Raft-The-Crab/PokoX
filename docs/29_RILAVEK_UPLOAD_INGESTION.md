# 29 — Rilavek Upload Ingestion Integration

**Status:** Optional integration; direct-to-R2 signed upload is the baseline.  
**Purpose:** Resumable creator uploads and controlled bulk ingestion.  
**Not a purpose:** Object storage, CDN, database, game networking or backup retention.

## 1. Service role

Rilavek accepts file transfers through supported ingress mechanisms, including HTTP/TUS, and routes the stream to an S3-compatible destination such as Cloudflare R2. It advertises a Free tier with 10 GB monthly transfer and seven days of transfer logs. It does not add persistent object capacity: the object is stored in the configured R2 bucket. Rilavek claims direct in-memory pass-through and no file-content storage on its own infrastructure; treat this as a vendor claim to verify against current Terms and Privacy documents.

## 2. PokoX-approved uses

- Optional large project/archive ingestion from Poko Studio.
- Resumable creator uploads on unstable networks.
- Controlled migration or bulk upload from existing creator repositories.
- Non-sensitive backup ingestion only after vendor/privacy review.

Direct signed R2 upload remains simpler for the initial MVP. Do not route ordinary game runtime downloads or live multiplayer packets through Rilavek.

## 3. Upload flow

```text
Poko Studio → Go API (authorize creator + submission)
           → Rilavek token API (backend Sender token only)
           → temporary TUS token returned to Studio
           → TUS upload → Rilavek pipe → private R2 quarantine bucket
           → signed webhook → Go API verifies HMAC + file_id
           → object existence/size check → technical validation queue
           → human moderation → approved release bucket/prefix
```

## 4. Credentials

- Store Rilavek private Sender API tokens only in backend secret storage.
- Never put `sk_` Sender tokens in Windows/Android clients, `.pk`, `.pkx`, `.pkB`, GitHub or logs.
- Backend requests a temporary `tmp_` upload token through the documented API; only that short-lived token reaches the upload client.
- Use separate Sender identities and destination credentials for distinct ingestion workflows where available.
- Rotate/revoke compromised Sender credentials.
- Use the narrowest destination permissions supported by R2/Rilavek. Prefer a write-only quarantine destination where feasible.

## 5. Webhook handling

- Read the raw request body before JSON parsing.
- Verify `X-Rilavek-Signature` as HMAC-SHA256 using the configured webhook secret and constant-time comparison.
- Reject missing/invalid signatures.
- Process `file_id` idempotently; duplicate webhooks must not create duplicate submissions.
- Treat the webhook as an event notification, not proof that the object is safe or approved.
- Independently confirm the expected R2 object key, size and checksum before queueing validation.
- Respond quickly; enqueue longer work and return success only after durable event recording.

## 6. Object layout

Use server-generated opaque keys. Do not trust the client filename as an object path.

```text
r2-quarantine/
  uploads/{submission_id}/{upload_id}/original
r2-approved/
  sha256/{prefix}/{content_hash}
r2-private-backups/
  creator/{creator_id}/{backup_id}
```

Quarantine objects are never publicly readable. Moderation approval references the exact content hash. Approved objects are promoted/copied by backend workflow; never make the quarantine bucket public.

## 7. Quota accounting

Rilavek transfer (10 GB/month on current Free page) and R2 storage (10 GB-month Standard/account on current Free tier) are independent counters. A 3 GB upload can consume approximately 3 GB transfer at ingress and contribute to monthly R2 storage usage while retained. Repeated transfers may consume Rilavek quota without increasing unique R2 storage if content is deduplicated. Set upload caps and pause optional Rilavek ingestion before quota exhaustion.

## 8. Privacy and safety review

Before accepting public UGC through this path, review Rilavek Terms, Privacy, data processing terms, logging metadata, subprocessors, retention, incident contact and webhook security. For sensitive child-safety evidence or suspected illegal content, use a legally reviewed direct-to-controlled-quarantine process; do not unnecessarily route it through third-party transfer infrastructure. The public upload path must still quarantine, validate and human-review content.

## 9. Fallback and outage

If Rilavek is unavailable or monthly transfer is exhausted, the backend may issue a direct signed R2 upload path if configured. The fallback must enforce the same account authorization, size limits, object scope, quarantine, checksum and moderation status. Do not bypass quarantine to keep uploads working.

## 10. Acceptance tests

- Client never receives private Sender API token.
- Expired temporary token cannot upload.
- Upload cannot escape its assigned pipe/destination.
- Invalid webhook signature is rejected.
- Duplicate `file_id` is idempotent.
- Untrusted filename cannot control R2 key.
- Upload goes to private quarantine only.
- Object size/hash is rechecked before review.
- Rilavek quota exhaustion produces a clear fallback/error.
- Direct-R2 fallback has equivalent authorization and review behavior.

## 11. Official references

- https://rilavek.com/
- https://rilavek.com/docs/sources/http
- https://rilavek.com/docs/senders
- https://rilavek.com/docs/webhooks
- https://developers.cloudflare.com/r2/pricing/
