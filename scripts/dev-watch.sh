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
RESET='\033[0m'

log()  { echo -e "${CYAN}[dev-watch]${RESET} $*"; }
ok()   { echo -e "${GREEN}[dev-watch]${RESET} $*"; }
warn() { echo -e "${YELLOW}[dev-watch]${RESET} $*"; }
err()  { echo -e "${RED}[dev-watch]${RESET} $*"; }

# --- Qt render backend otomatik sec ---
# GPU olmadan calisan sistemlerde (NVIDIA surucusu kurulu degil, VM, vb.)
# Qt'un EGL hatasi vermemesi icin fallback backend ayarla
setup_qt_env() {
    # Eger kullanici zaten bir backend secmisse dokunma
    if [[ -n "${QSG_RHI_BACKEND:-}" || -n "${QT_XCB_GL_INTEGRATION:-}" ]]; then
        return
    fi

    # EGL/DRI2 kullanilabilir mi kontrol et
    if command -v glxinfo &>/dev/null && glxinfo 2>/dev/null | grep -q "direct rendering: Yes"; then
        # Donanim hizlandirma var, varsayilan backend kullan
        log "OpenGL donanim hizlandirma mevcut, varsayilan renderer kullaniliyor."
    else
        # Yazilim renderer'a gec - GPU olmayan / surucusuz ortam
        warn "GPU/EGL hizlandirma bulunamadi, yazilim renderer'a geciliyor."
        warn "NVIDIA surucu kurulduktan sonra bu uyari kaybolacak."
        export QT_XCB_GL_INTEGRATION=none
        export LIBGL_ALWAYS_SOFTWARE=0
        export QSG_RENDERER_DEBUG=""
    fi
}

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

if [[ ! -f "$BINARY" ]]; then
    warn "Binary bulunamadi: $BINARY"
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
        log "Uygulama baslatiliyor..."
        "$BINARY" 2>/dev/null &
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

# --- Qt ortam degiskenlerini ayarla ---
setup_qt_env

# --- Baslangic ---
echo ""
log "ro-Control dev-watch modu"
log "Proje dizini  : $ROOT_DIR"
log "Build dizini  : $BUILD_DIR"
log "Izlenen dizin : $ROOT_DIR/src"
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
