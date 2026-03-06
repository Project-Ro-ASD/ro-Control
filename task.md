# ro-Control Task Board

Last updated: 2026-03-06

This file tracks project execution with a lightweight To-Do flow.

## Status Legend

- `[ ]` Backlog
- `[-]` In Progress
- `[r]` In Review
- `[x]` Done

## In Progress

- [-] Open PR from `feature/cmake-setup` to `dev` and complete review/merge.
  - Owner:
  - Branch: feature/cmake-setup
  - Notes: Feature branch now includes core backlog fixes from this board.

## Backlog (Next Improvements)

- [ ] Add richer Fedora integration tests with controlled mocks/fixtures for `dnf`, `mokutil`, `grubby`, and `nvidia-smi` command variants.
  - Owner:
  - Branch:
  - Notes: Current integration tests validate availability/health paths and skip safely when tools are absent.

- [ ] Introduce cancellable async job queue for long-running privileged tasks (UI progress + cancel button).
  - Owner:
  - Branch:
  - Notes: Timeout/retry support exists in `CommandRunner`; next step is job orchestration UX.

- [ ] Add real screenshots from the running app for AppStream/README.
  - Owner:
  - Branch:
  - Notes: Placeholder screenshot asset currently used.

## Done

- [x] Fix `.github/workflows/release.yml` indentation and validate workflow syntax.
  - Branch: feature/cmake-setup

- [x] Sync branches: merge `main` back into `dev` to remove branch drift.
  - Branch: dev
  - Ref: `dev` fast-forwarded to `origin/main` (`f8a1dc3`).

- [x] Replace `DnfManager` stub with functional package operation wrapper.
  - Files: `src/backend/system/dnfmanager.h`, `src/backend/system/dnfmanager.cpp`

- [x] Replace `PolkitHelper` stub with privilege helper methods.
  - Files: `src/backend/system/polkit.h`, `src/backend/system/polkit.cpp`

- [x] Add integration tests for Fedora-related system flows.
  - Files: `tests/test_system_integration.cpp`, `tests/CMakeLists.txt`

- [x] Convert command execution to support timeout/retry policy and structured run signals.
  - Files: `src/backend/system/commandrunner.h`, `src/backend/system/commandrunner.cpp`

- [x] Add RPM CI validation job (`rpmbuild` + `rpmlint`).
  - File: `.github/workflows/ci.yml`

- [x] Add release checklist documentation.
  - File: `docs/RELEASE.md`

- [x] Introduce Qt Linguist i18n scaffolding (`.ts` files + CMake wiring).
  - Files: `i18n/ro-control_en.ts`, `i18n/ro-control_tr.ts`, `i18n/README.md`, `CMakeLists.txt`

- [x] Improve release/readme metadata and packaging quality.
  - Files: `data/icons/ro-control.metainfo.xml`, `packaging/rpm/ro-control.spec`, `README.md`, `README.tr.md`

- [x] Add icon/screenshot assets for open-source presentation baseline.
  - Files: `data/icons/hicolor/scalable/apps/ro-control.svg`, `docs/screenshots/monitor-overview.svg`

- [x] Keep CI checks green after all updates.
  - Validation: local `clang-format --dry-run --Werror`, build, and `ctest` all passing.
