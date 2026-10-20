#!/usr/bin/env bash
# Run in a fresh Fedora 44 container on the RPM's native architecture.
set -euo pipefail
[[ "$(rpm -E '%{fedora}')" == 44 ]]
[[ "$(uname -m)" == "${RPM_ARCH}" ]]
mapfile -t packages < <(find release-rpms -maxdepth 1 -name "*.${RPM_ARCH}.rpm" -type f)
[[ "${#packages[@]}" == 1 ]]
package="${packages[0]}"
[[ "$(rpm -qp --qf '%{NVRA}.rpm' "${package}")" == "$(basename "${package}")" ]]
[[ "$(rpm -qp --qf '%{RELEASE}' "${package}")" == 1.fc44 ]]
rpm -qpR "${package}" | tee /tmp/ro-control.requires
if grep -q PRIVATE_API /tmp/ro-control.requires; then
  echo 'Forbidden Qt private ABI dependency' >&2
  exit 1
fi
# Resolve and run RPM's test transaction before touching the installed set.
# No --allowerasing: dependency and file conflicts must fail the gate.
dnf -y --refresh --setopt=install_weak_deps=False --setopt=tsflags=test install "${package}"
dnf -y --setopt=install_weak_deps=False install "${package}"
rpm -V ro-control
rpm -q polkit systemd
rpm -q --whatprovides /usr/bin/ro-control | grep '^ro-control-'
for command in ro-control pkexec dnf rpm free lspci; do command -v "${command}"; done
ldd -r /usr/bin/ro-control | tee /tmp/ro-control.ldd
if grep -E 'not found|undefined symbol' /tmp/ro-control.ldd; then exit 1; fi
ro-control --help | tee /tmp/ro-control.help
ro-control --version | tee /tmp/ro-control.version
grep -F "${VERSION}" /tmp/ro-control.version
# Exercise replacement without removing dependencies or masking conflicts.
dnf -y reinstall "${package}"
rpm -V ro-control
# Read the unchanged legacy release only, to verify the supported upgrade path.
dnf -y install curl
dnf -y remove ro-control
curl --fail --location --retry 3 \
  "https://github.com/Project-Ro-ASD/ro-Control/releases/download/v1.3.1/ro-control-1.3.1-1.${RPM_ARCH}.rpm" \
  --output /tmp/ro-control-baseline.rpm
dnf -y --setopt=install_weak_deps=False install /tmp/ro-control-baseline.rpm
dnf -y --setopt=install_weak_deps=False upgrade "${package}"
[[ "$(rpm -q --qf '%{VERSION}-%{RELEASE}' ro-control)" == "${VERSION}-1.fc44" ]]
rpm -V ro-control
ro-control --help >/dev/null
