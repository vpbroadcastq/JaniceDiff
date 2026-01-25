# Milestone 5 — Task Breakdown
**Theme:** Core line diff engine (deterministic, testable, UI-agnostic)

## M5-T1: Define diff engine public API and data model (Qt-free)
**Objective:** Lock the engine interface so UI/rendering can consume it without churn.
### Deliverables (in `src/core/diff/` or similar)
1) `enum class WhitespaceMode { Exact, IgnoreTrailing, IgnoreAll };`
2) `enum class LineOp { Equal, Insert, Delete };`
    * (Optionally `Replace`, but you can represent replace as delete+insert within a hunk.)
3) `struct DiffLine { LineOp op; std::size_t leftIndex; std::size_t rightIndex; };`
    * Convention: for `Insert`, `leftIndex = npos;` for `Delete`, `rightIndex = npos`.
4) `struct DiffHunk { std::size_t leftStart, leftCount; std::size_t rightStart, rightCount; std::vector<DiffLine> lines; };`
5) `struct DiffResult { WhitespaceMode mode; std::vector<DiffHunk> hunks; std::size_t leftLineCount; std::size_t rightLineCount; };`
6)  Public entry point:
    * `DiffResult DiffLines(std::span<const std::string> left, std::span<const std::string> right, WhitespaceMode mode);`
## Constraints
1) No Qt types
2) All indices refer to the original input vectors
3) Output must be deterministic for identical inputs
## Acceptance criteria
1) Compiles cleanly on Windows/Linux
2) A trivial unit test can call DiffLines and inspect a result.

## M5-T2: Whitespace normalization helpers
**Objective:** Implement the three whitespace modes consistently and cheaply.
### Deliverables
1) Helper that produces a "comparison key" string for a line:
    * `Exact`: unchanged
    * `IgnoreTrailing`: remove trailing whitespace (spaces/tabs; optionally all isspace at end)
    * `IgnoreAll`: remove all whitespace characters (at least space+tab; optionally all isspace)
### Constraints
1) Do not change original lines; only compute keys for comparison.
2) Define "whitespace" explicitly (recommend: ASCII space + tab + possibly \r).
### Acceptance criteria
1) Unit tests verifying:
    * `foo` vs `foo ` are equal in `IgnoreTrailing`
    * `f o o` vs `foo` are equal in `IgnoreAll`
    * `Exact` differs in both cases

## M5-T3: Implement line diff algorithm (Myers/LCS variant)
**Objective:** Compute a minimal (or near-minimal) edit script from left→right using normalized comparison keys.
### Deliverables
1) Implementation of diff producing a sequence of LineOp operations.
2) Must handle:
    * Empty left/right
    * Completely equal
    * Completely different
    * Repeated lines (avoid pathological mismatch)
### Constraints
1) Keep algorithm Qt-free and well-contained.
2) Complexity: Myers O(ND) is typical; acceptable.
### Acceptance criteria
1) Unit tests for basic cases:
    * Insert-only
    * Delete-only
    * Mixed changes
2)  Determinism: repeated runs yield identical operation sequences.

## M5-T4: Hunk generation from edit script
**Objective:** Convert raw op stream into "hunks" suitable for UI navigation and highlighting.
### Deliverables
1) A hunk builder that groups adjacent non-equal edits into a `DiffHunk` and includes some context lines, or not.
2) Important decision (make explicit in code)
    * For v1, no context lines required in the data model; UI can display all lines.
    * Still, hunks should represent contiguous edit regions:
3) A hunk is a maximal range containing at least one non-Equal op, optionally expanded by small context (0–3 lines) if you prefer.
### Constraints
1) Keep it deterministic: if you use context, make it fixed.
### Acceptance criteria
1) Tests verify:
    * A single isolated insertion produces exactly one hunk
    * Two separated change regions produce two hunks (when separated by ≥1 Equal line, depending on context policy)

## M5-T5: Provide "per-line classification" mapping convenience
**Objective:** Your spec calls for "per-line classification." Make consumption easy for rendering code.
### Deliverables
1) Convenience APIs:
    * `std::vector<LineOp> LeftLineClassification(const DiffResult&)` sized `leftLineCount`
    * `std::vector<LineOp> RightLineClassification(const DiffResult&)` sized `rightLineCount`
    * Or a unified list of aligned rows for side-by-side display:
2) `struct AlignedRow { std::optional<std::size_t> left; std::optional<std::size_t> right; LineOp op; };`
3) `std::vector<AlignedRow> BuildAlignedRows(const DiffResult&);`
### Constraints
1) Still no UI; this is purely data shaping.
2) Must preserve indices and allow later mapping to actual line content.
### Acceptance criteria
1) Tests confirm classification sizes match input line counts.
2) Tests confirm inserts don’t assign bogus indices on the wrong side.

## M5-T6: Golden tests and fixture organization
**Objective:** Lock correctness with stable fixtures, as required by Milestone 5. 
### Deliverables
1) Under tests/fixtures/diff/:
    * Several pairs of input text files (left/right)
    * Expected output stored as a simple text format or JSON you define (recommend JSON for clarity)
2) Tests that:
    * Load fixture pairs
    * Run `DiffLines` in each whitespace mode
    * Compare to expected output
### Constraints
1) Keep fixture sets small and readable.
2) Avoid dependency on external tools.
### Acceptance criteria
1) ctest passes on Windows/Linux.
2) At least one fixture per whitespace mode.

## M5-T7: Performance sanity test (non-benchmark)
**Objective:** Ensure the algorithm does not explode on moderately sized inputs.
### Deliverables
1) A unit test that constructs two vectors of, say, 5k–20k lines with small edits and asserts it completes within a reasonable time budget (not strict; just ensures it finishes).
2) If time-based tests are flaky, use a “doesn’t OOM / doesn’t hang” style test.
### Constraints
1) No strict timing assertions that will fail on slow CI (you’ve excluded CI for now, but still keep it stable).
### Acceptance criteria
1) Test runs reliably on your dev machines.

## Milestone 5 Exit Criteria
Milestone 5 is complete when:
1) A Qt-free diff module exists with stable API 
2) It produces line-based hunks and per-line classification 
3) Whitespace modes work: `Exact` / `IgnoreTrailing` / `IgnoreAll` 
4) Golden fixture tests exist and pass 
5) Output is deterministic for identical inputs



