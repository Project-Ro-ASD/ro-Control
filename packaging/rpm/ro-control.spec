Name:           ro-control
Version:        0.1.0
Release:        2%{?dist}
Summary:        Smart NVIDIA driver manager and system monitor

License:        GPL-3.0-or-later
URL:            https://github.com/Project-Ro-ASD/ro-Control
Source0:        %{name}-%{version}.tar.gz

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

Requires:       qt6-qtbase
Requires:       qt6-qtdeclarative
Requires:       qt6-qtwayland
Requires:       kf6-qqc2-desktop-style
Requires:       polkit
Requires:       dnf
Requires:       /usr/bin/pkexec

%description
ro-Control is a Qt6/KDE Plasma desktop application that helps users
manage NVIDIA drivers and monitor core system metrics.

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
%license LICENSE
%doc README.md README.tr.md CHANGELOG.md
%{_bindir}/ro-control
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
* Mon Mar 16 2026 ro-Control Maintainers <noreply@github.com> - 0.1.0-2
- Fix Fedora runtime dependencies for DNF and pkexec
- Restore standard RPM artifact naming to avoid output collisions

* Fri Mar 06 2026 ro-Control Maintainers <noreply@github.com> - 0.1.0-1
- Initial RPM packaging spec
