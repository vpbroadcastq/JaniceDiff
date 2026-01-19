# BenDiff

BenDiff is a native GUI diff tool (Qt Widgets) targeting Windows and Linux.

## Docs
- v1 spec (frozen): [docs/spec_v1_frozen.md](docs/spec_v1_frozen.md)
- Milestones: [docs/milestones.md](docs/milestones.md)
- Current milestone tasks: [docs/milestone_0.md](docs/milestone_0.md)

## Repository Layout
- `src/app/` — Qt Widgets UI shell (thin)
- `src/core/` — non-Qt business logic (mostly empty in Milestone 0)
- `src/common/` — small utilities shared across app/core
- `tests/` — unit tests (harness comes in a later M0 task)
- `cmake/` — helper CMake modules
- `third_party/` — vendored deps if needed (prefer empty)
- `docs/` — specifications, milestones, notes

Build + run instructions will be added in Milestone 0.
