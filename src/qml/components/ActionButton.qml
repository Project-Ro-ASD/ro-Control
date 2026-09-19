import QtQuick
import QtQuick.Controls

Button {
    id: control

    required property var theme
    property string tone: "neutral"
    // Compact is intended for toolbar and dialog actions. Keeping the same
    // visual language avoids falling back to the platform-default button.
    property bool compact: false
    property real uiScale: 1.0

    readonly property color fillColor: !enabled ? Qt.rgba(0, 0, 0, 0)
                                       : tone === "primary" ? theme.accentA
                                       : tone === "success" ? theme.success
                                       : tone === "warning" ? theme.warning
                                       : tone === "danger" ? theme.danger
                                       : theme.cardStrong
    readonly property color borderColor: !enabled ? theme.border
                                         : tone === "primary" ? Qt.tint(theme.accentA, "#33ffffff")
                                         : tone === "success" ? Qt.tint(theme.success, "#22ffffff")
                                         : tone === "warning" ? Qt.tint(theme.warning, "#18ffffff")
                                         : tone === "danger" ? Qt.tint(theme.danger, "#18ffffff")
                                         : theme.border
    readonly property color textColor: !enabled ? theme.textSoft
                                       : tone === "neutral" ? theme.text
                                       : "#ffffff"

    hoverEnabled: true
    implicitHeight: Math.round((compact ? 36 : 46) * uiScale)
    implicitWidth: Math.max(Math.round((compact ? 96 : 160) * uiScale), contentItem.implicitWidth + leftPadding + rightPadding)
    leftPadding: Math.round((compact ? 14 : 18) * uiScale)
    rightPadding: Math.round((compact ? 14 : 18) * uiScale)
    topPadding: Math.round((compact ? 8 : 12) * uiScale)
    bottomPadding: Math.round((compact ? 8 : 12) * uiScale)

    scale: !enabled ? 1.0 : (down ? 0.98 : (hovered ? 1.01 : 1.0))
    Behavior on scale {
        NumberAnimation { duration: 100; easing.type: Easing.OutCubic }
    }

    contentItem: Text {
        text: control.text
        color: control.textColor
        font.pixelSize: Math.round((compact ? 12 : 15) * control.uiScale)
        font.weight: Font.DemiBold
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        radius: Math.round((compact ? 9 : 14) * control.uiScale)
        color: control.down && control.enabled ? Qt.darker(control.fillColor, 1.08)
                                               : control.hovered && control.enabled ? Qt.tint(control.fillColor, "#08ffffff")
                                                                                   : control.fillColor
        border.width: control.activeFocus ? 2 : 1
        border.color: control.activeFocus ? control.theme.accentA : control.borderColor
        opacity: control.enabled ? 1.0 : 0.6

        Behavior on color {
            ColorAnimation { duration: 120 }
        }
    }
}
