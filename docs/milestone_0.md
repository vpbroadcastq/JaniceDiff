# Milestone 0 Tasks
## M0-T1: Initialize repository structure and conventions
**Objective:** Create a predictable, scalable layout that supports strict UI/business separation later.
### Deliverables
Directory layout (suggested):
```
src/app/ (Qt Widgets UI shell)
src/core/ (non-Qt business logic; mostly empty in M0)
src/common/ (logging, small utilities)
tests/ (unit tests; minimal harness in M0)
cmake/ (helper modules)
third_party/ (only if needed; preferably empty)
docs/ (place spec/milestones, build notes)
```
Add `docs/spec_v1_frozen.md` and `docs/milestones.md` (copies of your frozen docs)
### Acceptance criteria
1) Tree exists; no build yet required.
2) Spec/milestones are versioned in repo and referenced from README.

## M0-T2: Create CMake build that compiles a Qt Widgets executable
**Objective:** Build system that works on Windows and Linux and will scale to multiple targets.
### Deliverables
1) Top-level `CMakeLists.txt` + `src/app/CMakeLists.txt`
2) Qt6 Widgets target bendiff with:
3) C++ standard set (C++23)
4) warnings enabled per-compiler (MSVC/GCC/Clang)
5) AUTOMOC/AUTORCC/AUTOUIC enabled to enable toe use of .ui later (safe to enable now)
6) Clear configuration error if Qt6 not found
### Acceptance criteria
1) On both platforms, configure + build succeeds when Qt6 is installed.
2) Produces bendiff executable.

## M0-T3: Minimal Qt Widgets application that launches cleanly
**Objective:** Prove the app runs and event loop is correct.
### Deliverables
1) `main.cpp` creates `QApplication`, constructs a `MainWindow`, shows it, `exec()`
2) `MainWindow` is a `QMainWindow` with:
    * Title "BenDiff"
    * Empty central widget (placeholder)
    * Status bar showing "Ready"
### Acceptance criteria
1) Running bendiff opens a window and exits cleanly.
2) No crashes, no Qt warnings spam on startup.

## M0-T4: Logging infrastructure (minimal but durable)
**Objective:** Add logging that will be useful during AI iteration and later debugging.
### Deliverables
1) `src/common/logging.*` implementing:
2) A lightweight logger API (wrapper over `std::cerr`)
3) Log levels (at least: Debug, Info, Warn, Error)
4) Timestamp + level prefix formatting
5) In debug builds, logs go to console by default
6) Write to a rotating file in a predictable location (but keep it simple in M0)
### Acceptance criteria
1) On startup, app logs a single line like "BenDiff starting..."
2) Logging usable from both UI and future core code.

## M0-T5: Cross-platform “developer build” documentation
**Objective:** Ensure rebuild from scratch is documented.
### Deliverables
1) README.md containing:
    * Prereqs: CMake, compiler, Qt6 Widgets
    * Build instructions for Linux: `cmake -S . -B build && cmake --build build`
    * Build instructions for Windows (MSVC): equivalent generator instructions
    * Run instructions
    * Note about where logs go (if applicable)
### Acceptance criteria
A clean machine with Qt installed can follow README to build and run.

## M0-T6: Unit test harness skeleton (even if there are no real tests yet)
**Objective:** Establish the "tests exist from day 1" discipline stated in the spec `spec_v1_frozen.md`
### Deliverables
1) Add GoogleTest as a test framework:
2) Create `tests/test_smoke.cpp` with one trivial assertion
3) Hook tests into CTest: `ctest --test-dir` build works
### Acceptance criteria
1) ctest runs and passes.
2) Tests build on Windows and Linux.


## Milestone 0 Exit Criteria Checklist
Milestone 0 is complete when:
1) `cmake -S . -B build and cmake --build build` succeed on Windows and Linux 
2) Running `bendiff` shows an empty main window 
3) Logging works and is callable from anywhere 
4) ctest runs (at least one smoke test)
5) README documents the above















