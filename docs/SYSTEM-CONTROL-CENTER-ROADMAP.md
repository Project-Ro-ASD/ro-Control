# ro-Control System Control Center Roadmap

## Vision
Transform **ro-Control** from a focused NVIDIA driver + fan control utility into an integrated **System Control Center** — a Task Manager / System Monitor hybrid (like Windows Task Manager / GNOME System Monitor) that sits alongside existing driver/fan/power control modules.

The first phase adds full system visibility (processes, CPU/RAM/Disk/Network). Subsequent phases add control actions and expand to other system settings (display, audio, network, storage, security).

---

## Guiding Principles

| Principle | Description |
|-----------|-------------|
| **Backend‑first** | New capabilities as `QObject` backend modules following existing patterns (`CpuMonitor`, `RamMonitor`, `FanController`). |
| **No architectural rewrite** | Keep existing `GpuMonitor`/`HealthGuard`/`DBusService` etc.; add new modules alongside them. |
| **IPC reuse** | All new data flows through the same D‑Bus (`io.github.ProjectRoASD.rocontrol`) and CLI entry points; QML talks directly to backend objects in‑process. |
| **Polkit reuse** | Privileged actions (`kill process`, `renice`) reuse existing `ro-control-helper` / `CommandRunner::runAsRoot`. |
| **QSettings persistence** | UI preferences (selected tab, update interval, thresholds) stored under `~/.config/Project-Ro-ASD/ro-control.conf`. |
| **Incremental phases** | Each phase is independently build‑testable; no “big‑bang” rewrite. |
| **Fedora / Linux‑first** | Code reads `/proc`, `/sys`, runs `ps`, `ss`, `df` — fine for the target platform. |

---

## Current State (as of v1.3.5 + system-monitor work)

| Area | What exists |
|------|-------------|
| **Backends** | `NvidiaDetector` / `NvidiaInstaller` / `NvidiaUpdater` / `VersionParser`; `GpuMonitor` / `CpuMonitor` / `RamMonitor` / `ProcessMonitor` / `DiskMonitor` / `NetworkMonitor` / `FanController` / `PowerController` / `HealthGuard`; `SystemInfoProvider` / `RoControlDBusService`. |
| **QML UI** | `Main.qml` with tab bar (Driver / Monitor / Cooling & Fans / System / System Monitor) and stacked pages (`DriverPage`, `MonitorPage`, `FanPage`, `SystemPage`, `SystemMonitorPage`). Sidebar menu (`Install`, `Expert`, `Monitor`). Style tokens (`RoTokens`, `Theme`, `Motion`). |
| **CLI** | Commands: `status`, `diagnostics`, `driver`, `fan`, `power`, `processes` (lists GPU compute/display processes), `kill-process`, `gpus`, `select-gpu`, `help`, `--daemon`, plus `process-list`, `disk`, `network`, `resources`, `renice`. |
| **Daemon** | `--daemon` flag starts a `QCoreApplication` with `RoControlDBusService` exposing telemetry (CPU/GPU/RAM/disk/network), fan, power, health. |
| **Tests** | Unit tests for `cpu/gpu/ram/process/disk/network/fan/power/health/dbus/driver/cli/monitor` using QtTest; CI runs `make test`. 18/18 pass on the dev host (2026-11-17). |

---

## Gap Analysis vs “Control Center” Vision

*(All gaps below were closed by the Phase 1–5 implementation.)*

| Missing | Why it matters |
|---------|----------------|
| **Process list / Task Manager core** | Without a unified process table, there is no “system monitor” experience. |
| **Disk / network resource overview** | Users expect a single page showing storage and I/O. |
| **CLI & D‑Bus integration** | New commands & D‑Bus methods need wiring; otherwise the rest of the app can’t consume the data. |
| **GUI system monitor page** | QML page to present processes + resource graphs in a Task‑Manager-like layout. |
| **Persistence of monitor settings** | Update interval per‑monitor, selected tab, etc. currently not configurable beyond defaults. |

---

## Phased Roadmap

> **Status as of 2026-11-17: Phases 1–5 implemented.** All phases were completed on top of the v1.3.5 baseline. Deviations from the original plan are noted per phase below. Runtime verification of `/proc`-backed behaviour still has to happen on the Fedora/Ro-ASD target (development host is macOS); fixture-based (`RO_CONTROL_*_PATH`) and offscreen-QML verification pass on this host.

