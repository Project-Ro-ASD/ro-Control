import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    visible: true
    width: 980
    height: 640
    title: "ro-Control"
    readonly property bool darkMode: Qt.styleHints.colorScheme === Qt.Dark
    color: darkMode ? "#141822" : "#f4f6fb"

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        ToolBar {
            Layout.fillWidth: true
            background: Rectangle {
                color: root.darkMode ? "#1b2130" : "#ffffff"
            }

            RowLayout {
                anchors.fill: parent
                anchors.margins: 8

                Image {
                    source: "qrc:/qt/qml/rocontrol/assets/ro-control-logo.svg"
                    sourceSize.width: 30
                    sourceSize.height: 30
                    fillMode: Image.PreserveAspectFit
                    Layout.preferredWidth: 30
                    Layout.preferredHeight: 30
                }

                Item {
                    Layout.preferredWidth: 8
                    Layout.preferredHeight: 1
                }

                Label {
                    text: "ro-Control"
                    font.pixelSize: 20
                    font.bold: true
                    color: root.darkMode ? "#e8edfb" : "#121825"
                }

                Item {
                    Layout.fillWidth: true
                }

                Label {
                    text: root.darkMode ? "Tema: Sistem (Koyu)" : "Tema: Sistem (Acik)"
                    color: root.darkMode ? "#c8d0e7" : "#36435f"
                }
            }
        }

        TabBar {
            id: tabs
            Layout.fillWidth: true

            TabButton {
                text: "Surucu"
            }
            TabButton {
                text: "Izleme"
            }
            TabButton {
                text: "Ayarlar"
            }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabs.currentIndex

            DriverPage {}
            MonitorPage {}
            SettingsPage {
                darkMode: root.darkMode
            }
        }
    }
}
