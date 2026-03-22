# Architecture

ro-Control follows a strict **C++ Backend / QML Frontend** separation. The two layers communicate exclusively through Qt's property and signal/slot system — QML never calls system commands directly.

---

## High-Level Overview

```
┌─────────────────────────────────────────────────┐
│                   QML Frontend                  │
│   Main.qml · DriverPage · MonitorPage · ...     │
│   (Declarative UI, binds to C++ properties)     │
└───────────────────┬─────────────────────────────┘
                    │  Q_PROPERTY / signals / slots
┌───────────────────▼─────────────────────────────┐
│                  C++ Backend                    │
│   NvidiaDetector · Installer · GpuMonitor · ... │
│   (Business logic, system calls, DNF, Polkit)   │
└───────────────────┬─────────────────────────────┘
                    │  Shell commands / D-Bus
┌───────────────────▼─────────────────────────────┐
│                Linux System                     │
│   sysfs · nvidia-smi · dnf · pkexec · GRUB      │
└─────────────────────────────────────────────────┘
```

---

## Layer Responsibilities

### QML Frontend (`src/qml/`)

- Renders the UI using Qt Quick Controls 2 with KDE Plasma styling
- Binds to C++ properties — it reads state but never writes to the system directly
- Emits user actions (button clicks) which trigger C++ slots
- Bundles shared assets and `qmldir` metadata with the QML module
- No business logic lives here

### C++ Backend (`src/backend/`)

Divided into three modules:

#### `nvidia/` — Driver Management
| File | Responsibility |
|------|---------------|
| `detector.cpp` | Detect installed GPU, current driver version, kernel module status |
| `installer.cpp` | Install / remove drivers via DNF (`akmod-nvidia`) |
| `updater.cpp` | Check DNF updates for `akmod-nvidia`, trigger update |

#### `monitor/` — Live Statistics
| File | Responsibility |
|------|---------------|
| `gpumonitor.cpp` | Poll GPU temperature, load, VRAM via `nvidia-smi` or sysfs |
| `cpumonitor.cpp` | Poll CPU load and temperature via `/proc/stat` and hwmon |
| `rammonitor.cpp` | Poll RAM usage via `/proc/meminfo` |

#### `system/` — System Integration
| File | Responsibility |
|------|---------------|
| `commandrunner.cpp` | Execute shell commands, capture stdout/stderr |
| `dnfmanager.cpp` | Wrap DNF commands for install/remove/update |
| `polkit.cpp` | Privilege escalation via `pkexec` / PolicyKit D-Bus |

---

## C++ ↔ QML Communication

Qt's `QObject` system is the bridge. Backend objects are injected at startup from `main.cpp` into the root QML context, and their `Q_PROPERTY` values are then consumed by QML:

```cpp
// C++ side — gpumonitor.h
class GpuMonitor : public QObject {
    Q_OBJECT
    Q_PROPERTY(int temperature READ temperature NOTIFY temperatureChanged)
    Q_PROPERTY(int load        READ load        NOTIFY loadChanged)

public:
    int temperature() const { return m_temperature; }
    int load()        const { return m_load; }

signals:
    void temperatureChanged();
    void loadChanged();

private:
    int m_temperature = 0;
    int m_load = 0;
};
```

```qml
// QML side — MonitorPage.qml
Text {
    text: GpuMonitor.temperature + "°C"  // auto-updates via signal
}
```

No manual refresh needed — when C++ emits `temperatureChanged()`, QML re-renders automatically.

---

## Privilege Escalation

Driver operations (install, remove, GRUB edit) require root. We use **PolicyKit (pkexec)** — never `sudo` in a GUI app.

```
User clicks "Install Driver"
        │
        ▼
C++ calls `pkexec` on the dedicated `ro-control-helper`
        │
        ▼
System shows a Plasma authentication dialog
        │
        ▼
Privileged operation runs as root
        │
        ▼
Result emitted back to QML via signal
```

The PolicyKit action definition and helper entrypoint live in `data/polkit/` and `data/helpers/`.

---

## Build System

CMake 3.22+ with `qt_add_qml_module` for QML resource embedding. All QML files are compiled into the binary at build time — no loose `.qml` files needed at runtime.

See [BUILDING.md](BUILDING.md) for full build instructions.

---

## Directory Structure

```
ro-Control/
├── src/
│   ├── backend/
│   │   ├── nvidia/
│   │   │   ├── detector.h / detector.cpp
│   │   │   ├── installer.h / installer.cpp
│   │   │   ├── updater.h / updater.cpp
│   │   │   └── versionparser.h / versionparser.cpp
│   │   ├── monitor/
│   │   │   ├── gpumonitor.h / gpumonitor.cpp
│   │   │   ├── cpumonitor.h / cpumonitor.cpp
│   │   │   └── rammonitor.h / rammonitor.cpp
│   │   ├── system/
│   │   │   ├── commandrunner.h / commandrunner.cpp
│   │   │   ├── dnfmanager.h / dnfmanager.cpp
│   │   │   ├── polkit.h / polkit.cpp
│   │   │   └── sessionutil.h / sessionutil.cpp
│   │   └── cli/
│   │       └── cli.h / cli.cpp
│   ├── qml/
│   │   ├── assets/
│   │   │   ├── ro-control-logo.png
│   │   │   └── ro-control-logo.svg
│   │   ├── components/
│   │   │   ├── InfoBadge.qml
│   │   │   ├── SectionPanel.qml
│   │   │   ├── SidebarMenu.qml
│   │   │   ├── StatCard.qml
│   │   │   ├── StatusBanner.qml
│   │   │   └── qmldir
│   │   ├── pages/
│   │   │   ├── DriverPage.qml
│   │   │   ├── MonitorPage.qml
│   │   │   ├── SettingsPage.qml
│   │   │   └── qmldir
│   │   └── Main.qml
│   └── main.cpp
├── data/
│   ├── icons/
│   └── polkit/
├── docs/
├── i18n/
├── packaging/rpm/
└── tests/
```
