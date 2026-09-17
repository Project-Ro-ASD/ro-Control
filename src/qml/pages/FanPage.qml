import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components" as Components

Item {
    id: page
    required property var systemInfo
    required property var cpuMonitor
    required property var gpuMonitor
    required property var fanController

    property var theme: ({})
    property bool darkMode: false
    property bool showAdvancedInfo: true
    property real uiScale: 1.0
    property bool refreshAnimating: false
    property var fanSpeedHistory: []
    property var fanRpmHistory: []
    property var perFanHistories: ({})
    property var orderedFans: []
    // Kept false while the fixed thermal-priority ordering is intentionally
    // not user-configurable.
    readonly property bool reorderMode: false
    property int draggingFanIndex: -1
    readonly property bool hasDetectedFans: orderedFans.length > 0
    readonly property bool hasControllableFan: {
        for (var i = 0; i < orderedFans.length; ++i) {
            if (orderedFans[i].controllable)
                return true;
        }
        return false;
    }
    readonly property bool supportsBatteryFanSync: hasControllableFan
                                                   && (systemInfo.onBattery || systemInfo.deviceType === "Laptop")

    readonly property color bgColor: theme && theme.card ? theme.card : (page.darkMode ? "#29233B" : "#FFFFFF")
    readonly property color cardColor: theme && theme.cardStrong ? theme.cardStrong : (page.darkMode ? "#342D4A" : "#F1F5F9")
    readonly property color borderColor: theme && theme.border ? theme.border : (page.darkMode ? "#4D436B" : "#CBD5E1")
    readonly property color textColor: theme && theme.text ? theme.text : (page.darkMode ? "#F8FAFC" : "#0F172A")
    readonly property color softTextColor: theme && theme.textSoft ? theme.textSoft : (page.darkMode ? "#94A3B8" : "#64748B")
    readonly property color accentColor: theme && theme.accentA ? theme.accentA : (page.darkMode ? "#818CF8" : "#4F46E5")
    readonly property color accentButtonText: "#FFFFFF"
    readonly property color infoBg: theme && theme.infoBg ? theme.infoBg : (page.darkMode ? "#1E2548" : "#EFF6FF")
    readonly property color warningBg: theme && theme.warningBg ? theme.warningBg : (page.darkMode ? "#3A2E12" : "#FFFBEB")
    readonly property color warningText: theme && theme.warning ? theme.warning : (page.darkMode ? "#FBBF24" : "#D97706")
    readonly property color successBg: theme && theme.successBg ? theme.successBg : (page.darkMode ? "#143828" : "#ECFDF5")
    readonly property color successText: theme && theme.success ? theme.success : (page.darkMode ? "#4ADE80" : "#059669")

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

    function syncOrderedFans() {
        if (!page.fanController)
            return;
        var raw = page.fanController.systemFans || [];
        if (raw.length === 0) {
            page.orderedFans = [];
            return;
        }
        var result = raw.slice();
        result.sort(function(a, b) {
            function priority(fan) {
                if (fan.type === "CPU") return 0;
                if (fan.type === "GPU") return 1;
                return 2;
            }
            return priority(a) - priority(b);
        });
        page.orderedFans = result;
    }

    function moveFan(fromIndex, toIndex) {
        // Ordering represents thermal priority and is deliberately not mutable.
        page.syncOrderedFans();
    }

    onVisibleChanged: {
        if (visible) {
            syncOrderedFans();
            pushFanHistory();
        }
    }

    function pushFanHistory() {
        if (!page.visible)
            return;
        var spd = page.fanController ? page.fanController.currentFanSpeedPercent : 0;
        var rpm = page.fanController ? page.fanController.currentRpm : 0;

        var spdArr = page.fanSpeedHistory.slice();
        spdArr.push(spd);
        if (spdArr.length > 30) spdArr.shift();
        page.fanSpeedHistory = spdArr;

        var rpmArr = page.fanRpmHistory.slice();
        rpmArr.push(rpm);
        if (rpmArr.length > 30) rpmArr.shift();
        page.fanRpmHistory = rpmArr;

        if (page.fanController && page.fanController.systemFans) {
            var map = Object.assign({}, page.perFanHistories);
            var fans = page.fanController.systemFans;
            for (var i = 0; i < fans.length; ++i) {
                var fId = fans[i].id;
                var fSpd = fans[i].speedPercent !== undefined ? fans[i].speedPercent : spd;
                var fArr = map[fId] ? map[fId].slice() : [];
                fArr.push(fSpd);
                if (fArr.length > 30) fArr.shift();
                map[fId] = fArr;
            }
            page.perFanHistories = map;
        }
    }

    Connections {
        target: page.fanController
        enabled: page.visible
        function onSystemFansChanged() {
            if (page.visible) {
                page.syncOrderedFans();
                page.pushFanHistory();
            }
        }
        function onCurrentFanSpeedPercentChanged() {
            if (page.visible) page.pushFanHistory();
        }
        function onCurrentRpmChanged() {
            if (page.visible) page.pushFanHistory();
        }
    }

    function modeTitle(mode) {
        switch (mode) {
        case "silent": return qsTr("Silent");
        case "balanced": return qsTr("Balanced");
        case "performance": return qsTr("Performance");
        case "manual": return qsTr("Manual");
        case "custom": return qsTr("Custom");
        case "auto":
        default: return qsTr("Auto");
        }
    }

    function modeDescription(mode) {
        switch (mode) {
        case "silent":
            return qsTr("Acoustic priority profile. Keeps fans quiet and delays ramp-up for quiet operation.");
        case "balanced":
            return qsTr("Optimized profile dynamically balancing thermal dissipation and acoustic comfort.");
        case "performance":
            return qsTr("Aggressive cooling profile providing maximum sustained airflow for heavy workloads.");
        case "manual":
            return qsTr("Fixed fan speed percentage defined directly by the user slider.");
        case "custom":
            return qsTr("Custom temperature-to-speed curve with hysteresis and response smoothing.");
        case "auto":
        default:
            return qsTr("Default automatic profile managed natively by hardware VBIOS and kernel drivers.");
        }
    }

    function refreshAll() {
        if (page.refreshAnimating)
            return;
        page.refreshAnimating = true;
        if (page.fanController) {
            page.fanController.start();
            page.fanController.refresh();
        }
        if (page.gpuMonitor)
            page.gpuMonitor.refresh();
        if (page.cpuMonitor)
            page.cpuMonitor.refresh();
        refreshPulse.restart();
    }

    Timer {
        id: refreshPulse
        interval: 400
        repeat: false
        onTriggered: page.refreshAnimating = false
    }

    ScrollView {
        id: pageScroll
        anchors.fill: parent
        clip: true
        contentWidth: availableWidth

        ColumnLayout {
            width: pageScroll.availableWidth
            spacing: Math.round(14 * page.uiScale)

            // Section 1: Detected System Fans Grid
            Rectangle {
                Layout.fillWidth: true
                radius: 14
                color: page.cardColor
                border.width: 1
                border.color: page.borderColor
                implicitHeight: fansSectionLayout.implicitHeight + Math.round(24 * page.uiScale)

                ColumnLayout {
                    id: fansSectionLayout
                    anchors.fill: parent
                    anchors.margins: Math.round(14 * page.uiScale)
                    spacing: Math.round(12 * page.uiScale)

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            Layout.fillWidth: true
                            text: qsTr("Cooling Channels (%1)").arg(page.fanController ? page.fanController.systemFanCount : 0)
                            color: page.textColor
                            font.pixelSize: Math.round(15 * page.uiScale)
                            font.weight: Font.DemiBold
                        }

                        Button {
                            id: reorderDoneBtn
                            visible: false
                            text: qsTr("Done ✓")
                            implicitHeight: Math.round(34 * page.uiScale)
                            hoverEnabled: true

                            background: Rectangle {
                                radius: 8
                                color: page.accentColor
                            }

                            contentItem: Label {
                                text: reorderDoneBtn.text
                                color: page.accentButtonText
                                font.pixelSize: Math.round(12 * page.uiScale)
                                font.weight: Font.Bold
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            onClicked: {
                                page.reorderMode = false;
                                page.draggingFanIndex = -1;
                            }
                        }

                        Button {
                            id: rescanBtn
                            visible: true
                            text: qsTr("Fan Setup Wizard")
                            implicitHeight: Math.round(34 * page.uiScale)
                            leftPadding: Math.round(14 * page.uiScale)
                            rightPadding: Math.round(14 * page.uiScale)
                            hoverEnabled: true

                            background: Rectangle {
                                radius: 8
                                color: rescanBtn.down ? (page.darkMode ? "#3B3156" : "#E2E8F0")
                                                      : (rescanBtn.hovered ? (page.darkMode ? "#342D4A" : "#F1F5F9")
                                                                           : (page.darkMode ? "#29233B" : "#FFFFFF"))
                                border.width: 1
                                border.color: rescanBtn.hovered ? page.accentColor : page.borderColor
                            }

                            contentItem: Label {
                                text: rescanBtn.text
                                color: rescanBtn.hovered ? page.accentColor : page.textColor
                                font.pixelSize: Math.round(13 * page.uiScale)
                                font.weight: Font.DemiBold
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            onClicked: {
                                fanRescanPopup.openWizard();
                            }
                        }

                        Components.RefreshToolButton {
                            visible: true
                            busy: page.refreshAnimating
                            theme: page.theme
                            darkMode: page.darkMode
                            uiScale: page.uiScale
                            tooltip: qsTr("Refresh fan telemetry")
                            enabled: !page.refreshAnimating
                            onClicked: page.refreshAll()
                        }
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        // Match the Monitor summary-card breakpoints: CPU first,
                        // GPU second, then the remaining discovered channels.
                        columns: width > 900 ? 3 : (width > 560 ? 2 : 1)
                        columnSpacing: Math.round(10 * page.uiScale)
                        rowSpacing: Math.round(10 * page.uiScale)

                        Repeater {
                            model: page.orderedFans.length > 0 ? page.orderedFans : (page.fanController ? page.fanController.systemFans : [])

                            delegate: Rectangle {
                                id: fanCard
                                required property var modelData
                                required property int index
                                Layout.fillWidth: true
                                implicitHeight: Math.round(152 * page.uiScale)
                                radius: 12
                                scale: (page.reorderMode && page.draggingFanIndex === fanCard.index) ? 1.03 : 1.0
                                z: (page.reorderMode && page.draggingFanIndex === fanCard.index) ? 10 : 1
                                color: (page.reorderMode && page.draggingFanIndex === fanCard.index)
                                       ? (page.darkMode ? "#3D345C" : "#EDE9FE")
                                       : ((page.fanController && page.fanController.selectedFanIndex === fanCard.index)
                                          ? (page.darkMode ? "#383152" : "#E0E7FF")
                                          : page.bgColor)
                                border.width: (page.reorderMode && page.draggingFanIndex === fanCard.index) ? 2 : ((page.fanController && page.fanController.selectedFanIndex === fanCard.index) ? 2 : 1)
                                border.color: (page.reorderMode && page.draggingFanIndex === fanCard.index)
                                              ? page.accentColor
                                              : ((page.fanController && page.fanController.selectedFanIndex === fanCard.index) ? page.accentColor : page.borderColor)

                                Behavior on scale { NumberAnimation { duration: 150; easing.type: Easing.OutQuad } }

                                MouseArea {
                                    id: fanCardMouseArea
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: page.reorderMode ? Qt.SizeAllCursor : Qt.PointingHandCursor
                                    pressAndHoldInterval: 350
                                    onPressAndHold: { }
                                    onClicked: {
                                        if (page.reorderMode) {
                                            page.draggingFanIndex = fanCard.index;
                                        } else {
                                            if (page.fanController)
                                                page.fanController.selectFan(fanCard.index);
                                            fanSettingsPopup.openForFan(fanCard.modelData);
                                        }
                                    }
                                }

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: Math.round(12 * page.uiScale)
                                    spacing: Math.round(8 * page.uiScale)

                                    RowLayout {
                                        Layout.fillWidth: true
                                        spacing: 8

                                        Rectangle {
                                            implicitWidth: Math.round(54 * page.uiScale)
                                            implicitHeight: Math.round(24 * page.uiScale)
                                            radius: 6
                                            color: page.darkMode ? "#4A3E6D" : "#E2E8F0"

                                            Row {
                                                anchors.centerIn: parent
                                                spacing: Math.round(5 * page.uiScale)

                                                Label {
                                                    text: fanCard.modelData.type === "CPU" ? "▦"
                                                          : (fanCard.modelData.type === "GPU" ? "▰" : "✣")
                                                    color: page.accentColor
                                                    font.pixelSize: Math.round(13 * page.uiScale)
                                                    font.weight: Font.Bold
                                                }

                                                Label {
                                                    text: fanCard.modelData.type || "SYS"
                                                    color: page.textColor
                                                    font.pixelSize: Math.round(10 * page.uiScale)
                                                    font.weight: Font.Bold
                                                }
                                            }
                                        }

                                        Label {
                                            text: fanCard.modelData.name || qsTr("Fan Device")
                                            color: page.textColor
                                            font.pixelSize: Math.round(14 * page.uiScale)
                                            font.weight: Font.DemiBold
                                            elide: Text.ElideRight
                                            Layout.fillWidth: true
                                        }

                                        ToolButton {
                                            id: fanSettingsButton
                                            visible: !page.reorderMode
                                            text: "⚙"
                                            implicitWidth: Math.round(28 * page.uiScale)
                                            implicitHeight: Math.round(28 * page.uiScale)
                                            hoverEnabled: true
                                            background: Rectangle {
                                                radius: width / 2
                                                color: fanSettingsButton.hovered ? (page.darkMode ? "#3B3156" : "#E2E8F0") : "transparent"
                                            }
                                            contentItem: Label {
                                                text: fanSettingsButton.text
                                                color: fanSettingsButton.hovered ? page.accentColor : page.softTextColor
                                                font.pixelSize: Math.round(15 * page.uiScale)
                                                horizontalAlignment: Text.AlignHCenter
                                                verticalAlignment: Text.AlignVCenter
                                            }
                                            ToolTip.visible: hovered
                                            ToolTip.text: qsTr("Open fan settings")
                                            onClicked: {
                                                if (page.fanController)
                                                    page.fanController.selectFan(fanCard.index);
                                                fanSettingsPopup.openForFan(fanCard.modelData);
                                            }
                                        }
                                    }

                                    RowLayout {
                                        Layout.fillWidth: true
                                        spacing: Math.round(10 * page.uiScale)

                                        // Speed Metric
                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            spacing: 2

                                            Label {
                                                text: qsTr("SPEED")
                                                color: page.softTextColor
                                                font.pixelSize: Math.round(10 * page.uiScale)
                                                font.weight: Font.DemiBold
                                            }

                                            Label {
                                                text: fanCard.modelData && fanCard.modelData.speedPercent !== undefined
                                                      ? (fanCard.modelData.speedPercent + "%")
                                                      : qsTr("--")
                                                color: page.textColor
                                                font.pixelSize: Math.round(18 * page.uiScale)
                                                font.weight: Font.Bold
                                            }
                                        }

                                        // Vertical divider
                                        Rectangle {
                                            implicitWidth: 1
                                             implicitHeight: Math.round(28 * page.uiScale)
                                            color: page.borderColor
                                            opacity: 0.6
                                        }

                                        // RPM Metric
                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            spacing: 2

                                            Label {
                                                text: qsTr("RPM")
                                                color: page.softTextColor
                                                font.pixelSize: Math.round(10 * page.uiScale)
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
                                                color: page.textColor
                                                font.pixelSize: Math.round(18 * page.uiScale)
                                                font.weight: Font.Bold
                                                elide: Text.ElideRight
                                            }
                                        }

                                        // Vertical divider
                                        Rectangle {
                                            implicitWidth: 1
                                            implicitHeight: Math.round(28 * page.uiScale)
                                            color: page.borderColor
                                            opacity: 0.6
                                        }

                                        // Temperature Metric
                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            spacing: 2

                                            Label {
                                                text: qsTr("TEMPERATURE")
                                                color: page.softTextColor
                                                font.pixelSize: Math.round(10 * page.uiScale)
                                                font.weight: Font.DemiBold
                                            }

                                            Label {
                                                text: fanCard.modelData.temperatureC > 0 ? (fanCard.modelData.temperatureC + " °C") : qsTr("--")
                                                color: (fanCard.modelData && fanCard.modelData.temperatureC > 80) ? page.warningText : page.textColor
                                                font.pixelSize: Math.round(18 * page.uiScale)
                                                font.weight: Font.Bold
                                                elide: Text.ElideRight
                                            }
                                        }
                                    }

                                    // Real-time sparkline telemetry (per-fan live stream)
                                    TelemetrySparkline {
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: Math.round(28 * page.uiScale)
                                        values: (page.perFanHistories && fanCard.modelData && page.perFanHistories[fanCard.modelData.id])
                                                ? page.perFanHistories[fanCard.modelData.id]
                                                : page.fanSpeedHistory
                                        lineColor: (page.fanController && page.fanController.selectedFanIndex === fanCard.index)
                                                   ? page.accentColor
                                                   : (page.darkMode ? "#6366F1" : "#4F46E5")
                                    }

                                    // Reordering Controls Bar (visible in reorder mode)
                                    RowLayout {
                                        visible: page.reorderMode
                                        Layout.fillWidth: true
                                        spacing: 6

                                        Button {
                                            id: moveLeftBtn
                                            enabled: fanCard.index > 0
                                            implicitHeight: Math.round(28 * page.uiScale)
                                            implicitWidth: Math.round(40 * page.uiScale)
                                            hoverEnabled: true

                                            background: Rectangle {
                                                radius: 6
                                                color: moveLeftBtn.down ? (page.darkMode ? "#4A3E6D" : "#CBD5E1")
                                                                        : (moveLeftBtn.hovered ? (page.darkMode ? "#383152" : "#E2E8F0")
                                                                                               : page.bgColor)
                                                border.width: 1
                                                border.color: moveLeftBtn.hovered ? page.accentColor : page.borderColor
                                                opacity: moveLeftBtn.enabled ? 1.0 : 0.4
                                            }

                                            contentItem: Text {
                                                text: "←"
                                                color: moveLeftBtn.hovered ? page.accentColor : page.textColor
                                                font.pixelSize: Math.round(14 * page.uiScale)
                                                font.weight: Font.Bold
                                                horizontalAlignment: Text.AlignHCenter
                                                verticalAlignment: Text.AlignVCenter
                                            }

                                            onClicked: page.moveFan(fanCard.index, fanCard.index - 1)
                                        }

                                        Rectangle {
                                            Layout.fillWidth: true
                                            implicitHeight: Math.round(28 * page.uiScale)
                                            radius: 6
                                            color: page.darkMode ? "#2E2442" : "#F3E8FF"
                                            border.width: 1
                                            border.color: page.darkMode ? "#4C3872" : "#DDD6FE"

                                            RowLayout {
                                                anchors.centerIn: parent
                                                spacing: 4

                                                Label {
                                                    text: "⠿"
                                                    color: page.accentColor
                                                    font.pixelSize: Math.round(12 * page.uiScale)
                                                }

                                                Label {
                                                    text: qsTr("Slot %1").arg(fanCard.index + 1)
                                                    color: page.accentColor
                                                    font.pixelSize: Math.round(11 * page.uiScale)
                                                    font.weight: Font.Bold
                                                }
                                            }
                                        }

                                        Button {
                                            id: moveRightBtn
                                            enabled: fanCard.index < (page.orderedFans.length - 1)
                                            implicitHeight: Math.round(28 * page.uiScale)
                                            implicitWidth: Math.round(40 * page.uiScale)
                                            hoverEnabled: true

                                            background: Rectangle {
                                                radius: 6
                                                color: moveRightBtn.down ? (page.darkMode ? "#4A3E6D" : "#CBD5E1")
                                                                         : (moveRightBtn.hovered ? (page.darkMode ? "#383152" : "#E2E8F0")
                                                                                                : page.bgColor)
                                                border.width: 1
                                                border.color: moveRightBtn.hovered ? page.accentColor : page.borderColor
                                                opacity: moveRightBtn.enabled ? 1.0 : 0.4
                                            }

                                            contentItem: Text {
                                                text: "→"
                                                color: moveRightBtn.hovered ? page.accentColor : page.textColor
                                                font.pixelSize: Math.round(14 * page.uiScale)
                                                font.weight: Font.Bold
                                                horizontalAlignment: Text.AlignHCenter
                                                verticalAlignment: Text.AlignVCenter
                                            }

                                            onClicked: page.moveFan(fanCard.index, fanCard.index + 1)
                                        }
                                    }
                                }
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        visible: page.hasDetectedFans && page.orderedFans.length === 1
                        implicitHeight: singleChannelInfo.implicitHeight + Math.round(20 * page.uiScale)
                        radius: 8
                        color: page.infoBg
                        border.width: 1
                        border.color: page.borderColor

                        Label {
                            id: singleChannelInfo
                            anchors.fill: parent
                            anchors.margins: Math.round(10 * page.uiScale)
                            text: qsTr("Only one RPM channel is exposed by Linux. CPU and chassis fans will appear automatically when the motherboard firmware or kernel sensor driver publishes their RPM telemetry.")
                            color: page.softTextColor
                            wrapMode: Text.WordWrap
                            font.pixelSize: Math.round(11 * page.uiScale)
                        }
                    }
                }
            }

            // Section 2: Fan Control & Profile Settings Section
            Rectangle {
                Layout.fillWidth: true
                radius: 14
                color: page.cardColor
                border.width: 1
                border.color: page.borderColor
                implicitHeight: profileSectionLayout.implicitHeight + Math.round(28 * page.uiScale)

                ColumnLayout {
                    id: profileSectionLayout
                    anchors.fill: parent
                    anchors.margins: Math.round(16 * page.uiScale)
                    spacing: Math.round(14 * page.uiScale)

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            text: qsTr("Optimization Profiles & Control")
                            color: page.textColor
                            font.pixelSize: Math.round(16 * page.uiScale)
                            font.weight: Font.DemiBold
                            Layout.fillWidth: true
                        }

                        Rectangle {
                            implicitWidth: statusBadgeText.implicitWidth + 18
                            implicitHeight: Math.round(28 * page.uiScale)
                            radius: 6
                            color: page.fanController && page.fanController.controlSupported ? page.successBg : page.infoBg
                            border.width: 1
                            border.color: page.fanController && page.fanController.controlSupported ? page.successText : (page.darkMode ? "#4D5B9E" : "#93C5FD")

                            Label {
                                id: statusBadgeText
                                anchors.centerIn: parent
                                text: {
                                    if (page.fanController && page.fanController.controlSupported)
                                        return qsTr("ACTIVE: %1").arg(page.fanController.fanMode.toUpperCase());
                                    return qsTr("MANAGED: %1").arg(page.fanController ? page.fanController.fanMode.toUpperCase() : "AUTO");
                                }
                                color: page.fanController && page.fanController.controlSupported ? page.successText : page.accentColor
                                font.pixelSize: Math.round(11 * page.uiScale)
                                font.weight: Font.DemiBold
                            }
                        }
                    }

                    // Profile Selector Buttons
                    GridLayout {
                        Layout.fillWidth: true
                        columns: width > 900 ? 6 : (width > 560 ? 3 : 2)
                        columnSpacing: Math.round(10 * page.uiScale)
                        rowSpacing: Math.round(10 * page.uiScale)

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
                                implicitHeight: Math.round(52 * page.uiScale)
                                hoverEnabled: true

                                readonly property bool isCurrent: page.fanController && page.fanController.fanMode === modeBtn.modelData.mode
                                enabled: page.fanController && page.fanController.controlSupported

                                background: Rectangle {
                                    radius: 10
                                    color: modeBtn.isCurrent
                                           ? (page.darkMode ? "#342D4A" : "#EEF2FF")
                                           : (modeBtn.hovered ? (page.darkMode ? "#2E2742" : "#F8FAFC") : page.bgColor)
                                    border.width: modeBtn.isCurrent ? 2 : 1
                                    border.color: modeBtn.isCurrent ? page.accentColor : page.borderColor

                                }

                                contentItem: ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: Math.round(10 * page.uiScale)
                                    spacing: 2

                                    RowLayout {
                                        Layout.fillWidth: true
                                        spacing: 6

                                        Rectangle {
                                            implicitWidth: Math.round(8 * page.uiScale)
                                            implicitHeight: Math.round(8 * page.uiScale)
                                            radius: Math.round(4 * page.uiScale)
                                            color: modeBtn.isCurrent ? page.accentColor : (page.darkMode ? "#403858" : "#CBD5E1")
                                        }

                                        Label {
                                            Layout.fillWidth: true
                                            text: modeBtn.modelData.label
                                            color: modeBtn.isCurrent ? page.accentColor : page.textColor
                                            font.pixelSize: Math.round(13 * page.uiScale)
                                            font.weight: modeBtn.isCurrent ? Font.Bold : Font.DemiBold
                                            elide: Text.ElideRight
                                        }
                                    }

                                    Label {
                                        Layout.fillWidth: true
                                        text: modeBtn.modelData.desc
                                        color: page.softTextColor
                                        font.pixelSize: Math.round(10 * page.uiScale)
                                        elide: Text.ElideRight
                                    }
                                }

                                onClicked: {
                                    if (page.fanController)
                                        page.fanController.setFanMode(modeBtn.modelData.mode);
                                }
                            }
                        }
                    }

                    // Modern Custom Curve Studio (when Custom mode selected)
                    Rectangle {
                        Layout.fillWidth: true
                        radius: 12
                        color: page.bgColor
                        border.width: 1
                        border.color: page.borderColor
                        implicitHeight: customSummaryLayout.implicitHeight + Math.round(24 * page.uiScale)
                        visible: page.fanController && page.fanController.fanMode === "custom"

                        ColumnLayout {
                            id: customSummaryLayout
                            anchors.fill: parent
                            anchors.margins: Math.round(14 * page.uiScale)
                            spacing: Math.round(12 * page.uiScale)

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 1

                                    Label {
                                        text: qsTr("Custom Fan Curve Dynamics & Control Points")
                                        color: page.textColor
                                        font.pixelSize: Math.round(14 * page.uiScale)
                                        font.weight: Font.DemiBold
                                    }

                                    Label {
                                        text: qsTr("Multi-point linear temperature ramp curve mapped to cooling PWM controllers.")
                                        color: page.softTextColor
                                        font.pixelSize: Math.round(11 * page.uiScale)
                                    }
                                }

                                Button {
                                    id: configureStudioBtn
                                    text: qsTr("Open Curve Studio & Live Tuner ↗")
                                    implicitHeight: Math.round(34 * page.uiScale)
                                    hoverEnabled: true

                                    background: Rectangle {
                                        radius: 8
                                        color: configureStudioBtn.hovered ? (page.darkMode ? "#3B3156" : "#EDE9FE") : (page.darkMode ? "#2E2442" : "#F3E8FF")
                                        border.width: 1
                                        border.color: page.accentColor
                                    }

                                    contentItem: Text {
                                        text: configureStudioBtn.text
                                        color: page.accentColor
                                        font.pixelSize: Math.round(11 * page.uiScale)
                                        font.weight: Font.DemiBold
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }

                                    onClicked: {
                                        var fans = page.fanController ? page.fanController.systemFans : [];
                                        if (fans.length > 0)
                                            fanSettingsPopup.openForFan(fans[0]);
                                    }
                                }
                            }

                            // 4 Equal Sized Control Point Cards
                            GridLayout {
                                Layout.fillWidth: true
                                columns: width > 640 ? 4 : 2
                                columnSpacing: Math.round(8 * page.uiScale)
                                rowSpacing: Math.round(8 * page.uiScale)

                                Repeater {
                                    model: 4

                                    delegate: Rectangle {
                                        id: ptCard
                                        required property int index
                                        readonly property var ptData: {
                                            var pts = page.fanController ? page.fanController.customCurvePoints : [];
                                            if (pts && pts.length > ptCard.index)
                                                return pts[ptCard.index];
                                            return { temp: 40 + ptCard.index * 15, speed: 30 + ptCard.index * 20 };
                                        }

                                        Layout.fillWidth: true
                                        implicitHeight: Math.round(64 * page.uiScale)
                                        radius: 8
                                        color: page.cardColor
                                        border.width: 1
                                        border.color: page.borderColor

                                        ColumnLayout {
                                            anchors.fill: parent
                                            anchors.margins: Math.round(8 * page.uiScale)
                                            spacing: 4

                                            RowLayout {
                                                Layout.fillWidth: true

                                                Rectangle {
                                                    implicitWidth: Math.round(20 * page.uiScale)
                                                    implicitHeight: Math.round(18 * page.uiScale)
                                                    radius: 4
                                                    color: page.darkMode ? "#221C30" : "#E2E8F0"

                                                    Label {
                                                        anchors.centerIn: parent
                                                        text: "#" + (ptCard.index + 1)
                                                        color: page.accentColor
                                                        font.pixelSize: Math.round(10 * page.uiScale)
                                                        font.weight: Font.Bold
                                                    }
                                                }

                                                Label {
                                                    text: (ptCard.ptData.temp || 0) + " °C"
                                                    color: page.textColor
                                                    font.pixelSize: Math.round(12 * page.uiScale)
                                                    font.weight: Font.DemiBold
                                                }

                                                Item { Layout.fillWidth: true }

                                                Label {
                                                    text: (ptCard.ptData.speed || 0) + "%"
                                                    color: page.accentColor
                                                    font.pixelSize: Math.round(13 * page.uiScale)
                                                    font.weight: Font.Bold
                                                }
                                            }

                                            // Mini Speed Progress Bar
                                            Rectangle {
                                                Layout.fillWidth: true
                                                implicitHeight: Math.round(6 * page.uiScale)
                                                radius: 3
                                                color: page.darkMode ? "#1E2238" : "#E2E8F0"
                                                clip: true

                                                Rectangle {
                                                    anchors.left: parent.left
                                                    anchors.top: parent.top
                                                    anchors.bottom: parent.bottom
                                                    width: parent.width * Math.min(1.0, Math.max(0.0, (ptCard.ptData.speed || 0) / 100.0))
                                                    radius: 3
                                                    color: {
                                                        var s = ptCard.ptData.speed || 0;
                                                        if (s > 80) return page.warningText;
                                                        if (s > 50) return page.accentColor;
                                                        return page.darkMode ? "#34D399" : "#10B981";
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            // Preset Pills Row
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: Math.round(8 * page.uiScale)

                                Label {
                                    text: qsTr("Curve Presets:")
                                    color: page.softTextColor
                                    font.pixelSize: Math.round(11 * page.uiScale)
                                    font.weight: Font.DemiBold
                                }

                                Repeater {
                                    model: [
                                        { id: "stealth", name: qsTr("Zero-dB Stealth") },
                                        { id: "balanced", name: qsTr("Balanced") },
                                        { id: "aggressive", name: qsTr("Aggressive") },
                                        { id: "stepped", name: qsTr("Stepped") }
                                    ]

                                    delegate: Button {
                                        id: presetBtn
                                        required property var modelData
                                        text: presetBtn.modelData.name
                                        implicitHeight: Math.round(28 * page.uiScale)
                                        leftPadding: Math.round(10 * page.uiScale)
                                        rightPadding: Math.round(10 * page.uiScale)
                                        hoverEnabled: true

                                        background: Rectangle {
                                            radius: 6
                                            color: presetBtn.hovered ? (page.darkMode ? "#3B3156" : "#E2E8F0") : page.cardColor
                                            border.width: 1
                                            border.color: presetBtn.hovered ? page.accentColor : page.borderColor
                                        }

                                        contentItem: Text {
                                            text: presetBtn.text
                                            color: presetBtn.hovered ? page.accentColor : page.textColor
                                            font.pixelSize: Math.round(11 * page.uiScale)
                                            font.weight: Font.DemiBold
                                            horizontalAlignment: Text.AlignHCenter
                                            verticalAlignment: Text.AlignVCenter
                                        }

                                        onClicked: {
                                            if (page.fanController)
                                                page.fanController.applyCurvePreset(presetBtn.modelData.id);
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // Bottom Row: FanControl Dynamics & Battery Switches (Equal size)
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Math.round(12 * page.uiScale)
                        visible: page.hasControllableFan || page.supportsBatteryFanSync

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredWidth: 1
                            visible: page.hasControllableFan
                            implicitHeight: Math.round(58 * page.uiScale)
                            radius: 10
                            color: page.bgColor
                            border.width: 1
                            border.color: page.borderColor

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: Math.round(12 * page.uiScale)
                                spacing: 10

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 1

                                    Label {
                                        text: qsTr("Response Smoothing")
                                        color: page.textColor
                                        font.pixelSize: Math.round(13 * page.uiScale)
                                        font.weight: Font.DemiBold
                                    }

                                    Label {
                                        text: !page.fanController ? ""
                                              : (page.fanController.smoothingEnabled
                                                 ? qsTr("Active — GPU changes are rate-limited (%1°C hysteresis).").arg(page.fanController.hysteresisTempC)
                                                 : qsTr("Disabled — GPU changes apply immediately."))
                                        color: page.softTextColor
                                        font.pixelSize: Math.round(11 * page.uiScale)
                                    }
                                }

                                Switch {
                                    id: smoothingSwitch
                                    checked: page.fanController ? page.fanController.smoothingEnabled : false
                                    implicitWidth: Math.round(44 * page.uiScale)
                                    implicitHeight: Math.round(24 * page.uiScale)

                                    indicator: Rectangle {
                                        implicitWidth: Math.round(44 * page.uiScale)
                                        implicitHeight: Math.round(24 * page.uiScale)
                                        radius: Math.round(12 * page.uiScale)
                                        color: smoothingSwitch.checked ? page.accentColor : (page.darkMode ? "#342D4A" : "#CBD5E1")
                                        border.width: 1
                                        border.color: smoothingSwitch.checked ? page.accentColor : page.borderColor


                                        Rectangle {
                                            x: smoothingSwitch.checked ? (parent.width - width - 2) : 2
                                            y: (parent.height - height) / 2
                                            width: Math.round(20 * page.uiScale)
                                            height: Math.round(20 * page.uiScale)
                                            radius: Math.round(10 * page.uiScale)
                                            color: "#FFFFFF"

                                            Behavior on x { NumberAnimation { duration: 150; easing.type: Easing.InOutQuad } }
                                        }
                                    }

                                    onToggled: {
                                        if (page.fanController)
                                            page.fanController.setSmoothingEnabled(checked);
                                    }
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredWidth: 1
                            visible: page.supportsBatteryFanSync
                            implicitHeight: Math.round(58 * page.uiScale)
                            radius: 10
                            color: page.bgColor
                            border.width: 1
                            border.color: page.borderColor

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: Math.round(12 * page.uiScale)
                                spacing: 10

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 1

                                    Label {
                                        text: qsTr("Battery Profile Sync")
                                        color: page.textColor
                                        font.pixelSize: Math.round(13 * page.uiScale)
                                        font.weight: Font.DemiBold
                                    }

                                    Label {
                                        text: !page.fanController ? ""
                                              : (!page.fanController.batteryProfileSyncEnabled
                                                 ? qsTr("Disabled — does not alter fan profiles on battery.")
                                                 : (page.systemInfo.onBattery
                                                    ? qsTr("Active — controllable GPU fan uses Silent on battery.")
                                                    : qsTr("Armed — uses Silent when battery power begins.")))
                                        color: page.softTextColor
                                        font.pixelSize: Math.round(11 * page.uiScale)
                                    }
                                }

                                Switch {
                                    id: batterySyncSwitch
                                    checked: page.fanController ? page.fanController.batteryProfileSyncEnabled : false
                                    implicitWidth: Math.round(44 * page.uiScale)
                                    implicitHeight: Math.round(24 * page.uiScale)

                                    indicator: Rectangle {
                                        implicitWidth: Math.round(44 * page.uiScale)
                                        implicitHeight: Math.round(24 * page.uiScale)
                                        radius: Math.round(12 * page.uiScale)
                                        color: batterySyncSwitch.checked ? page.accentColor : (page.darkMode ? "#342D4A" : "#CBD5E1")
                                        border.width: 1
                                        border.color: batterySyncSwitch.checked ? page.accentColor : page.borderColor


                                        Rectangle {
                                            x: batterySyncSwitch.checked ? (parent.width - width - 2) : 2
                                            y: (parent.height - height) / 2
                                            width: Math.round(20 * page.uiScale)
                                            height: Math.round(20 * page.uiScale)
                                            radius: Math.round(10 * page.uiScale)
                                            color: "#FFFFFF"

                                            Behavior on x { NumberAnimation { duration: 150; easing.type: Easing.InOutQuad } }
                                        }
                                    }

                                    onToggled: {
                                        if (page.fanController)
                                            page.fanController.batteryProfileSyncEnabled = checked;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Components.FanSettingsPopup {
        id: fanSettingsPopup
        fanController: page.fanController
        theme: page.theme
        darkMode: page.darkMode
        uiScale: page.uiScale
    }

    Components.FanRescanPopup {
        id: fanRescanPopup
        fanController: page.fanController
        theme: page.theme
        darkMode: page.darkMode
        uiScale: page.uiScale
    }

    Component.onCompleted: {
        // Keep history empty until genuine telemetry arrives.
        page.fanSpeedHistory = [];
        page.fanRpmHistory = [];

        if (page.fanController) {
            page.fanController.start();
            page.fanController.refresh();
        }
        page.syncOrderedFans();
        page.pushFanHistory();
    }

    Timer {
        id: historySampler
        interval: 400
        repeat: true
        running: true
        onTriggered: page.pushFanHistory()
    }
}
