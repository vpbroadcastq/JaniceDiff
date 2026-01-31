# Milestone 6 — Task Breakdown
**Theme:** Diff rendering (inline + side-by-side) using the diff engine

## M6-T1: Define a UI-agnostic “render model” for diff views
**Objective:** Create an intermediate representation between `DiffResult` (core) and Qt widgets, so rendering stays manageable and testable.
### Deliverables
1) In `src/core/` (or `src/core/render/`), Qt-free:
    * `struct RenderLine { std::optional<std::size_t> leftLine; std::optional<std::size_t> rightLine; LineOp op; /* plus flags */ };`
    * `struct RenderBlock { enum Side { Left, Right, Both }; Side side; std::vector<RenderLine> lines; };`
    * `struct RenderDocument { std::vector<RenderBlock> blocks; };`
2) Builders:
    * `RenderDocument BuildSideBySideRender(const LoadedTextFile& left, const LoadedTextFile& right, const DiffResult& diff);`
    * `RenderDocument BuildInlineRender(...);` producing alternating left/right blocks per spec
### Constraints
1) No Qt types
2) Render model must carry enough information for:
    * line numbers (left and/or right)
    * whether a line is added/deleted/unchanged
### Acceptance criteria
1) Unit tests can build render docs for simple diffs and verify structure (counts, op types, alternating blocks in inline mode).

## M6-T2: Side-by-side view: map diff result to aligned rows
**Objective:** Implement the standard "two column diff table" representation that is easy to paint.
### Deliverables
1) Build aligned rows such that each visual row represents:
    * (left line number + left text) and (right line number + right text)
    * Insert → blank on left, text on right
    * Delete → text on left, blank on right
    * Equal → text on both
    * Ensure deterministic alignment and stable indices.
### Constraints
1) Use the diff engine output exactly; do not re-diff in UI
2) No intra-line highlights
### Acceptance criteria
1) For a known input pair, aligned row output matches expected sequence (can be tested without Qt).

## M6-T3: Inline view: build "alternating left/right blocks"
**Objective:** Implement your specific inline requirement: diff is displayed with alternating left/right blocks. 
### Deliverables
1) Render model builder that produces a sequence of blocks:
    * When encountering deletions → emit a "left block"
    * When encountering insertions → emit a "right block"
    * Equal lines may either:
        * be included as neutral blocks, or
        * be included within whichever block is active (choose one policy and keep consistent)
    * Deleted file behavior from spec remains compatible:
        * Inline deleted file appears as one big left-deletion block (already decided in Milestone 4 spec) 
### Constraints
1) No attempt to show both versions simultaneously in inline mode; it must alternate blocks.
### Acceptance criteria
1) Unit tests: for a replace (delete+insert), render produces left block followed by right block.

## M6-T4: Choose and implement Qt text rendering widgets
**Objective:** Replace any placeholder viewers with a concrete, maintainable implementation.  Recommended options:
1) Two `QPlainTextEdit` widgets (read-only) with custom formatting.
2) Custom `QAbstractScrollArea` that draws text rows (more work, more control)

For v1 and AI-assisted development, (A) is usually faster, but line-based background coloring in `QPlainTextEdit` requires careful use of `QTextEdit::ExtraSelection` or a custom `QSyntaxHighlighter`-like approach.
### Deliverables
1) A `DiffTextView` widget abstraction in `src/app/widgets/` that supports:
    * Setting a render model (rows/blocks)
    * Showing line numbers (either in a separate gutter widget or prefixed text)
    * Per-line background color based on op:
        * Added → green background
        * Deleted → red background 
        * Equal → default
### Constraints
1) No syntax highlighting
2) Read-only
### Acceptance criteria
1) Can display 1–2k lines without becoming unusable (informal check).
2) Colors appear correctly for added/deleted rows.

## M6-T5: Render side-by-side mode using DiffTextView x2
**Objective:** Connect the side-by-side render model to two viewers.
### Deliverables
1) In side-by-side mode:
    * Left viewer shows left rows with left line numbers
    * Right viewer shows right rows with right line numbers
    * Insertions show blank left lines; deletions show blank right lines
2) Ensure the "green vs red" semantics remain correct:
    * Added lines: green on the right side (insert)
    * Deleted lines: red on the left side (delete)
### Acceptance criteria
1) Manual test on a file with edits shows correct alignment and coloring.
2) Line numbers correspond to original file lines (not render-row indices).

## M6-T6: Render inline mode using a single DiffTextView
**Objective:** Implement inline mode rendering with alternating left/right blocks.
### Deliverables
1) Inline view shows blocks in sequence:
    * Left-only blocks for deletions (red)
    * Right-only blocks for insertions (green)
2) Line numbers:
    * For left blocks: show left line numbers
    * For right blocks: show right line numbers
    * For neutral/equal lines (if shown): you may show both or one; document the choice in code comments
### Acceptance criteria
1) Manual test shows alternating blocks for typical change patterns.
2) Deleted file appears as "all deleted" in inline mode per spec. 

## M6-T7: Scroll synchronization in side-by-side mode
**Objective:** Synchronize scrolling between left and right panes. 
### Deliverables
1) When side-by-side mode active:
    * Vertical scroll positions stay synchronized
    * Implementation can be:
        * Connect scroll bar valueChanged signals bidirectionally with guard reentrancy
        * Or a small controller object
### Constraints
1) Horizontal sync is optional
2) Avoid feedback loops
### Acceptance criteria
1) Scroll left; right follows.
2) Scroll right; left follows.
3) No jitter or oscillation.

## M6-T8: Integrate whitespace mode dropdown with re-diff and re-render
**Objective:** Make the whitespace dropdown actually affect rendering by recomputing the diff.
### Deliverables
1) On whitespace mode change:
    * Recompute diff via Milestone 5 engine
    * Rebuild render model
    * Update view(s)
### Constraints
1) Only applies to text files; binary stays unsupported
2) No threading required yet (may be added later if needed)
### Acceptance criteria
1) A file differing only by trailing whitespace becomes “equal” when mode is IgnoreTrailing.
2) View updates without restarting app.

## M6-T9: Regression tests for render model builders (Qt-free)
**Objective:** Ensure inline/side-by-side builders remain correct and deterministic.
### Deliverables
1) Unit tests that:
    * Create tiny left/right vectors
    * Run diff engine
    * Build render model
    * Compare to expected simplified representation (e.g., sequence of ops and block sides)
### Constraints
1) Do not attempt pixel-perfect GUI tests
2) Keep tests stable across platforms
### Acceptance criteria
1) ctest passes with render model tests.

## Milestone 6 Exit Criteria
Milestone 6 is complete when:
1) Inline renderer shows alternating left/right blocks 
2) Side-by-side renderer shows aligned rows and synchronized scrolling 
3) Added lines are green and deleted lines are red 
4) Line numbers are rendered 
5) Pane mode toggle remains functional at runtime
6) Whitespace dropdown triggers recompute and re-render (engine consumed, not modified)



