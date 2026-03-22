import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: sidebar
    width: 248
    required property var theme
    color: theme.sidebarBg
    clip: true

    property int currentIndex: 0
    readonly property var menuItems: [
        qsTr("Driver Management"),
        qsTr("System Monitoring"),
        qsTr("Settings")
    ]

    // Versiyon — alt köşe
    Label {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 20
        text: "v" + Qt.application.version
        font.pixelSize: 12
        color: theme.sidebarHint
        z: 1
    }

    ColumnLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        spacing: 0

        // Başlık
        Item {
            Layout.fillWidth: true
            implicitHeight: 82

            Label {
                anchors.left: parent.left
                anchors.leftMargin: 22
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("ro-Control")
                font.pixelSize: 25
                font.weight: Font.Bold
                color: theme.sidebarText
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            height: 1
            color: theme.sidebarBorder
        }

        Item {
            Layout.fillWidth: true
            implicitHeight: 16
        }

        Repeater {
            model: sidebar.menuItems

            delegate: Rectangle {
                id: menuItem
                required property int index
                required property string modelData

                Layout.fillWidth: true
                Layout.leftMargin: 10
                Layout.rightMargin: 10
                implicitHeight: 52
                radius: 14
                color: sidebar.currentIndex === menuItem.index ? theme.sidebarActive
                                                               : mouseArea.containsMouse ? theme.sidebarHover
                                                                                         : "transparent"
                border.width: sidebar.currentIndex === menuItem.index ? 1 : 0
                border.color: sidebar.currentIndex === menuItem.index ? theme.sidebarBorder : "transparent"

                Label {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 22
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    text: menuItem.modelData
                    font.pixelSize: 15
                    font.weight: sidebar.currentIndex === menuItem.index ? Font.DemiBold : Font.Medium
                    color: sidebar.currentIndex === menuItem.index ? theme.sidebarAccent : theme.sidebarMuted
                    elide: Text.ElideRight
                }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: sidebar.currentIndex = menuItem.index
                }
            }
        }
    }
}
