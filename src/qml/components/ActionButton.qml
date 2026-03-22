import QtQuick
import QtQuick.Controls

Button {
    id: control

    required property var theme
    property string tone: "neutral"

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

    implicitHeight: 46
    implicitWidth: Math.max(160, contentItem.implicitWidth + leftPadding + rightPadding)
    leftPadding: 18
    rightPadding: 18
    topPadding: 12
    bottomPadding: 12

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
        radius: 14
        color: control.down && control.enabled ? Qt.darker(control.fillColor, 1.08)
                                               : control.hovered && control.enabled ? Qt.tint(control.fillColor, "#08ffffff")
                                                                                   : control.fillColor
        border.width: 1
        border.color: control.borderColor
        opacity: control.enabled ? 1.0 : 0.6
    }
}
