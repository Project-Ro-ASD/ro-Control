import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: processSection
    Layout.fillWidth: true
    radius: 14
    color: theme && theme.cardStrong ? theme.cardStrong : (darkMode ? "#342D4A" : "#F1F5F9")
    border.width: 1
    border.color: theme && theme.border ? theme.border : (darkMode ? "#4D436B" : "#CBD5E1")
    implicitHeight: taskManagerLayout.implicitHeight + 24

    property var theme: ({})
    property bool darkMode: false
    property real uiScale: 1.0

    property var gpuMonitor: null
    property bool showAllGpuProcesses: false

    readonly property color bgColor: theme && theme.card ? theme.card : (darkMode ? "#29233B" : "#FFFFFF")
    readonly property color borderColor: theme && theme.border ? theme.border : (darkMode ? "#4D436B" : "#CBD5E1")
    readonly property color textColor: theme && theme.text ? theme.text : (darkMode ? "#F8FAFC" : "#0F172A")
    readonly property color softTextColor: theme && theme.textSoft ? theme.textSoft : (darkMode ? "#94A3B8" : "#64748B")
    readonly property color accentColor: theme && theme.accentA ? theme.accentA : (darkMode ? "#818CF8" : "#4F46E5")

    ColumnLayout {
        id: taskManagerLayout
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Label {
                Layout.fillWidth: true
                text: qsTr("GPU Task Manager & Active Processes")
                color: processSection.textColor
                font.pixelSize: Math.round(18 * processSection.uiScale)
                font.weight: Font.DemiBold
            }

            Button {
                id: processExpansionButton
                visible: processSection.gpuMonitor && processSection.gpuMonitor.gpuProcessCount > 4
                implicitWidth: processExpansionContent.implicitWidth + Math.round(28 * processSection.uiScale)
                implicitHeight: Math.round(36 * processSection.uiScale)
                hoverEnabled: true
                scale: down ? 0.98 : 1.0
                Behavior on scale { NumberAnimation { duration: 90; easing.type: Easing.OutQuad } }

                background: Rectangle {
                    radius: Math.round(9 * processSection.uiScale)
                    color: processExpansionButton.down
                           ? Qt.darker(processSection.accentColor, 1.16)
                           : (processExpansionButton.hovered ? "#8B8FFF" : processSection.accentColor)
                    border.width: 1
                    border.color: processExpansionButton.hovered ? "#B8BAFF" : processSection.accentColor
                }

                contentItem: RowLayout {
                    id: processExpansionContent
                    spacing: Math.round(8 * processSection.uiScale)
                    Text {
                        text: processSection.showAllGpuProcesses ? qsTr("Show less") : qsTr("All processes")
                        color: "#FFFFFF"
                        font.pixelSize: Math.round(12 * processSection.uiScale)
                        font.weight: Font.DemiBold
                    }
                    Rectangle {
                        implicitWidth: 1
                        implicitHeight: Math.round(16 * processSection.uiScale)
                        color: "#55FFFFFF"
                    }
                    Text {
                        text: processSection.showAllGpuProcesses ? "−" : (processSection.gpuMonitor ? processSection.gpuMonitor.gpuProcessCount : 0)
                        color: "#FFFFFF"
                        font.pixelSize: Math.round(11 * processSection.uiScale)
                        font.weight: Font.Bold
                    }
                    Text {
                        text: processSection.showAllGpuProcesses ? "⌃" : "⌄"
                        color: "#DDFFFFFF"
                        font.pixelSize: Math.round(13 * processSection.uiScale)
                    }
                }

                onClicked: processSection.showAllGpuProcesses = !processSection.showAllGpuProcesses
            }
        }

        // Empty state when no GPU processes are running
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: Math.round(72 * processSection.uiScale)
            radius: 10
            color: processSection.bgColor
            border.width: 1
            border.color: processSection.borderColor
            visible: !processSection.gpuMonitor || processSection.gpuMonitor.gpuProcessCount === 0

            RowLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 14

                Rectangle {
                    implicitWidth: Math.round(36 * processSection.uiScale)
                    implicitHeight: Math.round(36 * processSection.uiScale)
                    Layout.preferredWidth: Math.round(36 * processSection.uiScale)
                    Layout.preferredHeight: Math.round(36 * processSection.uiScale)
                    radius: 8
                    color: processSection.darkMode ? "#1E293B" : "#E2E8F0"

                    Label {
                        anchors.centerIn: parent
                        text: "✓"
                        color: "#22C55E"
                        font.pixelSize: Math.round(18 * processSection.uiScale)
                        font.weight: Font.Bold
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    Label {
                        text: qsTr("No Active GPU Processes")
                        color: processSection.textColor
                        font.pixelSize: Math.round(13 * processSection.uiScale)
                        font.weight: Font.DemiBold
                    }

                    Label {
                        text: qsTr("No applications are currently allocating VRAM or compute resources on this GPU.")
                        color: processSection.softTextColor
                        font.pixelSize: Math.round(11 * processSection.uiScale)
                        elide: Text.ElideRight
                    }
                }
            }
        }

        // Process list table when processes exist
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 6
            visible: processSection.gpuMonitor && processSection.gpuMonitor.gpuProcessCount > 0

            // Table header
            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 12
                Layout.rightMargin: 12
                spacing: 12

                Label {
                    Layout.preferredWidth: Math.round(60 * processSection.uiScale)
                    text: qsTr("PID")
                    color: processSection.softTextColor
                    font.pixelSize: Math.round(11 * processSection.uiScale)
                    font.weight: Font.Bold
                }

                Label {
                    Layout.fillWidth: true
                    text: qsTr("PROCESS NAME")
                    color: processSection.softTextColor
                    font.pixelSize: Math.round(11 * processSection.uiScale)
                    font.weight: Font.Bold
                }

                Label {
                    visible: processSection.width > 560
                    Layout.preferredWidth: Math.round(110 * processSection.uiScale)
                    text: qsTr("TYPE")
                    color: processSection.softTextColor
                    font.pixelSize: Math.round(11 * processSection.uiScale)
                    font.weight: Font.Bold
                }

                Label {
                    visible: processSection.width > 560
                    Layout.preferredWidth: Math.round(140 * processSection.uiScale)
                    text: qsTr("VRAM ALLOCATION")
                    color: processSection.softTextColor
                    font.pixelSize: Math.round(11 * processSection.uiScale)
                    font.weight: Font.Bold
                    horizontalAlignment: Text.AlignHCenter
                }

                Label {
                    Layout.preferredWidth: Math.round(82 * processSection.uiScale)
                    text: qsTr("ACTION")
                    color: processSection.softTextColor
                    font.pixelSize: Math.round(11 * processSection.uiScale)
                    font.weight: Font.Bold
                    horizontalAlignment: Text.AlignHCenter
                }
            }

            ScrollView {
                Layout.fillWidth: true
                implicitHeight: Math.min(Math.round(280 * processSection.uiScale), processRowsColumn.implicitHeight)
                clip: true
                ScrollBar.vertical.policy: processRowsColumn.implicitHeight > (280 * processSection.uiScale) ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded

                ColumnLayout {
                    id: processRowsColumn
                    width: parent.width
                    spacing: 6

                    Repeater {
                        model: {
                            var all = processSection.gpuMonitor ? processSection.gpuMonitor.gpuProcesses : [];
                            return processSection.showAllGpuProcesses ? all : all.slice(0, 4);
                        }

                        delegate: Rectangle {
                            required property var modelData
                            Layout.fillWidth: true
                            implicitHeight: Math.round(48 * processSection.uiScale)
                            radius: 8
                            color: processSection.bgColor
                            border.width: 1
                            border.color: processSection.borderColor

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 12
                                spacing: 12

                                Rectangle {
                                    Layout.preferredWidth: Math.round(60 * processSection.uiScale)
                                    Layout.preferredHeight: Math.round(24 * processSection.uiScale)
                                    radius: 4
                                    color: processSection.darkMode ? "#1E293B" : "#E2E8F0"

                                    Label {
                                        anchors.centerIn: parent
                                        text: modelData.pid
                                        color: processSection.textColor
                                        font.pixelSize: Math.round(11 * processSection.uiScale)
                                        font.weight: Font.DemiBold
                                        font.family: (Qt.platform.os === "osx") ? "Menlo" : "Monospace"
                                    }
                                }

                                Label {
                                    Layout.fillWidth: true
                                    text: modelData.name
                                    color: processSection.textColor
                                    font.pixelSize: Math.round(13 * processSection.uiScale)
                                    font.weight: Font.DemiBold
                                    elide: Text.ElideRight
                                }

                                Rectangle {
                                    visible: processSection.width > 560
                                    Layout.preferredWidth: Math.round(110 * processSection.uiScale)
                                    Layout.preferredHeight: Math.round(22 * processSection.uiScale)
                                    radius: 4
                                    color: (modelData.type && modelData.type.indexOf("Compute") !== -1)
                                           ? (processSection.darkMode ? "#312E81" : "#EEF2FF")
                                           : (processSection.darkMode ? "#064E3B" : "#ECFDF5")

                                    Label {
                                        anchors.centerIn: parent
                                        text: modelData.type || qsTr("Compute")
                                        color: (modelData.type && modelData.type.indexOf("Compute") !== -1)
                                               ? (processSection.darkMode ? "#A5B4FC" : "#4F46E5")
                                               : (processSection.darkMode ? "#6EE7B7" : "#059669")
                                        font.pixelSize: Math.round(10 * processSection.uiScale)
                                        font.weight: Font.DemiBold
                                    }
                                }

                                Rectangle {
                                    visible: processSection.width > 560
                                    Layout.preferredWidth: Math.round(140 * processSection.uiScale)
                                    Layout.preferredHeight: Math.round(26 * processSection.uiScale)
                                    radius: 6
                                    color: processSection.darkMode ? "#1E2238" : "#EEF2F6"
                                    border.width: 1
                                    border.color: processSection.darkMode ? "#342D4A" : "#E2E8F0"
                                    clip: true

                                    Rectangle {
                                        anchors.left: parent.left
                                        anchors.top: parent.top
                                        anchors.bottom: parent.bottom
                                        width: {
                                            var total = processSection.gpuMonitor ? processSection.gpuMonitor.memoryTotalMiB : 0;
                                            var pct = total > 0 ? Math.min(1.0, Math.max(0.04, modelData.vramMiB / total)) : 0;
                                            return parent.width * pct;
                                        }
                                        radius: 5
                                        color: processSection.darkMode ? "#4338CA" : "#C7D2FE"
                                        opacity: 0.7
                                    }

                                    Label {
                                        anchors.centerIn: parent
                                        text: {
                                            var total = processSection.gpuMonitor ? processSection.gpuMonitor.memoryTotalMiB : 0;
                                            if (total <= 0)
                                                return modelData.vramMiB + " MiB";
                                            var pct = ((modelData.vramMiB / total) * 100).toFixed(modelData.vramMiB > 1000 ? 0 : 1);
                                            return modelData.vramMiB + " MiB (" + pct + "%)";
                                        }
                                        color: processSection.textColor
                                        font.pixelSize: Math.round(11 * processSection.uiScale)
                                        font.weight: Font.Bold
                                    }
                                }

                                Button {
                                    id: endProcessBtn
                                    text: qsTr("End Task")
                                    implicitWidth: Math.round(82 * processSection.uiScale)
                                    implicitHeight: Math.round(28 * processSection.uiScale)

                                    background: Rectangle {
                                        radius: 6
                                        color: endProcessBtn.down ? (processSection.darkMode ? "#7F1D1D" : "#FEE2E2")
                                                                  : (endProcessBtn.hovered ? (processSection.darkMode ? "#450A0A" : "#FEF2F2")
                                                                                           : (processSection.darkMode ? "#29233B" : "#F8FAFC"))
                                        border.width: 1
                                        border.color: endProcessBtn.hovered ? (processSection.darkMode ? "#EF4444" : "#DC2626")
                                                                            : processSection.borderColor
                                    }

                                    contentItem: Text {
                                        text: endProcessBtn.text
                                        color: endProcessBtn.hovered ? (processSection.darkMode ? "#F87171" : "#DC2626")
                                                                     : processSection.textColor
                                        font.pixelSize: Math.round(11 * processSection.uiScale)
                                        font.weight: Font.DemiBold
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }

                                    onClicked: {
                                        terminateDialogLoader.active = true;
                                        if (terminateDialogLoader.item) {
                                            terminateDialogLoader.item.targetPid = modelData.pid;
                                            terminateDialogLoader.item.targetName = modelData.name;
                                            terminateDialogLoader.item.terminationError = "";
                                            terminateDialogLoader.item.open();
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Loader {
        id: terminateDialogLoader
        active: false
        source: "MonitorTerminateProcessDialog.qml"
        onLoaded: {
            item.theme = processSection.theme;
            item.darkMode = processSection.darkMode;
            item.uiScale = processSection.uiScale;
            item.gpuMonitor = processSection.gpuMonitor;
        }
    }
}
