# Milestone 0 prompts

We are implementing the BenDiff project.  The v1 spec is spec_v1_frozen.md.  The high-level milestones are described in milestones.md.  

## M0-T1: Initialize repository structure and conventions
You are implementing Milestone 0 Task M0-T1 for the BenDiff project, described in milestone_0.md.  Create the initial repository directory structure and add the frozen spec and milestones documents under docs/. Do not implement any Git logic, diff logic, or non-trivial UI yet.  I have already initialized the git repo.

## M0-T2: Create CMake build that compiles a Qt Widgets executable
Implement Milestone 0 Task M0-T2: create a CMake build that compiles a Qt6 Widgets executable named bendiff on Windows and Linux.

## M0-T3: Minimal Qt Widgets application that launches cleanly
Implement Milestone 0 Task M0-T3: create a Minimal Qt Widgets application that launches cleanly


## M0-T4: Logging infrastructure (minimal but durable)
Implement Milestone 0 Task M0-T4: Add logging that will be useful during AI iteration and later debugging.

## M0-T5: Cross-platform “developer build” documentation
Implement Milestone 0 Task M0-T5: Ensure rebuild from scratch is documented.

## M0-T6: Unit test harness skeleton (even if there are no real tests yet)
Implement Milestone 0 Task M0-T6: Establish the "tests exist from day 1" discipline stated in the spec `spec_v1_frozen.md`



(Note we're going 3, 4, 1, 2, 5, 6))

## M1-T3: Command-line parsing and invocation classification
Implement Milestone 1 Task M1-T3:  Implement argument parsing and mode detection as specified in milestone_1.md.  Review the full spec in spec_v1_frozen.md for context and aditional detail.  Note that tasks 1 and 2 have not been completed yet (we are not doing the Milestone 1 tasks in order).

## M1-T4: Error handling and exit-code policy
Implement Milestone 1 Task M1-T4:  Enforce the spec’s error-handling contract.  Review the full spec in spec_v1_frozen.md for context and aditional detail.  Note that tasks 1 and 2 have not been completed yet (we are not doing the Milestone 1 tasks in order).

## M1-T1: Main window UI shell (menus, toolbar, central container)
Implement Milestone 1 Task M1-T1:  Replace the empty Milestone 0 window with the structural UI defined in the spec.  Review the full spec in spec_v1_frozen.md for context and aditional detail.  Note that tasks 3 and 4 have been completed, but not task 2 (we are not doing the Milestone 1 tasks in order).

## M1-T2: Central pane layout and mode switching (2-pane vs 3-pane)
Implement Milestone 1 Task M1-T2:  Implement the pane topology rules from the spec.  Review the full spec in spec_v1_frozen.md for context and aditional detail.  Note that tasks 1, 3, and 4 have been completed (we are not doing the Milestone 1 tasks in order).

## M1-T5: Repo / folder selection via UI (File menu)
Implement Milestone 1 Task M1-T5:  Enable manual mode entry via the UI, even before logic exists.  Review the full spec in spec_v1_frozen.md for context and aditional detail.

## M1-T6: Placeholder file list pane and selection wiring
Implement Milestone 1 Task M1-T6:  Lay groundwork for Milestone 2 without implementing Git logic.  Review the full spec in spec_v1_frozen.md for context and aditional detail.





## M2-T1: Define core data model for “changed file” entries (Qt-free)
Implement Milestone 2 Task M2-T1:  Create a minimal, stable representation of repo file changes to drive UI and tests.  Review the full spec in spec_v1_frozen.md for context and aditional detail.  

## M2-T2: Repo discovery and validation (CWD + explicit path)
Implement Milestone 2 Task M2-T2:  Implement "find a git repo" semantics for `bendiff` and `bendiff <PathToGitRepo>`.  Review the full spec in spec_v1_frozen.md for context and aditional detail.  

## M2-T3: Git executable discovery and process runner (Qt-free utility)
Implement Milestone 2 Task M2-T3:  Create a robust way to invoke git and capture `stdout`/`stderr` for parsing and diagnostics.  Review the full spec in spec_v1_frozen.md for context and aditional detail.  

## M2-T4: Invoke git status --porcelain=v1 in repo mode
Implement Milestone 2 Task M2-T4:  Retrieve uncommitted changes in the repo root.  Review the full spec in spec_v1_frozen.md for context and aditional detail.  

## M2-T5: Porcelain v1 parser (including renames/deletes; excluding ignored and submodules)
Implement Milestone 2 Task M2-T5:  Parse the porcelain output into `RepoStatus.files` obeying inclusion rules.  Review the full spec in spec_v1_frozen.md for context and aditional detail.  

## M2-T6: Sorting and grouping for the file list pane (flat list with directory groups)
Implement Milestone 2 Task M2-T6:  Implement the ordering/grouping contract for the left pane.  Review the full spec in spec_v1_frozen.md for context and aditional detail.  













