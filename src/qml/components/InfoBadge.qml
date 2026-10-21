import QtQuick
import "../style" as Style
import QtQuick.Controls

Rectangle {
    id: badge
    Style.Theme { id: fallbackTheme; darkMode: false }

    property var theme: fallbackTheme
    property string text: ""
    property color backgroundColor: theme.accentC
    property color foregroundColor: theme.text

    radius: 999
    color: backgroundColor
    border.width: 1
    border.color: Qt.tint(backgroundColor, Qt.rgba(theme.window.r, theme.window.g, theme.window.b, 0.1333))
    implicitHeight: 36
    implicitWidth: badgeLabel.implicitWidth + 28

    Label {
        id: badgeLabel
        anchors.centerIn: parent
        text: badge.text
        font.pixelSize: 13
        font.weight: Font.DemiBold
        color: badge.foregroundColor
    }
}
