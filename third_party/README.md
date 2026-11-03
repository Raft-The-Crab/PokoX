# Third-Party Dependencies

External libraries and dependencies used by PokoX.

## Dependencies

- **DiligentCore**: Graphics API abstraction (OpenGL, OpenGL ES, Vulkan, Direct3D)
  - Repository: https://github.com/DiligentGraphics/DiligentCore
  - Purpose: Cross-platform graphics backend
  - Integration: Pinned submodule or FetchContent

- **SDL2**: Input, window, and event handling
  - Repository: https://github.com/libsdl-org/SDL
  - Purpose: Window creation, input abstraction, platform events
  - Integration: MSYS2 package on Windows, built for Android

## Dependency Governance

For every third-party dependency, record:
- Name and upstream URL
- Pinned version/commit
- License
- Supported platforms
- Binary/source mode
- Patches (if any)
- Known vulnerabilities
- Upgrade test procedure

## Policy

- Never fetch unpinned dependencies at build time for release builds
- Keep license texts/notices in distributed applications
- Generate software bill of materials (SBOM) for release builds
- Verify compatibility before upgrading
- Review licenses before adding new dependencies
