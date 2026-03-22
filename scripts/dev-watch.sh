#!/usr/bin/env bash
# dev-watch.sh — Kaynak degisikliklerini izler, otomatik build alir ve uygulamayi yeniden baslatir.
# Kullanim: ./scripts/dev-watch.sh
# Gereksinim: sudo dnf install inotify-tools

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build}"
BINARY="$BUILD_DIR/ro-control"
APP_PID=""

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
BOLD='\033[1m'
RESET='\033[0m'

log()  { echo -e "${CYAN}[dev-watch]${RESET} $*"; }
ok()   { echo -e "${GREEN}[dev-watch]${RESET} $*"; }
warn() { echo -e "${YELLOW}[dev-watch]${RESET} $*"; }
err()  { echo -e "${RED}[dev-watch]${RESET} $*"; }

# --- Bagimlilik kontrolu ---
if ! command -v inotifywait &>/dev/null; then
    err "inotify-tools bulunamadi. Kurmak icin:"
    err "  sudo dnf install inotify-tools"
    exit 1
fi

if [[ ! -d "$BUILD_DIR" || ! -f "$BUILD_DIR/CMakeCache.txt" ]]; then
    warn "Build dizini yok veya cmake yapilandirilmamis."
    warn "Once sunu calistir: ./scripts/fedora-bootstrap.sh"
    exit 1
fi

# --- Uygulamayi durdur ---
stop_app() {
    if [[ -n "$APP_PID" ]] && kill -0 "$APP_PID" 2>/dev/null; then
        log "Uygulama durduruluyor (PID: $APP_PID)..."
        kill "$APP_PID" 2>/dev/null || true
        wait "$APP_PID" 2>/dev/null || true
        APP_PID=""
    fi
}

# --- Incremental build + yeniden basla ---
build_and_run() {
    echo ""
    log "Incremental build basliyor..."
    if cmake --build "$BUILD_DIR" -j"$(nproc)" 2>&1; then
        ok "Build basarili"
        stop_app
        log "Uygulama baslatiliyor: $BINARY"
        "$BINARY" &
        APP_PID=$!
        ok "ro-control calisiyor (PID: $APP_PID)"
    else
        err "Build hatasi -- degisiklikleri kontrol et."
    fi
    echo ""
}

# --- Temiz cikis ---
cleanup() {
    echo ""
    warn "Cikis sinyali alindi."
    stop_app
    exit 0
}
trap cleanup SIGINT SIGTERM

# --- Baslangic ---
echo ""
log "ro-Control dev-watch modu"
log "Izlenen dizin : $ROOT_DIR/src"
log "Build dizini  : $BUILD_DIR"
log "Cikmak icin   : Ctrl+C"
echo ""

build_and_run

# --- Degisiklik izleme dongusu ---
inotifywait -m -r \
    --include '\.(cpp|h|qml|js|ts)$' \
    -e modify,create,delete,moved_to \
    --format "%w%f  [%e]" \
    "$ROOT_DIR/src" "$ROOT_DIR/i18n" 2>/dev/null \
| while IFS= read -r line; do
    log "Degisiklik algilandi: $line"
    sleep 0.8

    # Kuyruktaki diger olaylari bosalt (debounce)
    while IFS= read -t 0.1 -r _extra; do :; done <&0 2>/dev/null || true

    build_and_run
done
