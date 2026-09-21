import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "." as Components

Rectangle {
    id: root
    Layout.fillWidth: true
    radius: 14
    color: cardColor
    border.width: 1
    border.color: borderColor
    implicitHeight: profileSectionLayout.implicitHeight + Math.round(28 * root.uiScale)

    property var fanController: null
    property var systemInfo: null
    property var theme: ({})
    property bool darkMode: false
    property real uiScale: 1.0

    property bool hasControllableFan: false
    property bool supportsBatteryFanSync: false

    signal modeSelected(string mode)
    signal openStudioRequested()
    signal applyPresetRequested(string presetId)

    readonly property color cardColor: theme && theme.cardStrong ? theme.cardStrong : (darkMode ? "#342D4A" : "#F1F5F9")
    readonly property color borderColor: theme && theme.border ? theme.border : (darkMode ? "#4D436B" : "#CBD5E1")
    readonly property color textColor: theme && theme.text ? theme.text : (darkMode ? "#F8FAFC" : "#0F172A")
    readonly property color softTextColor: theme && theme.textSoft ? theme.textSoft : (darkMode ? "#94A3B8" : "#64748B")
    readonly property color accentColor: theme && theme.accentA ? theme.accentA : (darkMode ? "#818CF8" : "#4F46E5")
    readonly property color bgColor: theme && theme.card ? theme.card : (darkMode ? "#29233B" : "#FFFFFF")
    readonly property color infoBg: theme && theme.infoBg ? theme.infoBg : (darkMode ? "#1E2548" : "#EFF6FF")
    readonly property color successBg: theme && theme.successBg ? theme.successBg : (darkMode ? "#143828" : "#ECFDF5")
    readonly property color successText: theme && theme.success ? theme.success : (darkMode ? "#4ADE80" : "#059669")

    ColumnLayout {
        id: profileSectionLayout
        anchors.fill: parent
        anchors.margins: Math.round(16 * root.uiScale)
        spacing: Math.round(14 * root.uiScale)

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Label {
                text: qsTr("Optimization Profiles & Control")
                color: root.textColor
                font.pixelSize: Math.round(16 * root.uiScale)
                font.weight: Font.DemiBold
                Layout.fillWidth: true
            }

            Rectangle {
                implicitWidth: statusBadgeText.implicitWidth + 18
                implicitHeight: Math.round(28 * root.uiScale)
                radius: 6
                color: root.fanController && root.fanController.controlSupported ? root.successBg : root.infoBg
                border.width: 1
                border.color: root.fanController && root.fanController.controlSupported ? root.successText : (root.darkMode ? "#4D5B9E" : "#93C5FD")

                Label {
                    id: statusBadgeText
                    anchors.centerIn: parent
                    text: {
                        if (root.fanController && root.fanController.controlSupported)
                            return qsTr("ACTIVE: %1").arg(root.fanController.fanMode.toUpperCase());
                        return qsTr("MANAGED: %1").arg(root.fanController ? root.fanController.fanMode.toUpperCase() : "AUTO");
                    }
                    color: root.fanController && root.fanController.controlSupported ? root.successText : root.accentColor
                    font.pixelSize: Math.round(11 * root.uiScale)
                    font.weight: Font.DemiBold
                }
            }
        }

        // Profile Selector Buttons
        GridLayout {
            Layout.fillWidth: true
            columns: width > 900 ? 6 : (width > 560 ? 3 : 2)
            columnSpacing: Math.round(10 * root.uiScale)
            rowSpacing: Math.round(10 * root.uiScale)

            Repeater {
                model: [
                    { mode: "auto", label: qsTr("Auto"), desc: qsTr("Hardware dynamic") },
                    { mode: "silent", label: qsTr("Silent"), desc: qsTr("Zero-dB quiet") },
                    { mode: "balanced", label: qsTr("Balanced"), desc: qsTr("Optimized blend") },
                    { mode: "performance", label: qsTr("Performance"), desc: qsTr("Maximum airflow") },
                    { mode: "manual", label: qsTr("Manual"), desc: qsTr("Locked speed") },
                    { mode: "custom", label: qsTr("Custom"), desc: qsTr("User curve") }
                ]

                delegate: AbstractButton {
                    id: modeBtn
                    required property var modelData
                    Layout.fillWidth: true
                    implicitHeight: Math.round(52 * root.uiScale)
                    hoverEnabled: true

                    readonly property bool isCurrent: root.fanController && root.fanController.fanMode === modeBtn.modelData.mode
                    enabled: root.fanController && root.fanController.controlSupported

                    background: Rectangle {
                        radius: 10
                        color: modeBtn.isCurrent
                               ? (root.darkMode ? "#342D4A" : "#EEF2FF")
                               : (modeBtn.hovered ? (root.darkMode ? "#2E2742" : "#F8FAFC") : root.bgColor)
                        border.width: modeBtn.isCurrent ? 2 : 1
                        border.color: modeBtn.isCurrent ? root.accentColor : root.borderColor
                    }

                    contentItem: ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: Math.round(10 * root.uiScale)
                        spacing: 2

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 6

                            Rectangle {
                                implicitWidth: Math.round(8 * root.uiScale)
                                implicitHeight: Math.round(8 * root.uiScale)
                                radius: Math.round(4 * root.uiScale)
                                color: modeBtn.isCurrent ? root.accentColor : (root.darkMode ? "#403858" : "#CBD5E1")
                            }

                            Label {
                                Layout.fillWidth: true
                                text: modeBtn.modelData.label
                                color: modeBtn.isCurrent ? root.accentColor : root.textColor
                                font.pixelSize: Math.round(13 * root.uiScale)
                                font.weight: modeBtn.isCurrent ? Font.Bold : Font.DemiBold
                                elide: Text.ElideRight
                            }
                        }

                        Label {
                            Layout.fillWidth: true
                            text: modeBtn.modelData.desc
                            color: root.softTextColor
                            font.pixelSize: Math.round(10 * root.uiScale)
                            elide: Text.ElideRight
                        }
                    }

                    onClicked: root.modeSelected(modeBtn.modelData.mode)
                }
            }
        }

        // Custom Curve Studio (Visible in Custom Mode)
        Components.FanCustomCurveStudio {
            visible: root.fanController && root.fanController.fanMode === "custom"
            fanController: root.fanController
            theme: root.theme
            darkMode: root.darkMode
            uiScale: root.uiScale
            onOpenStudioRequested: root.openStudioRequested()
            onApplyPresetRequested: (presetId) => root.applyPresetRequested(presetId)
        }

        // Dynamics & Battery Sync Switches
        Components.FanDynamicsControls {
            fanController: root.fanController
            systemInfo: root.systemInfo
            theme: root.theme
            darkMode: root.darkMode
            uiScale: root.uiScale
            hasControllableFan: root.hasControllableFan
            supportsBatteryFanSync: root.supportsBatteryFanSync
        }
    }
}
