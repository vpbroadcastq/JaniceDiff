# Milestone 1 — Task Breakdown

**Theme:** Application shell + command-line entry + mode selection
**Explicit non-goals:** Git parsing, directory walking, diffing, rendering

## M1-T1: Main window UI shell (menus, toolbar, central container)
**Objective:** Replace the empty Milestone 0 window with the structural UI defined in the spec.
### Scope
1) UI scaffolding only
2) No real behavior behind actions yet
### Deliverables
1) MainWindow updated to include:
2) Menu bar with menus:
    * File
    * View
    * Diff
    * Help
3) File menu actions:
    * Open Repo…
    * Open Folders…
4) Toolbar with actions:
    * Open Repo
    * Open Folders
    * Refresh
    * Next Change
    * Previous Change
5) Toolbar widgets (Use simple QWidget placeholders (e.g., colored frames or labels)):
    * Whitespace-handling dropdown with values:
        * Exact
        * Ignore trailing whitespace
        * Ignore all whitespace
    * Toggle button for:
    * Inline diff (2-pane)
    * Side-by-side diff (3-pane)
    * Central widget:
    * Placeholder container for:
        * File list pane (left)
        * One or two diff panes (right)
### Constraints
1) No signals wired to real logic yet
2) Buttons may log “Not implemented”
### Acceptance criteria
1) App launches and shows menus + toolbar
2) Central area visibly has left/right pane placeholders
3) No crashes when clicking menu/toolbar items

## M1-T2: Central pane layout and mode switching (2-pane vs 3-pane)
**Objective:** Implement the pane topology rules from the spec.
### Scope
1) Layout behavior only
2) No file list contents
3) No diff rendering
### Deliverables
1) Implement central layout such that:
2) Left pane (file list placeholder) is always present
3) Right side switches between:
4) Inline mode: single diff pane
5) Side-by-side mode: two diff panes
    * Pane mode controlled by:
        * Toolbar toggle button
        * Default mode on startup:
6) Inline diff (2-pane)
### Constraints
1) Pane contents may be simple labels ("File list", "Diff view")
2) No persistence yet
### Acceptance criteria
1) Toggling mode updates the layout immediately
2) Layout does not leak widgets or crash
3) Status bar updates to reflect current mode (File / Folder, Inline / Side-by-side)

## M1-T3: Command-line parsing and invocation classification
**Objective:** Implement argument parsing and mode detection exactly as specified.
### Scope
1) Argument parsing only
2) No Git calls yet
3) No directory walking
### Deliverables
1) Parse argv and classify into one of:
2) Repo mode (implicit):
3) `bendiff`
4) Use CWD
5) Repo mode (explicit):
    * `bendiff <PathToGitRepo>`
    * `Folder-diff mode:`
    * `bendiff <leftPath> <rightPath>`
6) Invalid invocation

Store the detected mode in an internal AppMode enum:
    * RepoMode
    * FolderDiffMode
    * Invalid
### Constraints
1) No filesystem validation yet beyond:
2) Argument count
3) Basic existence check
4) Do not run Git yet
### Acceptance criteria
1) Running with 0, 1, or 2 args selects correct mode
2) Invalid arg counts result in Invalid mode
3) Mode is logged via the logging system

## M1-T4: Error handling and exit-code policy
**Objective:** Enforce the spec’s error-handling contract.
### Scope
1) UI + process exit only
2) No Git or diff errors yet
### Deliverables
1) For Invalid mode:
    * Show modal error dialog
    * Exit with code 2
2) For runtime startup errors (simulated for now):
    * Show modal error dialog
    * Exit with code 3
3) For valid modes:
    * Exit code remains 0 even if no files are shown
### Constraints
1) Use Qt modal dialogs (QMessageBox)
2) Exit codes must match the spec exactly
### Acceptance criteria
1) Invalid invocation produces dialog + exit code 2
2) Forced runtime error path produces dialog + exit code 3
3) Valid invocation exits normally

## M1-T5: Repo / folder selection via UI (File menu)
**Objective:** Enable manual mode entry via the UI, even before logic exists.
### Scope
1) File dialogs only
2) No Git or directory processing
### Deliverables
1) File → Open Repo…
    * Shows directory chooser
    * Sets application mode to RepoMode
    * Clears and resets central panes
2) File → Open Folders…
    * Prompts for two directories (either sequential dialogs or a custom dialog)
    * Sets mode to FolderDiffMode
    * Clears and resets central panes
3) Status bar reflects chosen mode and paths
### Constraints
1) No validation beyond directory existence
2) No file list population yet
### Acceptance criteria
1) Menu actions switch modes correctly
2) Paths selected are displayed in status bar
3) Switching modes resets previous UI state cleanly

## M1-T6: Placeholder file list pane and selection wiring
**Objective:** Lay groundwork for Milestone 2 without implementing Git logic.
### Scope
1) UI wiring only
2) Dummy data allowed
### Deliverables
1) Left-hand file list widget (e.g., QListView or QTreeView) (Populate with temporary placeholder entries depending on mode)
    * Repo mode: "(repo files will appear here)"
    * Folder mode: "(folder diff files will appear here)"
    * Selection handling:
        * Selecting an entry updates the diff pane placeholders
        * Only one selection allowed
### Constraints
1) No real file paths yet
2) No diff computation
### Acceptance criteria
1) File list exists and is selectable
2) Selection triggers visible change in diff pane labels
3) No crashes on repeated selections

## Milestone 1 Exit Criteria
Milestone 1 is complete when all of the following are true:
1) All CLI invocation forms work and select the correct mode 
2) Invalid invocation produces dialog + correct exit code
3) Main window matches spec structurally (menus, toolbar, panes)
4) Inline vs side-by-side mode switching works
5) Repo and folder modes can be entered via CLI or menu
6) No Git, diff, or directory logic has been introduced prematurely




