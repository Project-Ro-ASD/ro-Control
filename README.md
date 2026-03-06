# ro-Control

<div align="center">

![ro-Control Logo](data/icons/hicolor/scalable/apps/ro-control.svg)

**Smart NVIDIA Driver Manager & System Monitor for Linux**

[![License: GPL-3.0](https://img.shields.io/badge/license-GPL--3.0-blue?style=flat-square)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Fedora%2040%2B-51A2DA?style=flat-square)](https://getfedora.org/)
[![Built with Qt6](https://img.shields.io/badge/built%20with-Qt6%20%2B%20QML-41CD52?style=flat-square)](https://www.qt.io/)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C?style=flat-square)](https://isocpp.org/)

[Features](#features) • [Installation](#installation) • [Building](#building-from-source) • [Contributing](#contributing) • [License](#license)

[![README in Turkish](https://img.shields.io/badge/README-Türkçe-red?style=flat-square)](README.tr.md)

</div>

---

ro-Control is a native KDE Plasma desktop application built with **C++20** and **Qt6/QML** that simplifies NVIDIA GPU driver management and system monitoring on Fedora Linux. It provides a modern, Plasma-native interface for installing, updating, and monitoring graphics drivers — with full PolicyKit integration for secure privilege escalation.

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

### 🌍 Internationalization
- English and Turkish interface
- Extensible translation system

## Screenshots

> Screenshots will be added after the first UI milestone.

## Installation

### Fedora (RPM) — Recommended

Download the latest `.rpm` from [Releases](https://github.com/Acik-Kaynak-Gelistirme-Toplulugu/ro-Control/releases):

```bash
sudo dnf install ./ro-control-*.rpm
```

### Building from Source

See [docs/BUILDING.md](docs/BUILDING.md) for full instructions.

**Quick start:**

```bash
# Install dependencies (Fedora 40+)
sudo dnf install cmake extra-cmake-modules gcc-c++ \
  qt6-qtbase-devel \
  qt6-qtdeclarative-devel \
  qt6-qtwayland-devel \
  kf6-qqc2-desktop-style

# Clone and build
git clone https://github.com/Acik-Kaynak-Gelistirme-Toplulugu/ro-Control.git
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
├── packaging/rpm/        # Fedora RPM spec
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
| Fedora    | 40+            |
| Qt        | 6.6+           |
| CMake     | 3.22+          |
| GCC       | 13+ (C++20)    |
| GPU       | NVIDIA (any)   |

## License

This project is licensed under the [GNU General Public License v3.0](LICENSE).

