%global upstream_version %{!?upstream_version:0.2.1}%{?upstream_version}
%global debug_package %{nil}

Name:           ro-control
Version:        %{upstream_version}
Release:        1%{?dist}
Summary:        Smart NVIDIA driver manager and system monitor

License:        GPL-3.0-or-later
URL:            https://github.com/Project-Ro-ASD/ro-Control
Source0:        %{name}-%{version}.tar.gz
ExclusiveArch:  x86_64 aarch64

BuildRequires:  cmake
BuildRequires:  gcc-c++
BuildRequires:  extra-cmake-modules
BuildRequires:  ninja-build
BuildRequires:  qt6-qtbase-devel
BuildRequires:  qt6-qtbase-private-devel
BuildRequires:  qt6-qtdeclarative-devel
BuildRequires:  qt6-qttools-devel
BuildRequires:  qt6-qtwayland-devel
BuildRequires:  kf6-qqc2-desktop-style
BuildRequires:  polkit-devel

Requires:       %{name}-common = %{version}-%{release}
Requires:       qt6-qtbase
Requires:       qt6-qtdeclarative
Requires:       qt6-qtwayland

%description
ro-Control is a Qt6/KDE Plasma desktop application that helps users
manage NVIDIA drivers and monitor core system metrics.

%package common
Summary:        Shared assets for the ro-Control desktop application
BuildArch:      noarch

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

%description common
ro-Control common ships the desktop entry, helper script, shell completions,
metadata, icons, PolicyKit action, and documentation shared by all supported
CPU architectures.

%prep
%autosetup -c -T -n %{name}-%{version}
tar -xzf %{SOURCE0} --strip-components=1

%build
%cmake \
    -DBUILD_TESTS=ON \
    -DREQUIRE_TRANSLATIONS=ON
%cmake_build

%install
%cmake_install

%check
%ctest --output-on-failure

%files
%{_bindir}/ro-control

%files common
%license LICENSE
%doc README.md README.tr.md CHANGELOG.md
%{_datadir}/applications/io.github.projectroasd.rocontrol.desktop
%{_datadir}/man/man1/ro-control.1*
%{_datadir}/metainfo/io.github.projectroasd.rocontrol.metainfo.xml
%{_datadir}/icons/hicolor/256x256/apps/ro-control.png
%{_datadir}/icons/hicolor/scalable/apps/ro-control.svg
%{_datadir}/bash-completion/completions/ro-control
%{_datadir}/zsh/site-functions/_ro-control
%{_datadir}/fish/vendor_completions.d/ro-control.fish
%{_libexecdir}/ro-control-helper
%{_datadir}/polkit-1/actions/io.github.ProjectRoASD.rocontrol.policy

%changelog
* Mon Mar 30 2026 ro-Control Maintainers <noreply@github.com> - 0.2.1-1
- Limit release outputs to x86_64, aarch64, noarch, and src RPM artifacts
- Split shared desktop assets into a noarch companion package

* Mon Mar 30 2026 ro-Control Maintainers <noreply@github.com> - 0.2.0-1
- Fix installed helper path resolution for privileged operations on system installs
- Activate saved KDE-friendly interface preferences and theme switching in the UI
- Harden Fedora CI and release validation for metadata and RPM packaging
- Limit published RPM outputs to x86_64, aarch64, src, and noarch artifacts only

* Sun Mar 22 2026 ro-Control Maintainers <noreply@github.com> - 0.1.0-1
- Prepare first GitHub Release RPMs for i686, x86_64, and aarch64
- Add explicit Fedora runtime command dependencies and recommendations
- Align RPM release automation with tagged versioned source archives
