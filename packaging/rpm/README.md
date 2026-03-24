# RPM Packaging

This directory contains the RPM recipe for ro-Control.

## Goals

- Produce a reproducible RPM from a release tarball
- Require translation tooling so localized builds are never emitted partially
- Run the upstream Qt test suite during `%check`
- Publish GitHub Release RPMs for both `x86_64` and `aarch64`

## Source archive expectations

The spec assumes `Source0` is a `.tar.gz` archive containing a single top-level
directory. During `%prep`, the archive is unpacked with `--strip-components=1`
into a deterministic `%{name}-%{version}` build directory so release archives do
not need to preserve a specific upstream folder name.

## Build dependencies

- `cmake`
- `gcc-c++`
- `extra-cmake-modules`
- `qt6-qtbase-devel`
- `qt6-qtdeclarative-devel`
- `qt6-qttools-devel`
- `qt6-qtwayland-devel`
- `kf6-qqc2-desktop-style`
- `polkit-devel`

## Local build example

```bash
spectool -g -R packaging/rpm/ro-control.spec
rpmbuild -ba packaging/rpm/ro-control.spec
```

To build a different release version from the same spec, override the version
macro explicitly:

```bash
rpmbuild -ba packaging/rpm/ro-control.spec \
  --define "upstream_version 0.1.0"
```

If you build from a Git checkout instead of a published source archive, create
the tarball first so `%Source0` matches the spec contract.

The RPM installs the PolicyKit helper policy as
`io.github.ProjectRoASD.rocontrol.policy` and the privileged helper as
`/usr/libexec/ro-control-helper`.

It also installs the CLI manual page and shell completions for Bash, Zsh, and
Fish so command discovery works out of the box on release systems.

## Release automation

The GitHub release workflow builds:

- source archives (`.tar.gz`, `.zip`)
- one Fedora binary RPM for `x86_64`
- one Fedora binary RPM for `aarch64`
- one Fedora release bundle for `x86_64`
- one Fedora release bundle for `aarch64`
- one source RPM

Each architecture job also performs a smoke install with `dnf install` and
verifies that `ro-control --version` matches the tagged release version before
publishing assets. Each release bundle contains the architecture-specific RPM,
package metadata, dependency list, and checksum manifest for easier download
from the GitHub Releases page.
