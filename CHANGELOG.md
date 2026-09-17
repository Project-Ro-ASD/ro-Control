# Changelog

All notable changes to **ro-Control** will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [v1.3.1] - 2026-09-07

### Added
- **Localized GPU Names:**
  - Friendly, localized names for common NVIDIA GPU models across the system, NVIDIA, and monitor providers, now shown on the Monitor and System pages (TR/DE/ES).
- **Fan Rescan Wizard V2:**
  - Redesigned discovery wizard with a discovered cooling channel list (per-fan type icon, live RPM/speed, Controllable/Monitored badges).
  - **Quick Acoustic Test (4s):** briefly forces a fan to 100% to verify audible airflow, then automatically restores the previous fan control state. Supported for CPU and GPU channels via `testFanSpeedForFan()` / `restoreFanControlForFan()`.
  - Clear empty state when hardware exposes no direct PWM tachometers (ACPI standard thermal mode).
- **Hardware re-detection:** `rescanHardware` forces a full hardware and platform re-detection from the GUI.
- **zram/zswap telemetry:** summary card with compressed memory stats, compression ratio, and physical footprint.
- **Modernized diagnostic report:** dynamic report cards, search/filter, and Markdown preview.

### Changed
- Fan RPM polling now backs off from five seconds to 30 seconds when the
  optional `nvidia-settings` endpoint is unavailable, while keeping sysfs
  telemetry and thermal safety evaluation live. This avoids repeated UI-thread
  subprocess timeouts on unsupported or headless sessions.
- The Monitor page now dispatches manual GPU refreshes asynchronously instead
  of running `nvidia-smi` on the QML/UI thread.
- Driver and System pages revamped with hover feedback and responsive card/grid layouts; the GPU driver card hides cleanly when no NVIDIA GPU is detected.
- Desktop entry uses the standard `SingleMainWindow` key and a single `Settings;HardwareSettings;Qt` menu category (no duplicate menu items).
- Documentation: new `docs/CONFIGURATION.md` (QSettings reference); build/install/packaging docs now target Fedora-based Ro-ASD only, with Debian and Arch packaging removed.
- FIFO/control settings and CLI reference aligned with the D-Bus surface (`SetFanSmoothing`, `SetClockOffsets`, `GetFanModes`, `GetGpuHealth`, ...).

### Fixed
- Fan telemetry: unreachable `nvidia-settings` fallback branches for RPM and speed queries are now actually reached.
- Power profiles verify the active `tuned` state and reject stale system profile data.
- zram telemetry accuracy and fan channel RPM/hardware reporting.
- Potential static-initialization crash in the updater; CPU thermal warning threshold handling.
- Redundant factory-success check in the `nvidia-settings` RPM query cleanup.

### Tests
- Expanded unit/integration coverage; 12/12 test targets passing (PowerController, HealthGuard, GpuMonitor multi-GPU/process, D-Bus service, GPU-name localization, CLI, metadata).

---

## [v1.3.0] - 2026-09-01

### Added
- **Multi-GPU & Process Management Subsystem:**
  - Multi-GPU enumeration, dynamic device selection, and per-card monitoring (`ro-control gpus [--json]`, `ro-control select-gpu <index>`).
  - Active GPU compute and display process tracking with VRAM consumption (`ro-control processes [--json]`).
  - Safe GPU process termination helper via PolKit (`ro-control kill-process <pid>`).
- **GPU Clock Offsetting & Tuning:**
  - Core and memory clock offset adjustment via `PowerController` (`ro-control power set-clocks <core_mhz> <mem_mhz>`).
  - D-Bus slot `SetClockOffsets(int coreMhz, int memMhz)`.
- **Fan Smoothing & Anti-Hunting Engine:**
  - Configurable ramp-up/ramp-down rate limits and directional hysteresis in `FanController` to eliminate rapid fan acoustic cycling (`ro-control fan set-smoothing <on|off> [ramp_up] [ramp_down] [hysteresis]`).
  - D-Bus slot `SetFanSmoothing(bool enabled, int rampUp, int rampDown, int hysteresis)`.
- **Extended Hardware Telemetry:**
  - Real-time Hotspot and Memory Junction temperature sensors across NVML backend, D-Bus telemetry payload, and QML dashboard.
