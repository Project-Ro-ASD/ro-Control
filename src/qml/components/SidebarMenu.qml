pragma ComponentBehavior: Bound

import QtQuick
import "../style" as Style
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: sidebar
    Style.Theme { id: fallbackTheme; darkMode: false }
    width: 286
    required property var theme
    color: (theme && theme.sidebarBg) ? theme.sidebarBg : ((theme && theme.shell) ? theme.shell : theme.cardStrong)
    border.width: 1
    border.color: (theme && theme.sidebarBorder) ? theme.sidebarBorder : ((theme && theme.border) ? theme.border : theme.hover)
    clip: true

    property int currentIndex: 0
    readonly property var menuItems: [
        { title: qsTr("Install"), marker: "\u2193" },
        { title: qsTr("Expert"), marker: "\u2699" },
        { title: qsTr("Monitor"), marker: "\u223f" }
    ]

    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: Style.RoTokens.spaceXl
        anchors.bottomMargin: Style.RoTokens.spaceXl
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
                radius: Style.RoTokens.radiusLg
                color: sidebar.currentIndex === menuCard.index
                       ? ((sidebar.theme && sidebar.theme.sidebarActive) ? sidebar.theme.sidebarActive : ((sidebar.theme && sidebar.theme.cardStrong) ? sidebar.theme.cardStrong : theme.cardStrong))
                       : "transparent"
                border.width: sidebar.currentIndex === menuCard.index ? 1 : 0
                border.color: (sidebar.theme && sidebar.theme.sidebarBorder) ? sidebar.theme.sidebarBorder : ((sidebar.theme && sidebar.theme.border) ? sidebar.theme.border : theme.border)

                Rectangle {
                    visible: sidebar.currentIndex === menuCard.index
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    width: 4
                    height: parent.height - 20
                    radius: 2
                    color: (sidebar.theme && sidebar.theme.sidebarAccent) ? sidebar.theme.sidebarAccent : ((sidebar.theme && sidebar.theme.accentA) ? sidebar.theme.accentA : theme.accentA)
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: Style.RoTokens.spaceXl
                    anchors.rightMargin: Style.RoTokens.spaceXl
                    spacing: Style.RoTokens.spaceLg

                    Rectangle {
                        implicitWidth: 48
                        implicitHeight: 48
                        radius: 16
                        color: sidebar.currentIndex === menuCard.index
                               ? ((sidebar.theme && sidebar.theme.sidebarAccent) ? sidebar.theme.sidebarAccent : ((sidebar.theme && sidebar.theme.accentA) ? sidebar.theme.accentA : theme.accentA))
                               : ((sidebar.theme && sidebar.theme.cardStrong) ? sidebar.theme.cardStrong : theme.card)

                        Label {
                            anchors.centerIn: parent
                            text: menuCard.modelData.marker
                            color: sidebar.currentIndex === menuCard.index
                                   ? theme.card
                                   : ((sidebar.theme && sidebar.theme.sidebarMuted) ? sidebar.theme.sidebarMuted : ((sidebar.theme && sidebar.theme.textSoft) ? sidebar.theme.textSoft : theme.textMuted))
                            font.pixelSize: 18
                            font.weight: Font.DemiBold
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        text: menuCard.modelData.title
                        color: (sidebar.theme && sidebar.theme.sidebarText) ? sidebar.theme.sidebarText : ((sidebar.theme && sidebar.theme.text) ? sidebar.theme.text : theme.text)
                        font.pixelSize: 16
                        font.weight: sidebar.currentIndex === menuCard.index ? Font.DemiBold : Font.Medium
                    }

                    Rectangle {
                        implicitWidth: 10
                        implicitHeight: 10
                        radius: 5
                        visible: sidebar.currentIndex === menuCard.index
                        color: (sidebar.theme && sidebar.theme.sidebarAccent) ? sidebar.theme.sidebarAccent : ((sidebar.theme && sidebar.theme.accentA) ? sidebar.theme.accentA : theme.accentA)
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onEntered: {
                        if (sidebar.currentIndex !== menuCard.index)
                            menuCard.color = (sidebar.theme && sidebar.theme.sidebarHover) ? sidebar.theme.sidebarHover : ((sidebar.theme && sidebar.theme.card) ? sidebar.theme.card : theme.card);
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
