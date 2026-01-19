# v1 Specification Skeleton
## Description
This is a file diffing application for use by programmers.  The program is expected to be used in the context of a VCS repo (almost always Git).  The user will open the program and point it at a git repository and it will automatically diff the uncommitted changes (both tracked and untracked files).  It is not intended to be used to diff two arbitrary files (though this functionality may be added in a future version).  It is also intended to be used with the `git difftool --dir-diff` command where two input folders are diffed.

It is a traditional native gui application with a title bar, menu bar, and toolbar with buttons.  Below the toolbar are either two or three panes depending on the application mode.  The leftmost pane is a (vertical) list of files being diffed.  When a file is selected in the list (only one may be selected at once), and the application is in two-pane mode, the right hand pane shows the inline diff of the uncommitted version of the file with the latest commit (or if launched by `git difftool` it shows the diff of the files in the two directories).  In three-pane mode, the two right hand panes depict the same diff of the file in side-by-side mode.

## Scope
### In scope (v1)
1) Two-way diff:
    * Compare sets of files with uncommitted changes (comparison against the last committed version)
    * Compare two directories recursively (for git difftool --dir-diff compatibility).
2) Read-only UI:
    * No editing, no merge/copy operations.
3) Diff granularity
    * Line-based diff only (no intra-line highlights).
4) Platforms:
    * Windows + Linux.
5) Encoding:
    * UTF-8 only.
6) No file size limits:
    * No enforced warnings/limits in v1 spec.
7) Git integration
    * Must be able to detect which files have been modified (tracked and untracked)

### Out of scope (v1)
1) 3-way merge, conflict resolution.
2) Folder operations (copy/delete/sync).
3) Intra-line diff, syntax highlighting (optional later).
4) Non-UTF-8 encodings.
5) VCS integration with non-Git VCS

## External Interface Requirements
### Command line invocation (required for Git)
#### CLI contract:
1)  `bendiff`
    * Program reads the working directory and looks for a git repo.  If it isn't a git repo, no files are listed.
2) `bendiff <PathToGitRepo>`
    * Lefthand pane lists all files with uncommitted changes.  Selecting a file diffs the uncommitted version with the latest committed vesrion.
3) `bendiff <leftPath> <rightPath>`
    * Used with `git difftool --dir-diff`
    * Real directories containing a subset of changed files (often in temp dirs).  Potentially containing symlinks (Git may symlink in dir-diff mode depending on platform/config)
    * Treat entries as normal file system items (follow OS defaults).
    * Left hand pane lists all files in both folders.  Selecting a file diffs the version in the two folders.
    * If a file cannot be read → show “unreadable” status in folder view.

#### Exit code (v1):
- 0 on successful launch (even if differences exist)
- 2 invalid args / path mismatch
- 3 runtime error opening paths
Runtime errors (can't find the `git` executable, can't find a git repo anywhere, etc) are surfaced via modal dialogs.  Application does not crash.

## UI Requirements
### Main window
1) Menu bar: File, View, Diff, Help
    * File>open repo - Allows the user to select a git repo to open
    * File>open folders - Allows the user to select two folders to open (diffs all files across the two folders)
2) Toolbar: Open repo, Open folders, Refresh, Next/Prev change
    * Dropdown that specifies whitespace-handling mode ("exact", "ignore trailing whitespace", "ignore all whitespace")
    * A button toggles 2-pane (inline diff) and 3-pane (side-by-side diff) mode
    * “Ignore whitespace” toggle(s)
3) Central: Two or three panes depending on whether the user wants an inline diff or a side-by-side diff.  The leftmost pane is always the list of files being diffed.  Selecting a file causes the one right hand pane (or two right hand panes) to display the diff.
4) Status bar: current mode (File/Folder), path(s), diff stats when relevant

### File list pane
A vertical list of files being diffed.  Selecting a file causes the diff to appear in the righthand side pane or panes.  Files are grouped by directory (directories sorted alphabetically) and then within each directory group sorted alphabetically.  Although there is grouping by directory, it isn't a tree display.  It is a flat list with distinct groups.

### Diff pane (or panes)
In inline diff mode, the diff is displayed with alternating left/right blocks.  In both modes, added lines have a gree background highlight; deleted lines have a red background highlight.  
1) Line numbers
2) Highlight: Added/Removed/Changed line blocks (block-level background)


## Core Logic Requirements
1) Directory comparison algorithm (v1):  Walk both directories recursively.  Build union set of relative file paths (directories may be shown or not; v1 can be file-only list).  Do not ignore "special" directories like `.git`.
2) File diff engine (v1):  Load file as UTF-8 text, normalize line endings for internal representation (treat CRLF/LF as line breaks)
3) Compute line-based diff:
    * Minimal required output: list of “hunks” and per-line classification
    * Rendering uses the classification to highlight blocks and enable navigation.
    * (Implementation detail: Myers/LCS variant is typical; keep the engine Qt-free if practical.)
4) Binary detection
    * If UTF-8 decode fails, mark as "Binary/Unsupported for text diff"
    * Still allow byte-equality check for Same/Different in folder list
5) Refresh semantics
    * Refresh is automatic when a git repo is open.
    * In two-folder mode, refresh must be done manually either by pressing `F5` or by clicking a refresh button on the toolbar.

### Detection of files to be included in the set being diffed
In two-folder mode, all files in both folders are included.  In git repo mode, only files known to git are shown.  Specifically, 
- Deleted files are included.  In inline mode, the diff pane renders as if every single line has been deleted.  In side-by-side mode, the right side of the diff shows that all lines have been deleted.
- Renamed files appear in the list annotated as renamed
- Ignored files (via `.gitignore` are excluded, because they aren't "known to git"
- Submodules are ignored (v1)
- Binary files are included.  A binary file just shows an "unsupported" message in the diff pane(s).

### Persisted Settings
1) Window geometry/state
2) Last used diff options (e.g., ignore whitespace mode)


## Implementation and development notes
To the greatest extent possible, the application should be written to separate ui and business logic.  It should be possible to port the application to a different ui toolkit, for example, without having to rewrite all the business logic from scratch.  
- Git is used via parsing `git status --porcelain=v1` rather than using libgit2


## Testing
- The business logic should be unit testable, and unit tests should be written as the application is developed
- Fixture-based tests for directory comparison




