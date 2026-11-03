# Poko Mod (Moderator Console)

Windows 11+ moderator application for reviewing submissions and handling reports.

## Components

- **dashboard**: Rust + egui moderator UI with authentication, review queue, and decision UI
- **review_runtime**: Isolated shared-engine process that loads exact submitted packages
- **evidence**: Evidence capture, references, and access control
- **queue**: Review queue management, capacity tracking, and prioritization
- **audit**: Audit logging, reviewer actions, and access tracking

## Process Architecture

```
Poko Mod (trusted Rust/egui host)
  - Moderator auth/session
  - Review queue and decision UI
  - Audit/evidence references
  - No creator code execution
          |
          | narrow authenticated local IPC
          v
Poko Review Host (restricted process)
  - Shared Poko runtime
  - Exact submitted .pkx
  - Matching .pkB / authority harness
  - Synthetic test accounts
  - Temporary local storage
  - Restricted networking/filesystem
```

## Review Workspace

- Game viewport: play, pause, restart, fullscreen, camera/spectator
- Scene inspector: read-only hierarchy/object properties
- Content inspector: model/texture/audio/animation preview
- Script inspector: MUTE source, realm, capabilities, dependencies
- Network inspector: review-session messages and schema failures
- Authority inspector: synthetic player state and validation
- Performance panel: frame time, memory, asset load
- Decision panel: checklist, label, reasons, evidence

## Security

- Untrusted content runs outside privileged Poko Mod process
- Review runtime uses Windows restricted tokens, Job Objects, ACLs
- High-risk packages may use disposable VM isolation
- Evidence access is logged and restricted
- Sensitive incident escalation follows documented legal procedure