| Phase | Status | Notes |
|-------|--------|-------|
| 1 — Process & Resource Intelligence | **Done** | `ProcessMonitor` / `DiskMonitor` / `NetworkMonitor` implemented following the CpuMonitor pattern; env-path overrides for testability. |
| 2 — IPC & CLI Integration | **Done** | Telemetry merged into `GetTelemetry`; CLI `process-list`/`disk`/`network`/`resources`/`renice`. |
| 3 — GUI: System Monitor Page | **Done** | `SystemMonitorPage.qml` added as 5th tab (index 4). |
| 4 — Control Actions & Persistence | **Done** | Kill/renice via `CommandRunner`; interval preferences persisted and applied at startup. |
| 5 — Polish & Packaging | **Done** | i18n (de/es/tr fully translated), docs, man page, completions, 18/18 tests green. |

### **Phase 1 — Process & Resource Intelligence (Foundation)**
Goal: Add Task‑Manager‑grade monitoring surface within ro‑Control.

#### 1.1 ProcessMonitor (`src/backend/monitor/processmonitor.{h,cpp}`)
> **Implemented as noted, with deviations:** memory comes from `/proc/<pid>/stat` RSS (`rss` field × page size) rather than `/proc/<pid>/status`; self-time accounting uses deltas of `utime+stime` against `/proc/stat` totals sampled over the refresh interval; `/proc` root is overridable via `RO_CONTROL_PROC_PATH` for host-independent testing; per-process I/O read from `/proc/<pid>/io`.
| Item | Detail |
|------|--------|
| **Data source** | Read `/proc` directly, consistent with `CpuMonitor` (`/proc/stat`, `/proc/cpuinfo`) and `RamMonitor` (`/proc/meminfo`). |
| **Enumerate** | `QDir("/proc").entryList(QDir::AllDirs | QDir::NoDotAndDotDot)`; for each pid read `/proc/<pid>/stat` (utime/stime, state, ppid, etc.) and `/proc/<pid>/status` (VmRSS, VmSize, uid, pty). |
| **CPU %** | Delta from `/proc/stat` cumulative totals + per‑pid `utime`+`stime` over two refresh intervals → `%CPU`. |
| **Memory** | `VmRSS` from `/proc/<pid>/status` (resident set); `VmSize` (virtual). |
| **Threads** | Enumerate `/proc/<pid>/task/` entries; each thread’s `/proc/<pid>/task/<tid>/stat` contributes to per‑thread CPU if desired. |
| **Signals** | `processesChanged()`, `runningChanged()`, `updateIntervalChanged()`, `statusMessageChanged()`. |
| **Q_INVOKABLE API** | `start()`, `stop()`, `refresh()`, `setUpdateInterval(int ms)`, `killProcess(int pid, int signal = SIGTERM)`, `setProcessPriority(int pid, int nice)` (calls `renice -n <nice> -p <pid>`). |
| **Kill implementation** | Use `CommandRunner::run({"kill", "-<sig>", QString::number(pid)})` or `QProcess::start("kill", ...)`. Since the codebase routes privileged commands through `CommandRunner::runAsRoot`, we’ll follow that pattern. |
| **Nice implementation** | Use `CommandRunner::runAsRoot({"renice", "-n", QString::number(nice), "-p", QString::number(pid)})`. |
| **Refresh rate default** | 2000 ms (matching CpuMonitor default 2 s). |

#### 1.2 DiskMonitor (`src/backend/monitor/diskmonitor.{h,cpp}`)
> **Implemented as noted, with deviations:** usage via `statvfs`; `/proc/diskstats` counters mapped by base device name (partition suffix stripped); mount root overridable via `RO_CONTROL_MOUNTS_PATH`, diskstats via `RO_CONTROL_DISKSTATS_PATH`.
| Item | Detail |
|------|--------|
| **Mount enumeration** | Parse `/proc/mounts` → list of `{device, mountPoint, fstype}`. |
| **Usage per mount** | Use `statfs` syscall (`#include <sys/statvfs.h>`) on the mount path → `{f_bsize, f_blocks, f_bfree, f_bfavail, f_files, f_ffree, f_favail, f_frsize, f_flag}`. Compute `total`, `used`, `free` in MiB. |
| **I/O stats** | Parse `/proc/diskstats` → per‑device `{reads, merges, sectors_read, ms_read, writes, ...}`. Map device names to mount devices. |
| **Signals** | `disksChanged()`, `runningChanged()`, `updateIntervalChanged()`. |
| **Q_INVOKABLE API** | `start()`, `stop()`, `refresh()`, `setUpdateInterval(int ms)`. |

