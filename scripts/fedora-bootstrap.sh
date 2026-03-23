#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
GENERATOR="${GENERATOR:-Ninja}"
ENABLE_TESTS="${ENABLE_TESTS:-1}"
INSTALL_AFTER_BUILD="${INSTALL_AFTER_BUILD:-0}"
INSTALL_PREFIX="${INSTALL_PREFIX:-/usr/local}"

required_build_reqs=(
  cmake
  extra-cmake-modules
  gcc-c++
  ninja-build
  qt6-qtbase-devel
  qt6-qtbase-private-devel
  qt6-qtdeclarative-devel
  qt6-qttools-devel
  qt6-qtwayland-devel
  polkit-devel
)

runtime_tools=(
  dnf
  polkit
  pciutils
  mokutil
  kmod
  lm_sensors
  procps-ng
)

optional_build_reqs=(
  kf6-qqc2-desktop-style
)

has_dnf_package() {
  local package="$1"
  dnf repoquery --quiet --whatprovides "$package" >/dev/null 2>&1
}

install_packages() {
  local label="$1"
  shift

  if (($# == 0)); then
    return 0
  fi

  echo "$label"
  sudo dnf install -y "$@"
}

collect_available_packages() {
  local -n requested_packages_ref="$1"
  local -n available_packages_ref="$2"
  local -n missing_packages_ref="$3"

  local package
  for package in "${requested_packages_ref[@]}"; do
    if has_dnf_package "$package"; then
      available_packages_ref+=( "$package" )
    else
      missing_packages_ref+=( "$package" )
    fi
  done
}

available_optional_build_reqs=()
missing_optional_build_reqs=()
collect_available_packages optional_build_reqs available_optional_build_reqs missing_optional_build_reqs

install_packages "[1/4] Installing Fedora build dependencies..." "${required_build_reqs[@]}"

if ((${#available_optional_build_reqs[@]} > 0)); then
  install_packages "[1.1/4] Installing optional Fedora desktop integration packages..." \
    "${available_optional_build_reqs[@]}"
fi

if ((${#missing_optional_build_reqs[@]} > 0)); then
  printf '[info] Optional Fedora packages not available in enabled repositories: %s\n' \
    "${missing_optional_build_reqs[*]}"
fi

echo "[2/4] Installing runtime utilities used by diagnostics/driver workflows..."
sudo dnf install -y "${runtime_tools[@]}"

cmake_args=(
  -S "$ROOT_DIR"
  -B "$BUILD_DIR"
  -G "$GENERATOR"
  -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
  -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX"
)

if [[ "$ENABLE_TESTS" == "1" ]]; then
  cmake_args+=( -DBUILD_TESTS=ON )
else
  cmake_args+=( -DBUILD_TESTS=OFF )
fi

echo "[3/4] Configuring and building ($BUILD_TYPE)..."
cmake "${cmake_args[@]}"
cmake --build "$BUILD_DIR" -j"$(nproc)"

if [[ "$ENABLE_TESTS" == "1" ]]; then
  echo "[3.1/4] Running test suite..."
  ctest --test-dir "$BUILD_DIR" --output-on-failure
fi

echo "[4/4] Done."
echo "Run from build dir: $BUILD_DIR/ro-control"

if [[ "$INSTALL_AFTER_BUILD" == "1" ]]; then
  echo "Installing to $INSTALL_PREFIX..."
  sudo cmake --install "$BUILD_DIR"
fi
