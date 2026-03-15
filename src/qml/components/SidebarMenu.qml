import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: sidebar
    required property var theme
    required property var menuItems
    property int currentIndex: 0
    property bool darkMode: true
    signal navigate(int index)

    radius: 30
    color: Qt.tint(theme.panel, darkMode ? "#12000000" : "#08ffffff")
    border.width: 1
    border.color: theme.border

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 18

        Rectangle {
            Layout.fillWidth: true
            radius: 24
            color: Qt.tint(sidebar.theme.cardStrong, sidebar.darkMode ? "#20ff8a3d" : "#14f47b20")
            implicitHeight: brandLayout.implicitHeight + 26

            ColumnLayout {
                id: brandLayout
                anchors.fill: parent
                anchors.margins: 18
                spacing: 14

                RowLayout {
                    spacing: 12

                    Rectangle {
                        Layout.preferredWidth: 46
                        Layout.preferredHeight: 46
                        radius: 16
                        color: sidebar.theme.accentSoft

                        Image {
                            anchors.centerIn: parent
                            source: "qrc:/qt/qml/rocontrol/assets/ro-control-logo.png"
                            sourceSize.width: 30
                            sourceSize.height: 30
                            fillMode: Image.PreserveAspectFit
                        }
                    }

                    ColumnLayout {
                        spacing: 2

                        Label {
                            text: "ro-Control"
                            font.pixelSize: 21
                            font.bold: true
                            color: sidebar.theme.text
                        }

                        Label {
                            text: qsTr("Fedora NVIDIA center")
                            color: sidebar.theme.textMuted
                            font.pixelSize: 13
                        }
                    }
                }

                Label {
                    text: qsTr("A compact control surface for driver operations, telemetry and environment checks.")
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                    color: sidebar.theme.textSoft
                }
            }
        }

        Label {
            text: qsTr("Navigation")
            color: sidebar.theme.textSoft
            font.pixelSize: 12
            font.bold: true
            leftPadding: 6
        }

        Repeater {
            model: sidebar.menuItems

            delegate: Rectangle {
                required property int index
                required property string modelData

                Layout.fillWidth: true
                radius: 20
                implicitHeight: 58
                color: sidebar.currentIndex === index ? Qt.tint(sidebar.theme.cardMuted, sidebar.darkMode ? "#2b39a7ff" : "#18f47b20") : "transparent"
                border.width: sidebar.currentIndex === index ? 1 : 0
                border.color: sidebar.currentIndex === index ? sidebar.theme.border : "transparent"

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 14
                    spacing: 12

                    Rectangle {
                        Layout.preferredWidth: 30
                        Layout.preferredHeight: 30
                        radius: 10
                        color: sidebar.currentIndex === index ? sidebar.theme.accentSoft : sidebar.theme.cardMuted

                        Label {
                            anchors.centerIn: parent
                            text: (index + 1).toString()
                            font.bold: true
                            color: sidebar.currentIndex === index ? sidebar.theme.text : sidebar.theme.textMuted
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        text: modelData
                        font.pixelSize: 15
                        font.bold: sidebar.currentIndex === index
                        color: sidebar.currentIndex === index ? sidebar.theme.text : sidebar.theme.textMuted
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: sidebar.navigate(index)
                }
            }
        }

        Item {
            Layout.fillHeight: true
        }

        Rectangle {
            Layout.fillWidth: true
            radius: 22
            color: sidebar.theme.card
            border.width: 1
            border.color: sidebar.theme.border
            implicitHeight: footerLayout.implicitHeight + 22

            ColumnLayout {
                id: footerLayout
                anchors.fill: parent
                anchors.margins: 16
                spacing: 6

                Label {
                    text: qsTr("Build")
                    color: sidebar.theme.textSoft
                    font.pixelSize: 12
                }

                Label {
                    text: "v" + Qt.application.version
                    color: sidebar.theme.text
                    font.pixelSize: 17
                    font.bold: true
                }

                Label {
                    text: qsTr("Theme: %1").arg(sidebar.darkMode ? qsTr("Dark") : qsTr("Light"))
                    color: sidebar.theme.textMuted
                }
            }
        }
    }
}
