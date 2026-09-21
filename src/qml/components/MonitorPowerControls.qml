import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: powerSection
    Layout.fillWidth: true
    radius: 14
    color: theme && theme.cardStrong ? theme.cardStrong : (darkMode ? "#342D4A" : "#F1F5F9")
    border.width: 1
    border.color: theme && theme.border ? theme.border : (darkMode ? "#4D436B" : "#CBD5E1")
    implicitHeight: powerArea.implicitHeight + 24

    property var theme: ({})
    property bool darkMode: false
    property real uiScale: 1.0

    property var powerController: null

    readonly property color bgColor: theme && theme.card ? theme.card : (darkMode ? "#29233B" : "#FFFFFF")
    readonly property color borderColor: theme && theme.border ? theme.border : (darkMode ? "#4D436B" : "#CBD5E1")
    readonly property color textColor: theme && theme.text ? theme.text : (darkMode ? "#F8FAFC" : "#0F172A")
    readonly property color softTextColor: theme && theme.textSoft ? theme.textSoft : (darkMode ? "#94A3B8" : "#64748B")
    readonly property color accentColor: theme && theme.accentA ? theme.accentA : (darkMode ? "#818CF8" : "#4F46E5")

    ColumnLayout {
        id: powerArea
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Label {
                Layout.fillWidth: true
                text: powerSection.powerController && powerSection.powerController.supported
                      ? qsTr("GPU Power & Performance Management")
                      : qsTr("Power & Performance Management")
                color: powerSection.textColor
                font.pixelSize: Math.round(18 * powerSection.uiScale)
                font.weight: Font.DemiBold
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Rectangle {
                visible: powerSection.powerController && powerSection.powerController.supported
                Layout.maximumWidth: powerSection.width <= 560 ? 0 : implicitWidth
                implicitHeight: Math.round(30 * powerSection.uiScale)
                implicitWidth: currentDrawRow.implicitWidth + Math.round(16 * powerSection.uiScale)
                radius: 6
                color: powerSection.bgColor
                border.width: 1
                border.color: powerSection.borderColor

                RowLayout {
                    id: currentDrawRow
                    anchors.centerIn: parent
                    spacing: 4

                    Label {
                        text: qsTr("Draw:")
                        color: powerSection.softTextColor
                        font.pixelSize: Math.round(11 * powerSection.uiScale)
                        font.weight: Font.Medium
                    }

                    Label {
                        text: (powerSection.powerController ? powerSection.powerController.currentPowerDrawW.toFixed(1) : "0.0") + " W"
                        color: powerSection.textColor
                        font.pixelSize: Math.round(12 * powerSection.uiScale)
                        font.weight: Font.Bold
                    }
                }
            }

            Rectangle {
                visible: powerSection.powerController && powerSection.powerController.controlSupported
                Layout.maximumWidth: powerSection.width <= 560 ? 0 : implicitWidth
                implicitHeight: Math.round(30 * powerSection.uiScale)
                implicitWidth: powerLimitRow.implicitWidth + Math.round(16 * powerSection.uiScale)
                radius: 6
                color: powerSection.darkMode ? "#1E2548" : "#EEF2FF"
                border.width: 1
                border.color: powerSection.darkMode ? "#4338CA" : "#C7D2FE"

                RowLayout {
                    id: powerLimitRow
                    anchors.centerIn: parent
                    spacing: 4

                    Label {
                        text: qsTr("Limit:")
                        color: powerSection.accentColor
                        font.pixelSize: Math.round(11 * powerSection.uiScale)
                        font.weight: Font.Medium
                    }

                    Label {
                        text: (powerSection.powerController ? powerSection.powerController.powerLimitW.toFixed(0) : "N/A") + " W"
                        color: powerSection.accentColor
                        font.pixelSize: Math.round(12 * powerSection.uiScale)
                        font.weight: Font.Bold
                    }
                }
            }

            Item { Layout.fillWidth: true }

            RowLayout {
                spacing: Math.round(8 * powerSection.uiScale)
                visible: powerSection.powerController && (powerSection.powerController.controlSupported
                                                           || powerSection.powerController.systemPowerProfileSupported)

                Repeater {
                    model: [
                        { preset: "eco", label: qsTr("Eco") },
                        { preset: "balanced", label: qsTr("Balanced") },
                        { preset: "performance", label: qsTr("Performance") }
                    ]

                    delegate: Button {
                        id: powerPresetBtn
                        required property var modelData
                        implicitHeight: Math.round(34 * powerSection.uiScale)
                        implicitWidth: powerSection.width <= 560 ? Math.round(84 * powerSection.uiScale) : Math.round(96 * powerSection.uiScale)

                        background: Rectangle {
                            radius: 8
                            color: (powerSection.powerController && powerSection.powerController.activePreset === powerPresetBtn.modelData.preset)
                                   ? powerSection.accentColor
                                   : powerSection.bgColor
                            border.width: (powerSection.powerController && powerSection.powerController.activePreset === powerPresetBtn.modelData.preset) ? 2 : 1
                            border.color: (powerSection.powerController && powerSection.powerController.activePreset === powerPresetBtn.modelData.preset)
                                          ? powerSection.accentColor
                                          : powerSection.borderColor
                        }

                        contentItem: Text {
                            text: powerPresetBtn.modelData.label
                            color: (powerSection.powerController && powerSection.powerController.activePreset === powerPresetBtn.modelData.preset)
                                   ? "#FFFFFF"
                                   : powerSection.textColor
                            font.pixelSize: Math.round(12 * powerSection.uiScale)
                            font.weight: (powerSection.powerController && powerSection.powerController.activePreset === powerPresetBtn.modelData.preset) ? Font.Bold : Font.DemiBold
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            if (powerSection.powerController) {
                                if (!powerSection.powerController.applyPowerPreset(powerPresetBtn.modelData.preset))
                                    powerSection.powerController.refresh();
                            }
                        }
                    }
                }
            }
        }
    }
}