- **Modernized Driver UI Experience:**
  - Replaced legacy button layouts with modern typography-driven `DriverActionTile` components.
  - Accurate stack descriptions for **NVIDIA Official Release** (`akmod-nvidia`) and **Community Open-Source Release** (`akmod-nvidia-open`).
  - Streamlined `ModernMiniButton` and `ModernDialogButton` controls for activity log and confirmation modals.
- **GPU Power & TDP Management Subsystem:**
  - Dynamic power limit control (`PowerController`) via Polkit and `nvidia-smi`.
  - Power preset switching (`Eco`, `Balanced`, `Performance`, `Custom`).
  - NVIDIA Driver persistence mode toggle and status tracking.
  - CLI integration: `ro-control power status [--json]`, `ro-control power set-limit <watts>`, `ro-control power set-preset <preset>`, `ro-control power set-persistence <on|off>`.
- **System Tray & Thermal Health Guard:**
  - `HealthGuard` subsystem with configurable thermal warning and critical limits.
  - Native desktop notifications (`QSystemTrayIcon::showMessage`) for critical overheating warnings.
  - Live tooltip with GPU temperature, fan RPM, and live power draw.
  - Context menu with quick Fan profile and Power preset switching.
- **Sentinel D-Bus IPC Service & Daemon Mode:**
  - Native D-Bus service registered at `io.github.ProjectRoASD.rocontrol` (`/io/github/ProjectRoASD/rocontrol`).
  - Methods for external telemetry queries (`GetTelemetry`, `GetGpuDevices`, `GetGpuProcesses`, `GetThermalStatus`, `GetGpuHealth`) and hardware control (`SelectGpu`, `SetFanMode`, `SetFanSpeed`, `SetFanSmoothing`, `SetPowerLimit`, `SetClockOffsets`, `SetPersistenceMode`).
  - Signals: `ThermalAlert`, `TelemetryUpdated`.
  - Headless daemon CLI command: `ro-control --daemon`.
- **Test Suite Expansion:**
  - Comprehensive unit test suites for `PowerController`, `HealthGuard`, `GpuMonitor` multi-GPU/process features, and `RoControlDBusService` (12/12 test targets passing 100%).

---

## [v1.2.0] - 2026-08-27

### Added
- **Dedicated Cooling & Fan Management Subsystem:**
  - Hardware-aware multi-fan detection for CPU and GPU fans via `hwmon` and vendor interfaces.
  - Interactive fan cards with real-time RPM telemetry, percentage indicators, and mode presets (Auto, Quiet, Balanced, Performance).
  - Custom temperature-to-RPM fan curve mapping and persistent configuration.
- **Architecture & CLI Enhancements:**
  - `ro-control --fan-status`, `--set-fan-speed`, and related cooling CLI commands.
  - Unified fan topology reporting across diverse hardware configurations.
- **Build & CI Automation:**
  - CMake custom targets `format` (`clang-format -i`) and `check-format` (`clang-format --dry-run --Werror`).
  - Automated dual-architecture (`x86_64` and `aarch64`) RPM packaging and release pipeline.

### Fixed
- **Headless & Virtualized Runner Compatibility:**
  - Fixed topology initialization in environments without dedicated GPU fan control (virtualized/CI runners), ensuring robust test execution.
- **UI & UX Polish:**
  - Streamlined cooling view layout, cleaned up duplicate setting icons, and aligned GPU naming conventions.
  - Improved responsive sizing and theme integration under KDE Plasma / Wayland.
- **Code Quality:**
  - Standardized C++20 formatting across all source and test files.

---

## [v1.1.0] - 2026-05-10

### Added
- Standalone architecture RPMs for `x86_64` and `aarch64` without companion noarch requirement.
- Full AppStream metadata integration with live screenshots and localized descriptions.
- Shell completions for Bash, Zsh, and Fish.

### Fixed
- Polkit helper permission rules alignment for non-root system control actions.
- Target Fedora 43+ build matrix synchronization.

---

## [v0.2.1] - 2026-03-30

### Added
- Initial power profile management and battery charge threshold control.
- Core D-Bus service integration and daemon lifecycle management.
- Qt6 / QML desktop shell UI foundation.
