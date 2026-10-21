# PokoX Dependencies
# This file manages third-party library integration

# Diligent Engine (Renderer)
# Repository: https://github.com/DiligentGraphics/DiligentEngine
# License: Apache 2.0
# Selected Revision: v2.5.6 (September 2024)
# Build Toolchain: MinGW/GCC 16.2.0 (MSYS2) - see ADR-001
# Backend: OpenGL (primary)
# Note: D3D11/D3D12 not available with MinGW (requires Visual Studio ATL)
# Note: Vulkan linking fails with MinGW (DLL creation linker error)
# Linking: -lopengl32 (OpenGL)
# Warning: GCC 16.2.0 optimization downgrade (O2/O3 → O1) in release builds

# Third-party dependencies will be added here as needed
# Initial Stage 1 does not require external dependencies beyond the build system

message(STATUS "PokoX Dependencies: No external dependencies required for Stage 1")
message(STATUS "PokoX Renderer: Diligent Engine v2.5.6 (Apache 2.0) - integration pending")
message(STATUS "Renderer Backend: OpenGL (primary)")
message(STATUS "D3D11/D3D12: Not available with MinGW (requires Visual Studio ATL)")
message(STATUS "Vulkan: Available but linking fails with MinGW - deferred")
