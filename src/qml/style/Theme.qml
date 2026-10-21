import QtQuick
import "Palettes.js" as Palettes

QtObject {
    id: theme
    property bool darkMode: false
    property bool followSystem: false
    readonly property var tokens: RoTokens
    readonly property var fallback: darkMode ? Palettes.dark : Palettes.light
    readonly property SystemPalette nativePalette: SystemPalette { colorGroup: SystemPalette.Active }
    readonly property bool nativeDark: nativePalette.window.hslLightness < 0.5
    readonly property bool useNative: followSystem && nativeDark === darkMode
    function mix(base, foreground, amount) {
        return Qt.rgba(base.r * (1 - amount) + foreground.r * amount,
                       base.g * (1 - amount) + foreground.g * amount,
                       base.b * (1 - amount) + foreground.b * amount, 1);
    }
    readonly property color window: useNative ? nativePalette.window : fallback.bg
    readonly property color shell: window
    readonly property color shellAlt: card
    readonly property color card: useNative ? nativePalette.base : fallback.surface
    readonly property color text: useNative ? nativePalette.windowText : fallback.text
    readonly property color textMuted: fallback.textSecondary
    readonly property color textSoft: textMuted
    readonly property color cardStrong: useNative ? nativePalette.button : fallback.surfaceAlt
    readonly property color border: fallback.border
    readonly property color controlBorder: fallback.controlBorder
    readonly property color accentA: useNative ? nativePalette.highlight : fallback.accent
    readonly property color accentB: accentA
    readonly property color accentC: mix(card, accentA, tokens.opacityListCurrent)
    readonly property color onAccent: useNative ? nativePalette.highlightedText : fallback.selectionText
    readonly property color success: fallback.positive
    readonly property color warning: fallback.neutral
    readonly property color danger: fallback.negative
    readonly property color successBg: mix(card, success, tokens.opacityMixCard)
    readonly property color warningBg: mix(card, warning, tokens.opacityMixCard)
    readonly property color dangerBg: mix(card, danger, tokens.opacityMixCard)
    readonly property color infoBg: mix(card, accentA, tokens.opacityMixCard)
    readonly property color hover: mix(card, text, tokens.opacityHover)
    readonly property color pressed: mix(card, text, tokens.opacityMixPressed)
}
