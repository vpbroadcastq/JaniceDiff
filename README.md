# BenDiff

BenDiff is a native GUI diff tool (Qt Widgets) targeting Windows and Linux.

## Docs
- v1 spec (frozen): [docs/spec_v1_frozen.md](docs/spec_v1_frozen.md)
- Milestones: [docs/milestones.md](docs/milestones.md)
- Current milestone tasks: [docs/milestone_0.md](docs/milestone_0.md)

## Repository Layout
- `src/app/` — Qt Widgets UI shell (thin)
- `src/core/` — non-Qt business logic (mostly empty in Milestone 0)
- `src/common/` — small utilities shared across app/core
- `tests/` — unit tests (harness comes in a later M0 task)
- `cmake/` — helper CMake modules
- `third_party/` — vendored deps if needed (prefer empty)
- `docs/` — specifications, milestones, notes

## Prerequisites
- CMake 3.24+
- A C++ compiler with C++23 support
	- Linux: GCC or Clang
	- Windows: Visual Studio 2022 (MSVC)
- Qt 6 with the Widgets module (Qt6::Widgets)

## Build (Linux)
From the repo root:

```bash
cmake -S . -B build
cmake --build build
```

## Build (Windows / MSVC)
From a “x64 Native Tools Command Prompt for VS 2022”:

```bat
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug
```

## Run
Linux:

```bash
./build/src/app/bendiff
```

Windows (Debug config):

```bat
build\src\app\Debug\bendiff.exe
```

## Logs
On startup the app logs a line like `BenDiff starting...`.

- Linux (preferred): `$XDG_STATE_HOME/bendiff/logs/bendiff.log`
- Linux (fallback): `~/.local/state/bendiff/logs/bendiff.log`
- Windows: `%LOCALAPPDATA%\BenDiff\logs\bendiff.log`

Files rotate at ~1 MiB and keep `bendiff.log`, `bendiff.log.1`, `bendiff.log.2`, `bendiff.log.3`.

## Tests
After building:

```bash
ctest --test-dir build
```

On Windows multi-config generators you may need:

```bat
ctest --test-dir build -C Debug
```
