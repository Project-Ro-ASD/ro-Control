# Building ro-Control from Source

This guide covers building ro-Control from source on Linux systems with Qt 6 and CMake.

---

## Requirements

| Component | Minimum Version | Check |
|-----------|----------------|-------|
| GCC       | 13+            | `gcc --version` |
| CMake     | 3.22+          | `cmake --version` |
| Qt        | 6.6+           | `rpm -q qt6-qtbase-devel` |
| Qt Linguist Tools | Required for release-grade builds | `rpm -q qt6-qttools-devel` |
| Ninja     | Any            | `ninja --version` (optional, faster builds) |

---

## Fedora Quick Bootstrap

```bash
./scripts/fedora-bootstrap.sh
```

The script installs Fedora dependencies, builds the app, and runs tests by default.
For Fedora-specific runtime notes, see [FEDORA.md](FEDORA.md).

---

## Install Dependencies

```bash
sudo dnf install \
  cmake \
  extra-cmake-modules \
  gcc-c++ \
  ninja-build \
  qt6-qtbase-devel \
  qt6-qtbase-private-devel \
  qt6-qtdeclarative-devel \
  qt6-qttools-devel \
  qt6-qtwayland-devel \
  kf6-qqc2-desktop-style \
  polkit-devel
```

If `kf6-qqc2-desktop-style` is not available in your enabled repositories, you
can skip it for local builds. The Fedora bootstrap script already treats it as
optional.

Runtime tools used by diagnostics and driver operations:

```bash
sudo dnf install dnf polkit pciutils mokutil kmod lm_sensors procps-ng
```

---

## Clone the Repository

```bash
git clone https://github.com/Project-Ro-ASD/ro-Control.git
cd ro-Control
```

---

## Build

### Debug Build (for development)

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
```

### Release Build

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

### Refresh translations (recommended before release)

```bash
lupdate src -ts i18n/ro-control_en.ts i18n/ro-control_tr.ts
cmake --build build --target ro-control_lrelease
```

---

## Run

```bash
# From the repository root
./build/ro-control
```

CLI examples:

```bash
./build/ro-control help
./build/ro-control version
./build/ro-control status
./build/ro-control diagnostics --json
./build/ro-control driver install --proprietary --accept-license
./build/ro-control driver update
```

> **Note:** Driver install/remove operations require PolicyKit authentication. The UI will prompt you automatically.

After `cmake --install`, the CLI integration also installs:

- `man ro-control`
- Bash completion: `share/bash-completion/completions/ro-control`
- Zsh completion: `share/zsh/site-functions/_ro-control`
- Fish completion: `share/fish/vendor_completions.d/ro-control.fish`

---

## Install System-Wide

```bash
sudo cmake --install build
```

This installs:
- Binary -> `/usr/local/bin/ro-control`
- Privileged helper -> `/usr/local/libexec/ro-control-helper`
- Desktop entry -> `/usr/local/share/applications/`
- Icons -> `/usr/local/share/icons/`
- AppStream metadata -> `/usr/local/share/metainfo/`
- PolicyKit policy -> `/usr/local/share/polkit-1/actions/`

---

## Build with Tests

```bash
cmake -S . -B build -G Ninja -DBUILD_TESTS=ON
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure
```

---

## Uninstall

`make uninstall` is not currently defined in this project.
Use your package manager or the install manifest to remove a local install.

---

## Common Issues

**`qt6-qtdeclarative-devel` not found**
```bash
sudo dnf install qt6-qtdeclarative-devel
```

**CMake can't find Qt6**
```bash
# Make sure Qt6 is installed, then specify path manually:
cmake .. -DCMAKE_PREFIX_PATH=/usr/lib64/cmake/Qt6
```

**Build fails with C++20 errors**
Ensure GCC 13 or newer is installed:
```bash
sudo dnf install gcc-c++
gcc --version  # Should be 13+
```

**Translations do not update**
```bash
rm -rf build/.qt build/CMakeFiles
cmake -S . -B build
lupdate src -ts i18n/ro-control_en.ts i18n/ro-control_tr.ts
cmake --build build --target ro-control_lrelease
```

---

## Contributing

After making changes, always verify the build passes before submitting a PR:

```bash
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure
```

See [CONTRIBUTING.md](../CONTRIBUTING.md) for the full contribution guide.
