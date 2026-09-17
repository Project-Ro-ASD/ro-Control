pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: sidebar
    width: 286
    required property var theme
    color: (theme && theme.sidebarBg) ? theme.sidebarBg : ((theme && theme.shell) ? theme.shell : "#241F33")
    border.width: 1
    border.color: (theme && theme.sidebarBorder) ? theme.sidebarBorder : ((theme && theme.border) ? theme.border : "#383152")
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
                Layout.topMargin: menuCard.index === 0 ? 0 : 10
                Layout.fillWidth: true
                implicitHeight: 90
                radius: 22
                color: sidebar.currentIndex === menuCard.index
                       ? ((sidebar.theme && sidebar.theme.sidebarActive) ? sidebar.theme.sidebarActive : ((sidebar.theme && sidebar.theme.cardStrong) ? sidebar.theme.cardStrong : "#342D4A"))
                       : "transparent"
                border.width: sidebar.currentIndex === menuCard.index ? 1 : 0
                border.color: (sidebar.theme && sidebar.theme.sidebarBorder) ? sidebar.theme.sidebarBorder : ((sidebar.theme && sidebar.theme.border) ? sidebar.theme.border : "#4D436B")

                Rectangle {
                    visible: sidebar.currentIndex === menuCard.index
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    width: 4
                    height: parent.height - 20
                    radius: 2
                    color: (sidebar.theme && sidebar.theme.sidebarAccent) ? sidebar.theme.sidebarAccent : ((sidebar.theme && sidebar.theme.accentA) ? sidebar.theme.accentA : "#818CF8")
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 22
                    anchors.rightMargin: 22
                    spacing: 16

                    Rectangle {
                        implicitWidth: 48
                        implicitHeight: 48
                        radius: 16
                        color: sidebar.currentIndex === menuCard.index
                               ? ((sidebar.theme && sidebar.theme.sidebarAccent) ? sidebar.theme.sidebarAccent : ((sidebar.theme && sidebar.theme.accentA) ? sidebar.theme.accentA : "#818CF8"))
                               : ((sidebar.theme && sidebar.theme.cardStrong) ? sidebar.theme.cardStrong : "#29233B")

                        Label {
                            anchors.centerIn: parent
                            text: menuCard.modelData.marker
                            color: sidebar.currentIndex === menuCard.index
                                   ? "#ffffff"
                                   : ((sidebar.theme && sidebar.theme.sidebarMuted) ? sidebar.theme.sidebarMuted : ((sidebar.theme && sidebar.theme.textSoft) ? sidebar.theme.textSoft : "#94A3B8"))
                            font.pixelSize: 18
                            font.weight: Font.DemiBold
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        text: menuCard.modelData.title
                        color: (sidebar.theme && sidebar.theme.sidebarText) ? sidebar.theme.sidebarText : ((sidebar.theme && sidebar.theme.text) ? sidebar.theme.text : "#F8FAFC")
                        font.pixelSize: 16
                        font.weight: sidebar.currentIndex === menuCard.index ? Font.DemiBold : Font.Medium
                    }

                    Rectangle {
                        implicitWidth: 10
                        implicitHeight: 10
                        radius: 5
                        visible: sidebar.currentIndex === menuCard.index
                        color: (sidebar.theme && sidebar.theme.sidebarAccent) ? sidebar.theme.sidebarAccent : ((sidebar.theme && sidebar.theme.accentA) ? sidebar.theme.accentA : "#818CF8")
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onEntered: {
                        if (sidebar.currentIndex !== menuCard.index)
                            menuCard.color = (sidebar.theme && sidebar.theme.sidebarHover) ? sidebar.theme.sidebarHover : ((sidebar.theme && sidebar.theme.card) ? sidebar.theme.card : "#29233B");
                    }
                    onExited: {
                        if (sidebar.currentIndex !== menuCard.index)
                            menuCard.color = "transparent";
                    }
                    onClicked: sidebar.currentIndex = menuCard.index
                }
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
