# Milestone 4 — Task Breakdown
**Theme:** File loading pipeline + binary detection + deleted-file rendering behavior

## M4-T1: Define core "loaded text file" model (Qt-free)
**Objective:** Create a testable representation of a loaded file suitable for later diffing.
### Deliverables (in `src/core/`)
1) `enum class LoadStatus { Ok, NotFound, Unreadable, NotUtf8 };`
2) `struct LoadedTextFile { std::filesystem::path absolutePath; LoadStatus status; std::vector<std::string> lines; bool hadFinalNewline; };`
3) A helper to normalize line endings and split into lines deterministically.
### Constraints
1) No Qt types
2) Use UTF-8 bytes with validation; lines stored as UTF-8 strings
### Acceptance criteria
1) Unit tests can construct LoadedTextFile and validate line splitting.

## M4-T2: UTF-8 loader + line ending normalization
**Objective:** Implement UTF-8-only file reading and line splitting.
### Deliverables
1) Function: `LoadedTextFile LoadUtf8TextFile(std::filesystem::path absolutePath);` with behavior:
    * Read bytes
    * Validate UTF-8:
    * If invalid → NotUtf8
    * Normalize line breaks:
        * Treat CRLF and LF as line breaks
        * Treat lone CR as line break
        * Preserve whether final newline existed (optional but useful later)
### Constraints
1) Must be safe for “large enough” files; streaming is fine but not required yet
2) No size limits or warnings per spec
### Acceptance criteria
1) Unit tests covering:
    * LF-only file
    * CRLF file
    * Mixed endings (optional)
2) Invalid UTF-8 → NotUtf8

## M4-T3: Repo-mode "left vs right path resolution" (no diff yet)
**Objective:** Given a selected ChangedFile (from Milestone 2), compute the paths needed to display content.
### Deliverables
1) Core helper that produces the two sides’ "content sources" for display:
    * For folder mode: left absolute path, right absolute path (direct)
    * For repo mode:
        * Left side: last committed version (from Git)
        * Right side: working tree version
        * For deleted files: working tree path missing → special handling

**Important note:** This milestone still avoids line diff, but repo-mode needs access to the committed version’s content to show something meaningful. The simplest v1 approach consistent with the spec is:
Run `git show HEAD:<path>` (or `:<path>` depending on desired semantics) to obtain committed content bytes.
### Constraints
1) Keep this logic in core (Git invocation via the process runner from Milestone 2)
2) For renamed files:  Display should use the "new path" in list; committed version may use renameFrom for old name if necessary (you can defer perfect rename content correctness to later if needed, but the spec requires annotation at least)
### Acceptance criteria
1) Selecting a modified tracked file in repo mode yields two "content sources" that can be loaded into views:
    * committed content bytes available
    * working tree file bytes available

## M4-T4: Binary detection behavior in both modes
**Objective:** Implement the spec’s rule: if UTF-8 decode fails, show "unsupported".
### Deliverables
1) When loading either side:
    * If NotUtf8, treat as binary/unsupported
    * For folder mode:
        * Status in list already includes Same/Different via byte compare (Milestone 3)
        * But opening the diff view should show “Binary/Unsupported” if either side is non-UTF-8 
    * For repo mode:
        * Selecting a binary file shows “unsupported” message in diff pane(s) 
### Constraints
1) Do not attempt to render binary bytes as text
2) Avoid guessing encodings
### Acceptance criteria
1) Unit tests: invalid UTF-8 results in "unsupported" display state
2) Manual test: select a known binary file; UI shows message, not garbage

### M4-T5: Deleted-file handling (inline + side-by-side semantics)
**Objective:** Implement your explicit deleted-file rendering requirements.
### Deliverables
1) When a file is deleted in repo mode:
    * Inline mode:  Render as if every line has been deleted 
    * Side-by-side mode: Left pane shows committed content, Right pane shows empty and indicates deletion 
### Constraints
1) No diff engine yet:
    * "Render as if every line deleted" means: in inline mode, show the committed lines marked as deletions (visual style can be placeholder until Milestone 6, but the structure should exist)
    * Keep representation consistent with later diff rendering (e.g., a simple "render model" describing blocks)
### Acceptance criteria
1) Unit tests for render model generation for deleted files (at least structural)
2) Manual test: delete a file in repo and select it; UI matches expectations.

## M4-T6: Replace diff pane placeholders with raw-content viewers
**Objective:** Move from "labels" to real text display widgets.
### Deliverables
1) Diff pane widgets become read-only text views:
2) Inline mode: one viewer capable of showing alternating blocks (for now, just show one side and annotate blocks minimally)
3) Side-by-side mode: two read-only viewers
4) Populate viewers with loaded content for:
    * folder mode: left/right files
    * repo mode: committed vs working-tree
5) Display line numbers (basic; can be refined in Milestone 6)
### Constraints
1) No block highlighting required yet (Milestone 6)
2) Must be stable and not laggy for normal source files
### Acceptance criteria
1) Selecting a file shows actual content text in the viewer(s)
2) Side-by-side mode shows different content on each side when applicable
3) Binary shows “unsupported” message instead of text

## M4-T7: Tests for loader + binary + deleted semantics
**Objective:** Lock the milestone’s behaviors with unit tests.
### Deliverables
1) Tests for:
    * UTF-8 decoding and line splitting
    * CRLF vs LF normalization
    * Invalid UTF-8 classification
    * Deleted-file “render model” output
### Constraints
1) Keep tests cross-platform
2) For Git-committed content retrieval tests:
3) Prefer isolating Git calls behind an interface and testing parsing/logic with fixtures, or create a tiny temporary repo in tests only if you’re comfortable (optional; can be deferred)
### Acceptance criteria
1) ctest passes on Windows and Linux
2) Loader tests do not depend on external network/resources

##Milestone 4 Exit Criteria
Milestone 4 is complete when:
1) UTF-8 loader exists and normalizes CRLF/LF 
2) Binary detection works via UTF-8 decode failure and shows “unsupported” 
3) Deleted files render per spec in both modes 
4) Diff panes show raw loaded content (not diffs) for non-deleted text files
5) Unit tests cover decoding, binary, and deleted semantics 





