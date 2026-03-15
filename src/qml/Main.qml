import QtCore
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "components"
import "pages"

ApplicationWindow {
    id: root
    property var nvidiaDetector
    property var nvidiaInstaller
    property var nvidiaUpdater
    property var cpuMonitor
    property var gpuMonitor
    property var ramMonitor

    Settings {
        id: uiSettings
        category: "ui"
        property string themeMode: "dark"
    }

    property string themeMode: uiSettings.themeMode
    readonly property bool darkMode: themeMode === "dark"
    readonly property string githubUrl: "https://github.com/Project-Ro-ASD/ro-Control"
    readonly property var theme: darkMode ? ({
        windowTop: "#09111d",
        windowBottom: "#131f33",
        shell: "#0f1727",
        panel: "#162033",
        panelAlt: "#1a2740",
        card: "#18253b",
        cardStrong: "#1f304c",
        cardMuted: "#22314f",
        border: "#2b436a",
        text: "#edf4ff",
        textMuted: "#9db0cd",
        textSoft: "#7e94b6",
        accentA: "#ff8a3d",
        accentB: "#39a7ff",
        accentSoft: "#203b60",
        success: "#44d17f",
        warning: "#ffba57",
        danger: "#ff6d79",
        shadow: "#70081423"
    }) : ({
        windowTop: "#fff3e8",
        windowBottom: "#e8f3ff",
        shell: "#f6f9ff",
        panel: "#ffffff",
        panelAlt: "#f5f8fe",
        card: "#ffffff",
        cardStrong: "#f8fbff",
        cardMuted: "#eef4fc",
        border: "#cedcf1",
        text: "#132238",
        textMuted: "#4e617d",
        textSoft: "#687d98",
        accentA: "#f47b20",
        accentB: "#1677ff",
        accentSoft: "#dbe9ff",
        success: "#138a52",
        warning: "#b77700",
        danger: "#cb3d4f",
        shadow: "#33091a34"
    })
    readonly property var pageTitles: [
        qsTr("Driver Hub"),
        qsTr("System Monitor"),
        qsTr("Settings")
    ]
    readonly property var pageDescriptions: [
        qsTr("Install, switch and verify NVIDIA driver states without leaving the app."),
        qsTr("Track CPU, RAM and NVIDIA telemetry with a compact live dashboard."),
        qsTr("Adjust the app appearance and inspect product metadata.")
    ]

    function setThemeMode(mode) {
        themeMode = mode
        uiSettings.themeMode = mode
    }

    visible: true
    minimumWidth: 1220
    minimumHeight: 760
    width: 1360
    height: 860
    title: "ro-Control"
    color: "transparent"

    background: Rectangle {
        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: root.theme.windowTop
            }
            GradientStop {
                position: 0.38
                color: Qt.tint(root.theme.windowBottom, "#40ff8a3d")
            }
            GradientStop {
                position: 1.0
                color: Qt.tint(root.theme.windowBottom, "#30249bff")
            }
        }

        Rectangle {
            anchors.top: parent.top
            anchors.right: parent.right
            width: parent.width * 0.42
            height: parent.height * 0.35
            radius: width / 2
            color: "#22ffffff"
            opacity: root.darkMode ? 0.12 : 0.28
            rotation: -12
            x: parent.width * 0.66
            y: -height * 0.34
        }

        Rectangle {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            width: parent.width * 0.35
            height: parent.height * 0.28
            radius: width / 2
            color: root.darkMode ? "#18ff8a3d" : "#20f47b20"
            x: -width * 0.2
            y: parent.height - height * 0.65
        }
    }

    Dialog {
        id: aboutDialog
        modal: true
        focus: true
        anchors.centerIn: Overlay.overlay
        width: Math.min(root.width * 0.72, 880)
        height: Math.min(root.height * 0.78, 720)
        padding: 0
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            radius: 28
            border.width: 1
            border.color: root.theme.border
            color: root.theme.panel
        }

        contentItem: ScrollView {
            clip: true

            ColumnLayout {
                width: parent.width
                spacing: 18

                Rectangle {
                    Layout.fillWidth: true
                    radius: 26
                    color: Qt.tint(root.theme.cardStrong, root.darkMode ? "#30ff8a3d" : "#1af47b20")
                    implicitHeight: headerColumn.implicitHeight + 28

                    ColumnLayout {
                        id: headerColumn
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 10

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 14

                            Rectangle {
                                Layout.preferredWidth: 54
                                Layout.preferredHeight: 54
                                radius: 18
                                color: root.theme.accentSoft

                                Image {
                                    anchors.centerIn: parent
                                    source: "qrc:/qt/qml/rocontrol/assets/ro-control-logo.png"
                                    sourceSize.width: 34
                                    sourceSize.height: 34
                                    fillMode: Image.PreserveAspectFit
                                }
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 4

                                Label {
                                    text: qsTr("About ro-Control")
                                    font.pixelSize: 26
                                    font.bold: true
                                    color: root.theme.text
                                }

                                Label {
                                    text: qsTr("A focused Fedora control surface for NVIDIA driver lifecycle and live telemetry.")
                                    wrapMode: Text.Wrap
                                    Layout.fillWidth: true
                                    color: root.theme.textMuted
                                }
                            }
                        }

                        RowLayout {
                            spacing: 10

                            Rectangle {
                                radius: 999
                                color: root.theme.cardMuted
                                implicitHeight: 32
                                implicitWidth: versionLabel.implicitWidth + 24

                                Label {
                                    id: versionLabel
                                    anchors.centerIn: parent
                                    text: qsTr("App Version %1").arg(Qt.application.version)
                                    color: root.theme.text
                                    font.pixelSize: 13
                                    font.bold: true
                                }
                            }

                            Rectangle {
                                radius: 999
                                color: root.theme.cardMuted
                                implicitHeight: 32
                                implicitWidth: repoLabel.implicitWidth + 24

                                Label {
                                    id: repoLabel
                                    anchors.centerIn: parent
                                    text: qsTr("Fedora x86_64 NVIDIA Flow")
                                    color: root.theme.textMuted
                                    font.pixelSize: 13
                                }
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 16

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredWidth: 1
                        radius: 22
                        color: root.theme.card
                        border.width: 1
                        border.color: root.theme.border
                        implicitHeight: versionInfoColumn.implicitHeight + 24

                        ColumnLayout {
                            id: versionInfoColumn
                            anchors.fill: parent
                            anchors.margins: 18
                            spacing: 10

                            Label {
                                text: qsTr("Version Overview")
                                font.pixelSize: 19
                                font.bold: true
                                color: root.theme.text
                            }

                            Label {
                                text: qsTr("Installed driver: %1").arg(root.nvidiaUpdater.currentVersion.length > 0 ? root.nvidiaUpdater.currentVersion : qsTr("Not installed"))
                                color: root.theme.textMuted
                            }

                            Label {
                                text: qsTr("Latest repo version: %1").arg(root.nvidiaUpdater.latestVersion.length > 0 ? root.nvidiaUpdater.latestVersion : qsTr("Unknown"))
                                color: root.theme.textMuted
                            }

                            Label {
                                text: qsTr("Available repo versions: %1").arg(root.nvidiaUpdater.availableVersions.length)
                                color: root.theme.textMuted
                            }

                            RowLayout {
                                spacing: 10

                                Button {
                                    text: qsTr("Check Versions")
                                    enabled: !root.nvidiaUpdater.busy
                                    onClicked: root.nvidiaUpdater.checkForUpdate()
                                }

                                Button {
                                    text: qsTr("Open GitHub")
                                    onClicked: Qt.openUrlExternally(root.githubUrl)
                                }
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredWidth: 1
                        radius: 22
                        color: root.theme.card
                        border.width: 1
                        border.color: root.theme.border
                        implicitHeight: releaseNotesColumn.implicitHeight + 24

                        ColumnLayout {
                            id: releaseNotesColumn
                            anchors.fill: parent
                            anchors.margins: 18
                            spacing: 10

                            Label {
                                text: qsTr("Release Notes")
                                font.pixelSize: 19
                                font.bold: true
                                color: root.theme.text
                            }

                            Repeater {
                                model: [
                                    qsTr("Selectable NVIDIA versions were added alongside a direct update-to-latest flow."),
                                    qsTr("Long-running driver operations now execute asynchronously so the interface stays responsive."),
                                    qsTr("Detection, monitoring and package-sync logic were hardened for Fedora NVIDIA systems.")
                                ]

                                delegate: Label {
                                    required property string modelData
                                    Layout.fillWidth: true
                                    wrapMode: Text.Wrap
                                    text: "\u2022 " + modelData
                                    color: root.theme.textMuted
                                }
                            }
                        }
                    }
                }

                Item {
                    Layout.fillHeight: true
                    Layout.minimumHeight: 1
                }
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 22
        spacing: 18

        SidebarMenu {
            id: sidebar
            Layout.fillHeight: true
            Layout.preferredWidth: 272
            currentIndex: contentStack.currentIndex
            darkMode: root.darkMode
            theme: root.theme
            menuItems: root.pageTitles
            onNavigate: contentStack.currentIndex = index
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 30
            color: "#11000000"
            border.width: 1
            border.color: root.darkMode ? "#24ffffff" : "#26a4b7d0"

            Rectangle {
                anchors.fill: parent
                anchors.margins: 1
                radius: 29
                color: root.theme.shell

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 22
                    spacing: 18

                    Rectangle {
                        Layout.fillWidth: true
                        radius: 26
                        color: root.theme.panel
                        border.width: 1
                        border.color: root.theme.border
                        implicitHeight: topBarLayout.implicitHeight + 24

                        RowLayout {
                            id: topBarLayout
                            anchors.fill: parent
                            anchors.margins: 20
                            spacing: 16

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 4

                                Label {
                                    text: root.pageTitles[contentStack.currentIndex]
                                    font.pixelSize: 28
                                    font.bold: true
                                    color: root.theme.text
                                }

                                Label {
                                    text: root.pageDescriptions[contentStack.currentIndex]
                                    Layout.fillWidth: true
                                    wrapMode: Text.Wrap
                                    color: root.theme.textMuted
                                }
                            }

                            Rectangle {
                                radius: 18
                                color: root.theme.cardMuted
                                implicitWidth: modeRow.implicitWidth + 16
                                implicitHeight: modeRow.implicitHeight + 12

                                RowLayout {
                                    id: modeRow
                                    anchors.centerIn: parent
                                    spacing: 8

                                    Button {
                                        text: qsTr("Light")
                                        flat: root.themeMode !== "light"
                                        highlighted: root.themeMode === "light"
                                        onClicked: root.setThemeMode("light")
                                    }

                                    Button {
                                        text: qsTr("Dark")
                                        flat: root.themeMode !== "dark"
                                        highlighted: root.themeMode === "dark"
                                        onClicked: root.setThemeMode("dark")
                                    }
                                }
                            }

                            Button {
                                text: qsTr("About")
                                onClicked: aboutDialog.open()
                            }
                        }
                    }

                    StackLayout {
                        id: contentStack
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        DriverPage {
                            nvidiaDetector: root.nvidiaDetector
                            nvidiaInstaller: root.nvidiaInstaller
                            nvidiaUpdater: root.nvidiaUpdater
                            theme: root.theme
                            darkMode: root.darkMode
                        }

                        MonitorPage {
                            cpuMonitor: root.cpuMonitor
                            gpuMonitor: root.gpuMonitor
                            ramMonitor: root.ramMonitor
                            theme: root.theme
                            darkMode: root.darkMode
                        }

                        SettingsPage {
                            themeMode: root.themeMode
                            darkMode: root.darkMode
                            theme: root.theme
                            githubUrl: root.githubUrl
                            onThemeModeRequested: root.setThemeMode(mode)
                            onAboutRequested: aboutDialog.open()
                        }
                    }
                }
            }
        }
    }
}
