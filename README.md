# ro-Control

**Central Hardware, Graphics, and Thermal Management Suite for Ro-ASD.**

---

## Overview

**ro-Control** is the central hardware and graphics control suite developed for the **Ro-ASD** operating system. It integrates NVIDIA driver lifecycle management, intelligent multi-fan cooling control, live hardware diagnostics, and background thermal safety protection into a native desktop experience and a headless CLI.

### Core Capabilities

- **Automated Driver Lifecycle:** One-click NVIDIA graphics driver detection, installation, updates, MOK Secure Boot validation, and kernel module recompilation (`akmods`, `dracut`).
- **Intelligent Cooling & Multi-Fan Suite:** Hardware-aware discovery across GPU, CPU, and chassis fans. Features 6 cooling modes (`auto`, `silent`, `balanced`, `performance`, `manual`, `custom`), custom temperature curves with hysteresis, 0 RPM zero-noise idle detection, and emergency thermal overrides.
- **Real-Time Hardware Diagnostics:** Sub-second telemetry tracking for GPU/CPU clock frequencies, VRAM allocation, power draw (TDP), thermals, and per-process GPU memory maps.
- **Headless CLI & Scripting:** Full command-line interface with structured `--json` output for automated system maintenance and telemetry piping.
- **Native D-Bus IPC Service:** Standardized desktop integration via `io.github.ProjectRoASD.rocontrol` for shell widgets, scripts, and status monitors.

---

## CLI Quick Reference

When launched without arguments, `ro-control` opens the graphical desktop interface. For headless administration and scripting, use the CLI commands:

| Command | Description |
| :--- | :--- |
| `ro-control help` | Prints usage information and available commands |
| `ro-control version` | Prints the application version |
| `ro-control status [--json]` | Displays brief hardware, driver, and system status summary |
| `ro-control fan status [--json]` | Displays active fan modes, speeds, RPMs, and temperatures |
| `ro-control fan set-speed <percent>` | Sets fixed manual fan speed percentage (0–100%) |
| `ro-control fan set-mode <profile>` | Sets active fan cooling mode (`auto`, `silent`, `balanced`, `performance`, `manual`, `custom`) |
| `ro-control fan set-smoothing <on\|off> [opts]` | Configures fan ramp rate smoothing and directional hysteresis |
| `ro-control fan reset` | Restores default hardware cooling curve |
| `ro-control power status [--json]` | Displays current GPU power draw, TDP bounds, and persistence state |
| `ro-control power set-limit <watts>` | Sets GPU maximum power draw limit in Watts |
| `ro-control power set-preset <preset>` | Applies predefined power and clock profile (`eco`, `balanced`, `performance`, `custom`) |
| `ro-control power set-clocks <core> <mem>` | Configures GPU core and memory clock offsets in MHz |
| `ro-control power set-persistence <on\|off>` | Toggles NVIDIA driver persistence daemon mode |
| `ro-control processes [--json]` | Lists active applications using GPU VRAM |
| `ro-control kill-process <pid>` | Terminates a running GPU process via Polkit helper |
| `ro-control gpus [--json]` | Enumerates all detected graphics cards |
| `ro-control select-gpu <index>` | Selects active target GPU for monitoring and tuning |
| `ro-control diagnostics [--json]` | Generates comprehensive hardware, kernel, and driver report |
| `ro-control driver install [opts]` | Triggers driver installation via Polkit (`--proprietary`, `--open-source`) |
| `ro-control driver remove` | Removes installed NVIDIA driver packages |
| `ro-control driver update` | Updates installed NVIDIA driver packages |
| `ro-control driver deep-clean` | Cleans up broken module trees, akmods residues, and leftover artifacts |
| `ro-control --daemon` | Launches background headless D-Bus telemetry service |

---

## Documentation

Comprehensive guides for administrators, maintainers, and developers:

- 📦 **[Installation & Deployment Guide](docs/INSTALL.md):** System requirements, RPM package deployment, Discover store setup, and systemd service management.
- 🛠️ **[Build & Development Guide](docs/BUILD.md):** Toolchain prerequisites, compiling from source (CMake/Ninja), running CTest suites, and packaging commands.
- 🏗️ **[Architecture & IPC Specification](docs/ARCHITECTURE.md):** Subsystem structure, backend modules, hardware sysfs integration, and D-Bus IPC methods/signals.
- ⚙️ **[Configuration Reference](docs/CONFIGURATION.md):** QSettings keys for thermal thresholds, power, fan curves, and UI preferences.
- 🤝 **[Contributing Guidelines](CONTRIBUTING.md):** Development workflow, pull request guidelines, and code standards.
- 🔒 **[Security Policy](SECURITY.md):** Vulnerability reporting and Polkit privilege boundaries.

---

## License

ro-Control is open-source software licensed under the **GNU General Public License v3.0 or later** (GPL-3.0-or-later). See [LICENSE](LICENSE) for details.
