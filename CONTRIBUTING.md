# Contributing to ro-Control

Thank you for your interest in contributing to ro-Control! This document explains how to get involved.

---

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [How to Contribute](#how-to-contribute)
- [Branch Strategy](#branch-strategy)
- [Commit Message Format](#commit-message-format)
- [Development Setup](#development-setup)
- [Pull Request Process](#pull-request-process)
- [Translations](#translations)
- [Reporting Bugs](#reporting-bugs)

---

## Code of Conduct

Please read and follow our [Code of Conduct](CODE_OF_CONDUCT.md). We are committed to providing a welcoming and respectful environment for everyone.

---

## How to Contribute

You can contribute in several ways:

- **Bug reports** — Open an issue using the bug report template
- **Feature requests** — Open an issue using the feature request template
- **Code contributions** — Fix a bug or implement a feature
- **Translations** — Add or improve language support
- **Documentation** — Improve docs, README, or code comments

---

## Branch Strategy

We use a structured branching model:

```
main        ← Stable, release-ready code only. Never push directly.
dev         ← Active development. All features merge here first.
feature/*   ← New features (branched from dev)
fix/*       ← Bug fixes (branched from dev)
release/*   ← Release preparation (branched from dev)
```

**Always branch from `dev`, not `main`.**

```bash
git checkout dev
git pull origin dev
git checkout -b feature/your-feature-name
```

---

## Commit Message Format

We follow [Conventional Commits](https://www.conventionalcommits.org/):

```
<type>: <short description>
```

| Type       | When to use                              |
|------------|------------------------------------------|
| `feat`     | New feature                              |
| `fix`      | Bug fix                                  |
| `docs`     | Documentation only                       |
| `refactor` | Code change that doesn't add/fix         |
| `test`     | Adding or updating tests                 |
| `chore`    | Build system, CI, dependencies           |
| `style`    | Formatting, whitespace (no logic change) |

**Examples:**

```
feat: add real-time GPU temperature monitoring
fix: crash when no NVIDIA GPU is detected
docs: update build instructions
chore: update CMake minimum version to 3.22
```

---

## Development Setup

### Requirements

| Component | Minimum |
|-----------|---------|
| GCC       | 13+     |
| CMake     | 3.22+   |
| Qt        | 6.6+    |

### Install Dependencies

```bash
sudo dnf install cmake extra-cmake-modules gcc-c++ \
  qt6-qtbase-devel \
  qt6-qtdeclarative-devel \
  qt6-qttools-devel \
  qt6-qtwayland-devel \
  kf6-qqc2-desktop-style \
  polkit-devel
```

### Build

```bash
git clone https://github.com/Project-Ro-ASD/ro-Control.git
cd ro-Control
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
./ro-control
```

### Before Submitting

```bash
# Format your code (clang-format)
find src \( -name "*.cpp" -o -name "*.h" \) -print0 | xargs -0 clang-format -i

# Run tests
cd build && ctest --output-on-failure
```

---

## Pull Request Process

1. Fork the repository
2. Create a branch from `dev`: `git checkout -b feature/your-feature`
3. Make your changes with clear commits
4. Push to your fork: `git push origin feature/your-feature`
5. Open a Pull Request targeting the **`dev`** branch (not `main`)
6. Fill in the PR template completely
7. Wait for review — we aim to respond within 72 hours

**PRs to `main` will be rejected.** All contributions go through `dev` first.

---

## Translations

ro-Control uses the Qt Linguist pipeline for UI localization.

Translation rules:

- Use `qsTr(...)` for QML strings and `tr(...)` for C++ strings
- Keep English as the source language in code
- Update `.ts` files under `i18n/`
- Verify the UI at 980x640 and at a larger desktop size
- Include screenshots if a translation materially changes layout

Recommended workflow:

```bash
sudo dnf install qt6-qttools-devel
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
lupdate src -ts i18n/ro-control_en.ts i18n/ro-control_tr.ts
linguist i18n/ro-control_tr.ts
cmake --build build
```

When adding a new language:

1. Copy `i18n/ro-control_en.ts` to `i18n/ro-control_<locale>.ts`
2. Add the new file to `TS_FILES` in `CMakeLists.txt`
3. Translate all entries
4. Verify runtime locale loading and layout integrity

---

## Reporting Bugs

Use the [bug report template](.github/ISSUE_TEMPLATE/bug_report.md) and include:

- Distribution / platform details
- GPU model (`lspci | grep -i nvidia`)
- Current driver version (`nvidia-smi`)
- Steps to reproduce
- Expected vs actual behavior
- Relevant terminal output, `coredumpctl info ro-control`, or recent journal entries
