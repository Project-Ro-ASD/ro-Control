# Installation & Deployment Guide

This document details installation, package deployment, system requirements, and service integration for **ro-Control** on **Ro-ASD** and Fedora-based systems.

---

## 1. System Requirements

- **Operating System:** Ro-ASD / Fedora Linux 40+ (x86_64 or aarch64)
- **Desktop Environment:** KDE Plasma 6 (KF6) / Wayland or X11
- **Graphics Hardware:** NVIDIA GeForce, Quadro, RTX, or Tesla GPUs (PCIe)
- **Privilege Manager:** Polkit (PolicyKit 1.0+)
- **Package Manager:** DNF / RPM

---

## 2. Installing via Package Manager (RPM)

### Direct RPM Installation
To install the pre-built release package using DNF:

```bash
# Install local RPM with automatic dependency resolution
sudo dnf install -y ./ro-control-<version>-1.<arch>.rpm
```

Replace `<version>` and `<arch>` with the release you are installing (for
example `ro-control-1.3.0-1.x86_64.rpm`).

### Installation via KDE Discover
1. Open the `.rpm` file with **Discover** (`plasma-discover ro-control-<version>-1.<arch>.rpm`) or double-click in Dolphin.
2. Review package details (License: `GPL-3.0-or-later`, Publisher: `Sopwit`).
3. Click **Install** and authenticate via the system Polkit dialog.

---

## 3. Package Contents & System Locations

| Component | Path | Description |
| :--- | :--- | :--- |
| **Binary** | `/usr/bin/ro-control` | Main GUI application and CLI dispatcher |
| **Privileged Helper** | `/usr/libexec/ro-control-helper` | Polkit-authenticated driver management backend |
| **Polkit Policy** | `/usr/share/polkit-1/actions/io.github.ProjectRoASD.rocontrol.policy` | Privilege rules for hardware and driver transactions |
| **System Service** | `/usr/lib/systemd/system/ro-control.service` | System-wide thermal guard and hardware monitoring daemon |
| **User Service** | `/usr/lib/systemd/user/ro-control.service` | User-session background D-Bus telemetry service |
| **Desktop Entry** | `/usr/share/applications/io.github.projectroasd.rocontrol.desktop` | Application menu and desktop launcher |
| **AppStream Metadata** | `/usr/share/metainfo/io.github.projectroasd.rocontrol.metainfo.xml` | Discover store catalog metadata and screenshots |
| **Man Page** | `/usr/share/man/man1/ro-control.1.gz` | Manual page (`man ro-control`) |
| **Shell Completions** | `/usr/share/bash-completion/`, `/usr/share/zsh/`, `/usr/share/fish/` | Tab completions for Bash, Zsh, and Fish |

---

## 4. Post-Installation & Service Management

The RPM package automatically runs icon cache and desktop database triggers upon installation.

### Enabling the Background Service
```bash
# Enable and start the system-level thermal monitor service
sudo systemctl enable --now ro-control.service

# Or enable the user-session D-Bus telemetry service
systemctl --user enable --now ro-control.service
```

### Verifying Service Status
```bash
systemctl status ro-control.service
```

---

## 5. Uninstallation

To remove ro-Control and clean up system configurations:

```bash
sudo dnf remove -y ro-control
```
