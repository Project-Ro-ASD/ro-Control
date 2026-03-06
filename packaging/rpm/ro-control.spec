Name:           ro-control
Version:        0.1.0
Release:        1%{?dist}
Summary:        Smart NVIDIA driver manager and system monitor for Fedora

License:        GPL-3.0-or-later
URL:            https://github.com/Acik-Kaynak-Gelistirme-Toplulugu/ro-Control
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  cmake
BuildRequires:  gcc-c++
BuildRequires:  extra-cmake-modules
BuildRequires:  qt6-qtbase-devel
BuildRequires:  qt6-qtdeclarative-devel
BuildRequires:  qt6-qtwayland-devel
BuildRequires:  kf6-qqc2-desktop-style
BuildRequires:  polkit-devel

Requires:       qt6-qtbase
Requires:       qt6-qtdeclarative
Requires:       qt6-qtwayland
Requires:       kf6-qqc2-desktop-style
Requires:       polkit

%description
ro-Control is a Qt6/KDE Plasma desktop application for Fedora that helps users
manage NVIDIA drivers and monitor core system metrics.

%prep
%autosetup -n %{name}-%{version}

%build
%cmake -DBUILD_TESTS=OFF
%cmake_build

%install
%cmake_install

%files
%license LICENSE
%doc README.md README.tr.md CHANGELOG.md
%{_bindir}/ro-control
%{_datadir}/applications/ro-control.desktop
%{_datadir}/metainfo/ro-control.metainfo.xml
%{_datadir}/icons/hicolor/scalable/apps/ro-control.svg
%{_datadir}/polkit-1/actions/com.github.AcikKaynakGelistirmeToplulugu.rocontrol.policy

%changelog
* Fri Mar 06 2026 ro-Control Maintainers <noreply@github.com> - 0.1.0-1
- Initial RPM packaging spec for Fedora builds
