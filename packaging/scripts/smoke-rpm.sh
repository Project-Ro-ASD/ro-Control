#!/usr/bin/env bash
# Run in a fresh Fedora container on the RPM's native architecture.
set -euo pipefail
FEDORA_RELEASE="${FEDORA_RELEASE:-44}"
[[ "${FEDORA_RELEASE}" == 44 || "${FEDORA_RELEASE}" == 45 ]]
[[ "$(rpm -E '%{fedora}')" == "${FEDORA_RELEASE}" ]]
[[ "$(uname -m)" == "${RPM_ARCH}" ]]
mapfile -t packages < <(find release-rpms -maxdepth 1 -name "*.${RPM_ARCH}.rpm" -type f)
[[ "${#packages[@]}" == 1 ]]
package="${packages[0]}"
[[ "$(rpm -qp --qf '%{NVRA}.rpm' "${package}")" == "$(basename "${package}")" ]]
[[ "$(rpm -qp --qf '%{RELEASE}' "${package}")" == "1.fc${FEDORA_RELEASE}" ]]
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
dnf -y remove ro-control
if [[ "${FEDORA_RELEASE}" == 45 ]]; then
  # Verify that a Fedora 44 installation can be replaced by the Fedora 45 build.
  mapfile -t baselines < <(find upgrade-rpms -maxdepth 1 -type f -name "*.${RPM_ARCH}.rpm")
  [[ "${#baselines[@]}" == 1 ]]
  [[ "$(rpm -qp --qf '%{VERSION}-%{RELEASE}' "${baselines[0]}")" == "${VERSION}-1.fc44" ]]
  dnf -y --setopt=install_weak_deps=False install "${baselines[0]}"
else
  # Read the unchanged legacy release only; never modify it.
  dnf -y install curl
  curl --fail --location --retry 3 \
    "https://github.com/Project-Ro-ASD/ro-Control/releases/download/v1.3.1/ro-control-1.3.1-1.${RPM_ARCH}.rpm" \
    --output /tmp/ro-control-baseline.rpm
  dnf -y --setopt=install_weak_deps=False install /tmp/ro-control-baseline.rpm
fi
dnf -y --setopt=install_weak_deps=False upgrade "${package}"
[[ "$(rpm -q --qf '%{VERSION}-%{RELEASE}' ro-control)" == "${VERSION}-1.fc${FEDORA_RELEASE}" ]]
rpm -V ro-control
ro-control --help >/dev/null
# Add test tools only after clean dependency and upgrade tests have passed.
dnf -y install python3 dbus-daemon /usr/bin/gdbus
dbus-run-session -- python3 packaging/scripts/smoke-installed-runtime.py
# Load the full installed QML application using software/offscreen rendering.
set +e
QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software QT_QUICK_CONTROLS_STYLE=Basic \
  timeout 15s dbus-run-session -- ro-control >/tmp/ro-control-gui.log 2>&1
gui_status=$?
set -e
cat /tmp/ro-control-gui.log
[[ "${gui_status}" == 124 ]]
if grep -Ei 'failed to load component|module .*is not installed|Type .*unavailable|Cannot load library' /tmp/ro-control-gui.log; then exit 1; fi
