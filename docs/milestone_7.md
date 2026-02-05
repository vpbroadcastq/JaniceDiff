# Milestone 7 — Task Breakdown
**Theme:** Navigation + refresh semantics + UX polish

## M7-T1: Define “change navigation” API over DiffResult (Qt-free)
**Objective:** Create a small abstraction that identifies "change locations" and supports next/prev navigation without UI knowledge.
### Deliverables
1) In core (Qt-free), add:
    * `struct ChangeLocation { std::size_t renderRowIndex; /* or hunk index + offset */ };`
    * `std::vector<ChangeLocation> EnumerateChanges(const DiffResult&, /* maybe render model */);`
    * `std::optional<std::size_t> NextChangeIndex(currentIndex, changes)` and `Prev` analog
1) Decide navigation unit:
    * Prefer hunk-based navigation for programmer tools (next/prev hunk), consistent with "Next/Prev change"
### Constraints
1) Do not depend on Qt widgets
2) Should work for both inline and side-by-side render models
### Acceptance criteria
1) Unit tests: given a known diff, enumerated changes match expected hunks and order.

## M7-T2: UI wiring: Next/Prev change actions move view + update selection
**Objective:** Make toolbar actions actually navigate changes.
### Deliverables
1) Hook toolbar/menu shortcuts:
    * Next Change
    * Previous Change
2) On activation:
    * Compute next/prev change target for the currently selected file
    * Scroll viewer(s) to the target row
    * Optionally highlight the current hunk region
### Constraints
1) Must work in both inline and side-by-side modes
2) If no changes (files equal), actions no-op and status bar reflects "0 changes"
### Acceptance criteria
1) Manual test: repeatedly pressing Next cycles through hunks and scrolls accordingly.

## M7-T3: Status bar diff stats (per selected file)
**Objective:** Populate status bar with meaningful stats.
### Deliverables
1) When a file is selected and diff computed:
    * Status bar shows:
        * Mode: Repo / Folder
        * Paths (repo root or folder roots)
        * Diff stats: e.g., "Hunks: N, +A, -D" (added/deleted line counts)
        * Current hunk position when navigating: "Hunk i/N"
        * For binary/unsupported:
            * Show "Binary/unsupported" instead of line stats
        * For deleted files:
            * Counts reflect deletions appropriately
### Constraints
1) Do not add persistent settings (Milestone 8)
2) Stats should be computed from existing diff/render model, not recomputed expensively
### Acceptance criteria
1) Manual tests on simple diffs show accurate numbers.
2) Navigating updates "Hunk i/N".

## M7-T4: Automatic refresh behavior in repo mode
**Objective:** Implement spec’s "Refresh is automatic when a git repo is open."
## Deliverables
1) Define what "automatic" means in v1:
    * Recommended: poll for working tree changes on a timer, and refresh file list + selected diff if changes detected.
2) Implement a `QTimer` (e.g., 1–2 seconds) that triggers:
    * `git status --porcelain=v1 refresh`
    * Compare to last seen status; if changed, update file list model
    * If currently selected file is still present, refresh its diff view
3) Respect UI responsiveness:  If git invocation blocks, move to a worker thread (optional but recommended)
### Constraints
1) Timer runs only when repo mode active
2) Avoid heavy diff recomputation if nothing changed
### Acceptance criteria
1) Modify a tracked file externally; within the polling interval the file list updates.
2) If the selected file changes, its diff view updates without manual refresh.

## M7-T5: Manual refresh behavior in folder mode (F5 + toolbar)
**Objective:** Confirm spec’s folder mode refresh remains manual and stable. 
### Deliverables
1) Ensure:
    * F5 triggers directory re-scan and list refresh
    * Toolbar refresh triggers same
2) Ensure:
    * No background polling in folder mode
### Acceptance criteria
1) Changing folder contents does not update list until F5/Refresh pressed.  (You likely implemented part of this in Milestone 3; this milestone ensures it still works after adding auto refresh in repo mode.)

## M7-T6: Reasonable default focus and selection behavior
**Objective:** Polish UX so the tool is fast to operate with keyboard/mouse.
### Deliverables
1) On opening repo/folders:
    * File list gets keyboard focus
    * First file auto-selected if list non-empty
    * Diff view renders immediately for that selection
2) After refresh:
    * Preserve selected file if it still exists
    * If not, select the nearest neighbor (next item, else previous)
3) Ensure group headers (directory groups) are not selectable (or selection skips them)
### Acceptance criteria
1) Opening a repo shows a diff immediately without extra clicks (when changes exist).
2) Refresh does not unexpectedly clear selection unless necessary.

## M7-T7: Robustness for “no changes” and error edge cases
**Objective:** Make common edge cases graceful, not confusing.
### Deliverables
1) Repo mode:
    * Clean working tree → list is empty and diff panes show “No changes”
    * Git errors mid-session (e.g., repo removed, git missing):
    * Show modal dialog
    * Stop auto-refresh timer until repo reopened
2) Folder mode:
    * If roots disappear or become unreadable:
        * Show modal dialog
        * Clear list
### Acceptance criteria
1) No crashes in these scenarios.
2) UI messaging is clear and status bar reflects state.

## M7-T8: Minimal integration tests / fixture verification for navigation and refresh logic
**Objective:** Add tests where practical without UI automation.
### Deliverables
1) Qt-free unit tests:
    * Change enumeration from diff results
    * Next/prev behavior
    * Stats computation (+/- and hunk counts)
2) Optional integration test harness (non-UI):
    * A small temporary repo created during test run to validate status refresh (only if you are comfortable; otherwise defer)
### Acceptance criteria
1) ctest passes on Windows and Linux.
2) Core navigation logic is covered.

## Milestone 7 Exit Criteria
Milestone 7 is complete when:
1) Next/Prev change navigation works end-to-end 
2) Status bar displays diff stats and updates during navigation 
3) Repo mode refresh is automatic (polling or equivalent) 
4) Folder mode refresh remains manual (F5/toolbar) 
5) Default focus/selection behavior makes the tool usable immediately 
6) Edge cases (no changes, errors) are graceful



