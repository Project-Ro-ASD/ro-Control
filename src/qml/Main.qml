import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "pages"

ApplicationWindow {
    id: root
    property var nvidiaDetector
    property var nvidiaInstaller
    property var nvidiaUpdater
    property var cpuMonitor
    property var gpuMonitor
    property var ramMonitor

    visible: true
    width: 980
    height: 640
    title: "ro-Control"
    readonly property bool darkMode: Application.styleHints.colorScheme === Qt.ColorScheme.Dark
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
                    source: "qrc:/qt/qml/rocontrol/assets/ro-control-logo.png"
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
                    text: root.darkMode ? qsTr("Theme: System (Dark)") : qsTr("Theme: System (Light)")
                    color: root.darkMode ? "#c8d0e7" : "#36435f"
                }
            }
        }

        TabBar {
            id: tabs
            Layout.fillWidth: true

            TabButton {
                text: qsTr("Driver")
            }
            TabButton {
                text: qsTr("Monitor")
            }
            TabButton {
                text: qsTr("Settings")
            }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabs.currentIndex

            DriverPage {
                nvidiaDetector: root.nvidiaDetector
                nvidiaInstaller: root.nvidiaInstaller
                nvidiaUpdater: root.nvidiaUpdater
            }
            MonitorPage {
                cpuMonitor: root.cpuMonitor
                gpuMonitor: root.gpuMonitor
                ramMonitor: root.ramMonitor
            }
            SettingsPage {
                darkMode: root.darkMode
            }
        }
    }
}
