# Milestone 2 — Task Breakdown
**Theme:** Git integration & file set detection (repo mode)

## M2-T1: Define core data model for “changed file” entries (Qt-free)
**Objective:** Create a minimal, stable representation of repo file changes to drive UI and tests.
### Deliverables
1) In `src/core/`:
    * `enum class ChangeKind { Modified, Added, Deleted, Renamed, Unmerged, Unknown }`
    * `struct ChangedFile { std::string repoRelativePath; ChangeKind kind; std::optional<std::string> renameFrom; }`
    * `struct RepoStatus { std::filesystem::path repoRoot; std::vector<ChangedFile> files; }`
### Constraints
1) No Qt types in `src/core/`
2) Keep fields to the minimum needed by spec:
    * Annotate renames
    * Include deleted files
### Acceptance criteria
1) Code builds and is used by later M2 tasks.
2) Unit tests can construct these structures without Qt.

## M2-T2: Repo discovery and validation (CWD + explicit path)
**Objective:** Implement "find a git repo" semantics for `bendiff` and `bendiff <PathToGitRepo>`.
### Deliverables
1) Core function, Qt-free:
    * `std::optional<std::filesystem::path> FindGitRepoRoot(std::filesystem::path startDir);`
    * Walk up parents looking for .git (v1 does not handle worktrees; `.git` dir only)
2) UI integration:
    * When in repo mode and repo not found → show empty list (per spec: "If it isn't a git repo, no files are listed.") 
    * Status bar indicates "No git repo found"
### Constraints
1) Do not run git yet (that’s next task)
2) Do not parse file changes yet
### Acceptance criteria
1) Running in a non-repo directory yields empty file list and no crash.
2) Running inside a repo identifies the repo root.

## M2-T3: Git executable discovery and process runner (Qt-free utility)
**Objective:** Create a robust way to invoke git and capture `stdout`/`stderr` for parsing and diagnostics.
### Deliverables
1) A small process runner in `src/core/` or `src/common/` (Qt-free preferred):
2) `struct ProcessResult { int exitCode; std::string stdoutText; std::string stderrText; };`
3) `ProcessResult RunProcess(const std::vector<std::string>& argv, std::filesystem::path workingDir);`
4) On Windows, use `CreateProcess` or `_popen`-style; on Linux use popen/fork+exec as you prefer (keep it simple but correct).
5) Errors:
    * If git not found / cannot execute → return error and let UI show modal dialog (runtime errors are modal; no crash) 
### Constraints
Avoid Qt’s QProcess in core unless you explicitly want Qt dependency in business logic. The spec prefers portability of business logic. 
### Acceptance criteria
1) A unit test can run `git --version` when available OR
2) At minimum, the runner is compile-tested and covered by small deterministic tests (mocking is fine; you can also gate integration tests behind env var).

## M2-T4: Invoke git status --porcelain=v1 in repo mode
**Objective:** Retrieve uncommitted changes in the repo root.
### Deliverables
1) In core:
    * `RepoStatus GetRepoStatus(std::filesystem::path repoRoot);`
    * Runs: `git status --porcelain=v1 -z` (recommended for robust parsing)
    *If you stick to non-`-z`, you must be careful about spaces/newlines.
2) UI:
    * When repo opened (CLI or File→Open Repo), call status retrieval and populate model (even if parsing is stubbed at first)
### Constraints
1) Do not implement automatic refresh behavior yet (Milestone 7)
2) Keep this synchronous for now unless it blocks UI; threading can come later
### Acceptance criteria
1) In a repo with known changes, `stdout` is non-empty.
2) In a clean repo, `stdout` empty and file list empty.

## M2-T5: Porcelain v1 parser (including renames/deletes; excluding ignored and submodules)
**Objective:** Parse the porcelain output into `RepoStatus.files` obeying inclusion rules. 
### Deliverables
1) Parser function:
    * `std::vector<ChangedFile> ParsePorcelainV1(std::string_view text, bool nulSeparated);`
2) Parser function must produce:
    * Modified
    * Added (including untracked)
    * Deleted
    * Renamed (with renameFrom populated, and display annotation later)
3)  Parser function must exclude:
    * Ignored files (these should not appear in porcelain unless you ask for them, so keep default)
    * Submodules ignored (in practice, treat S or “modified content in submodule” entries as excluded)
4) Recommended robust format
    * Use `-z` and parse `NUL`-delimited records; handle rename pairs.
### Acceptance criteria
1) Unit tests with fixture porcelain output covering:
    * modified tracked file
    * new untracked file
    * deleted file
    * rename (old→new)
    * submodule change (excluded)
2) Tests verify the exact `ChangedFile.kind` and path fields.

## M2-T6: Sorting and grouping for the file list pane (flat list with directory groups)
**Objective:** Implement the ordering/grouping contract for the left pane. 
### Deliverables
1) Function that maps `std::vector<ChangedFile>` to a UI-ready list:
2) Group key: directory portion of `repoRelativePath` (empty/root for top-level files)
3) Sort groups alphabetically
    * Sort entries within each group alphabetically by path
    * Decide how grouping appears in a flat list:
    * Recommended: insert non-selectable "header rows" (directory names) followed by files
### Constraints
1) Still no diff view
### Acceptance criteria
1) Given a mixed-path fixture list, UI ordering matches spec precisely.
2) Headers are visually distinct and non-selectable (or selection ignored).

## M2-T7: Wire repo mode UI to real data (replace placeholders)
**Objective:** Make the repo file list actually show detected changes.
### Deliverables
1) When repo mode is active (CLI or Open Repo):
    * Determine repo root (M2-T2)
    * Invoke git status (M2-T4)
    * Parse (M2-T5)
    * Populate the file list widget with grouped, sorted rows (M2-T6)
2) On selection:
    * Update diff pane placeholder text to show selected path and change kind (still no diffing)
### Constraints
1) No content diffing
2) No folder-diff mode work (Milestone 3)
### Acceptance criteria
1) In a real repo with changes, the left pane lists correct files.
2) Renames are annotated (e.g., "newname (renamed from oldname)").
3) Deleted files appear.

## M2-T8: Error surfacing for Git failures (modal dialogs)
**Objective:** Make runtime failure modes user-visible and non-crashing. 
### Deliverables
1) If git executable not found / cannot run:
    * Show modal dialog explaining Git could not be executed
    * Repo list stays empty
2) If `git status` returns nonzero:
    * Show modal with `stderr` summary
    * Do not crash
### Acceptance criteria
1) Simulate by temporarily renaming git on PATH or setting PATH empty when launching:
2) App shows dialog, continues running.

## Milestone 2 Exit Criteria
Milestone 2 is complete when:
1) Repo mode identifies repo root from CWD or explicit path 
2) App invokes git status --porcelain=v1 and parses results 
3) File list shows correct set (modified/added/untracked/deleted/renamed)
4) Ignored files are excluded; submodules ignored 
5) File list is grouped by directory (flat grouped list) and sorted properly 
6) Unit tests exist for porcelain parsing fixtures 


