# Optimization & Modularization Plan

This plan turns the architecture audit into small, independently verifiable
deliverables. It does not treat smaller source files as a performance win by
itself: extraction must either isolate blocking work, reduce repeated work, or
make behaviour easier to test.

## Phase 0 — Baseline and Safety Net

- [x] Record the current architecture audit and identify retired QML sources.
- [x] Confirm the build, C++ formatting check, and all 13 CTest targets pass.
- [ ] Capture startup time, idle CPU, memory use, and refresh latency on a
  supported Fedora/NVIDIA machine before changing telemetry behaviour.
- [ ] Add focused regression coverage before each extracted backend or QML
  surface changes ownership.

**Exit criteria:** a baseline exists for real hardware, and every following
phase keeps the existing build, formatting, and CTest gates green.

## Phase 1 — Keep Blocking Driver Work Off the UI Thread

- [x] Route every QML-initiated GPU telemetry refresh through the existing
  single-flight asynchronous path.
- [x] Make diagnostic-report generation wait for the asynchronous telemetry
  result instead of reading stale values or blocking the UI thread.
- [ ] Move fan-control discovery and command execution behind cancellable,
  timeout-bound worker jobs; preserve emergency thermal behaviour.
- [x] Coalesce concurrent refresh requests and expose an explicit busy/error
  state to QML.

**Exit criteria:** normal refresh, diagnostic report generation, a fan action,
and a timed-out driver command remain responsive and have automated coverage.

## Phase 2 — Split Backend Responsibilities

- [ ] Split `FanController` into capability discovery, telemetry, profile
  persistence/import-export, and hardware-write collaborators without changing
  its public QML/D-Bus contract.
- [ ] Split `GpuMonitor` into an NVIDIA query/parser adapter, generic Linux
  fallback reader, and process/device inventory service.
- [ ] Split `SystemInfoProvider` into platform probes and diagnostic-report
  formatting/preferences.
- [ ] Extract CLI command execution, daemon bootstrap, and GUI/tray bootstrap
  from `main.cpp`.

**Exit criteria:** no public contract changes without a matching test; each
extracted unit has a narrow dependency surface and isolated tests.

## Phase 3 — Split QML by User Task

- [ ] Extract Driver page action tiles, operation log, and confirmation dialogs.
- [ ] Extract System page diagnostic report preview and firmware-restart flow.
- [ ] Extract Monitor telemetry summary, power controls, and process list.
- [ ] Extract Fan page channel cards, curve summary, and settings subpanels.
- [ ] Keep theme tokens and shared controls in `components/`; do not duplicate
  button, focus, loading, or error styling in pages.

**Exit criteria:** pages retain responsive, empty/error, keyboard-focus, and
loading states, with no user-visible regression.

## Phase 4 — Reduce Telemetry Cost

- [x] Use adaptive polling based on page visibility, hardware availability,
  thermal state, and active operations.
- [ ] Cache device topology and process inventory independently from fast
  temperature/power metrics.
- [ ] Avoid emitting identical QVariant/QML model data and unnecessary Canvas
  repaints.
- [ ] Evaluate a persistent NVML-backed telemetry adapter only after profiling
  shows `nvidia-smi` process startup is a material bottleneck.

**Exit criteria:** measured idle cost and refresh latency improve on target
hardware without weakening fallback telemetry or thermal safeguards.

## Phase 5 — Delivery Evidence

- [ ] Run formatting, build, CTest, RPM metadata validation, and install-tree
  checks on Fedora.
- [ ] Verify a real desktop flow: refresh telemetry, inspect a report, change a
  supported fan mode, and recover from unavailable NVIDIA telemetry.
- [ ] Update architecture, build, configuration, and release documentation for
  every completed public-surface change.

**Exit criteria:** the measured results, user-flow evidence, and package
validation are documented with the completed release.
