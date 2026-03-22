# ro-Control

<div align="center">

![ro-Control Logo](data/icons/hicolor/scalable/apps/ro-control.svg)

**Smart NVIDIA Driver Manager & System Monitor for Linux**

[![License: GPL-3.0](https://img.shields.io/badge/license-GPL--3.0-blue?style=flat-square)](LICENSE)
[![Built with Qt6](https://img.shields.io/badge/built%20with-Qt6%20%2B%20QML-41CD52?style=flat-square)](https://www.qt.io/)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C?style=flat-square)](https://isocpp.org/)

[Features](#features) • [Installation](#installation) • [Building](#building-from-source) • [Contributing](#contributing) • [License](#license)

[![README in Turkish](https://img.shields.io/badge/README-Türkçe-red?style=flat-square)](README.tr.md)

</div>

---

ro-Control is a native KDE Plasma desktop application built with **C++20** and **Qt6/QML** that simplifies NVIDIA GPU driver management and system monitoring on Linux. It provides a modern, Plasma-native interface for installing, updating, and monitoring graphics drivers — with full PolicyKit integration for secure privilege escalation.

## Features

### 🚀 Driver Management
- **One-click install** — NVIDIA driver setup via RPM Fusion (`akmod-nvidia`)
- **Driver update** — Detect and apply newer driver versions
- **Clean removal** — Remove old driver artifacts to prevent conflicts
- **Secure Boot** — Detection and warnings for unsigned kernel modules

### 📊 Live System Monitor
- Real-time GPU temperature, load, and VRAM usage
- CPU load and temperature tracking
- RAM usage monitoring
- Color-coded progress indicators

### 🖥 Display & System
- **Wayland support** — Automatic `nvidia-drm.modeset=1` GRUB configuration
- **Hybrid graphics** — Switch between NVIDIA, Intel, and On-Demand modes
- **PolicyKit integration** — Secure privilege escalation without running as root

## Development

The easiest way to develop ro-Control rapidly on Fedora is using the provided `dev-watch.sh` script, which automatically rebuilds and restarts the application on file save:
```bash
# Setup Fedora dependencies
./scripts/fedora-bootstrap.sh

# Start the live development watcher
./scripts/dev-watch.sh
```

> **Note:** If you launch the tool on a system without a working NVIDIA driver or encounter `libEGL` errors, UI elements may overlap. The `dev-watch.sh` script detects this and automatically exports `QT_XCB_GL_INTEGRATION=none` to force software rendering as a fallback.

### 🌍 Internationalization
- Runtime locale loading with Qt translations (`.ts` / `.qm`)
- English source strings with Turkish translation included
- Extensible translation workflow for additional languages

### 🧰 CLI Support
- `ro-control help` for usage
- `ro-control version` for application version
- `ro-control status` for concise system and driver state
- `ro-control diagnostics --json` for machine-readable diagnostics
- `ro-control driver install|remove|update|deep-clean` for scripted driver management
- Installed `man ro-control` page and Bash/Zsh/Fish shell completions

## Screenshots

Preview assets are available under [`docs/screenshots/`](docs/screenshots/).
Additional PNG screenshots should be added before wider store distribution.

## Installation

### RPM Package

Download the latest `.rpm` from [Releases](https://github.com/Project-Ro-ASD/ro-Control/releases):

```bash
sudo dnf install ./ro-control-*.rpm
```

### Building from Source

See [docs/BUILDING.md](docs/BUILDING.md) for full instructions.

Fedora quick bootstrap:

```bash
./scripts/fedora-bootstrap.sh
```

For Fedora-specific runtime notes, see [docs/FEDORA.md](docs/FEDORA.md).

### CLI Quick Examples

```bash
ro-control help
ro-control version
ro-control status
ro-control diagnostics --json
ro-control driver install --proprietary --accept-license
ro-control driver update
```

**Quick start:**

```bash
# Install dependencies
sudo dnf install cmake extra-cmake-modules gcc-c++ \
  qt6-qtbase-devel \
  qt6-qtdeclarative-devel \
  qt6-qttools-devel \
  qt6-qtwayland-devel \
  kf6-qqc2-desktop-style \
  polkit-devel

# Clone and build
git clone https://github.com/Project-Ro-ASD/ro-Control.git
cd ro-Control
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Install
sudo make install
```

## Project Structure

```
ro-Control/
├── src/
│   ├── backend/          # C++ business logic
│   │   ├── nvidia/       # Driver detection, install, update
│   │   ├── monitor/      # GPU/CPU/RAM statistics
│   │   └── system/       # Polkit, DNF, command runner
│   ├── qml/              # Qt Quick UI
│   │   ├── pages/        # Main application pages
│   │   └── components/   # Reusable UI components
│   └── main.cpp
├── data/                 # Icons, .desktop, PolicyKit, AppStream
├── packaging/rpm/        # RPM packaging
├── docs/                 # Architecture and build docs
├── tests/                # Unit tests
└── CMakeLists.txt
```

## Contributing

Contributions are welcome! Please read [CONTRIBUTING.md](CONTRIBUTING.md) before submitting a pull request.
For release flow details, see [docs/RELEASE.md](docs/RELEASE.md).
For localization scaffolding, see [i18n/README.md](i18n/README.md).

Quick contribution flow:

```bash
git checkout dev
git checkout -b feature/your-feature-name
# ... make your changes ...
git commit -m "feat: describe your change"
git push origin feature/your-feature-name
# Open a Pull Request → dev
```

## Requirements

| Component | Minimum Version |
|-----------|----------------|
| Qt        | 6.6+           |
| CMake     | 3.22+          |
| GCC       | 13+ (C++20)    |
| GPU       | NVIDIA (any)   |

## License

This project is licensed under the [GNU General Public License v3.0](LICENSE).
