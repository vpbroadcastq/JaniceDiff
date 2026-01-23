# Milestone 3 — Task Breakdown
**Theme:** Directory comparison + folder mode UI wiring + manual refresh

## M3-T1: Define folder-diff core data model (Qt-free)
**Objective:** Create stable, testable representations for folder comparison results.
### Deliverables (in `src/core/`)
1) `enum class DirEntryStatus { Same, Different, LeftOnly, RightOnly, Unreadable };`
2) `struct DirEntry { std::string relativePath; DirEntryStatus status; bool isBinaryHint; /* optional */ };`
3) `struct DirDiffResult { std::filesystem::path leftRoot; std::filesystem::path rightRoot; std::vector<DirEntry> entries; };`
### Constraints
1) No Qt types
2) Only fields needed for list display and later "open file diff" action
### Acceptance criteria
1) Compiles and is covered by a minimal unit test constructing entries.

## M3-T2: Recursive directory walker (enumeration of files)
**Objective:** Enumerate all files under a root directory recursively, producing relative paths.
### Deliverables
1) Core function:
    * `std::vector<std::string> ListFilesRecursive(std::filesystem::path root);`
2) Behavior:
    * Include all files (do not ignore .git or anything else) 
    * Return relative paths normalized to a consistent separator for internal use (recommend / in model; translate to native for IO)
### Constraints
1) Symlinks: follow OS defaults as spec says; use std::filesystem default recursion options unless you explicitly choose otherwise 
2) Handle permission errors gracefully (don’t throw; record unreadable later)
### Acceptance criteria
1) Fixture-based unit test where a small directory tree yields expected relative path set.

## M3-T3: File comparison primitive (byte-equality, streaming)
**Objective:** Determine Same vs Different for files that exist on both sides.
### Deliverables
1) Core function:
    * enum class FileCompareResult { Same, Different, Unreadable };
    * FileCompareResult CompareFilesBytewise(path left, path right);
2) Behavior:
    * If either cannot be opened → Unreadable
    * If sizes differ → Different
        * Else streaming compare → Same/Different
### Constraints
1) No hashing required
2) Must be safe for larger files (stream in chunks)
### Acceptance criteria
1) Unit tests:
    * Same content → Same
    * Different content → Different
    * Missing or permission denied → Unreadable

## M3-T4: Build union set and classify statuses
**Objective:** Produce final DirDiffResult.entries from two roots.
### Deliverables
1) Core function:
    * `DirDiffResult DiffDirectories(leftRoot, rightRoot);`
    * Algorithm per spec 
        * Walk both directories recursively
        * Build union of relative paths
        * Classify:
            * LeftOnly / RightOnly
            * Same / Different based on byte compare
            * Unreadable if either side can’t be read (even if exists)
### Constraints
1) v1 can be file-only list (no separate directory entries)
2) Do not ignore .git
### Acceptance criteria
1) Fixture-based directory trees covering all statuses.
2) Stable ordering (sort lexicographically by relativePath) for deterministic UI/tests.

## M3-T5: Folder mode UI integration (populate file list pane)
**Objective:* Replace placeholders in folder mode with real directory diff results.
### Deliverables
1) When the app enters folder-diff mode (CLI or File→Open Folders):
    * Call DiffDirectories
    * Populate the left pane list with entries
    * Display:
        * Relative path (as shown text)
        * Status indicator (icon or suffix text: [Same], [Different], etc.)
2) Sorting/grouping:
    * Same grouping behavior as repo mode list (directory group headers, flat list) OR
    * Simplest acceptable for v1: flat sorted list (but spec expects grouped-by-directory list generally; keep consistent if possible) 
### Constraints
1) Still no real diff rendering; selecting items updates placeholder pane text.
### Acceptance criteria
1) For a known pair of directories, list contents match expected statuses.
2) Selecting an entry updates the diff pane placeholder labels to show left/right full paths.

## M3-T6: Manual refresh (F5 + toolbar button) for folder mode
**Objective:** Implement folder mode refresh semantics precisely. 
### Deliverables
1) In folder-diff mode only:
    * Pressing F5 triggers re-scan and repopulates list
    * Toolbar “Refresh” triggers same behavior
2) In repo mode:
    * Refresh may be a no-op or show “automatic” (full automatic refresh is Milestone 7)
### Constraints
1) Avoid multi-threading in v1 unless UI stalls; you can defer threading
### Acceptance criteria
1) Modify a file under left/right folder; press F5; status updates.
2) Refresh does not crash if folders were deleted (shows modal error / empties list).

## M3-T7: Double-click opens placeholder diff view
**Objective:** Complete the “works end-to-end” requirement for bendiff <left> <right>.
### Deliverables
1) On double-clicking a folder-diff list entry:
    * If status is LeftOnly/RightOnly/Unreadable:
        * Show placeholder diff view message explaining cannot diff both sides
    * If status is Same/Different:
        * Open the diff view placeholder for that file pair:
    * Update the right pane(s) to show:
        * Left file path
        * Right file path
### Constraints
1) No file loading into text panes yet (Milestone 4)
2) No diff engine invocation
### Acceptance criteria
1) Double-click on an entry changes the diff pane placeholders.
2) No crashes on double-clicking headers (if headers exist).

## M3-T8: Fixture-based tests for directory comparison (required)
**Objective:** Lock in correctness with deterministic tests.
### Deliverables
1) Under tests/fixtures/dir_diff/ create a small fixture set:
    * left-only file
    * right-only file
    * same file
    * different file
    * unreadable case (platform-dependent; can simulate by referencing a missing file or using a temp dir with permissions if feasible)
2) Tests for DiffDirectories verifying expected statuses
### Constraints
1) Keep tests cross-platform:
    * Prefer “missing file” or “locked file” simulations rather than chmod-based unreadable (Windows differs)
    * If unreadable is hard cross-platform, mark it as best-effort and still test on at least one OS by conditional compilation, but keep the test suite green
### Acceptance criteria
1) ctest passes on Windows and Linux.
2) At least Same/Different/LeftOnly/RightOnly are covered unconditionally.

## Milestone 3 Exit Criteria
Milestone 3 is complete when:
1) bendiff <leftPath> <rightPath> produces a populated list with correct statuses 
2) Directory walk is recursive and union-based 
3) Statuses include Left-only / Right-only / Same / Different / Unreadable 
4) Refresh is manual in folder mode via F5/toolbar 
5) Double-click opens a placeholder diff view 
6) Fixture-based tests exist and pass 




