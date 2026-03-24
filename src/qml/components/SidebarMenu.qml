import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: sidebar
    width: 286
    required property var theme
    color: theme.sidebarBg
    border.width: 1
    border.color: theme.sidebarBorder
    clip: true

    property int currentIndex: 0
    readonly property var menuItems: [
        { title: qsTr("Install"), marker: "\u2193" },
        { title: qsTr("Expert"), marker: "\u2699" },
        { title: qsTr("Monitor"), marker: "\u223f" }
    ]

    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: 22
        anchors.bottomMargin: 22
        spacing: 0

        Repeater {
            model: sidebar.menuItems

            delegate: Rectangle {
                id: menuCard
                required property int index
                required property var modelData

                Layout.leftMargin: 22
                Layout.rightMargin: 22
                Layout.topMargin: index === 0 ? 0 : 10
                Layout.fillWidth: true
                implicitHeight: 90
                radius: 22
                color: sidebar.currentIndex === index ? theme.sidebarActive
                                                      : "transparent"
                border.width: sidebar.currentIndex === index ? 1 : 0
                border.color: theme.sidebarBorder

                Rectangle {
                    visible: sidebar.currentIndex === index
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    width: 4
                    height: parent.height - 20
                    radius: 2
                    color: theme.sidebarAccent
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 22
                    anchors.rightMargin: 22
                    spacing: 16

                    Rectangle {
                        width: 48
                        height: 48
                        radius: 16
                        color: sidebar.currentIndex === index ? theme.sidebarAccent : theme.cardStrong

                        Label {
                            anchors.centerIn: parent
                            text: menuCard.modelData.marker
                            color: sidebar.currentIndex === index ? "#ffffff" : theme.sidebarMuted
                            font.pixelSize: 20
                            font.weight: Font.Bold
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        text: menuCard.modelData.title
                        color: sidebar.currentIndex === index ? theme.sidebarText : theme.sidebarText
                        font.pixelSize: 18
                        font.weight: sidebar.currentIndex === index ? Font.Bold : Font.DemiBold
                    }

                    Rectangle {
                        width: 10
                        height: 10
                        radius: 5
                        visible: sidebar.currentIndex === index
                        color: theme.sidebarAccent
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onEntered: {
                        if (sidebar.currentIndex !== index)
                            menuCard.color = theme.sidebarHover;
                    }
                    onExited: {
                        if (sidebar.currentIndex !== index)
                            menuCard.color = "transparent";
                    }
                    onClicked: sidebar.currentIndex = index
                }
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
