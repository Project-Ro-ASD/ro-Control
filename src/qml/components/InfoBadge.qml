import QtQuick
import QtQuick.Controls

Rectangle {
    id: badge

    property string text: ""
    property color backgroundColor: "#e5eefc"
    property color foregroundColor: "#15304f"

    radius: 999
    color: backgroundColor
    border.width: 1
    border.color: Qt.tint(backgroundColor, "#22000000")
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
