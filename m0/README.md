# M0 — feasibility applications

These targets answer environment questions. They are **not** engine code and are
expected to be deleted or absorbed when the real modules land in M1.

**Note:** M0 is disabled by default in the build. To build it, use:
```bash
cmake -S . -B build/windows-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug -DPOKO_BUILD_M0=ON
```

| Target | Requirement | What it proves |
|---|---|---|
| `poko_m0_smoke` | A | the CMake/Ninja/compiler pipeline works, C++20 is really enabled |
| `poko_m0_sdl2_window` | B | a resizable Win32 window, key/mouse/window events, clean shutdown |
| `poko_m0_diligent_triangle` | C | Diligent Core builds under GCC, an OpenGL device is created on an SDL window, a swap chain presents, resize and teardown work |
| `poko_m0_abi_caller` | D | the C ABI is callable from C, with versioning and error handling |

## Running

```bash
cmake -S . -B build/windows-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/windows-debug
ctest --test-dir build/windows-debug --output-on-failure
```

Interactive (leave the automation flags off):

```bash
./build/windows-debug/bin/poko_m0_sdl2_window.exe
./build/windows-debug/bin/poko_m0_diligent_triangle.exe
```

Automated, so a script can assert on them:

```bash
poko_m0_sdl2_window.exe --self-test --frames 30
poko_m0_diligent_triangle.exe --frames 120 --dump-frame frame.bmp
```

## Conventions used by every M0 app

- Exit code `0` means the check passed; anything else prints `FAILED` first.
- `--frames N` bounds the run so nothing depends on a human closing a window.
- Diagnostics go to stdout and include the exact versions in use, because the
  point of M0 is to record what the environment actually is.
- No app silently degrades: if a backend or feature is unavailable, the app says
  so and returns non-zero.
