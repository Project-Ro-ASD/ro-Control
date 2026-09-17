import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components" as Components

Item {
    id: page
    required property var systemInfo
    required property var cpuMonitor
    required property var gpuMonitor
    required property var ramMonitor
    required property var fanController
    required property var nvidiaDetector
    property var powerController: null
    property var healthGuard: null

    property var theme: ({})
    property bool darkMode: false
    property bool showAdvancedInfo: true
    property real uiScale: 1.0
    property bool telemetryRefreshAnimating: false
    property int telemetryRefreshStep: 0
    property var cpuUsageHistory: []
    property var gpuLoadHistory: []
    property var ramUsageHistory: []
    property int pendingTerminationPid: -1
    property string pendingTerminationName: ""
    property bool showAllGpuProcesses: false

    readonly property color bgColor: theme && theme.card ? theme.card : (page.darkMode ? "#29233B" : "#FFFFFF")
    readonly property color cardColor: theme && theme.cardStrong ? theme.cardStrong : (page.darkMode ? "#342D4A" : "#F1F5F9")
    readonly property color borderColor: theme && theme.border ? theme.border : (page.darkMode ? "#4D436B" : "#CBD5E1")
    readonly property color textColor: theme && theme.text ? theme.text : (page.darkMode ? "#F8FAFC" : "#0F172A")
    readonly property color softTextColor: theme && theme.textSoft ? theme.textSoft : (page.darkMode ? "#94A3B8" : "#64748B")
    readonly property color infoBg: theme && theme.infoBg ? theme.infoBg : (page.darkMode ? "#1E2548" : "#EFF6FF")
    readonly property color accentColor: theme && theme.accentA ? theme.accentA : (page.darkMode ? "#818CF8" : "#4F46E5")
    readonly property color successColor: theme && theme.success ? theme.success : (page.darkMode ? "#4ADE80" : "#059669")
    readonly property color activeCardColor: theme && theme.card ? theme.card : (page.darkMode ? "#342D4A" : "#E2E8F0")
    readonly property int summaryCardHeight: Math.round(152 * page.uiScale)
    readonly property bool nvidiaGpuDetected: page.nvidiaDetector && page.nvidiaDetector.gpuFound
    readonly property bool gpuTelemetryAvailable: page.nvidiaGpuDetected && page.gpuMonitor && page.gpuMonitor.available

    component TelemetrySparkline: Canvas {
        id: sparkline
        property var values: []
        property color lineColor: page.accentColor
        property color fillColor: Qt.rgba(lineColor.r, lineColor.g, lineColor.b, 0.15)
        property real maxValue: 100.0

        renderTarget: Canvas.Image
        renderStrategy: Canvas.Immediate
        visible: page.visible

        onValuesChanged: if (page.visible) Qt.callLater(requestPaint)
        onWidthChanged: if (page.visible) Qt.callLater(requestPaint)
        onHeightChanged: if (page.visible) Qt.callLater(requestPaint)

        onPaint: {
            var ctx = getContext("2d");
            ctx.reset();
            var w = width;
            var h = height;
            if (w <= 0 || h <= 0 || !values || values.length < 2)
                return;

            ctx.clearRect(0, 0, w, h);

            var len = values.length;
            var step = w / (len - 1);

            ctx.beginPath();
            for (var i = 0; i < len; ++i) {
                var val = Math.max(0, Math.min(maxValue, values[i]));
                var x = i * step;
                var y = h - (val / maxValue) * (h - 4) - 2;
                if (i === 0) {
                    ctx.moveTo(x, y);
                } else {
                    ctx.lineTo(x, y);
                }
            }

            ctx.strokeStyle = lineColor;
            ctx.lineWidth = 1.8;
            ctx.stroke();

            ctx.lineTo(w, h);
            ctx.lineTo(0, h);
            ctx.closePath();
            ctx.fillStyle = fillColor;
            ctx.fill();
        }
    }

    function formatTemp(value) {
        if (value > 0)
            return value + " °C";
        if (page.systemInfo && page.systemInfo.virtualMachine)
            return qsTr("VM sensor unavailable");
        return qsTr("Unavailable");
    }

    function formatRam(used, total) {
        return total > 0 ? used + " / " + total + " MiB" : qsTr("Unavailable");
    }

    function safeText(value) {
        return value && value.length > 0 ? value : qsTr("Unavailable");
    }

    function deviceTypeLabel() {
        if (!page.systemInfo)
            return qsTr("Unavailable");
        return page.systemInfo.deviceType && page.systemInfo.deviceType.length > 0
               ? page.systemInfo.deviceType
               : qsTr("Unavailable");
    }

    function localizeGpuName(name) {
        if (!name || name.toString().trim().length === 0)
            return "";
        return page.systemInfo ? page.systemInfo.localizeGpuName(name) : name;
    }

    function gpuUnavailableMessage() {
        if (!page.nvidiaGpuDetected)
            return qsTr("No NVIDIA GPU detected. CPU and memory monitoring remain available; NVIDIA telemetry, power controls, and process monitoring are disabled.");
        if (page.gpuMonitor && page.gpuMonitor.statusMessage.length > 0)
            return page.gpuMonitor.statusMessage;
        return qsTr("NVIDIA GPU telemetry is unavailable. Check the driver and session permissions, then refresh.");
    }

    function modeTitle(mode) {
        switch (mode) {
        case "silent": return qsTr("Silent (Acoustic)");
        case "balanced": return qsTr("Balanced (Optimized)");
        case "performance": return qsTr("Performance (High Cooling)");
        case "manual": return qsTr("Manual (Fixed Speed)");
        case "custom": return qsTr("Custom Curve");
        case "auto":
        default: return qsTr("Auto (VBIOS / Driver)");
        }
    }

    function modeBadgeText() {
        if (page.fanController && page.fanController.safetyOverrideActive)
            return qsTr("SAFETY OVERRIDE 100%");
        if (!page.fanController || !page.fanController.supported)
            return qsTr("HARDWARE AUTO");
        if (!page.fanController.controlSupported)
            return qsTr("HARDWARE MANAGED");
        return page.fanController.fanMode.toUpperCase();
    }

    function pushTelemetryHistory() {
        if (!page.visible)
            return;
        var cpuVal = page.cpuMonitor ? page.cpuMonitor.usagePercent : 0;
        var gpuVal = page.gpuMonitor ? page.gpuMonitor.utilizationPercent : 0;
        var ramVal = page.ramMonitor ? page.ramMonitor.usagePercent : 0;

        var cpuArr = page.cpuUsageHistory.slice();
        cpuArr.push(cpuVal);
        if (cpuArr.length > 30) cpuArr.shift();
        page.cpuUsageHistory = cpuArr;

        var gpuArr = page.gpuLoadHistory.slice();
        gpuArr.push(gpuVal);
        if (gpuArr.length > 30) gpuArr.shift();
        page.gpuLoadHistory = gpuArr;

        var ramArr = page.ramUsageHistory.slice();
        ramArr.push(ramVal);
        if (ramArr.length > 30) ramArr.shift();
        page.ramUsageHistory = ramArr;
    }

    function refreshTelemetry() {
        if (page.telemetryRefreshAnimating)
            return;
        page.telemetryRefreshAnimating = true;
        page.telemetryRefreshStep = 0;
        telemetryRefreshQueue.restart();
    }

    function refreshTelemetryStep() {
        if (page.telemetryRefreshStep === 0 && page.ramMonitor) {
            page.ramMonitor.start();
            page.ramMonitor.refresh();
        } else if (page.telemetryRefreshStep === 1 && page.cpuMonitor) {
            page.cpuMonitor.start();
            page.cpuMonitor.refresh();
        } else if (page.telemetryRefreshStep === 2 && page.gpuMonitor) {
            page.gpuMonitor.start();
            page.gpuMonitor.requestRefresh();
        } else if (page.telemetryRefreshStep === 3 && page.fanController) {
            page.fanController.start();
            page.fanController.refresh();
        }

        page.telemetryRefreshStep += 1;
        if (page.telemetryRefreshStep < 4) {
            telemetryRefreshQueue.restart();
        } else {
            telemetryRefreshPulse.restart();
        }
    }

    ScrollView {
        id: pageScroll
        anchors.fill: parent
        clip: true
        contentWidth: availableWidth

        ColumnLayout {
            width: pageScroll.availableWidth
            spacing: 12

            Rectangle {
                visible: page.nvidiaGpuDetected && !page.gpuTelemetryAvailable
                Layout.fillWidth: true
                implicitHeight: unavailableGpuLabel.implicitHeight + 24
                radius: 10
                color: page.infoBg
                border.width: 1
                border.color: page.borderColor
                Label {
                    id: unavailableGpuLabel
                    anchors.fill: parent
                    anchors.margins: 12
                    text: page.gpuUnavailableMessage()
                    color: page.textColor
                    verticalAlignment: Text.AlignVCenter
                    wrapMode: Text.WordWrap
                }
            }

            GridLayout {
                Layout.fillWidth: true
                columns: {
                    var count = (page.gpuTelemetryAvailable ? 1 : 0) + 1 + (page.ramMonitor && page.ramMonitor.zramAvailable ? 1 : 0) + 1;
                    if (count >= 4)
                        return width > 1180 ? 4 : (width > 560 ? 2 : 1);
                    if (count === 3)
                        return width > 900 ? 3 : (width > 560 ? 2 : 1);
                    return width > 560 ? 2 : 1;
                }
                columnSpacing: Math.round(10 * page.uiScale)
                rowSpacing: Math.round(10 * page.uiScale)

                // CPU Card
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: Math.round(152 * page.uiScale)
                    Layout.minimumHeight: Math.round(152 * page.uiScale)
                    radius: 14
                    color: page.cardColor
                    border.width: 1
                    border.color: page.borderColor

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: Math.round(14 * page.uiScale)
                        spacing: Math.round(4 * page.uiScale)

                        Label {
                            text: qsTr("CPU")
                            color: page.softTextColor
                            font.weight: Font.DemiBold
                            font.pixelSize: Math.round(13 * page.uiScale)
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: Math.round(12 * page.uiScale)

                            ColumnLayout {
                                spacing: 1
                                Label {
                                    text: qsTr("USAGE")
                                    color: page.softTextColor
                                    font.pixelSize: Math.round(10 * page.uiScale)
                                    font.weight: Font.DemiBold
                                }
                                Label {
                                    text: page.cpuMonitor ? page.cpuMonitor.usagePercent.toFixed(1) + "%" : "--"
                                    color: page.textColor
                                    font.pixelSize: Math.round(22 * page.uiScale)
                                    font.weight: Font.Bold
                                }
                            }

                            Item { Layout.fillWidth: true }

                            ColumnLayout {
                                spacing: 1
                                Label {
                                    text: qsTr("TEMPERATURE")
                                    color: page.softTextColor
                                    font.pixelSize: Math.round(10 * page.uiScale)
                                    font.weight: Font.DemiBold
                                }
                                Label {
                                    text: page.formatTemp(page.cpuMonitor ? page.cpuMonitor.temperatureC : -1)
                                    color: (page.cpuMonitor && page.cpuMonitor.temperatureC > 80) ? (page.theme && page.theme.warning ? page.theme.warning : "#EF4444") : page.textColor
                                    font.pixelSize: Math.round(22 * page.uiScale)
                                    font.weight: Font.Bold
                                }
                            }
                        }

                        TelemetrySparkline {
                            Layout.fillWidth: true
                            Layout.preferredHeight: Math.round(30 * page.uiScale)
                            values: page.cpuUsageHistory
                            lineColor: page.darkMode ? "#818CF8" : "#4F46E5"
                        }
                    }
                }

                // GPU Card
                Rectangle {
                    visible: page.gpuTelemetryAvailable
                    Layout.fillWidth: page.gpuTelemetryAvailable
                    Layout.preferredHeight: Math.round(152 * page.uiScale)
                    Layout.minimumHeight: Math.round(152 * page.uiScale)
                    radius: 14
                    color: page.cardColor
                    border.width: 1
                    border.color: page.borderColor

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: Math.round(14 * page.uiScale)
                        spacing: Math.round(4 * page.uiScale)

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            Label {
                                text: qsTr("GPU")
                                color: page.softTextColor
                                font.weight: Font.DemiBold
                                font.pixelSize: Math.round(13 * page.uiScale)
                            }

                            Item { Layout.fillWidth: true }

                            Rectangle {
                                id: gpuSelectorButton
                                visible: page.gpuTelemetryAvailable
                                implicitHeight: Math.round(24 * page.uiScale)
                                implicitWidth: (page.gpuMonitor && page.gpuMonitor.gpuCount > 1)
                                               ? gpuSelectorRow.implicitWidth + Math.round(14 * page.uiScale)
                                               : Math.round(26 * page.uiScale)
                                radius: 6
                                color: gpuSelectorMouse.hovered
                                       ? (page.darkMode ? "#342D4A" : "#E2E8F0")
                                       : (page.darkMode ? "#1E2548" : "#EEF2FF")
                                border.width: 1
                                border.color: gpuSelectorMouse.hovered
                                              ? page.accentColor
                                              : (page.darkMode ? "#4338CA" : "#C7D2FE")

                                ToolTip {
                                    id: gpuTooltip
                                    visible: Boolean(gpuSelectorMouse.hovered && !gpuMenu.visible)
                                    delay: 300
                                    text: {
                                        if (!page.gpuMonitor) return "";
                                        var count = page.gpuMonitor.gpuCount;
                                        if (count > 1) {
                                            return qsTr("Active: %1\nClick to switch GPU (%2 available)").arg(page.localizeGpuName(page.gpuMonitor.gpuName)).arg(count);
                                        }
                                        return page.gpuMonitor.gpuName.length > 0 ? page.localizeGpuName(page.gpuMonitor.gpuName) : qsTr("Active Graphics Processor");
                                    }
                                }

                                MouseArea {
                                    id: gpuSelectorMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: (page.gpuMonitor && page.gpuMonitor.gpuCount > 1) ? Qt.PointingHandCursor : Qt.ArrowCursor
                                    onClicked: {
                                        if (page.gpuMonitor && page.gpuMonitor.gpuCount > 1) {
                                            gpuMenu.open();
                                        }
                                    }
                                }

                                RowLayout {
                                    id: gpuSelectorRow
                                    anchors.centerIn: parent
                                    spacing: 4

                                    Label {
                                        text: "⚡"
                                        font.pixelSize: Math.round(11 * page.uiScale)
                                    }

                                    Label {
                                        visible: page.gpuMonitor && page.gpuMonitor.gpuCount > 1
                                        text: "GPU " + (page.gpuMonitor ? page.gpuMonitor.selectedGpuIndex : 0) + " ▾"
                                        color: page.accentColor
                                        font.pixelSize: Math.round(10 * page.uiScale)
                                        font.weight: Font.Bold
                                    }
                                }

                                Menu {
                                    id: gpuMenu
                                    y: parent.height + 4
                                    x: parent.width - width

                                    Repeater {
                                        model: page.gpuMonitor ? page.gpuMonitor.gpuDevices : []

                                        MenuItem {
                                            required property var modelData
                                            required property int index
                                            text: page.localizeGpuName(modelData.name || ("GPU " + index)) + ((page.gpuMonitor && page.gpuMonitor.selectedGpuIndex === index) ? "  ✓" : "")
                                            font.pixelSize: Math.round(11 * page.uiScale)
                                            font.weight: (page.gpuMonitor && page.gpuMonitor.selectedGpuIndex === index) ? Font.Bold : Font.Normal
                                            onTriggered: {
                                                if (page.gpuMonitor) {
                                                    page.gpuMonitor.setSelectedGpuIndex(index);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: Math.round(12 * page.uiScale)

                            ColumnLayout {
                                spacing: 1
                                Label {
                                    text: qsTr("LOAD")
                                    color: page.softTextColor
                                    font.pixelSize: Math.round(10 * page.uiScale)
                                    font.weight: Font.DemiBold
                                }
                                Label {
                                    text: page.gpuTelemetryAvailable ? page.gpuMonitor.utilizationPercent + "%" : qsTr("Unavailable")
                                    color: page.gpuTelemetryAvailable ? page.textColor : page.softTextColor
                                    font.pixelSize: Math.round(22 * page.uiScale)
                                    font.weight: Font.Bold
                                }
                            }

                            Item { Layout.fillWidth: true }

                            ColumnLayout {
                                spacing: 1
                                Label {
                                    text: qsTr("TEMPERATURE")
                                    color: page.softTextColor
                                    font.pixelSize: Math.round(10 * page.uiScale)
                                    font.weight: Font.DemiBold
                                }
                                RowLayout {
                                    spacing: 4
                                    Label {
                                        text: page.gpuTelemetryAvailable ? page.formatTemp(page.gpuMonitor.temperatureC) : qsTr("Unavailable")
                                        color: (page.gpuTelemetryAvailable && page.gpuMonitor.temperatureC > 80) ? (page.theme && page.theme.warning ? page.theme.warning : "#EF4444") : (page.gpuTelemetryAvailable ? page.textColor : page.softTextColor)
                                        font.pixelSize: Math.round(22 * page.uiScale)
                                        font.weight: Font.Bold
                                    }
                                }
                            }
                        }

                        TelemetrySparkline {
                            Layout.fillWidth: true
                            Layout.preferredHeight: Math.round(30 * page.uiScale)
                            values: page.gpuTelemetryAvailable ? page.gpuLoadHistory : []
                            lineColor: page.darkMode ? "#34D399" : "#10B981"
                        }
                    }
                }

                // Memory Card
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: Math.round(152 * page.uiScale)
                    Layout.minimumHeight: Math.round(152 * page.uiScale)
                    radius: 14
                    color: page.cardColor
                    border.width: 1
                    border.color: page.borderColor

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: Math.round(14 * page.uiScale)
                        spacing: Math.round(4 * page.uiScale)

                        Label {
                            text: qsTr("Memory")
                            color: page.softTextColor
                            font.weight: Font.DemiBold
                            font.pixelSize: Math.round(13 * page.uiScale)
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: Math.round(12 * page.uiScale)

                            ColumnLayout {
                                spacing: 1
                                Label {
                                    text: qsTr("USAGE")
                                    color: page.softTextColor
                                    font.pixelSize: Math.round(10 * page.uiScale)
                                    font.weight: Font.DemiBold
                                }
                                Label {
                                    text: page.ramMonitor ? page.ramMonitor.usagePercent + "%" : "--"
                                    color: page.textColor
                                    font.pixelSize: Math.round(22 * page.uiScale)
                                    font.weight: Font.Bold
                                }
                            }

                            Item { Layout.fillWidth: true }

                            ColumnLayout {
                                spacing: 1
                                Label {
                                    text: qsTr("ALLOCATED")
                                    color: page.softTextColor
                                    font.pixelSize: Math.round(10 * page.uiScale)
                                    font.weight: Font.DemiBold
                                }
                                Label {
                                    text: page.formatRam(page.ramMonitor ? page.ramMonitor.usedMiB : 0, page.ramMonitor ? page.ramMonitor.totalMiB : 0)
                                    color: page.textColor
                                    font.pixelSize: Math.round(18 * page.uiScale)
                                    font.weight: Font.Bold
                                }
                            }
                        }

                        TelemetrySparkline {
                            Layout.fillWidth: true
                            Layout.preferredHeight: Math.round(30 * page.uiScale)
                            values: page.ramUsageHistory
                            lineColor: page.darkMode ? "#FBBF24" : "#D97706"
                        }
                    }
                }

                // ZRAM Card — exposed only when the kernel has an active zram device.
                Rectangle {
                    visible: page.ramMonitor && page.ramMonitor.zramAvailable
                    Layout.fillWidth: true
                    Layout.preferredHeight: Math.round(152 * page.uiScale)
                    Layout.minimumHeight: Math.round(152 * page.uiScale)
                    radius: 14
                    color: page.cardColor
                    border.width: 1
                    border.color: page.borderColor

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: Math.round(14 * page.uiScale)
                        spacing: Math.round(4 * page.uiScale)

                        Label {
                            text: qsTr("ZRAM")
                            color: page.softTextColor
                            font.weight: Font.DemiBold
                            font.pixelSize: Math.round(13 * page.uiScale)
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: Math.round(12 * page.uiScale)

                            ColumnLayout {
                                spacing: 1
                                Label {
                                    text: qsTr("USAGE")
                                    color: page.softTextColor
                                    font.pixelSize: Math.round(10 * page.uiScale)
                                    font.weight: Font.DemiBold
                                }
                                Label {
                                    text: {
                                        if (!page.ramMonitor || page.ramMonitor.zramTotalMiB <= 0) return "--"
                                        const pct = (page.ramMonitor.zramUsedMiB / page.ramMonitor.zramTotalMiB) * 100
                                        if (page.ramMonitor.zramUsedMiB > 0 && pct < 1.0) return pct.toFixed(1) + "%"
                                        return Math.round(pct) + "%"
                                    }
                                    color: page.textColor
                                    font.pixelSize: Math.round(22 * page.uiScale)
                                    font.weight: Font.Bold
                                }
                            }

                            Item { Layout.fillWidth: true }

                            ColumnLayout {
                                spacing: 1
                                Label {
                                    text: qsTr("ALLOCATED")
                                    color: page.softTextColor
                                    font.pixelSize: Math.round(10 * page.uiScale)
                                    font.weight: Font.DemiBold
                                }
                                Label {
                                    text: page.ramMonitor.zramUsedMiB + " / " + page.ramMonitor.zramTotalMiB + " MiB"
                                    color: page.textColor
                                    font.pixelSize: Math.round(18 * page.uiScale)
                                    font.weight: Font.Bold
                                }
                            }
                        }

                        Label {
                            Layout.fillWidth: true
                            visible: page.ramMonitor.zramCompressionRatio > 0 || page.ramMonitor.zswapEnabled || page.ramMonitor.zramPhysicalMiB > 0
                            text: (page.ramMonitor.zramCompressionRatio > 0
                                   ? qsTr("Compression: %1×").arg(page.ramMonitor.zramCompressionRatio.toFixed(1))
                                   : "")
                                  + (page.ramMonitor.zramPhysicalMiB > 0
                                     ? (page.ramMonitor.zramCompressionRatio > 0 ? " • " : "") + qsTr("RAM: %1 MiB").arg(page.ramMonitor.zramPhysicalMiB)
                                     : "")
                                  + (page.ramMonitor.zswapEnabled
                                     ? (page.ramMonitor.zramCompressionRatio > 0 || page.ramMonitor.zramPhysicalMiB > 0 ? " • " : "") + qsTr("zswap enabled")
                                     : "")
                            color: page.softTextColor
                            font.pixelSize: Math.round(10 * page.uiScale)
                            font.weight: Font.DemiBold
                            elide: Text.ElideRight
                        }

                        Item {
                            Layout.fillWidth: true
                            Layout.preferredHeight: Math.round(4 * page.uiScale)

                            Rectangle {
                                anchors.fill: parent
                                radius: 2
                                color: page.darkMode ? "#2D2644" : "#E2E8F0"
                            }

                            Rectangle {
                                anchors.left: parent.left
                                anchors.top: parent.top
                                anchors.bottom: parent.bottom
                                radius: 2
                                width: Math.min(parent.width, Math.max(0, parent.width * (page.ramMonitor.zramTotalMiB > 0 ? (page.ramMonitor.zramUsedMiB / page.ramMonitor.zramTotalMiB) : 0)))
                                color: page.darkMode ? "#F59E0B" : "#D97706"
                                Behavior on width { NumberAnimation { duration: 250; easing.type: Easing.OutCubic } }
                            }
                        }
                    }
                }
            }

            // GPU Performance & Power Telemetry Section (4 Tiles in 1 Row)
            Rectangle {
                Layout.fillWidth: true
                radius: 14
                color: page.cardColor
                border.width: 1
                border.color: page.borderColor
                implicitHeight: gpuPerfLayout.implicitHeight + 24
                visible: page.showAdvancedInfo && page.gpuTelemetryAvailable

                ColumnLayout {
                    id: gpuPerfLayout
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 10

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            Layout.fillWidth: true
                            text: qsTr("GPU Performance & Power")
                            color: page.textColor
                            font.pixelSize: Math.round(18 * page.uiScale)
                            font.weight: Font.DemiBold
                        }
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        columns: width > 920 ? 4 : (width > 560 ? 2 : 1)
                        columnSpacing: 8
                        rowSpacing: 8

                        Repeater {
                            model: [
                                {
                                    title: qsTr("Core / Memory Clocks"),
                                    value: (page.gpuMonitor && page.gpuMonitor.graphicsClockMHz > 0)
                                           ? (page.gpuMonitor.graphicsClockMHz + " MHz • " + page.gpuMonitor.memoryClockMHz + " MHz")
                                           : qsTr("Dynamic Clock")
                                },
                                {
                                    title: qsTr("Power Draw / TDP Limit"),
                                    value: (page.gpuMonitor && page.gpuMonitor.powerDrawW > 0)
                                           ? (page.gpuMonitor.powerDrawW.toFixed(1) + " W / " + page.gpuMonitor.powerLimitW.toFixed(0) + " W")
                                           : qsTr("Dynamic Power")
                                },
                                {
                                    title: qsTr("VRAM Allocation"),
                                    value: (page.gpuMonitor && page.gpuMonitor.memoryTotalMiB > 0)
                                           ? (page.gpuMonitor.memoryUsedMiB + " / " + page.gpuMonitor.memoryTotalMiB + " MiB (" + page.gpuMonitor.memoryUsagePercent + "%)")
                                           : qsTr("Unavailable")
                                },
                                {
                                    title: qsTr("Thermals & Hotspot"),
                                    value: (page.gpuMonitor && page.gpuMonitor.temperatureC > 0)
                                           ? (qsTr("Core: %1°C").arg(page.gpuMonitor.temperatureC) +
                                              (page.gpuMonitor.hotspotTemperatureC > 0 ? qsTr(" • Hotspot: %1°C").arg(page.gpuMonitor.hotspotTemperatureC) : "") +
                                              (page.gpuMonitor.memoryTemperatureC > 0 ? qsTr(" • VRAM: %1°C").arg(page.gpuMonitor.memoryTemperatureC) : ""))
                                           : (page.gpuMonitor && page.gpuMonitor.temperatureC > 0 ? page.formatTemp(page.gpuMonitor.temperatureC) : qsTr("Unavailable"))
                                }
                            ]

                            delegate: Rectangle {
                                required property var modelData
                                Layout.fillWidth: true
                                implicitHeight: Math.round(64 * page.uiScale)
                                radius: 10
                                color: page.bgColor
                                border.width: 1
                                border.color: page.borderColor

                                Column {
                                    anchors.fill: parent
                                    anchors.margins: Math.round(8 * page.uiScale)
                                    spacing: Math.round(3 * page.uiScale)

                                    Label {
                                        width: parent.width
                                        text: modelData.title
                                        color: page.softTextColor
                                        font.pixelSize: Math.round(11 * page.uiScale)
                                        font.weight: Font.DemiBold
                                        elide: Text.ElideRight
                                    }

                                    Label {
                                        width: parent.width
                                        text: modelData.value
                                        color: page.textColor
                                        font.pixelSize: Math.round(13 * page.uiScale)
                                        font.weight: Font.DemiBold
                                        elide: Text.ElideRight
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // GPU Power & TDP Management Section
            Rectangle {
                Layout.fillWidth: true
                radius: 14
                color: page.cardColor
                border.width: 1
                border.color: page.borderColor
                implicitHeight: powerArea.implicitHeight + 24
                visible: page.powerController !== null && (page.powerController.supported
                                                           || page.powerController.systemPowerProfileSupported)

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
                            text: page.powerController && page.powerController.supported
                                  ? qsTr("GPU Power & Performance Management")
                                  : qsTr("Power & Performance Management")
                            color: page.textColor
                            font.pixelSize: Math.round(18 * page.uiScale)
                            font.weight: Font.DemiBold
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10

                        Rectangle {
                            visible: page.powerController && page.powerController.supported
                            implicitHeight: Math.round(30 * page.uiScale)
                            implicitWidth: currentDrawRow.implicitWidth + Math.round(16 * page.uiScale)
                            radius: 6
                            color: page.bgColor
                            border.width: 1
                            border.color: page.borderColor

                            RowLayout {
                                id: currentDrawRow
                                anchors.centerIn: parent
                                spacing: 4

                                Label {
                                    text: qsTr("Draw:")
                                    color: page.softTextColor
                                    font.pixelSize: Math.round(11 * page.uiScale)
                                    font.weight: Font.Medium
                                }

                                Label {
                                    text: (page.powerController ? page.powerController.currentPowerDrawW.toFixed(1) : "0.0") + " W"
                                    color: page.textColor
                                    font.pixelSize: Math.round(12 * page.uiScale)
                                    font.weight: Font.Bold
                                }
                            }
                        }

                        Rectangle {
                            visible: page.powerController && page.powerController.controlSupported
                            implicitHeight: Math.round(30 * page.uiScale)
                            implicitWidth: powerLimitRow.implicitWidth + Math.round(16 * page.uiScale)
                            radius: 6
                            color: page.darkMode ? "#1E2548" : "#EEF2FF"
                            border.width: 1
                            border.color: page.darkMode ? "#4338CA" : "#C7D2FE"

                            RowLayout {
                                id: powerLimitRow
                                anchors.centerIn: parent
                                spacing: 4

                                Label {
                                    text: qsTr("Limit:")
                                    color: page.accentColor
                                    font.pixelSize: Math.round(11 * page.uiScale)
                                    font.weight: Font.Medium
                                }

                                Label {
                                    text: (page.powerController ? page.powerController.powerLimitW.toFixed(0) : "N/A") + " W"
                                    color: page.accentColor
                                    font.pixelSize: Math.round(12 * page.uiScale)
                                    font.weight: Font.Bold
                                }
                            }
                        }

                        Item { Layout.fillWidth: true }

                        RowLayout {
                            spacing: Math.round(8 * page.uiScale)
                            visible: page.powerController && (page.powerController.controlSupported
                                                               || page.powerController.systemPowerProfileSupported)

                            Repeater {
                                model: [
                                    { preset: "eco", label: qsTr("Eco") },
                                    { preset: "balanced", label: qsTr("Balanced") },
                                    { preset: "performance", label: qsTr("Performance") }
                                ]

                                delegate: Button {
                                    id: powerPresetBtn
                                    required property var modelData
                                    implicitHeight: Math.round(34 * page.uiScale)
                                    implicitWidth: Math.round(96 * page.uiScale)

                                    background: Rectangle {
                                        radius: 8
                                        color: (page.powerController && page.powerController.activePreset === powerPresetBtn.modelData.preset)
                                               ? page.accentColor
                                               : page.bgColor
                                        border.width: (page.powerController && page.powerController.activePreset === powerPresetBtn.modelData.preset) ? 2 : 1
                                        border.color: (page.powerController && page.powerController.activePreset === powerPresetBtn.modelData.preset)
                                                      ? page.accentColor
                                                      : page.borderColor
                                    }

                                    contentItem: Text {
                                        text: powerPresetBtn.modelData.label
                                        color: (page.powerController && page.powerController.activePreset === powerPresetBtn.modelData.preset)
                                               ? "#FFFFFF"
                                               : page.textColor
                                        font.pixelSize: Math.round(12 * page.uiScale)
                                        font.weight: (page.powerController && page.powerController.activePreset === powerPresetBtn.modelData.preset) ? Font.Bold : Font.DemiBold
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }

                                    onClicked: {
                                        if (page.powerController) {
                                            if (!page.powerController.applyPowerPreset(powerPresetBtn.modelData.preset))
                                                page.powerController.refresh();
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // GPU Task Manager Section
            Rectangle {
                Layout.fillWidth: true
                radius: 14
                color: page.cardColor
                border.width: 1
                border.color: page.borderColor
                implicitHeight: taskManagerLayout.implicitHeight + 24
                visible: page.showAdvancedInfo && page.gpuTelemetryAvailable

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
                            color: page.textColor
                            font.pixelSize: Math.round(18 * page.uiScale)
                            font.weight: Font.DemiBold
                        }
                        Button {
                            id: processExpansionButton
                            visible: page.gpuMonitor && page.gpuMonitor.gpuProcessCount > 4
                            implicitWidth: processExpansionContent.implicitWidth + Math.round(28 * page.uiScale)
                            implicitHeight: Math.round(36 * page.uiScale)
                            hoverEnabled: true
                            scale: down ? 0.98 : 1.0
                            Behavior on scale { NumberAnimation { duration: 90; easing.type: Easing.OutQuad } }
                            background: Rectangle {
                                radius: Math.round(9 * page.uiScale)
                                color: processExpansionButton.down
                                       ? Qt.darker(page.accentColor, 1.16)
                                       : (processExpansionButton.hovered ? "#8B8FFF" : page.accentColor)
                                border.width: 1
                                border.color: processExpansionButton.hovered ? "#B8BAFF" : page.accentColor

                            }
                            contentItem: RowLayout {
                                id: processExpansionContent
                                spacing: Math.round(8 * page.uiScale)
                                Text {
                                    text: page.showAllGpuProcesses ? qsTr("Show less") : qsTr("All processes")
                                    color: "#FFFFFF"
                                    font.pixelSize: Math.round(12 * page.uiScale)
                                    font.weight: Font.DemiBold
                                }
                                Rectangle {
                                    implicitWidth: 1
                                    implicitHeight: Math.round(16 * page.uiScale)
                                    color: "#55FFFFFF"
                                }
                                Text {
                                    text: page.showAllGpuProcesses ? "−" : (page.gpuMonitor ? page.gpuMonitor.gpuProcessCount : 0)
                                    color: "#FFFFFF"
                                    font.pixelSize: Math.round(11 * page.uiScale)
                                    font.weight: Font.Bold
                                }
                                Text {
                                    text: page.showAllGpuProcesses ? "⌃" : "⌄"
                                    color: "#DDFFFFFF"
                                    font.pixelSize: Math.round(13 * page.uiScale)
                                }
                            }
                            onClicked: page.showAllGpuProcesses = !page.showAllGpuProcesses
                        }
                    }

                    // Empty state when no GPU processes are running
                    Rectangle {
                        Layout.fillWidth: true
                        implicitHeight: Math.round(72 * page.uiScale)
                        radius: 10
                        color: page.bgColor
                        border.width: 1
                        border.color: page.borderColor
                        visible: !page.gpuMonitor || page.gpuMonitor.gpuProcessCount === 0

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 14

                            Rectangle {
                                implicitWidth: Math.round(36 * page.uiScale)
                                implicitHeight: Math.round(36 * page.uiScale)
                                Layout.preferredWidth: Math.round(36 * page.uiScale)
                                Layout.preferredHeight: Math.round(36 * page.uiScale)
                                radius: 8
                                color: page.darkMode ? "#1E293B" : "#E2E8F0"

                                Label {
                                    anchors.centerIn: parent
                                    text: "✓"
                                    color: "#22C55E"
                                    font.pixelSize: Math.round(18 * page.uiScale)
                                    font.weight: Font.Bold
                                }
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2

                                Label {
                                    text: qsTr("No Active GPU Processes")
                                    color: page.textColor
                                    font.pixelSize: Math.round(13 * page.uiScale)
                                    font.weight: Font.DemiBold
                                }

                                Label {
                                    text: qsTr("No applications are currently allocating VRAM or compute resources on this GPU.")
                                    color: page.softTextColor
                                    font.pixelSize: Math.round(11 * page.uiScale)
                                    elide: Text.ElideRight
                                }
                            }
                        }
                    }

                    // Process list table when processes exist
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 6
                        visible: page.gpuMonitor && page.gpuMonitor.gpuProcessCount > 0

                        // Table header
                        RowLayout {
                            Layout.fillWidth: true
                            Layout.leftMargin: 12
                            Layout.rightMargin: 12
                            spacing: 12

                            Label {
                                Layout.preferredWidth: Math.round(60 * page.uiScale)
                                text: qsTr("PID")
                                color: page.softTextColor
                                font.pixelSize: Math.round(11 * page.uiScale)
                                font.weight: Font.Bold
                            }

                            Label {
                                Layout.fillWidth: true
                                text: qsTr("PROCESS NAME")
                                color: page.softTextColor
                                font.pixelSize: Math.round(11 * page.uiScale)
                                font.weight: Font.Bold
                            }

                            Label {
                                Layout.preferredWidth: Math.round(110 * page.uiScale)
                                text: qsTr("TYPE")
                                color: page.softTextColor
                                font.pixelSize: Math.round(11 * page.uiScale)
                                font.weight: Font.Bold
                            }

                            Label {
                                Layout.preferredWidth: Math.round(140 * page.uiScale)
                                text: qsTr("VRAM ALLOCATION")
                                color: page.softTextColor
                                font.pixelSize: Math.round(11 * page.uiScale)
                                font.weight: Font.Bold
                                horizontalAlignment: Text.AlignHCenter
                            }

                            Label {
                                Layout.preferredWidth: Math.round(82 * page.uiScale)
                                text: qsTr("ACTION")
                                color: page.softTextColor
                                font.pixelSize: Math.round(11 * page.uiScale)
                                font.weight: Font.Bold
                                horizontalAlignment: Text.AlignHCenter
                            }
                        }

                        ScrollView {
                            Layout.fillWidth: true
                            implicitHeight: Math.min(Math.round(280 * page.uiScale), processRowsColumn.implicitHeight)
                            clip: true
                            ScrollBar.vertical.policy: processRowsColumn.implicitHeight > (280 * page.uiScale) ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded

                            ColumnLayout {
                                id: processRowsColumn
                                width: parent.width
                                spacing: 6

                                Repeater {
                                    model: {
                                        var all = page.gpuMonitor ? page.gpuMonitor.gpuProcesses : [];
                                        return page.showAllGpuProcesses ? all : all.slice(0, 4);
                                    }

                                    delegate: Rectangle {
                                        required property var modelData
                                        Layout.fillWidth: true
                                        implicitHeight: Math.round(48 * page.uiScale)
                                        radius: 8
                                        color: page.bgColor
                                        border.width: 1
                                        border.color: page.borderColor

                                        RowLayout {
                                            anchors.fill: parent
                                            anchors.leftMargin: 12
                                            anchors.rightMargin: 12
                                            spacing: 12

                                            Rectangle {
                                                Layout.preferredWidth: Math.round(60 * page.uiScale)
                                                Layout.preferredHeight: Math.round(24 * page.uiScale)
                                                radius: 4
                                                color: page.darkMode ? "#1E293B" : "#E2E8F0"

                                                Label {
                                                    anchors.centerIn: parent
                                                    text: modelData.pid
                                                    color: page.textColor
                                                    font.pixelSize: Math.round(11 * page.uiScale)
                                                    font.weight: Font.DemiBold
                                                    font.family: (Qt.platform.os === "osx") ? "Menlo" : "Monospace"
                                                }
                                            }

                                            Label {
                                                Layout.fillWidth: true
                                                text: modelData.name
                                                color: page.textColor
                                                font.pixelSize: Math.round(13 * page.uiScale)
                                                font.weight: Font.DemiBold
                                                elide: Text.ElideRight
                                            }

                                            Rectangle {
                                                Layout.preferredWidth: Math.round(110 * page.uiScale)
                                                Layout.preferredHeight: Math.round(22 * page.uiScale)
                                                radius: 4
                                                color: (modelData.type && modelData.type.indexOf("Compute") !== -1)
                                                       ? (page.darkMode ? "#312E81" : "#EEF2FF")
                                                       : (page.darkMode ? "#064E3B" : "#ECFDF5")

                                                Label {
                                                    anchors.centerIn: parent
                                                    text: modelData.type || qsTr("Compute")
                                                    color: (modelData.type && modelData.type.indexOf("Compute") !== -1)
                                                           ? (page.darkMode ? "#A5B4FC" : "#4F46E5")
                                                           : (page.darkMode ? "#6EE7B7" : "#059669")
                                                    font.pixelSize: Math.round(10 * page.uiScale)
                                                    font.weight: Font.DemiBold
                                                }
                                            }

                                            Rectangle {
                                                Layout.preferredWidth: Math.round(140 * page.uiScale)
                                                Layout.preferredHeight: Math.round(26 * page.uiScale)
                                                radius: 6
                                                color: page.darkMode ? "#1E2238" : "#EEF2F6"
                                                border.width: 1
                                                border.color: page.darkMode ? "#342D4A" : "#E2E8F0"
                                                clip: true

                                                Rectangle {
                                                    anchors.left: parent.left
                                                    anchors.top: parent.top
                                                    anchors.bottom: parent.bottom
                                                    width: {
                                                        var total = page.gpuMonitor ? page.gpuMonitor.memoryTotalMiB : 0;
                                                        var pct = total > 0 ? Math.min(1.0, Math.max(0.04, modelData.vramMiB / total)) : 0;
                                                        return parent.width * pct;
                                                    }
                                                    radius: 5
                                                    color: page.darkMode ? "#4338CA" : "#C7D2FE"
                                                    opacity: 0.7
                                                }

                                                Label {
                                                    anchors.centerIn: parent
                                                    text: {
                                                        var total = page.gpuMonitor ? page.gpuMonitor.memoryTotalMiB : 0;
                                                        if (total <= 0)
                                                            return modelData.vramMiB + " MiB";
                                                        var pct = ((modelData.vramMiB / total) * 100).toFixed(modelData.vramMiB > 1000 ? 0 : 1);
                                                        return modelData.vramMiB + " MiB (" + pct + "%)";
                                                    }
                                                    color: page.textColor
                                                    font.pixelSize: Math.round(11 * page.uiScale)
                                                    font.weight: Font.Bold
                                                }
                                            }

                                            Button {
                                                id: endProcessBtn
                                                text: qsTr("End Task")
                                                implicitWidth: Math.round(82 * page.uiScale)
                                                implicitHeight: Math.round(28 * page.uiScale)

                                                background: Rectangle {
                                                    radius: 6
                                                    color: endProcessBtn.down ? (page.darkMode ? "#7F1D1D" : "#FEE2E2")
                                                                              : (endProcessBtn.hovered ? (page.darkMode ? "#450A0A" : "#FEF2F2")
                                                                                                       : (page.darkMode ? "#29233B" : "#F8FAFC"))
                                                    border.width: 1
                                                    border.color: endProcessBtn.hovered ? (page.darkMode ? "#EF4444" : "#DC2626")
                                                                                        : page.borderColor
                                                }

                                                contentItem: Text {
                                                    text: endProcessBtn.text
                                                    color: endProcessBtn.hovered ? (page.darkMode ? "#F87171" : "#DC2626")
                                                                                 : page.textColor
                                                    font.pixelSize: Math.round(11 * page.uiScale)
                                                    font.weight: Font.DemiBold
                                                    horizontalAlignment: Text.AlignHCenter
                                                    verticalAlignment: Text.AlignVCenter
                                                }

                                                onClicked: {
                                                    page.pendingTerminationPid = modelData.pid;
                                                    page.pendingTerminationName = modelData.name;
                                                    terminateProcessPopup.open();
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
        }
    }

    Popup {
        id: terminateProcessPopup
        modal: true
        focus: true
        anchors.centerIn: parent
        width: Math.min(parent ? parent.width - 40 : 420, Math.round(420 * page.uiScale))
        padding: Math.round(18 * page.uiScale)
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            radius: 12
            color: page.cardColor
            border.width: 1
            border.color: page.borderColor
        }

        contentItem: ColumnLayout {
            spacing: 12
            Label {
                Layout.fillWidth: true
                text: qsTr("End GPU process?")
                color: page.textColor
                font.pixelSize: Math.round(17 * page.uiScale)
                font.weight: Font.DemiBold
            }
            Label {
                Layout.fillWidth: true
                text: qsTr("%1 (PID %2) will be terminated. Unsaved work may be lost.")
                      .arg(page.pendingTerminationName).arg(page.pendingTerminationPid)
                color: page.softTextColor
                wrapMode: Text.WordWrap
                font.pixelSize: Math.round(12 * page.uiScale)
            }
            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
                Button {
                    text: qsTr("Cancel")
                    onClicked: terminateProcessPopup.close()
                }
                Button {
                    text: qsTr("End process")
                    enabled: page.pendingTerminationPid > 0
                    onClicked: {
                        if (page.gpuMonitor)
                            page.gpuMonitor.killProcess(page.pendingTerminationPid);
                        terminateProcessPopup.close();
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        var initArr = [];
        for (var i = 0; i < 30; ++i) {
            initArr.push(0);
        }
        page.cpuUsageHistory = initArr.slice();
        page.gpuLoadHistory = initArr.slice();
        page.ramUsageHistory = initArr.slice();
        page.pushTelemetryHistory();

        if (page.systemInfo)
            page.systemInfo.refresh();
        if (page.fanController) {
            page.fanController.start();
            page.fanController.refresh();
        }
    }

    Timer {
        id: historySampler
        interval: 1000
        repeat: true
        running: true
        onTriggered: page.pushTelemetryHistory()
    }

    Timer {
        id: telemetryRefreshPulse
        interval: 300
        repeat: false
        onTriggered: page.telemetryRefreshAnimating = false
    }

    Timer {
        id: telemetryRefreshQueue
        interval: 180
        repeat: false
        onTriggered: page.refreshTelemetryStep()
    }
}
