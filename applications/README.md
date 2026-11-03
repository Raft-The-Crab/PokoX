# Poko Applications

Application hosts that use the shared Poko Engine.

## Applications

- **pokox_client_host**: Windows 11+ and Android 11+ player library, login, install, runtime launch, social and session UX
- **poko_studio_host**: Windows 11+ and Android 11+ creation environment, world editor, UI Maker, MUTE editor, asset pipeline, testing, server configuration and publishing
- **poko_mod_host**: Windows 11+ moderator console with reports, publishing review, live diagnostics. Does NOT execute untrusted creator code in this process
- **poko_review_host**: Separate restricted process that loads exact submitted packages using shared runtime libraries
- **poko_authority_host**: Headless C++ server process for game authority. Does not link renderer or desktop UI

## Architecture

- Windows UI: Rust + egui calling C ABI
- Android UI: Kotlin calling C ABI via JNI
- Shared engine: C++ modules with stable C ABI boundary
- Trust boundaries: Each application has separate privileges; shared engine does not imply shared process execution
