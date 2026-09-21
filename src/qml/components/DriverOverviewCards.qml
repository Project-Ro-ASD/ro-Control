import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root
    spacing: 10

    required property var nvidiaDetector
    required property var nvidiaUpdater
    property var systemInfo: null

    property var theme: ({})
    property bool darkMode: false
    property real uiScale: 1.0

    property string gpuMainLabel: ""
    property string driverVersionMainLabel: ""
    property string driverVersionStatusLabel: ""
    property color driverVersionStatusColor: "#94A3B8"
    property string secureBootStatusDetail: ""
    property bool virtualMachine: false

    signal openMokGuide()

    readonly property color textColor: theme && theme.text ? theme.text : (darkMode ? "#F8FAFC" : "#0F172A")
    readonly property color softTextColor: theme && theme.textSoft ? theme.textSoft : (darkMode ? "#94A3B8" : "#64748B")
    readonly property color borderColor: theme && theme.border ? theme.border : (darkMode ? "#4D436B" : "#CBD5E1")
    readonly property color cardColor: theme && theme.cardStrong ? theme.cardStrong : (darkMode ? "#342D4A" : "#F1F5F9")
    readonly property color warningBg: theme && theme.warningBg ? theme.warningBg : (darkMode ? "#3A2E12" : "#FFFBEB")
    readonly property color warningColor: theme && theme.warning ? theme.warning : (darkMode ? "#FBBF24" : "#D97706")

    Rectangle {
        visible: root.nvidiaDetector && root.nvidiaDetector.activeDriver.indexOf("Restart Required") !== -1
        Layout.fillWidth: true
        implicitHeight: 44
        radius: 10
        color: root.warningBg
        border.width: 1
        border.color: root.warningColor

        Label {
            anchors.fill: parent
            anchors.margins: 12
            text: qsTr("Restart required — the installed NVIDIA driver will be active after reboot.")
            color: root.textColor
            verticalAlignment: Text.AlignVCenter
            font.weight: Font.DemiBold
        }
    }

    GridLayout {
        Layout.fillWidth: true
        columns: width > 900 ? (root.nvidiaDetector.gpuFound ? 3 : 2) : 1
        columnSpacing: 10
        rowSpacing: 10

        // Card 1: GPU
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: Math.max(Math.round(132 * root.uiScale), gpuInfoColumn.implicitHeight + Math.round(28 * root.uiScale))
            radius: 14
            color: root.cardColor
            border.width: 1
            border.color: root.borderColor

            Column {
                id: gpuInfoColumn
                anchors.fill: parent
                anchors.margins: Math.round(14 * root.uiScale)
                spacing: Math.round(6 * root.uiScale)

                Label {
                    text: qsTr("GPU")
                    color: root.softTextColor
                    font.weight: Font.DemiBold
                    font.pixelSize: Math.round(13 * root.uiScale)
                }

                Label {
                    width: parent.width
                    text: root.gpuMainLabel
                    color: root.textColor
                    font.pixelSize: Math.round(20 * root.uiScale)
                    font.weight: Font.Bold
                    wrapMode: Text.Wrap
                    maximumLineCount: 2
                    elide: Text.ElideRight
                }

                Label {
                    width: parent.width
                    visible: !root.nvidiaDetector.gpuFound
                    text: root.virtualMachine ? qsTr("VM display. Use NVIDIA passthrough.")
                                              : qsTr("NVIDIA hardware required.")
                    color: root.softTextColor
                    font.pixelSize: Math.round(12 * root.uiScale)
                    wrapMode: Text.Wrap
                    maximumLineCount: 2
                }
            }
        }

        // Card 2: Driver
        Rectangle {
            visible: root.nvidiaDetector.gpuFound
            Layout.fillWidth: true
            implicitHeight: Math.round(132 * root.uiScale)
            radius: 14
            color: root.cardColor
            border.width: 1
            border.color: root.borderColor

            Column {
                anchors.fill: parent
                anchors.margins: Math.round(14 * root.uiScale)
                spacing: Math.round(6 * root.uiScale)

                RowLayout {
                    width: parent.width
                    spacing: 8

                    Label {
                        text: qsTr("Driver")
                        color: root.softTextColor
                        font.weight: Font.DemiBold
                        font.pixelSize: Math.round(13 * root.uiScale)
                    }

                    Item { Layout.fillWidth: true }

                    Rectangle {
                        visible: root.nvidiaUpdater.updateAvailable
                        implicitHeight: Math.round(22 * root.uiScale)
                        implicitWidth: updateBadgeText.implicitWidth + Math.round(14 * root.uiScale)
                        radius: 5
                        color: root.darkMode ? "#3B2E10" : "#FEF3C7"
                        border.width: 1
                        border.color: root.darkMode ? "#D97706" : "#F59E0B"

                        Label {
                            id: updateBadgeText
                            anchors.centerIn: parent
                            text: qsTr("UPDATE AVAILABLE")
                            color: root.darkMode ? "#FBBF24" : "#D97706"
                            font.pixelSize: Math.round(9 * root.uiScale)
                            font.weight: Font.Bold
                        }
                    }
                }

                Label {
                    text: root.driverVersionMainLabel
                    color: root.textColor
                    font.pixelSize: Math.round(20 * root.uiScale)
                    font.weight: Font.Bold
                    elide: Text.ElideRight
                    width: parent.width
                }

                Label {
                    width: parent.width
                    text: root.driverVersionStatusLabel
                    color: root.driverVersionStatusColor
                    font.pixelSize: Math.round(12 * root.uiScale)
                    font.weight: Font.Medium
                    wrapMode: Text.Wrap
                    maximumLineCount: 2
                }
            }
        }

        // Card 3: Secure Boot
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: Math.max(Math.round(132 * root.uiScale), secureBootCol.implicitHeight + Math.round(28 * root.uiScale))
            radius: 14
            color: root.cardColor
            border.width: 1
            border.color: root.borderColor

            Column {
                id: secureBootCol
                anchors.fill: parent
                anchors.margins: Math.round(14 * root.uiScale)
                spacing: Math.round(6 * root.uiScale)

                RowLayout {
                    width: parent.width
                    spacing: 8

                    Label {
                        text: qsTr("Secure Boot")
                        color: root.softTextColor
                        font.weight: Font.DemiBold
                        font.pixelSize: Math.round(13 * root.uiScale)
                    }

                    Item { Layout.fillWidth: true }

                    Rectangle {
                        visible: root.nvidiaDetector.secureBootEnabled
                        implicitHeight: Math.round(22 * root.uiScale)
                        implicitWidth: mokBadgeLayout.implicitWidth + Math.round(14 * root.uiScale)
                        radius: 5
                        color: mokBadgeMouse.hovered
                               ? (root.darkMode ? "#402E5C" : "#EDE9FE")
                               : (root.darkMode ? "#2E2442" : "#F3E8FF")
                        border.width: 1
                        border.color: mokBadgeMouse.hovered
                                      ? (root.darkMode ? "#A855F7" : "#7C3AED")
                                      : (root.darkMode ? "#7C3AED" : "#C084FC")

                        MouseArea {
                            id: mokBadgeMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.openMokGuide()
                        }

                        RowLayout {
                            id: mokBadgeLayout
                            anchors.centerIn: parent
                            spacing: 4

                            Label {
                                id: mokBadgeText
                                text: qsTr("MOK GUIDE ↗")
                                color: root.darkMode ? "#C084FC" : "#7C3AED"
                                font.pixelSize: Math.round(9 * root.uiScale)
                                font.weight: Font.Bold
                            }
                        }
                    }
                }

                Label {
                    text: root.nvidiaDetector.secureBootKnown
                          ? (root.nvidiaDetector.secureBootEnabled ? qsTr("Enabled")
                                                                   : qsTr("Disabled"))
                          : qsTr("Unknown")
                    color: root.textColor
                    font.weight: Font.Bold
                    font.pixelSize: Math.round(20 * root.uiScale)
                }

                Label {
                    width: parent.width
                    text: root.nvidiaDetector.secureBootEnabled
                          ? qsTr("UEFI Secure Boot is active.\nThird-party akmod modules require MOK signing.")
                          : root.secureBootStatusDetail
                    color: root.softTextColor
                    font.pixelSize: Math.round(12 * root.uiScale)
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignLeft
                }
            }
        }
    }
}