#### 1.3 NetworkMonitor (`src/backend/monitor/networkmonitor.{h,cpp}`)
> **Implemented as noted, with deviations:** interface counters only (per the plan's scope decision — connections deferred); net-dev path overridable via `RO_CONTROL_NET_DEV_PATH`.
| Item | Detail |
|------|--------|
| **Interface counters** | Parse `/proc/net/dev` → `{name, rx_bytes, rx_packets, rx_errors, tx_bytes, tx_packets, tx_errors}`. |
| **Connections (basic)** | Parse `/proc/net/tcp` / `/proc/net/tcp6` for listening/established sockets (state, local address, remote address, inode). Map inode → pid by scanning `/proc/*/fd/*` symlinks (lightweight for top‑N). Or simply expose raw socket stats via CommandRunner (`ss -tnp`). We’ll start with interface counters only; connections can be added later. |
| **Signals** | `interfacesChanged()`, `runningChanged()`, `updateIntervalChanged()`. |
| **Q_INVOKABLE API** | `start()`, `stop()`, `refresh()`, `setUpdateInterval(int ms)`. |

#### 1.4 System‑wide wiring
> **Implemented as noted, with deviations:** CLI commands are named `process-list`, `disk`, `network`, `resources` (separate from the GPU-scoped `processes` command, whose semantics are unchanged); test skeletons exist for all three monitors and pass on the dev host.

- Register new objects in `GuiInstance` / `Main.qml` initial properties (add `processMonitor`, `diskMonitor`, `networkMonitor`).
- Add D‑Bus methods in `RoControlDBusAdaptor` / `RoControlDBusService` (see Phase 2).
- Add CLI commands in `cli.cpp` (`processes`, `resource`, maybe `top`‑like output).
- Update `CMakeLists.txt` to include new source files (`processmonitor.cpp`, `diskmonitor.cpp`, `networkmonitor.cpp`).
- Add unit‑test skeletons in `tests/test_processmonitor.cpp`, etc. (following `test_monitor.cpp` pattern).

---

### **Phase 2 — IPC & CLI Integration**
Goal: Expose the new monitors to D‑Bus and CLI so the rest of the app can consume them.

| Item | Detail |
|------|--------|
| **D‑Bus methods** (`RoControlDBusAdaptor`) | `GetProcessList(int limit = 0, const QString &sortBy = "cpu")` → JSON array of process summaries. <br> `GetSystemResources()` → `{cpu: {...}, memory: {...}, disks: [...], interfaces: [...]}`. <br> `KillProcess(int pid, int signal)` → `bool success`. <br> `SetProcessPriority(int pid, int nice)` → `bool success`. |
| **D‑Bus signals** | `ProcessSelected(int pid)`, `ResourceUpdated(QJsonObject)`. |
| **CLI additions** (`cli.cpp`) | `processes [--json] [--limit N] [--sort cpu|memory]` — list top processes. <br> `resource [--json]` — concise system resources summary. <br> `kill <pid>` — terminate process (routed through Polkit). <br> `renice <nice> <pid>` — change nice value. |
| **main.cpp wiring** | In `executeCliCommand`, add cases for `PrintProcessesText/Json` (already partially there for GPU processes; extend to all processes), `PrintResourceText/Json`, `KillProcess`, `SetProcessPriority`. |
| **Default interval exposure** | `updateInterval` property on each monitor; UI can change via QSettings. |

> **As built:** no new D-Bus *methods* were added; the new telemetry is folded into the existing `GetTelemetry()` JSON under `disk`/`network` keys (plus disk/network blocks restored alongside the existing ones) and rides the live `TelemetryUpdated` signal — keeping the IPC surface single-call, as the existing service does. Kill/renice are D-Bus-free: the GUI calls the in-process `ProcessMonitor` Q_INVOKABLEs, and the CLI routes them through `CommandRunner`. CLI names: `process-list [--json] [--limit N]`, `disk`, `network`, `resources`, `renice <nice> <pid>`. `processes`, `kill-process` remain GPU-scoped, unchanged.

---

### **Phase 3 — GUI: System Monitor Page**
> **As built:** implemented as a chart-free page (stat cards + sortable process table + Kill/renice actions + SIGTERM confirmation popup) instead of sparklines; tab index 4 confirmed and wired in `test_main_window`. Page file: `src/qml/pages/SystemMonitorPage.qml`; registered in `qmldir` + CMake; `Main.qml` passes `uiPreferences` so the process list limit follows `ui/processListLimit`.
>
Goal: Present a Task‑Manager‑style page inside the existing GUI.

| Item | Detail |
|------|--------|
| **New QML page** (`src/qml/pages/SystemMonitorPage.qml`) | Or integrate as a new tab in `Main.qml`. Since `Main.qml` uses `StackLayout` keyed by `tabBar.currentIndex`, adding a 5th tab requires updating all `currentIndex === N` guards throughout the file (≈30 occurrences). To avoid ripple risk, we’ll **add the system monitor as an additional page in the existing StackLayout**, keeping tab indices 0‑3 unchanged and inserting the new page at index 4 (after `SystemPage`). The `tabBar` model will gain a 5th `TabButton` (“Processes”). All `currentIndex` guards that currently check 0‑3 will automatically ignore index 4 unless explicitly added; we’ll add guards only where needed. |
| **Page contents** | - **Top summary cards** (CPU % (sparkline), RAM used/sparkline, Disk total/used, Net rx/tx). <br> - **Process table** (columns: PID, Name, User, CPU %, Memory, RSS, State, Threads). <br> - **Filter/search** bar (text filter on name/user). <br> - **Context menu** (right‑click → Kill, Set Priority, Open File, Show Details). <br> - **Sparkline graphs** reuse existing `TelemetrySparkline` component from `MonitorPage.qml`. |
| **Model‑view** | `QAbstractListModel` subclass in `ProcessMonitor` exposing `QVariantMap` per row; QML `ListView` with `delegate`. |
| **Integration with existing UI** | - Connect `processMonitor.processesChanged` → refresh list. <br> - Connect `cpuMonitor`, `ramMonitor`, `diskMonitor`, `networkMonitor` signals to update summary cards. <br> - Connect tray icon tooltip to include top process info (optional). |
| **QML Types registration** | Register `ProcessMonitor` etc. via `qmlRegisterType` or context property from `main.cpp` (same pattern as `cpuMonitor`, `gpuMonitor`, `ramMonitor`). |
| **CMake** | Add `src/qml/pages/SystemMonitorPage.qml` to `qt_add_qml_module` `QML_FILES` list. Ensure `SystemMonitorPage` is exposed to main QML engine. |
| **Tests** | `test_main_window.cpp` will need a regression check that the new tab index (4) is handled; add minimal guard or leave as‑is if we only add the page via context property without a new tab bar button (alternative: add a “Processes” button in the sidebar or a menu entry). We'll keep it simple: expose the page as a `StackLayout` page without a new tab bar button initially; access via `ro-control --processes` CLI or a hidden easter‑egg. Actually better: add a new tab “Processes” (index 4). The test already loops over indices; adding index 4 is safe if we add `currentIndex === 4` checks only where behavior differs. We'll add those guards incrementally. |

---

### **Phase 4 — Control Actions & Persistence**
> **As built:** kill uses `ProcessMonitor::killProcess` (SIGTERM) via `CommandRunner`; nice changes use `setProcessPriority` → `renice` via `CommandRunner`, with nice range validated to -20…19 before exec. Persistence keys: `ui/monitorRefreshInterval`, `ui/processRefreshInterval`, `ui/processListLimit` in QSettings; GUI applies them at startup and reacts to live changes. HealthGuard threshold additions deferred.
>
Goal: Make the monitoring actionable; persist user choices.

| Item | Detail |
|------|--------|
| **Kill process from GUI** | Row context menu → `killProcess(pid)` → `CommandRunner::runAsRoot({"kill", "-15", QString::number(pid)})` (SIGTERM first, then SIGKILL if no response). |
| **Set priority (nice)** | Row context menu → `renice <value> <pid>` → `CommandRunner::runAsRoot({"renice", "-n", QString::value(arg), "-p", QString::number(pid)})`. |
| **Update interval persistence** | Add QSettings keys: `monitor/updateInterval` (int, ms, default 2000). Each monitor reads this on construction; UI slider to change it. |
| **Threshold persistence** (extend HealthGuard) | Already have GPU/CPU warning/critical thresholds in QSettings (docs/CONFIGURATION.md). Could add a “process‑CPU‑high” threshold to trigger alerts. |
| **CLI `renice` & `kill`** already partially exist (`kill-process <pid>` kills GPU processes via `GpuMonitor::killProcess`). We’ll extend to any PID. |

---

### **Phase 5 — Polish & Packaging**
> **As built:** all new UI strings translated in de/es/tr TS files (0 unfinished); README, man page, `docs/CONFIGURATION.md`, `docs/ARCHITECTURE.md`, and shell completions updated; `test_processmonitor.cpp` rewritten with a fake procfs fixture; `test_diskmonitor.cpp`/`test_networkmonitor.cpp` added; `test_dbus_service.cpp` extended for the new telemetry blocks; 18/18 tests pass on the dev host. RPM packaging unchanged (no new binaries; all new logic lives in the existing `ro-control` target).
>
| Item | Detail |
|------|--------|
| **Translations** | Add `.ts` entries for new QML strings (process, disk, network). |
| **Documentation** | Update `docs/CONFIGURATION.md` with new keys (`MonitorUpdateInterval`, `ProcessListSort`, etc.). Add `docs/SYSTEM-CONTROL-CENTER-ROADMAP.md` (this file). |
| **RPM / package** | Ensure `CMakeLists.txt` and helper scripts include new binaries. |
| **Unit tests** | Expand `tests/test_processmonitor.cpp`, `test_diskmonitor.cpp`, `test_networkmonitor.cpp`; add integration test for D‑Bus `GetProcessList`. |
| **Optional: UI preferences page** | Add “System Monitor” subsection in existing Settings popup (language/theme already there) to set update interval, auto‑kill, show threads, etc. |
| **Optional: Full Control Center** | After Phase 4, decide whether to add remaining system settings (display brightness via DRM, audio output selection via PipeWire, network profiles, storage management). Scope for now is **monitor + basic control**; remaining settings can be a later v2. |

---

## Implementation Priority Order

| Phase | Effort (est.) | Delivers |
|-------|---------------|----------|
| **1** (Process/ Disk/ Network monitors) | Medium (3 weeks) | Full system‑resource visibility; D‑Bus/CLI skeleton. |
| **2** (IPC/CLI wiring) | Low (1 week) | `ro-control processes`, `ro-control resource`, D‑Bus methods. |
| **3** (GUI page) | Medium‑High (3 weeks) | Task‑Manager‑style page inside ro‑Control GUI. |
| **4** (Control actions) | Low (1 week) | Kill/renice from GUI/CLI; persisted intervals. |
| **5** (Polish) | Low (1 week) | Docs, tests, packaging. |

Total estimated: **~9 weeks** of focused work (≈2 person‑months) to reach a functional **System Control Center** MVP with process monitoring, resource summary, and basic control.

---

## Getting Started — Phase 1 Kick‑off

The immediate next step is to **implement `ProcessMonitor`** following the pattern of `CpuMonitor`/`RamMonitor`. This single module will deliver the core “Task Manager” experience and lay the groundwork for the rest.

**Suggested first commit:**
1. Create `src/backend/monitor/processmonitor.h` & `processmonitor.cpp`.
2. Register it in `CMakeLists.txt` `BACKEND_SOURCES`.
3. Expose it from `main.cpp` as `root.processMonitor` (add to `GuiInstance` initial properties).
4. Write a minimal unit test `tests/test_processmonitor.cpp` (borrow from `test_monitor.cpp`).
5. Verify `cmake --build . && ./tests/test_processmonitor` runs.

Once Phase 1 is verified, we proceed to Phase 2 (D‑Bus/CLI), then Phase 3 (QML page).

Let me know if you’d like me to start **Phase 1** right now (create the `ProcessMonitor` files), or if you prefer to review the full roadmap doc first. I can also generate a **git feature branch** and open a PR‑ready set of changes.

---
*Roadmap generated 2026‑11‑17 for the ro‑Control project.*