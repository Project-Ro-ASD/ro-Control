# Changelog

All notable changes to ro-Control will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [Unreleased]

### Added
- NVIDIA detection pipeline with driver/module verification report
- Secure Boot detection and session type (Wayland/X11) detection
- Driver install flows for proprietary and open-source (nouveau) options
- Session-aware post-install and post-update handling for Wayland/X11
- Real system monitors for CPU, GPU, and RAM with live QML bindings
- Driver update check/apply flow and deep-clean operation support
- Linux build and test workflow with CMake + Qt6

### Changed
- Driver management UI is wired to backend operations instead of placeholders
- Documentation updated for current architecture and build instructions
- Test suite expanded to cover monitor metric ranges and detector reporting
- Repository metadata and packaging references aligned with the active GitHub organization
- Privileged command flow now uses a dedicated allowlisted helper instead of raw `pkexec` command dispatch

### Fixed
- Command execution path preserves stdout reliably
- RPM repository URL resolution and repository failure handling improved
- Updater API/header alignment and monitor test compatibility issues resolved
- Repository cleanup for stray macOS metadata files
- PolicyKit metadata, helper install path, and packaged action identifiers are now consistent

---

<!-- 
TEMPLATE — copy this block for each new release:

## [X.Y.Z] - YYYY-MM-DD

### Added
- New features

### Changed
- Changes to existing functionality

### Fixed
- Bug fixes

### Removed
- Removed features

### Security
- Security fixes
-->
