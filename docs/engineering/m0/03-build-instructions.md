# M0 — Build Instructions (Windows 11, MSYS2 UCRT64)

These instructions were executed on the development machine and the results are
recorded in [`02-toolchain-report.md`](./02-toolchain-report.md). They are written
so that another machine can reproduce them without reading the source.

---

## 1. Prerequisites

Install MSYS2, then run the following **in an MSYS2 UCRT64 shell** (not PowerShell,
not the plain `MSYS` shell):

```bash
pacman -Syu
# If MSYS2 asks to close the terminal, close it, reopen UCRT64, repeat until done.

pacman -S --needed \
  mingw-w64-ucrt-x86_64-toolchain \
  mingw-w64-ucrt-x86_64-cmake \
  mingw-w64-ucrt-x86_64-ninja \
  mingw-w64-ucrt-x86_64-pkgconf \
  mingw-w64-ucrt-x86_64-SDL2 \
  mingw-w64-ucrt-x86_64-git
```

The resulting versions on the reference machine:

| Tool | Version |
|---|---|
| GCC / G++ | 16.2.0 |
| CMake | 4.4.2 |
| Ninja | 1.13.2 |
| pkgconf | 2.32.10 |
| SDL2 | 2.32.10 |
| Git | 2.55.0 |

Requirements validated by the build: **CMake ≥ 3.24**, a C11 and C++20 compiler,
Ninja, SDL2 discoverable through `find_package(SDL2 CONFIG)` or `pkg-config`.

> Do not mix toolchains. A binary produced by `MINGW64` or `CLANG64` must not be
> linked against libraries from `UCRT64`; the runtime DLLs differ. Pick UCRT64 and
> stay there for the Windows build.

---

## 2. Configure

From the repository root, in the UCRT64 shell:

```bash
cmake -S . -B build/windows-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
```

The configure prints a summary and explicitly lists every module that is **not
implemented yet**:

```
-- poko: not implemented yet, skipped -> engine/core
-- poko: not implemented yet, skipped -> engine/math
...
-- PokoX 0.1.0 build summary
--   generator        : Ninja
--   C++ compiler     : GNU 16.2.0
--   SDL2             : system package, version 2.32.10
--   Diligent Core    : FetchContent v2.5.6 (content-pinned), OpenGL backend only
```

A skipped module is not a warning and not an error: it means the directory is
empty, which is the truthful state of the tree.

### Useful options

| Option | Default | Meaning |
|---|---|---|
| `POKO_BUILD_M0` | `ON` | the feasibility applications in `m0/` |
| `POKO_WITH_DILIGENT` | `ON` | build Diligent Core and the triangle test |
| `POKO_USE_SYSTEM_SDL2` | `ON` | use the MSYS2 SDL2 package instead of building SDL2 |
| `BUILD_TESTING` | `ON` | register the CTest suite |
| `POKO_BUILD_CLIENT` / `_STUDIO` / `_MOD` / `_AUTHORITY` | `ON` | application hosts (skipped until implemented) |

Example — toolchain-only build, no graphics dependency:

```bash
cmake -S . -B build/windows-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug -DPOKO_WITH_DILIGENT=OFF
```

---

## 3. Build

```bash
cmake --build build/windows-debug
```

Executables are written to `build/windows-debug/bin/`. Runtime DLLs (SDL2) are
copied next to the executables by a post-build step, so they also start from
Explorer without the MSYS2 environment.

---

## 4. Test

```bash
ctest --test-dir build/windows-debug --output-on-failure
```

The suite is bounded and unattended: every windowed test runs a fixed number of
frames and exits by itself.

---

## 5. Run interactively

```bash
./build/windows-debug/bin/poko_m0_sdl2_window.exe        # Esc or close to quit
./build/windows-debug/bin/poko_m0_diligent_triangle.exe  # Esc or close to quit
```

---

## 6. Reproducibility notes

- **Dependency pins.** Diligent Core is downloaded from a release archive and
  verified against a SHA-256 hash; a mismatch fails the configure. The pins in
  force are written to `build/windows-debug/poko-dependency-pins.txt` after every
  configure, so an existing build can be audited.
- **Offline builds.** Set `-DFETCHCONTENT_SOURCE_DIR_DILIGENTCORE=<path>` to use an
  existing Diligent checkout instead of downloading.
- **Line endings.** `.gitattributes` forces LF for source and CMake files. CRLF in
  a `CMakeLists.txt` or shell fragment breaks MSYS2 builds in ways that are hard
  to read, so this is enforced rather than left to the editor.

---

## 7. Troubleshooting

| Symptom | Cause | Fix |
|---|---|---|
| `SDL2 was not found` | package missing, or configuring outside UCRT64 | `pacman -S mingw-w64-ucrt-x86_64-SDL2`, reconfigure from a clean build dir |
| `undefined reference to SDL_*` at link time | a second, incompatible SDL2 was picked up | do not add another SDL2 to the path; check `POKO_USE_SYSTEM_SDL2` |
| Configure hangs with no output while fetching Diligent | `git clone` of the Diligent repository stalls on some networks (reproduced here: pack file received zero bytes twice) | this project pins the **archive**, not the git ref; if a mirror is needed, use `FETCHCONTENT_SOURCE_DIR_DILIGENTCORE` |
| `poko_m0_sdl2_window.exe` starts and immediately closes | run with `--frames 30` from a terminal to see the diagnostics | n/a |
| Build directory behaves strangely after changing toolchain | CMake cache still points at the old compiler | delete `build/windows-debug` and reconfigure |

---

## 8. Clean build from scratch

```bash
rm -rf build
cmake -S . -B build/windows-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/windows-debug
ctest --test-dir build/windows-debug --output-on-failure
```

This is the sequence used to verify the M0 results; a build is only considered
reproducible when it survives it.
