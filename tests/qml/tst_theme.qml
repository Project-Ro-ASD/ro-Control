import QtQuick
import QtTest
import "../../src/qml/style" as Style
import "../../src/qml/components" as Components

TestCase {
    id: testCase
    name: "RoTheme"
    visible: true
    when: windowShown
    width: 640
    height: 480
    Style.Theme { id: theme }
    Components.ActionButton {
        id: action
        theme: theme
        text: "Apply"
        tone: "primary"
    }
    SignalSpy { id: clicked; target: action; signalName: "clicked" }
    Components.StatusBanner {
        id: status
        y: 70
        width: parent.width
        theme: theme
        tone: "error"
    }
    Components.StatCard {
        id: stat
        y: 150
        width: parent.width
        theme: theme
        title: "CPU"
        value: "Unavailable"
    }
    function luminance(c) {
        function linear(v) { return v <= 0.04045 ? v / 12.92 : Math.pow((v + 0.055) / 1.055, 2.4); }
        return 0.2126 * linear(c.r) + 0.7152 * linear(c.g) + 0.0722 * linear(c.b);
    }
    function contrast(a, b) {
        let x = luminance(a), y = luminance(b);
        return (Math.max(x, y) + 0.05) / (Math.min(x, y) + 0.05);
    }
    function init() { clicked.clear(); action.enabled = true; status.text = ""; stat.busy = false; }
    function test_palette_data() { return [{tag: "light", dark: false}, {tag: "dark", dark: true}]; }
    function test_palette(data) {
        theme.darkMode = data.dark;
        for (let surface of [theme.window, theme.card, theme.cardStrong]) {
            verify(contrast(theme.text, surface) >= 4.5);
            verify(contrast(theme.textMuted, surface) >= 4.5);
        }
        verify(contrast(theme.onAccent, theme.accentA) >= 4.5);
        verify(contrast(theme.controlBorder, theme.card) >= 3);
        for (let state of [theme.success, theme.warning, theme.danger])
            verify(contrast(state, theme.card) >= 4.5);
    }
    function test_reducedMotion() {
        Style.Motion.factor = 0;
        compare(Style.Motion.fastMs, 0);
        Style.Motion.factor = 1;
        compare(Style.Motion.fastMs, Style.RoTokens.motionFastMs);
    }
    function test_keyboardAction() {
        action.forceActiveFocus(Qt.TabFocusReason);
        verify(action.activeFocus);
        verify(action.visualFocus);
        verify(findChild(action, "keyboardFocusRing").visible);
        keyClick(Qt.Key_Space);
        compare(clicked.count, 1);
        action.enabled = false;
        keyClick(Qt.Key_Space);
        compare(clicked.count, 1);
    }
    function test_loadingEmptyError() {
        compare(stat.value, "Unavailable");
        stat.busy = true;
        wait(30);
        verify(stat.busy);
        verify(!status.visible);
        status.text = "Driver installation failed. Retry after checking the package source.";
        verify(status.visible);
        compare(status.tone, "error");
        for (let width of [320, 600]) {
            status.width = width;
            wait(30);
            verify(status.implicitHeight > 0);
            let screenshot = grabImage(status);
            compare(screenshot.width, width);
        }
    }
}
