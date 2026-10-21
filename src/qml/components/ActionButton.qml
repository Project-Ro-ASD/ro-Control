import QtQuick
import "../style" as Style
import QtQuick.Controls

Button {
    id: control
    Style.Theme { id: fallbackTheme; darkMode: false }

    required property var theme
    property string tone: "neutral"

    readonly property color fillColor: tone === "primary" ? theme.accentA : theme.cardStrong
    readonly property color borderColor: tone === "danger" ? theme.danger
                                         : tone === "warning" ? theme.warning
                                         : tone === "success" ? theme.success
                                         : tone === "primary" ? theme.accentA : theme.controlBorder
    readonly property color textColor: !enabled ? theme.textSoft
                                       : tone === "primary" ? theme.onAccent
                                       : tone === "danger" ? theme.danger
                                       : tone === "warning" ? theme.warning
                                       : tone === "success" ? theme.success : theme.text
    activeFocusOnTab: true
    opacity: enabled ? 1 : Style.RoTokens.opacityDisabled

    implicitHeight: 46
    implicitWidth: Math.max(160, contentItem.implicitWidth + leftPadding + rightPadding)
    leftPadding: Style.RoTokens.spaceLg
    rightPadding: Style.RoTokens.spaceLg
    topPadding: Style.RoTokens.spaceMd
    bottomPadding: Style.RoTokens.spaceMd

    contentItem: Text {
        text: control.text
        color: control.textColor
        font.pixelSize: 15
        font.weight: Font.DemiBold
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        radius: Style.RoTokens.radiusMd
        color: control.down && control.tone !== "primary" ? control.theme.pressed
                            : control.hovered && control.tone !== "primary" ? control.theme.hover
                            : control.fillColor
        border.width: control.visualFocus ? 2 * Style.RoTokens.borderWidth : Style.RoTokens.borderWidth
        border.color: control.visualFocus ? control.theme.accentA : control.borderColor
    }
}
