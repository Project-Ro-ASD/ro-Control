import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: summarySection
    implicitWidth: mainColumn.implicitWidth
    implicitHeight: mainColumn.implicitHeight

    property var theme: ({})
    property bool darkMode: false
    property real uiScale: 1.0

    property var systemInfo: null
    property var cpuMonitor: null
    property var gpuMonitor: null
    property var ramMonitor: null
    property var nvidiaDetector: null

    property var cpuUsageHistory: []
    property var gpuLoadHistory: []
    property var ramUsageHistory: []
    property bool telemetryRefreshAnimating: false

    signal refreshClicked()

    readonly property color bgColor: theme && theme.card ? theme.card : (darkMode ? "#29233B" : "#FFFFFF")
    readonly property color cardColor: theme && theme.cardStrong ? theme.cardStrong : (darkMode ? "#342D4A" : "#F1F5F9")
    readonly property color borderColor: theme && theme.border ? theme.border : (darkMode ? "#4D436B" : "#CBD5E1")
    readonly property color textColor: theme && theme.text ? theme.text : (darkMode ? "#F8FAFC" : "#0F172A")
    readonly property color softTextColor: theme && theme.textSoft ? theme.textSoft : (darkMode ? "#94A3B8" : "#64748B")
    readonly property color infoBg: theme && theme.infoBg ? theme.infoBg : (darkMode ? "#1E2548" : "#EFF6FF")
    readonly property color accentColor: theme && theme.accentA ? theme.accentA : (darkMode ? "#818CF8" : "#4F46E5")
    readonly property color warningColor: theme && theme.warning ? theme.warning : "#EF4444"

    readonly property bool nvidiaGpuDetected: nvidiaDetector && nvidiaDetector.gpuFound
    readonly property bool gpuTelemetryAvailable: nvidiaGpuDetected && gpuMonitor && gpuMonitor.available
    readonly property bool cpuTelemetryAvailable: cpuMonitor && cpuMonitor.available
    readonly property bool ramTelemetryAvailable: ramMonitor && ramMonitor.available

    function formatTemp(value) {
        if (value > 0)
            return value + " °C";
        if (systemInfo && systemInfo.virtualMachine)
            return qsTr("VM sensor unavailable");
        return qsTr("Unavailable");
    }

    function formatRam(used, total) {
        return total > 0 ? used + " / " + total + " MiB" : qsTr("Unavailable");
    }

    function localizeGpuName(name) {
        if (!name || name.toString().trim().length === 0)
            return "";
        return systemInfo ? systemInfo.localizeGpuName(name) : name;
    }

    function gpuUnavailableMessage() {
        if (!nvidiaGpuDetected)
            return qsTr("No NVIDIA GPU detected. CPU and memory monitoring remain available; NVIDIA telemetry, power controls, and process monitoring are disabled.");
        if (gpuMonitor && gpuMonitor.statusMessage.length > 0)
            return gpuMonitor.statusMessage;
        return qsTr("NVIDIA GPU telemetry is unavailable. Check the driver and session permissions, then refresh.");
    }

    ColumnLayout {
        id: mainColumn
        anchors.fill: parent
        spacing: Math.round(10 * summarySection.uiScale)

        // Unavailable GPU Notice Banner
        Rectangle {
            visible: summarySection.nvidiaGpuDetected && !summarySection.gpuTelemetryAvailable
            Layout.fillWidth: true
            implicitHeight: unavailableGpuLabel.implicitHeight + 24
            radius: 10
            color: summarySection.infoBg
            border.width: 1
            border.color: summarySection.borderColor

            Label {
                id: unavailableGpuLabel
                anchors.fill: parent
                anchors.margins: 12
                text: summarySection.gpuUnavailableMessage()
                color: summarySection.textColor
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WordWrap
            }
        }

        GridLayout {
            id: telemetryGrid
            Layout.fillWidth: true
            columns: {
                var count = (summarySection.gpuTelemetryAvailable ? 1 : 0) + 1 + (summarySection.ramMonitor && summarySection.ramMonitor.zramAvailable ? 1 : 0) + 1;
                if (count >= 4)
                    return width > 1180 ? 4 : (width > 560 ? 2 : 1);
                if (count === 3)
                    return width > 900 ? 3 : (width > 560 ? 2 : 1);
                return width > 560 ? 2 : 1;
            }
            columnSpacing: Math.round(10 * summarySection.uiScale)
            rowSpacing: Math.round(10 * summarySection.uiScale)

            ActionButton {
                Layout.columnSpan: telemetryGrid.columns
                Layout.alignment: Qt.AlignRight
                text: summarySection.telemetryRefreshAnimating ? qsTr("Refreshing telemetry…") : qsTr("Refresh telemetry")
                enabled: !summarySection.telemetryRefreshAnimating
                theme: summarySection.theme
                tone: "primary"
                compact: true
                uiScale: summarySection.uiScale
                onClicked: summarySection.refreshClicked()
            }

            // CPU Card
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: Math.round(152 * summarySection.uiScale)
                Layout.minimumHeight: Math.round(152 * summarySection.uiScale)
                radius: 14
                color: summarySection.cardColor
                border.width: 1
                border.color: summarySection.borderColor

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: Math.round(14 * summarySection.uiScale)
                    spacing: Math.round(4 * summarySection.uiScale)

                    Label {
                        text: qsTr("CPU")
                        color: summarySection.softTextColor
                        font.weight: Font.DemiBold
                        font.pixelSize: Math.round(13 * summarySection.uiScale)
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Math.round(12 * summarySection.uiScale)

                        ColumnLayout {
                            spacing: 1
                            Label {
                                text: qsTr("USAGE")
                                color: summarySection.softTextColor
                                font.pixelSize: Math.round(10 * summarySection.uiScale)
                                font.weight: Font.DemiBold
                            }
                            Label {
                                text: summarySection.cpuTelemetryAvailable ? summarySection.cpuMonitor.usagePercent.toFixed(1) + "%" : qsTr("Unavailable")
                                color: summarySection.textColor
                                font.pixelSize: Math.round(22 * summarySection.uiScale)
                                font.weight: Font.Bold
                            }
                        }

                        Item { Layout.fillWidth: true }

                        ColumnLayout {
                            spacing: 1
                            Label {
                                text: qsTr("TEMPERATURE")
                                color: summarySection.softTextColor
                                font.pixelSize: Math.round(10 * summarySection.uiScale)
                                font.weight: Font.DemiBold
                            }
                            Label {
                                text: summarySection.cpuTelemetryAvailable ? summarySection.formatTemp(summarySection.cpuMonitor.temperatureC) : qsTr("Unavailable")
                                color: (summarySection.cpuMonitor && summarySection.cpuMonitor.temperatureC > 80) ? summarySection.warningColor : summarySection.textColor
                                font.pixelSize: Math.round(22 * summarySection.uiScale)
                                font.weight: Font.Bold
                            }
                        }
                    }

                    TelemetrySparkline {
                        Layout.fillWidth: true
                        Layout.preferredHeight: Math.round(30 * summarySection.uiScale)
                        values: summarySection.cpuUsageHistory
                        lineColor: summarySection.darkMode ? "#818CF8" : "#4F46E5"
                    }
                }
            }

            // GPU Card
            Rectangle {
                visible: summarySection.gpuTelemetryAvailable
                Layout.fillWidth: summarySection.gpuTelemetryAvailable
                Layout.preferredHeight: Math.round(152 * summarySection.uiScale)
                Layout.minimumHeight: Math.round(152 * summarySection.uiScale)
                radius: 14
                color: summarySection.cardColor
                border.width: 1
                border.color: summarySection.borderColor

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: Math.round(14 * summarySection.uiScale)
                    spacing: Math.round(4 * summarySection.uiScale)

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            text: qsTr("GPU")
                            color: summarySection.softTextColor
                            font.weight: Font.DemiBold
                            font.pixelSize: Math.round(13 * summarySection.uiScale)
                        }

                        Item { Layout.fillWidth: true }

                        Rectangle {
                            id: gpuSelectorButton
                            visible: summarySection.gpuTelemetryAvailable
                            implicitHeight: Math.round(24 * summarySection.uiScale)
                            implicitWidth: (summarySection.gpuMonitor && summarySection.gpuMonitor.gpuCount > 1)
                                           ? gpuSelectorRow.implicitWidth + Math.round(14 * summarySection.uiScale)
                                           : Math.round(26 * summarySection.uiScale)
                            radius: 6
                            color: gpuSelectorMouse.hovered
                                   ? (summarySection.darkMode ? "#342D4A" : "#E2E8F0")
                                   : (summarySection.darkMode ? "#1E2548" : "#EEF2FF")
                            border.width: 1
                            border.color: gpuSelectorMouse.hovered
                                          ? summarySection.accentColor
                                          : (summarySection.darkMode ? "#4338CA" : "#C7D2FE")

                            ToolTip {
                                id: gpuTooltip
                                visible: Boolean(gpuSelectorMouse.hovered && !gpuMenu.visible)
                                delay: 300
                                text: {
                                    if (!summarySection.gpuMonitor) return "";
                                    var count = summarySection.gpuMonitor.gpuCount;
                                    if (count > 1) {
                                        return qsTr("Active: %1\nClick to switch GPU (%2 available)").arg(summarySection.localizeGpuName(summarySection.gpuMonitor.gpuName)).arg(count);
                                    }
                                    return summarySection.gpuMonitor.gpuName.length > 0 ? summarySection.localizeGpuName(summarySection.gpuMonitor.gpuName) : qsTr("Active Graphics Processor");
                                }
                            }

                            MouseArea {
                                id: gpuSelectorMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                focus: true
                                activeFocusOnTab: true
                                cursorShape: (summarySection.gpuMonitor && summarySection.gpuMonitor.gpuCount > 1) ? Qt.PointingHandCursor : Qt.ArrowCursor
                                onClicked: {
                                    if (summarySection.gpuMonitor && summarySection.gpuMonitor.gpuCount > 1) {
                                        gpuMenu.open();
                                    }
                                }
                                Keys.onReturnPressed: {
                                    if (summarySection.gpuMonitor && summarySection.gpuMonitor.gpuCount > 1)
                                        gpuMenu.open();
                                }
                                Keys.onSpacePressed: {
                                    if (summarySection.gpuMonitor && summarySection.gpuMonitor.gpuCount > 1)
                                        gpuMenu.open();
                                }
                            }

                            RowLayout {
                                id: gpuSelectorRow
                                anchors.centerIn: parent
                                spacing: 4

                                Label {
                                    text: "⚡"
                                    font.pixelSize: Math.round(11 * summarySection.uiScale)
                                }

                                Label {
                                    visible: summarySection.gpuMonitor && summarySection.gpuMonitor.gpuCount > 1
                                    text: "GPU " + (summarySection.gpuMonitor ? summarySection.gpuMonitor.selectedGpuIndex : 0) + " ▾"
                                    color: summarySection.accentColor
                                    font.pixelSize: Math.round(10 * summarySection.uiScale)
                                    font.weight: Font.Bold
                                }
                            }

                            Menu {
                                id: gpuMenu
                                y: parent.height + 4
                                x: parent.width - width

                                Repeater {
                                    model: summarySection.gpuMonitor ? summarySection.gpuMonitor.gpuDevices : []

                                    MenuItem {
                                        required property var modelData
                                        required property int index
                                        text: summarySection.localizeGpuName(modelData.name || ("GPU " + index)) + ((summarySection.gpuMonitor && summarySection.gpuMonitor.selectedGpuIndex === index) ? "  ✓" : "")
                                        font.pixelSize: Math.round(11 * summarySection.uiScale)
                                        font.weight: (summarySection.gpuMonitor && summarySection.gpuMonitor.selectedGpuIndex === index) ? Font.Bold : Font.Normal
                                        onTriggered: {
                                            if (summarySection.gpuMonitor) {
                                                summarySection.gpuMonitor.setSelectedGpuIndex(index);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Math.round(12 * summarySection.uiScale)

                        ColumnLayout {
                            spacing: 1
                            Label {
                                text: qsTr("LOAD")
                                color: summarySection.softTextColor
                                font.pixelSize: Math.round(10 * summarySection.uiScale)
                                font.weight: Font.DemiBold
                            }
                            Label {
                                text: summarySection.gpuTelemetryAvailable ? summarySection.gpuMonitor.utilizationPercent + "%" : qsTr("Unavailable")
                                color: summarySection.gpuTelemetryAvailable ? summarySection.textColor : summarySection.softTextColor
                                font.pixelSize: Math.round(22 * summarySection.uiScale)
                                font.weight: Font.Bold
                            }
                        }

                        Item { Layout.fillWidth: true }

                        ColumnLayout {
                            spacing: 1
                            Label {
                                text: qsTr("TEMPERATURE")
                                color: summarySection.softTextColor
                                font.pixelSize: Math.round(10 * summarySection.uiScale)
                                font.weight: Font.DemiBold
                            }
                            RowLayout {
                                spacing: 4
                                Label {
                                    text: summarySection.gpuTelemetryAvailable ? summarySection.formatTemp(summarySection.gpuMonitor.temperatureC) : qsTr("Unavailable")
                                    color: (summarySection.gpuTelemetryAvailable && summarySection.gpuMonitor.temperatureC > 80) ? summarySection.warningColor : (summarySection.gpuTelemetryAvailable ? summarySection.textColor : summarySection.softTextColor)
                                    font.pixelSize: Math.round(22 * summarySection.uiScale)
                                    font.weight: Font.Bold
                                }
                            }
                        }
                    }

                    TelemetrySparkline {
                        Layout.fillWidth: true
                        Layout.preferredHeight: Math.round(30 * summarySection.uiScale)
                        values: summarySection.gpuTelemetryAvailable ? summarySection.gpuLoadHistory : []
                        lineColor: summarySection.darkMode ? "#34D399" : "#10B981"
                    }
                }
            }

            // Memory Card
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: Math.round(152 * summarySection.uiScale)
                Layout.minimumHeight: Math.round(152 * summarySection.uiScale)
                radius: 14
                color: summarySection.cardColor
                border.width: 1
                border.color: summarySection.borderColor

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: Math.round(14 * summarySection.uiScale)
                    spacing: Math.round(4 * summarySection.uiScale)

                    Label {
                        text: qsTr("Memory")
                        color: summarySection.softTextColor
                        font.weight: Font.DemiBold
                        font.pixelSize: Math.round(13 * summarySection.uiScale)
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Math.round(12 * summarySection.uiScale)

                        ColumnLayout {
                            spacing: 1
                            Label {
                                text: qsTr("USAGE")
                                color: summarySection.softTextColor
                                font.pixelSize: Math.round(10 * summarySection.uiScale)
                                font.weight: Font.DemiBold
                            }
                            Label {
                                text: summarySection.ramTelemetryAvailable ? summarySection.ramMonitor.usagePercent + "%" : qsTr("Unavailable")
                                color: summarySection.textColor
                                font.pixelSize: Math.round(22 * summarySection.uiScale)
                                font.weight: Font.Bold
                            }
                        }

                        Item { Layout.fillWidth: true }

                        ColumnLayout {
                            spacing: 1
                            Label {
                                text: qsTr("ALLOCATED")
                                color: summarySection.softTextColor
                                font.pixelSize: Math.round(10 * summarySection.uiScale)
                                font.weight: Font.DemiBold
                            }
                            Label {
                                text: summarySection.ramTelemetryAvailable ? summarySection.formatRam(summarySection.ramMonitor.usedMiB, summarySection.ramMonitor.totalMiB) : qsTr("Unavailable")
                                color: summarySection.textColor
                                font.pixelSize: Math.round(18 * summarySection.uiScale)
                                font.weight: Font.Bold
                            }
                        }
                    }

                    TelemetrySparkline {
                        Layout.fillWidth: true
                        Layout.preferredHeight: Math.round(30 * summarySection.uiScale)
                        values: summarySection.ramUsageHistory
                        lineColor: summarySection.darkMode ? "#FBBF24" : "#D97706"
                    }
                }
            }

            // ZRAM Card
            Rectangle {
                visible: summarySection.ramMonitor && summarySection.ramMonitor.zramAvailable
                Layout.fillWidth: true
                Layout.preferredHeight: Math.round(152 * summarySection.uiScale)
                Layout.minimumHeight: Math.round(152 * summarySection.uiScale)
                radius: 14
                color: summarySection.cardColor
                border.width: 1
                border.color: summarySection.borderColor

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: Math.round(14 * summarySection.uiScale)
                    spacing: Math.round(4 * summarySection.uiScale)

                    Label {
                        text: qsTr("ZRAM")
                        color: summarySection.softTextColor
                        font.weight: Font.DemiBold
                        font.pixelSize: Math.round(13 * summarySection.uiScale)
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Math.round(12 * summarySection.uiScale)

                        ColumnLayout {
                            spacing: 1
                            Label {
                                text: qsTr("USAGE")
                                color: summarySection.softTextColor
                                font.pixelSize: Math.round(10 * summarySection.uiScale)
                                font.weight: Font.DemiBold
                            }
                            Label {
                                text: {
                                    if (!summarySection.ramMonitor || summarySection.ramMonitor.zramTotalMiB <= 0) return "--"
                                    const pct = (summarySection.ramMonitor.zramUsedMiB / summarySection.ramMonitor.zramTotalMiB) * 100
                                    if (summarySection.ramMonitor.zramUsedMiB > 0 && pct < 1.0) return pct.toFixed(1) + "%"
                                    return Math.round(pct) + "%"
                                }
                                color: summarySection.textColor
                                font.pixelSize: Math.round(22 * summarySection.uiScale)
                                font.weight: Font.Bold
                            }
                        }

                        Item { Layout.fillWidth: true }

                        ColumnLayout {
                            spacing: 1
                            Label {
                                text: qsTr("ALLOCATED")
                                color: summarySection.softTextColor
                                font.pixelSize: Math.round(10 * summarySection.uiScale)
                                font.weight: Font.DemiBold
                            }
                            Label {
                                text: (summarySection.ramMonitor ? summarySection.ramMonitor.zramUsedMiB : 0) + " / " + (summarySection.ramMonitor ? summarySection.ramMonitor.zramTotalMiB : 0) + " MiB"
                                color: summarySection.textColor
                                font.pixelSize: Math.round(18 * summarySection.uiScale)
                                font.weight: Font.Bold
                            }
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        visible: summarySection.ramMonitor && (summarySection.ramMonitor.zramCompressionRatio > 0 || summarySection.ramMonitor.zswapEnabled || summarySection.ramMonitor.zramPhysicalMiB > 0)
                        text: (summarySection.ramMonitor && summarySection.ramMonitor.zramCompressionRatio > 0
                               ? qsTr("Compression: %1×").arg(summarySection.ramMonitor.zramCompressionRatio.toFixed(1))
                               : "")
                              + (summarySection.ramMonitor && summarySection.ramMonitor.zramPhysicalMiB > 0
                                 ? ((summarySection.ramMonitor.zramCompressionRatio > 0 ? " • " : "") + qsTr("RAM: %1 MiB").arg(summarySection.ramMonitor.zramPhysicalMiB))
                                 : "")
                              + (summarySection.ramMonitor && summarySection.ramMonitor.zswapEnabled
                                 ? ((summarySection.ramMonitor.zramCompressionRatio > 0 || summarySection.ramMonitor.zramPhysicalMiB > 0 ? " • " : "") + qsTr("zswap enabled"))
                                 : "")
                        color: summarySection.softTextColor
                        font.pixelSize: Math.round(10 * summarySection.uiScale)
                        font.weight: Font.DemiBold
                        elide: Text.ElideRight
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: Math.round(4 * summarySection.uiScale)

                        Rectangle {
                            anchors.fill: parent
                            radius: 2
                            color: summarySection.darkMode ? "#2D2644" : "#E2E8F0"
                        }

                        Rectangle {
                            anchors.left: parent.left
                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                            radius: 2
                            width: Math.min(parent.width, Math.max(0, parent.width * (summarySection.ramMonitor && summarySection.ramMonitor.zramTotalMiB > 0 ? (summarySection.ramMonitor.zramUsedMiB / summarySection.ramMonitor.zramTotalMiB) : 0)))
                            color: summarySection.darkMode ? "#F59E0B" : "#D97706"
                            Behavior on width { NumberAnimation { duration: 250; easing.type: Easing.OutCubic } }
                        }
                    }
                }
            }
        }
    }
}
