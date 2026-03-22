# Fedora Run Guide

This guide is focused on running `ro-control` on Fedora Workstation/Spin systems.

## 1) Bootstrap (recommended)

From the repository root:

```bash
./scripts/fedora-bootstrap.sh
```

Optional flags via environment variables:

```bash
ENABLE_TESTS=0 BUILD_TYPE=Debug ./scripts/fedora-bootstrap.sh
INSTALL_AFTER_BUILD=1 INSTALL_PREFIX=/usr ./scripts/fedora-bootstrap.sh
```

## 2) Manual dependency install (equivalent)

Build dependencies:

```bash
sudo dnf install -y \
  cmake \
  extra-cmake-modules \
  gcc-c++ \
  ninja-build \
  qt6-qtbase-devel \
  qt6-qtdeclarative-devel \
  qt6-qttools-devel \
  qt6-qtwayland-devel \
  kf6-qqc2-desktop-style \
  polkit-devel
```

Runtime tools used by diagnostics and driver workflows:

```bash
sudo dnf install -y dnf polkit pciutils mokutil kmod lm_sensors procps-ng
```

## 3) Build and run

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON
cmake --build build -j"$(nproc)"
ctest --test-dir build --output-on-failure
./build/ro-control
```

## 4) Install (optional)

```bash
sudo cmake --install build
```

## 5) Driver management prerequisites

- `ro-control` invokes privileged operations through `pkexec`.
- For proprietary NVIDIA flow, the app enables RPM Fusion and installs
  `akmod-nvidia` using `dnf`.
- A reboot is required after install/update/remove flows.
- On Secure Boot systems, kernel module signing policy may still require manual steps.

## 6) Quick verification

```bash
./build/ro-control diagnostics --json
./build/ro-control status
```

If GPU telemetry is unavailable, verify `nvidia-smi` is present and working.
If CPU temperature is unavailable, verify `lm_sensors` is installed and sensors are exposed on the host.
If RAM telemetry is unavailable, verify `/proc/meminfo` and `free --mebi` are accessible on the host.
