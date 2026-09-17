%global upstream_version %{!?upstream_version:1.3.1}%{?upstream_version}
%global debug_package %{nil}

Name:           ro-control
Version:        %{upstream_version}
Release:        1
Summary:        Smart NVIDIA driver manager and hardware monitor for Ro-ASD

License:        GPL-3.0-or-later
Vendor:         Sopwit
URL:            https://github.com/Project-Ro-ASD/ro-Control
Source0:        https://github.com/Project-Ro-ASD/ro-Control/archive/refs/tags/v%{version}.tar.gz#/%{name}-%{version}.tar.gz
ExclusiveArch:  x86_64 aarch64

BuildRequires:  cmake
BuildRequires:  gcc-c++
BuildRequires:  extra-cmake-modules
BuildRequires:  ninja-build
BuildRequires:  qt6-qtbase-devel
BuildRequires:  qt6-qtdeclarative-devel
BuildRequires:  qt6-qttools-devel
BuildRequires:  qt6-qtwayland-devel
BuildRequires:  kf6-qqc2-desktop-style
BuildRequires:  polkit-devel
BuildRequires:  systemd-rpm-macros
BuildRequires:  desktop-file-utils

Requires:       qt6-qtbase
Requires:       qt6-qtdeclarative
Requires:       qt6-qtwayland

Requires:       kf6-qqc2-desktop-style
Requires:       polkit
Requires:       /usr/bin/dnf
Requires:       /usr/bin/pkexec
Requires:       /usr/bin/rpm
Requires:       pciutils
Requires:       /usr/bin/free
Recommends:     mokutil
Recommends:     kmod
Recommends:     /usr/bin/sensors
Recommends:     /usr/sbin/akmods
Recommends:     /usr/bin/dracut
Recommends:     /usr/sbin/grubby

%description
ro-Control is the central hardware management utility developed for Ro-ASD.
It provides automated NVIDIA graphics driver installation and updates,
multi-fan cooling control with custom temperature curves, real-time
hardware diagnostics (GPU, CPU, RAM usage, temperatures, and power draw),
and emergency thermal protection.

%prep
%autosetup -c -T -n %{name}-%{version}
tar -xzf %{SOURCE0} --strip-components=1

%build
%cmake \
    -DBUILD_TESTS=ON \
    -DREQUIRE_TRANSLATIONS=ON \
    -DCMAKE_SKIP_INSTALL_RPATH=ON
%cmake_build

%install
%cmake_install

%check
export QT_QPA_PLATFORM=offscreen
export QT_QUICK_CONTROLS_STYLE=Basic
%ctest --output-on-failure
desktop-file-validate %{buildroot}%{_datadir}/applications/io.github.projectroasd.rocontrol.desktop

%post
/bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null || :
/usr/bin/update-desktop-database &>/dev/null || :
%systemd_post ro-control.service
%systemd_user_post ro-control.service

%preun
%systemd_preun ro-control.service
%systemd_user_preun ro-control.service

%postun
if [ $1 -eq 0 ] ; then
    /bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null || :
    /usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :
    /usr/bin/update-desktop-database &>/dev/null || :
fi
%systemd_postun_with_restart ro-control.service
%systemd_user_postun_with_restart ro-control.service

%posttrans
/usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :

%files
%license LICENSE
%{_bindir}/ro-control
%{_datadir}/applications/io.github.projectroasd.rocontrol.desktop
%{_datadir}/man/man1/ro-control.1*
%{_datadir}/metainfo/io.github.projectroasd.rocontrol.metainfo.xml
%{_datadir}/icons/hicolor/256x256/apps/ro-control.png
%{_datadir}/icons/hicolor/256x256/apps/io.github.projectroasd.rocontrol.png
%{_datadir}/icons/hicolor/scalable/apps/ro-control.svg
%{_datadir}/icons/hicolor/scalable/apps/io.github.projectroasd.rocontrol.svg
%{_datadir}/bash-completion/completions/ro-control
%{_datadir}/zsh/site-functions/_ro-control
%{_datadir}/fish/vendor_completions.d/ro-control.fish
%{_libexecdir}/ro-control-helper
%{_datadir}/polkit-1/actions/io.github.ProjectRoASD.rocontrol.policy
%{_prefix}/lib/systemd/system/ro-control.service
%{_prefix}/lib/systemd/user/ro-control.service

%changelog
* Mon Sep 07 2026 Sopwit <sopwith.osdev@gmail.com> - 1.3.1-1
- Keep GPU refresh work off the QML/UI thread.
- Back off optional NVIDIA fan RPM probes when NV-CONTROL is unavailable.
- Preserve live sysfs telemetry and thermal safety checks during probe backoff.

* Tue Sep 01 2026 Sopwit <sopwith.osdev@gmail.com> - 1.3.0-1
- SystemInfoProvider integration across Monitor, Driver, and Fan suites
- Battery profile auto-sync and power source status indicators
- Enhanced fan controller with GPU topology synchronization and dynamic curve points
- Code quality hardening, qmllint stabilization, and AppStream 1.0 alignment

* Thu Aug 27 2026 ro-Control Maintainers <noreply@github.com> - 1.2.0-1
- Add advanced fan management and telemetry subsystem
- Implement dedicated per-fan control, curves, and mode presets
- Hardware-aware GPU and CPU fan topology discovery
- Robust CI and headless test stabilization

* Sun May 10 2026 ro-Control Maintainers <noreply@github.com> - 1.1.0-2
- Merge runtime assets back into the main architecture RPM
- Make each release RPM installable on its own without a companion noarch package
- Keep AppStream, desktop entry, icons, helper, and policy metadata in the main package

* Sun May 10 2026 ro-Control Maintainers <noreply@github.com> - 1.1.0-1
- Target Fedora 43 for CI, RPM validation, and release builds
- Validate RPM compatibility for x86_64 and aarch64 with store metadata checks
- Align AppStream metadata with real application screenshots and improved package identity

* Mon Mar 30 2026 ro-Control Maintainers <noreply@github.com> - 0.2.1-1
- Limit release outputs to x86_64, aarch64, noarch, and src RPM artifacts
- Split shared desktop assets into a noarch companion package

* Mon Mar 30 2026 ro-Control Maintainers <noreply@github.com> - 0.2.0-1
- Fix installed helper path resolution for privileged operations on system installs
- Activate saved KDE-friendly interface preferences and theme switching in the UI
- Harden Ro-ASD CI and release validation for metadata and RPM packaging
- Limit published RPM outputs to x86_64, aarch64, src, and noarch artifacts only

* Sun Mar 22 2026 ro-Control Maintainers <noreply@github.com> - 0.1.0-1
- Prepare first GitHub Release RPMs for x86_64 and aarch64
- Add explicit Ro-ASD runtime command dependencies and recommendations
- Align RPM release automation with tagged versioned source archives
