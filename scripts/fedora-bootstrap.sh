#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
GENERATOR="${GENERATOR:-Ninja}"
ENABLE_TESTS="${ENABLE_TESTS:-1}"
INSTALL_AFTER_BUILD="${INSTALL_AFTER_BUILD:-0}"
INSTALL_PREFIX="${INSTALL_PREFIX:-/usr/local}"

build_reqs=(
  cmake
  extra-cmake-modules
  gcc-c++
  ninja-build
  qt6-qtbase-devel
  qt6-qtdeclarative-devel
  qt6-qttools-devel
  qt6-qtwayland-devel
  kf6-qqc2-desktop-style
  polkit-devel
)

runtime_tools=(
  dnf
  polkit
  pciutils
  mokutil
  kmod
)

echo "[1/4] Installing Fedora build dependencies..."
sudo dnf install -y "${build_reqs[@]}"

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
