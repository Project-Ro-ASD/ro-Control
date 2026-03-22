import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    visible: true
    width: 1220
    height: 760
    minimumWidth: 980
    minimumHeight: 680
    title: qsTr("ro-Control")

    readonly property bool darkMode: Qt.styleHints.colorScheme === Qt.Dark
    property bool compactMode: false
    property bool showAdvancedInfo: true

    readonly property var pageTitles: [
        qsTr("Driver Control Center"),
        qsTr("System Monitor"),
        qsTr("Preferences")
    ]
    readonly property var pageSubtitles: [
        qsTr("Install, verify and update NVIDIA drivers with guided system checks."),
        qsTr("Track live CPU, GPU and memory telemetry in one place."),
        qsTr("Tune the interface and review diagnostic context before support work.")
    ]
    readonly property var theme: darkMode ? ({
        window: "#0f1420",
        shell: "#121927",
        card: "#182131",
        cardStrong: "#223049",
        border: "#2c3952",
        text: "#edf3ff",
        textMuted: "#b6c2d9",
        textSoft: "#93a3bd",
        accentA: "#4f8cff",
        accentB: "#0ea5a3",
        accentC: "#ff9f43",
        success: "#2eb67d",
        warning: "#f2a93b",
        danger: "#ef5d68",
        successBg: "#152a21",
        warningBg: "#34280e",
        dangerBg: "#35171b",
        infoBg: "#152338",
        heroStart: "#172238",
        heroEnd: "#0f1726",
        sidebarBg: "#0b1120",
        sidebarText: "#edf3ff",
        sidebarMuted: "#97a6be",
        sidebarAccent: "#8ab4ff",
        sidebarActive: "#1b2840",
        sidebarHover: "#141c2e",
        sidebarBorder: "#23314b",
        sidebarHint: "#6f7d95"
    }) : ({
        window: "#f4f7fb",
        shell: "#eef3f9",
        card: "#ffffff",
        cardStrong: "#f5f9ff",
        border: "#d7e0ec",
        text: "#132033",
        textMuted: "#4f6178",
        textSoft: "#6f7f96",
        accentA: "#2563eb",
        accentB: "#0f766e",
        accentC: "#ea580c",
        success: "#198754",
        warning: "#b7791f",
        danger: "#dc3545",
        successBg: "#e8f6ee",
        warningBg: "#fff4df",
        dangerBg: "#fde9eb",
        infoBg: "#eaf2ff",
        heroStart: "#ffffff",
        heroEnd: "#edf4ff",
        sidebarBg: "#122033",
        sidebarText: "#edf3ff",
        sidebarMuted: "#aebcd2",
        sidebarAccent: "#8ab4ff",
        sidebarActive: "#223651",
        sidebarHover: "#1a2a42",
        sidebarBorder: "#2e4566",
        sidebarHint: "#7d8ba0"
    })

    color: theme.window

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: root.theme.heroStart
            }
            GradientStop {
                position: 1.0
                color: root.theme.shell
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        SidebarMenu {
            id: sidebar
            theme: root.theme
            Layout.fillHeight: true
            currentIndex: 0
            onCurrentIndexChanged: stack.currentIndex = currentIndex
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "transparent"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: root.compactMode ? 18 : 24
                spacing: root.compactMode ? 14 : 18

                Rectangle {
                    Layout.fillWidth: true
                    radius: 28
                    color: root.theme.card
                    border.width: 1
                    border.color: root.theme.border
                    implicitHeight: heroLayout.implicitHeight + 40

                    RowLayout {
                        id: heroLayout
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.margins: 20
                        spacing: 18

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 6

                            Label {
                                text: root.pageTitles[sidebar.currentIndex]
                                font.pixelSize: 28
                                font.bold: true
                                color: root.theme.text
                            }

                            Label {
                                text: root.pageSubtitles[sidebar.currentIndex]
                                wrapMode: Text.Wrap
                                color: root.theme.textSoft
                                Layout.fillWidth: true
                            }
                        }

                        InfoBadge {
                            text: root.darkMode ? qsTr("System Dark") : qsTr("System Light")
                            backgroundColor: root.theme.infoBg
                            foregroundColor: root.theme.text
                        }

                        InfoBadge {
                            text: root.compactMode ? qsTr("Compact Layout") : qsTr("Comfort Layout")
                            backgroundColor: root.theme.cardStrong
                            foregroundColor: root.theme.text
                        }
                    }
                }

                StackLayout {
                    id: stack
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    currentIndex: sidebar.currentIndex

                    DriverPage {
                        theme: root.theme
                        darkMode: root.darkMode
                        compactMode: root.compactMode
                        showAdvancedInfo: root.showAdvancedInfo
                    }

                    MonitorPage {
                        theme: root.theme
                        darkMode: root.darkMode
                        compactMode: root.compactMode
                        showAdvancedInfo: root.showAdvancedInfo
                    }

                    SettingsPage {
                        theme: root.theme
                        darkMode: root.darkMode
                        compactMode: root.compactMode
                        showAdvancedInfo: root.showAdvancedInfo
                        onCompactModeChanged: root.compactMode = compactMode
                        onShowAdvancedInfoChanged: root.showAdvancedInfo = showAdvancedInfo
                    }
                }
            }
        }
    }
}
