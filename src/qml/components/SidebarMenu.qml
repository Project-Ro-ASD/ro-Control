import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: sidebar
    width: 220
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
        anchors.bottomMargin: 16
        text: "v" + Qt.application.version
        font.pixelSize: 11
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
            implicitHeight: 70

            Label {
                anchors.centerIn: parent
                text: qsTr("ro-Control")
                font.pixelSize: 22
                font.bold: true
                color: theme.sidebarText
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.leftMargin: 16
            Layout.rightMargin: 16
            height: 1
            color: theme.sidebarBorder
        }

        Item {
            Layout.fillWidth: true
            implicitHeight: 12
        }

        Repeater {
            model: sidebar.menuItems

            delegate: Rectangle {
                id: menuItem
                required property int index
                required property string modelData

                Layout.fillWidth: true
                Layout.leftMargin: 8
                Layout.rightMargin: 8
                implicitHeight: 44
                radius: 8
                color: sidebar.currentIndex === menuItem.index ? theme.sidebarActive
                                                               : mouseArea.containsMouse ? theme.sidebarHover
                                                                                         : "transparent"

                Label {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 16
                    anchors.right: parent.right
                    anchors.rightMargin: 8
                    text: menuItem.modelData
                    font.pixelSize: 14
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
