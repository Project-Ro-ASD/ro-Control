# Build & Development Guide

This document details the build prerequisites, compilation steps, test execution, and packaging procedures for developers and OS maintainers. ro-Control targets **Fedora-based Ro-ASD** only.

---

## 1. Build Prerequisites

### Fedora / Ro-ASD
```bash
sudo dnf install -y \
  cmake extra-cmake-modules gcc-c++ ninja-build \
  qt6-qtbase-devel qt6-qtdeclarative-devel qt6-qttools-devel qt6-qtwayland-devel \
  kf6-qqc2-desktop-style polkit-devel systemd-rpm-macros desktop-file-utils \
  pciutils mokutil kmod lm_sensors rpm-build
```

---

## 2. Compiling from Source

```bash
# 1. Clone the repository
git clone https://github.com/Project-Ro-ASD/ro-Control.git
cd ro-Control

# 2. Configure build tree with CMake and Ninja
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTS=ON

# 3. Compile all targets
cmake --build build --parallel $(nproc)
```

---

## 3. Running Tests

The test suite is built on **QtTest** and managed by **CTest** (12 suites covering hardware detection, fan curves, telemetry parsing, Polkit helper, D-Bus service, CLI, and AppStream metadata):

```bash
ctest --test-dir build --output-on-failure --verbose
```

---

## 4. Building RPM Packages Locally

To generate distribution-ready RPM packages:

```bash
# Create source tarball
mkdir -p ~/rpmbuild/SOURCES ~/rpmbuild/SPECS
VERSION=$(awk '/^project\(ro-control/{getline; print $2}' CMakeLists.txt)
tar -czf ~/rpmbuild/SOURCES/ro-control-${VERSION}.tar.gz \
  --exclude-vcs --transform "s,^\.,ro-control-${VERSION}," .

# Build Binary and Source RPM
rpmbuild -ba packaging/rpm/ro-control.spec
```

The `VERSION` substitution reads the current project version from
`CMakeLists.txt`, so the tarball name always matches what
`packaging/rpm/ro-control.spec` expects. The resulting packages will be placed
in:

- Binary RPM: `~/rpmbuild/RPMS/x86_64/ro-control-<version>-1.x86_64.rpm`
- Source RPM: `~/rpmbuild/SRPMS/ro-control-<version>-1.src.rpm`

---

## 5. Local Code Health & Validation

Run the automated health checker before opening PRs or tagging releases:

```bash
bcheck ro-control
```
