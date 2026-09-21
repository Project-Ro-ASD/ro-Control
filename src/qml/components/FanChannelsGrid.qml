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
    implicitHeight: fansSectionLayout.implicitHeight + Math.round(24 * root.uiScale)

    property var fanController: null
    property var theme: ({})
    property bool darkMode: false
    property real uiScale: 1.0

    property var orderedFans: []
    property var perFanHistories: ({})
    property var fanSpeedHistory: []
    property bool refreshAnimating: false
    property bool reorderMode: false
    property int draggingFanIndex: -1
    readonly property bool hasDetectedFans: orderedFans.length > 0

    signal openWizardRequested()
    signal refreshRequested()
    signal fanSettingsRequested(var fanData, int fanIndex)
    signal moveFanRequested(int fromIndex, int toIndex)

    readonly property color bgColor: theme && theme.card ? theme.card : (darkMode ? "#29233B" : "#FFFFFF")
    readonly property color cardColor: theme && theme.cardStrong ? theme.cardStrong : (darkMode ? "#342D4A" : "#F1F5F9")
    readonly property color borderColor: theme && theme.border ? theme.border : (darkMode ? "#4D436B" : "#CBD5E1")
    readonly property color textColor: theme && theme.text ? theme.text : (darkMode ? "#F8FAFC" : "#0F172A")
    readonly property color softTextColor: theme && theme.textSoft ? theme.textSoft : (darkMode ? "#94A3B8" : "#64748B")
    readonly property color accentColor: theme && theme.accentA ? theme.accentA : (darkMode ? "#818CF8" : "#4F46E5")
    readonly property color infoBg: theme && theme.infoBg ? theme.infoBg : (darkMode ? "#1E2548" : "#EFF6FF")
    readonly property color warningText: theme && theme.warning ? theme.warning : (darkMode ? "#FBBF24" : "#D97706")

    ColumnLayout {
        id: fansSectionLayout
        anchors.fill: parent
        anchors.margins: Math.round(14 * root.uiScale)
        spacing: Math.round(12 * root.uiScale)

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Label {
                Layout.fillWidth: true
                text: qsTr("Cooling Channels (%1)").arg(root.fanController ? root.fanController.systemFanCount : 0)
                color: root.textColor
                font.pixelSize: Math.round(15 * root.uiScale)
                font.weight: Font.DemiBold
            }

            Button {
                id: rescanBtn
                text: qsTr("Fan Setup Wizard")
                implicitHeight: Math.round(34 * root.uiScale)
                leftPadding: Math.round(14 * root.uiScale)
                rightPadding: Math.round(14 * root.uiScale)
                hoverEnabled: true

                background: Rectangle {
                    radius: 8
                    color: rescanBtn.down ? (root.darkMode ? "#3B3156" : "#E2E8F0")
                                          : (rescanBtn.hovered ? (root.darkMode ? "#342D4A" : "#F1F5F9")
                                                               : (root.darkMode ? "#29233B" : "#FFFFFF"))
                    border.width: 1
                    border.color: rescanBtn.hovered ? root.accentColor : root.borderColor
                }

                contentItem: Label {
                    text: rescanBtn.text
                    color: rescanBtn.hovered ? root.accentColor : root.textColor
                    font.pixelSize: Math.round(13 * root.uiScale)
                    font.weight: Font.DemiBold
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: root.openWizardRequested()
            }

            Components.RefreshToolButton {
                busy: root.refreshAnimating
                theme: root.theme
                darkMode: root.darkMode
                uiScale: root.uiScale
                tooltip: qsTr("Refresh fan telemetry")
                enabled: !root.refreshAnimating
                onClicked: root.refreshRequested()
            }
        }

        GridLayout {
            Layout.fillWidth: true
            columns: width > 900 ? 3 : (width > 560 ? 2 : 1)
            columnSpacing: Math.round(10 * root.uiScale)
            rowSpacing: Math.round(10 * root.uiScale)

            Repeater {
                model: root.orderedFans.length > 0 ? root.orderedFans : (root.fanController ? root.fanController.systemFans : [])

                delegate: Rectangle {
                    id: fanCard
                    required property var modelData
                    required property int index
                    Layout.fillWidth: true
                    implicitHeight: Math.round(152 * root.uiScale)
                    radius: 12
                    scale: (root.reorderMode && root.draggingFanIndex === fanCard.index) ? 1.03 : 1.0
                    z: (root.reorderMode && root.draggingFanIndex === fanCard.index) ? 10 : 1
                    color: (root.reorderMode && root.draggingFanIndex === fanCard.index)
                           ? (root.darkMode ? "#3D345C" : "#EDE9FE")
                           : ((root.fanController && root.fanController.selectedFanIndex === fanCard.index)
                              ? (root.darkMode ? "#383152" : "#E0E7FF")
                              : root.bgColor)
                    border.width: (root.reorderMode && root.draggingFanIndex === fanCard.index) ? 2 : ((root.fanController && root.fanController.selectedFanIndex === fanCard.index) ? 2 : 1)
                    border.color: (root.reorderMode && root.draggingFanIndex === fanCard.index)
                                  ? root.accentColor
                                  : ((root.fanController && root.fanController.selectedFanIndex === fanCard.index) ? root.accentColor : root.borderColor)

                    Behavior on scale { NumberAnimation { duration: 150; easing.type: Easing.OutQuad } }

                    MouseArea {
                        id: fanCardMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: root.reorderMode ? Qt.SizeAllCursor : Qt.PointingHandCursor
                        onClicked: {
                            if (root.reorderMode) {
                                root.draggingFanIndex = fanCard.index;
                            } else {
                                root.fanSettingsRequested(fanCard.modelData, fanCard.index);
                            }
                        }
                    }

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: Math.round(12 * root.uiScale)
                        spacing: Math.round(8 * root.uiScale)

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            Rectangle {
                                implicitWidth: Math.round(54 * root.uiScale)
                                implicitHeight: Math.round(24 * root.uiScale)
                                radius: 6
                                color: root.darkMode ? "#4A3E6D" : "#E2E8F0"

                                Row {
                                    anchors.centerIn: parent
                                    spacing: Math.round(5 * root.uiScale)

                                    Label {
                                        text: fanCard.modelData.type === "CPU" ? "▦"
                                              : (fanCard.modelData.type === "GPU" ? "▰" : "✣")
                                        color: root.accentColor
                                        font.pixelSize: Math.round(13 * root.uiScale)
                                        font.weight: Font.Bold
                                    }

                                    Label {
                                        text: fanCard.modelData.type || "SYS"
                                        color: root.textColor
                                        font.pixelSize: Math.round(10 * root.uiScale)
                                        font.weight: Font.Bold
                                    }
                                }
                            }

                            Label {
                                text: fanCard.modelData.name || qsTr("Fan Device")
                                color: root.textColor
                                font.pixelSize: Math.round(14 * root.uiScale)
                                font.weight: Font.DemiBold
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }

                            ToolButton {
                                id: fanSettingsButton
                                visible: !root.reorderMode
                                text: "⚙"
                                implicitWidth: Math.round(28 * root.uiScale)
                                implicitHeight: Math.round(28 * root.uiScale)
                                hoverEnabled: true
                                background: Rectangle {
                                    radius: width / 2
                                    color: fanSettingsButton.hovered ? (root.darkMode ? "#3B3156" : "#E2E8F0") : "transparent"
                                }
                                contentItem: Label {
                                    text: fanSettingsButton.text
                                    color: fanSettingsButton.hovered ? root.accentColor : root.softTextColor
                                    font.pixelSize: Math.round(15 * root.uiScale)
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                ToolTip.visible: hovered
                                ToolTip.text: qsTr("Open fan settings")
                                onClicked: root.fanSettingsRequested(fanCard.modelData, fanCard.index)
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: Math.round(10 * root.uiScale)

                            // Speed Metric
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2

                                Label {
                                    text: qsTr("SPEED")
                                    color: root.softTextColor
                                    font.pixelSize: Math.round(10 * root.uiScale)
                                    font.weight: Font.DemiBold
                                }

                                Label {
                                    text: fanCard.modelData && fanCard.modelData.speedPercent !== undefined
                                          ? (fanCard.modelData.speedPercent + "%")
                                          : qsTr("--")
                                    color: root.textColor
                                    font.pixelSize: Math.round(18 * root.uiScale)
                                    font.weight: Font.Bold
                                }
                            }

                            Rectangle {
                                implicitWidth: 1
                                implicitHeight: Math.round(28 * root.uiScale)
                                color: root.borderColor
                                opacity: 0.6
                            }

                            // RPM Metric
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2

                                Label {
                                    text: qsTr("RPM")
                                    color: root.softTextColor
                                    font.pixelSize: Math.round(10 * root.uiScale)
                                    font.weight: Font.DemiBold
                                }

                                Label {
                                    text: {
                                        if (!fanCard.modelData)
                                            return qsTr("--");
                                        if (fanCard.modelData.rpm > 0)
                                            return fanCard.modelData.rpm + " RPM";
                                        return qsTr("0 RPM");
                                    }
                                    color: root.textColor
                                    font.pixelSize: Math.round(18 * root.uiScale)
                                    font.weight: Font.Bold
                                    elide: Text.ElideRight
                                }
                            }

                            Rectangle {
                                implicitWidth: 1
                                implicitHeight: Math.round(28 * root.uiScale)
                                color: root.borderColor
                                opacity: 0.6
                            }

                            // Temperature Metric
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2

                                Label {
                                    text: qsTr("TEMPERATURE")
                                    color: root.softTextColor
                                    font.pixelSize: Math.round(10 * root.uiScale)
                                    font.weight: Font.DemiBold
                                }

                                Label {
                                    text: fanCard.modelData.temperatureC > 0 ? (fanCard.modelData.temperatureC + " °C") : qsTr("--")
                                    color: (fanCard.modelData && fanCard.modelData.temperatureC > 80) ? root.warningText : root.textColor
                                    font.pixelSize: Math.round(18 * root.uiScale)
                                    font.weight: Font.Bold
                                    elide: Text.ElideRight
                                }
                            }
                        }

                        // Shared reusable TelemetrySparkline
                        Components.TelemetrySparkline {
                            Layout.fillWidth: true
                            Layout.preferredHeight: Math.round(28 * root.uiScale)
                            values: (root.perFanHistories && fanCard.modelData && root.perFanHistories[fanCard.modelData.id])
                                    ? root.perFanHistories[fanCard.modelData.id]
                                    : root.fanSpeedHistory
                            lineColor: (root.fanController && root.fanController.selectedFanIndex === fanCard.index)
                                       ? root.accentColor
                                       : (root.darkMode ? "#6366F1" : "#4F46E5")
                        }

                        // Reordering Controls Bar
                        RowLayout {
                            visible: root.reorderMode
                            Layout.fillWidth: true
                            spacing: 6

                            Button {
                                id: moveLeftBtn
                                enabled: fanCard.index > 0
                                implicitHeight: Math.round(28 * root.uiScale)
                                implicitWidth: Math.round(40 * root.uiScale)
                                hoverEnabled: true

                                background: Rectangle {
                                    radius: 6
                                    color: moveLeftBtn.down ? (root.darkMode ? "#4A3E6D" : "#CBD5E1")
                                                            : (moveLeftBtn.hovered ? (root.darkMode ? "#383152" : "#E2E8F0")
                                                                                   : root.bgColor)
                                    border.width: 1
                                    border.color: moveLeftBtn.hovered ? root.accentColor : root.borderColor
                                    opacity: moveLeftBtn.enabled ? 1.0 : 0.4
                                }

                                contentItem: Text {
                                    text: "←"
                                    color: moveLeftBtn.hovered ? root.accentColor : root.textColor
                                    font.pixelSize: Math.round(14 * root.uiScale)
                                    font.weight: Font.Bold
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }

                                onClicked: root.moveFanRequested(fanCard.index, fanCard.index - 1)
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                implicitHeight: Math.round(28 * root.uiScale)
                                radius: 6
                                color: root.darkMode ? "#2E2442" : "#F3E8FF"
                                border.width: 1
                                border.color: root.darkMode ? "#4C3872" : "#DDD6FE"

                                RowLayout {
                                    anchors.centerIn: parent
                                    spacing: 4

                                    Label {
                                        text: "⠿"
                                        color: root.accentColor
                                        font.pixelSize: Math.round(12 * root.uiScale)
                                    }

                                    Label {
                                        text: qsTr("Slot %1").arg(fanCard.index + 1)
                                        color: root.accentColor
                                        font.pixelSize: Math.round(11 * root.uiScale)
                                        font.weight: Font.Bold
                                    }
                                }
                            }

                            Button {
                                id: moveRightBtn
                                enabled: fanCard.index < (root.orderedFans.length - 1)
                                implicitHeight: Math.round(28 * root.uiScale)
                                implicitWidth: Math.round(40 * root.uiScale)
                                hoverEnabled: true

                                background: Rectangle {
                                    radius: 6
                                    color: moveRightBtn.down ? (root.darkMode ? "#4A3E6D" : "#CBD5E1")
                                                             : (moveRightBtn.hovered ? (root.darkMode ? "#383152" : "#E2E8F0")
                                                                                    : root.bgColor)
                                    border.width: 1
                                    border.color: moveRightBtn.hovered ? root.accentColor : root.borderColor
                                    opacity: moveRightBtn.enabled ? 1.0 : 0.4
                                }

                                contentItem: Text {
                                    text: "→"
                                    color: moveRightBtn.hovered ? root.accentColor : root.textColor
                                    font.pixelSize: Math.round(14 * root.uiScale)
                                    font.weight: Font.Bold
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }

                                onClicked: root.moveFanRequested(fanCard.index, fanCard.index + 1)
                            }
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            visible: root.hasDetectedFans && root.orderedFans.length === 1
            implicitHeight: singleChannelInfo.implicitHeight + Math.round(20 * root.uiScale)
            radius: 8
            color: root.infoBg
            border.width: 1
            border.color: root.borderColor

            Label {
                id: singleChannelInfo
                anchors.fill: parent
                anchors.margins: Math.round(10 * root.uiScale)
                text: qsTr("Only one RPM channel is exposed by Linux. CPU and chassis fans will appear automatically when the motherboard firmware or kernel sensor driver publishes their RPM telemetry.")
                color: root.softTextColor
                wrapMode: Text.WordWrap
                font.pixelSize: Math.round(11 * root.uiScale)
            }
        }
    }
}
