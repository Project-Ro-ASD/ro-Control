pragma ComponentBehavior: Bound

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

    property var theme: ({})
    property bool darkMode: false
    property bool showAdvancedInfo: true
    property real uiScale: 1.0
    property var cpuUsageHistory: []
    property var gpuLoadHistory: []
    property var ramUsageHistory: []
    property int pendingTerminationPid: -1
    property string pendingTerminationName: ""
    property bool showAllGpuProcesses: false
    property string terminationError: ""

    readonly property color bgColor: theme && theme.card ? theme.card : (page.darkMode ? "#29233B" : "#FFFFFF")
    readonly property color cardColor: theme && theme.cardStrong ? theme.cardStrong : (page.darkMode ? "#342D4A" : "#F1F5F9")
    readonly property color borderColor: theme && theme.border ? theme.border : (page.darkMode ? "#4D436B" : "#CBD5E1")
    readonly property color textColor: theme && theme.text ? theme.text : (page.darkMode ? "#F8FAFC" : "#0F172A")
    readonly property color softTextColor: theme && theme.textSoft ? theme.textSoft : (page.darkMode ? "#94A3B8" : "#64748B")
    readonly property color infoBg: theme && theme.infoBg ? theme.infoBg : (page.darkMode ? "#1E2548" : "#EFF6FF")
    readonly property color accentColor: theme && theme.accentA ? theme.accentA : (page.darkMode ? "#818CF8" : "#4F46E5")
    readonly property bool nvidiaGpuDetected: page.nvidiaDetector && page.nvidiaDetector.gpuFound
    readonly property bool gpuTelemetryAvailable: page.nvidiaGpuDetected && page.gpuMonitor && page.gpuMonitor.available
    readonly property bool cpuTelemetryAvailable: page.cpuMonitor && page.cpuMonitor.available
    readonly property bool ramTelemetryAvailable: page.ramMonitor && page.ramMonitor.available

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
        // Canvas contents are dropped while the tab is hidden; repaint the
        // retained history as soon as the page becomes visible again.
        onVisibleChanged: if (visible) Qt.callLater(requestPaint)

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

    // Values for the four GPU performance tiles. Kept as a function (not as
    // part of the Repeater model) so a telemetry change updates only the
    // labels instead of recreating all four delegates every tick.
    function gpuPerfValue(index) {
        const gpu = page.gpuMonitor;
        switch (index) {
        case 0:
            return (gpu && gpu.graphicsClockMHz > 0)
                   ? (gpu.graphicsClockMHz + " MHz • " + gpu.memoryClockMHz + " MHz")
                   : qsTr("Dynamic Clock");
        case 1:
            return (gpu && gpu.powerDrawW > 0)
                   ? (gpu.powerDrawW.toFixed(1) + " W / " + gpu.powerLimitW.toFixed(0) + " W")
                   : qsTr("Dynamic Power");
        case 2:
            return (gpu && gpu.memoryTotalMiB > 0)
                   ? (gpu.memoryUsedMiB + " / " + gpu.memoryTotalMiB + " MiB (" + gpu.memoryUsagePercent + "%)")
                   : qsTr("Unavailable");
        case 3:
            if (gpu && gpu.temperatureC > 0) {
                return qsTr("Core: %1°C").arg(gpu.temperatureC)
                       + (gpu.hotspotTemperatureC > 0 ? qsTr(" • Hotspot: %1°C").arg(gpu.hotspotTemperatureC) : "")
                       + (gpu.memoryTemperatureC > 0 ? qsTr(" • VRAM: %1°C").arg(gpu.memoryTemperatureC) : "");
            }
            return qsTr("Unavailable");
        default:
            return "";
        }
    }

    function pushTelemetryHistory() {
        if (!page.visible)
            return;
        var cpuVal = page.cpuTelemetryAvailable ? page.cpuMonitor.usagePercent : null;
        var gpuVal = page.gpuTelemetryAvailable ? page.gpuMonitor.utilizationPercent : null;
        var ramVal = page.ramTelemetryAvailable ? page.ramMonitor.usagePercent : null;

        var cpuArr = page.cpuUsageHistory.slice();
        if (cpuVal !== null) cpuArr.push(cpuVal);
        if (cpuArr.length > 30) cpuArr.shift();
        page.cpuUsageHistory = cpuArr;

        var gpuArr = page.gpuLoadHistory.slice();
        if (gpuVal !== null) gpuArr.push(gpuVal);
        if (gpuArr.length > 30) gpuArr.shift();
        page.gpuLoadHistory = gpuArr;

        var ramArr = page.ramUsageHistory.slice();
        if (ramVal !== null) ramArr.push(ramVal);
        if (ramArr.length > 30) ramArr.shift();
        page.ramUsageHistory = ramArr;
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
                id: telemetryGrid
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
                                    text: page.cpuTelemetryAvailable ? page.cpuMonitor.usagePercent.toFixed(1) + "%" : qsTr("Unavailable")
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
                                    text: page.cpuTelemetryAvailable ? page.formatTemp(page.cpuMonitor.temperatureC) : qsTr("Unavailable")
                                    color: (page.cpuTelemetryAvailable && page.cpuMonitor.temperatureC > 80) ? (page.theme && page.theme.warning ? page.theme.warning : "#EF4444") : page.textColor
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
                                visible: page.gpuTelemetryAvailable && page.gpuMonitor && page.gpuMonitor.gpuCount > 1
                                implicitHeight: Math.round(24 * page.uiScale)
                                implicitWidth: gpuSelectorRow.implicitWidth + Math.round(14 * page.uiScale)
                                radius: 6
                                color: gpuSelectorMouse.containsMouse
                                       ? (page.darkMode ? "#342D4A" : "#E2E8F0")
                                       : (page.darkMode ? "#1E2548" : "#EEF2FF")
                                border.width: 1
                                border.color: gpuSelectorMouse.containsMouse
                                              ? page.accentColor
                                              : (page.darkMode ? "#4338CA" : "#C7D2FE")

                                ToolTip {
                                    id: gpuTooltip
                                    visible: Boolean(gpuSelectorMouse.containsMouse && !gpuMenu.visible)
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
                                    focus: true
                                    activeFocusOnTab: true
                                    cursorShape: (page.gpuMonitor && page.gpuMonitor.gpuCount > 1) ? Qt.PointingHandCursor : Qt.ArrowCursor
                                    onClicked: {
                                        if (page.gpuMonitor && page.gpuMonitor.gpuCount > 1) {
                                            gpuMenu.open();
                                        }
                                    }
                                    Keys.onReturnPressed: {
                                        if (page.gpuMonitor && page.gpuMonitor.gpuCount > 1)
                                            gpuMenu.open();
                                    }
                                    Keys.onSpacePressed: {
                                        if (page.gpuMonitor && page.gpuMonitor.gpuCount > 1)
                                            gpuMenu.open();
                                    }
                                }

                                RowLayout {
                                    id: gpuSelectorRow
                                    anchors.centerIn: parent
                                    spacing: 4

                                    Label {
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
                                    text: page.ramTelemetryAvailable ? page.ramMonitor.usagePercent + "%" : qsTr("Unavailable")
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
                                    text: page.ramTelemetryAvailable ? page.formatRam(page.ramMonitor.usedMiB, page.ramMonitor.totalMiB) : qsTr("Unavailable")
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
                                    text: page.ramMonitor ? (page.ramMonitor.zramUsedMiB + " / " + page.ramMonitor.zramTotalMiB + " MiB") : qsTr("Unavailable")
                                    color: page.textColor
                                    font.pixelSize: Math.round(18 * page.uiScale)
                                    font.weight: Font.Bold
                                }
                            }
                        }

                        Label {
                            Layout.fillWidth: true
                            visible: page.ramMonitor && (page.ramMonitor.zramCompressionRatio > 0 || page.ramMonitor.zswapEnabled || page.ramMonitor.zramPhysicalMiB > 0)
                            text: {
                                if (!page.ramMonitor)
                                    return "";
                                return (page.ramMonitor.zramCompressionRatio > 0
                                        ? qsTr("Compression: %1×").arg(page.ramMonitor.zramCompressionRatio.toFixed(1))
                                        : "")
                                      + (page.ramMonitor.zramPhysicalMiB > 0
                                         ? (page.ramMonitor.zramCompressionRatio > 0 ? " • " : "") + qsTr("RAM: %1 MiB").arg(page.ramMonitor.zramPhysicalMiB)
                                         : "")
                                      + (page.ramMonitor.zswapEnabled
                                         ? (page.ramMonitor.zramCompressionRatio > 0 || page.ramMonitor.zramPhysicalMiB > 0 ? " • " : "") + qsTr("zswap enabled")
                                         : "")
                            }
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
                                width: Math.min(parent.width, Math.max(0, parent.width * ((page.ramMonitor && page.ramMonitor.zramTotalMiB > 0) ? (page.ramMonitor.zramUsedMiB / page.ramMonitor.zramTotalMiB) : 0)))
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
                            // Static titles only: the reactive value bindings
                            // live in the delegate so a telemetry change does
                            // not rebuild every tile.
                            model: [
                                qsTr("Core / Memory Clocks"),
                                qsTr("Power Draw / TDP Limit"),
                                qsTr("VRAM Allocation"),
                                qsTr("Thermals & Hotspot")
                            ]

                            delegate: Rectangle {
                                id: perfTile
                                required property var modelData
                                required property int index
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
                                        text: perfTile.modelData
                                        color: page.softTextColor
                                        font.pixelSize: Math.round(11 * page.uiScale)
                                        font.weight: Font.DemiBold
                                        elide: Text.ElideRight
                                    }

                                    Label {
                                        width: parent.width
                                        text: page.gpuPerfValue(perfTile.index)
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
                            Layout.maximumWidth: page.width <= 560 ? 0 : implicitWidth
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
                            Layout.maximumWidth: page.width <= 560 ? 0 : implicitWidth
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
                                    implicitWidth: page.width <= 560 ? Math.round(84 * page.uiScale) : Math.round(96 * page.uiScale)

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
                                visible: page.width > 560
                                Layout.preferredWidth: Math.round(110 * page.uiScale)
                                text: qsTr("TYPE")
                                color: page.softTextColor
                                font.pixelSize: Math.round(11 * page.uiScale)
                                font.weight: Font.Bold
                            }

                            Label {
                                visible: page.width > 560
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
                                        id: processRow
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
                                                    text: processRow.modelData.pid
                                                    color: page.textColor
                                                    font.pixelSize: Math.round(11 * page.uiScale)
                                                    font.weight: Font.DemiBold
                                                    font.family: (Qt.platform.os === "osx") ? "Menlo" : "Monospace"
                                                }
                                            }

                                            Label {
                                                Layout.fillWidth: true
                                                text: processRow.modelData.name
                                                color: page.textColor
                                                font.pixelSize: Math.round(13 * page.uiScale)
                                                font.weight: Font.DemiBold
                                                elide: Text.ElideRight
                                            }

                                            Rectangle {
                                                visible: page.width > 560
                                                Layout.preferredWidth: Math.round(110 * page.uiScale)
                                                Layout.preferredHeight: Math.round(22 * page.uiScale)
                                                radius: 4
                                                color: (processRow.modelData.type && processRow.modelData.type.indexOf("Compute") !== -1)
                                                       ? (page.darkMode ? "#312E81" : "#EEF2FF")
                                                       : (page.darkMode ? "#064E3B" : "#ECFDF5")

                                                Label {
                                                    anchors.centerIn: parent
                                                    text: processRow.modelData.type || qsTr("Compute")
                                                    color: (processRow.modelData.type && processRow.modelData.type.indexOf("Compute") !== -1)
                                                           ? (page.darkMode ? "#A5B4FC" : "#4F46E5")
                                                           : (page.darkMode ? "#6EE7B7" : "#059669")
                                                    font.pixelSize: Math.round(10 * page.uiScale)
                                                    font.weight: Font.DemiBold
                                                }
                                            }

                                            Rectangle {
                                                visible: page.width > 560
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
                                                        var pct = total > 0 ? Math.min(1.0, Math.max(0, processRow.modelData.vramMiB / total)) : 0;
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
                                                            return processRow.modelData.vramMiB + " MiB";
                                                        var pct = ((processRow.modelData.vramMiB / total) * 100).toFixed(processRow.modelData.vramMiB > 1000 ? 0 : 1);
                                                        return processRow.modelData.vramMiB + " MiB (" + pct + "%)";
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
                                                    page.pendingTerminationPid = processRow.modelData.pid;
                                                    page.pendingTerminationName = processRow.modelData.name;
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
            Label {
                visible: page.terminationError.length > 0
                Layout.fillWidth: true
                text: page.terminationError
                color: page.theme && page.theme.danger ? page.theme.danger : "#DC2626"
                wrapMode: Text.WordWrap
                font.pixelSize: Math.round(11 * page.uiScale)
            }
            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
                Components.ActionButton {
                    text: qsTr("Cancel")
                    theme: page.theme
                    compact: true
                    uiScale: page.uiScale
                    onClicked: terminateProcessPopup.close()
                }
                Components.ActionButton {
                    text: qsTr("End process")
                    enabled: page.pendingTerminationPid > 0
                    theme: page.theme
                    tone: "danger"
                    compact: true
                    uiScale: page.uiScale
                    onClicked: {
                        page.terminationError = "";
                        if (!page.gpuMonitor) {
                            page.terminationError = qsTr("GPU monitor is unavailable.");
                        } else if (page.gpuMonitor.killProcess(page.pendingTerminationPid)) {
                            terminateProcessPopup.close();
                        } else {
                            page.terminationError = page.gpuMonitor.statusMessage;
                        }
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        page.cpuUsageHistory = [];
        page.gpuLoadHistory = [];
        page.ramUsageHistory = [];
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
        running: page.visible
        onTriggered: page.pushTelemetryHistory()
    }

    onVisibleChanged: {
        if (page.visible)
            page.pushTelemetryHistory();
    }

}
