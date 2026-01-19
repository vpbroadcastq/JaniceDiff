# Milestones

## Milestone 0 — Repository & Build Foundation
### Goal
Establish a clean, repeatable build and execution baseline.
### Deliverables
1) Git repository initialized
2) CMake-based build that produces a Qt Widgets executable
3) Application launches with an empty main window
4) Logging infrastructure in place (even if minimal)
### Key outcomes
1) You can build on Windows and Linux
2) You can run the app under a debugger
3) All later work has a stable base
### Notes
1) No Git logic
2) No diff logic
3) No real UI beyond the frame

## Milestone 1 — Application Shell & Command-Line Entry
### Goal
Implement the entry semantics defined in the spec.
### Deliverables
1) Main window with:
    * Menu bar
    * Toolbar
    * Status bar
    * Central pane container
2) Command-line parsing for:
    * `bendiff`
    * `bendiff <repo>`
    * `bendiff <left> <right>`
3) Mode detection
    * Repo mode
    * Folder-diff mode
    * Invalid args → error dialog + exit code
### Key outcomes
1) All invocation paths work
2) Errors are surfaced cleanly
3) App exits with correct codes when appropriate
### Notes
File list and diff panes may be placeholders.  This milestone is about control flow, not diffing.

## Milestone 2 — Git Integration & File Set Detection
### Goal
Populate the left-hand file list correctly in Git repo mode.
### Deliverables
1) Detection of Git repo (CWD or provided path)
2) Invocation of git status --porcelain=v1
3) Parsing logic that produces:
    * Modified files
    * Added files
    * Deleted files
    * Renamed files (annotated)
    * Ignored files excluded
    * Submodules excluded
4) File list pane populated and grouped by directory
### Key outcomes
1) File list exactly matches spec-defined inclusion rules
2) No diff view yet—selection may be a no-op
3) Tests expected:
    * Unit tests for porcelain parsing
    * Fixture-based tests using recorded git status output

## Milestone 3 — Folder Diff Mode (Directory Comparison)
### Goal
Implement the non-Git directory comparison path.
### Deliverables
1) Recursive directory walker
2) Union of relative paths
3) Status determination:
    * Left-only
    * Right-only
    * Same
    * Different
    * Unreadable
4) Folder diff file list populated correctly
5) Manual refresh (F5 / toolbar) implemented
### Key outcomes
1) `bendiff <left> <right>` works end-to-end
2) Double-clicking files can open a placeholder diff view
### Tests expected
1) Fixture-based directory trees
2) Byte-equality vs difference cases

## Milestone 4 — File Loading & Binary Detection
### Goal
Correctly load file contents for diffing.
### Deliverables
1) UTF-8 file loader
2) Line ending normalization (CRLF/LF)
3) Binary detection:
    * UTF-8 decode failure
    * “Unsupported” placeholder rendering
4) Deleted-file handling:
5) Inline mode: all lines deleted
6) Side-by-side: empty right pane
### Key outcomes
1) File content pipeline is correct and testable
2) No diff algorithm yet—just raw content display
### Tests expected
1) UTF-8 decoding tests
2) Binary file detection tests
3) Deleted file rendering tests

## Milestone 5 — Line-Based Diff Engine (Core Logic)
### Goal
Implement the core diff algorithm, isolated from UI.
### Deliverables
1) Qt-free diff engine library/module
2) Line-based diff producing:
    * Hunks
    * Per-line classification
    * Whitespace-handling modes:
    * Exact
    * Ignore trailing whitespace
    * Ignore all whitespace
### Key outcomes
1) Deterministic diff output
2) Diff engine independently unit-testable
### Tests expected
1) Golden tests for known file pairs
2) Whitespace mode behavior tests

## Milestone 6 — Diff Rendering (Inline + Side-by-Side)
### Goal
Render diffs according to the spec.
### Deliverables
1) Inline diff renderer:
2) Alternating left/right blocks
3) Side-by-side renderer:
4) Left and right panes synchronized
5) Color semantics:
    * Green = added
    * Red = deleted
6) Line numbers rendered
### Key outcomes
1) Visual diff matches spec
2) Both modes toggleable at runtime
### Notes
This milestone is UI-heavy.  The diff engine is consumed but not modified.

## Milestone 7 — Navigation, Refresh, and UX Polish
### Goal
Make the tool usable for real work.
### Deliverables
1) Next/Prev change navigation
2) Status bar diff stats
3) Automatic refresh in repo mode
4) Manual refresh in folder mode
5) Reasonable default focus and selection behavior
### Key outcomes
1) End-to-end usable v1 tool
2) No missing core workflows

## Milestone 8 — Persistence, Cleanup, and v1 Hardening
### Goal
1) Stabilize v1 for real usage.
### Deliverables
1) QSettings persistence:
2) Window geometry/state
3) Whitespace mode
4) Pane mode (2 vs 3)
5) Logging cleanup
6) Error-path verification
### Documentation
1) Build instructions
2) Usage examples
### Key outcomes
1) v1-ready build
2) Spec fully implemented




