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
    readonly property color infoBg: theme && theme.infoBg ? theme.infoBg : (page.darkMode ? "#1E2548" : "#EFF6FF")
    readonly property color warningBg: theme && theme.warningBg ? theme.warningBg : (page.darkMode ? "#3A2E12" : "#FFFBEB")
    readonly property color warningText: theme && theme.warning ? theme.warning : (page.darkMode ? "#FBBF24" : "#D97706")
    readonly property color successBg: theme && theme.successBg ? theme.successBg : (page.darkMode ? "#143828" : "#ECFDF5")
    readonly property color successText: theme && theme.success ? theme.success : (page.darkMode ? "#4ADE80" : "#059669")

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
            page.gpuMonitor.requestRefresh();
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

    function openFanSettings(fanData, fanIndex) {
        if (page.fanController && fanIndex !== undefined)
            page.fanController.selectFan(fanIndex);
        fanSettingsLoader.active = true;
        fanSettingsLoader.item.openForFan(fanData);
    }

    function openFanWizard() {
        fanRescanLoader.active = true;
        fanRescanLoader.item.openWizard();
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
            Components.FanChannelsGrid {
                fanController: page.fanController
                theme: page.theme
                darkMode: page.darkMode
                uiScale: page.uiScale
                orderedFans: page.orderedFans
                perFanHistories: page.perFanHistories
                fanSpeedHistory: page.fanSpeedHistory
                refreshAnimating: page.refreshAnimating
                reorderMode: page.reorderMode
                draggingFanIndex: page.draggingFanIndex
                onOpenWizardRequested: page.openFanWizard()
                onRefreshRequested: page.refreshAll()
                onFanSettingsRequested: (fanData, fanIndex) => page.openFanSettings(fanData, fanIndex)
                onMoveFanRequested: (fromIndex, toIndex) => page.moveFan(fromIndex, toIndex)
            }

            // Section 2: Fan Control & Profile Settings Section
            Components.FanProfileControls {
                fanController: page.fanController
                systemInfo: page.systemInfo
                theme: page.theme
                darkMode: page.darkMode
                uiScale: page.uiScale
                hasControllableFan: page.hasControllableFan
                supportsBatteryFanSync: page.supportsBatteryFanSync
                onModeSelected: (mode) => {
                    if (page.fanController)
                        page.fanController.setFanMode(mode);
                }
                onOpenStudioRequested: {
                    var fans = page.fanController ? page.fanController.systemFans : [];
                    if (fans.length > 0)
                        page.openFanSettings(fans[0], 0);
                }
                onApplyPresetRequested: (presetId) => {
                    if (page.fanController)
                        page.fanController.applyCurvePreset(presetId);
                }
            }
        }
    }

    Loader {
        id: fanSettingsLoader
        active: false
        sourceComponent: Components.FanSettingsPopup {
            fanController: page.fanController
            theme: page.theme
            darkMode: page.darkMode
            uiScale: page.uiScale
        }
    }

    Loader {
        id: fanRescanLoader
        active: false
        sourceComponent: Components.FanRescanPopup {
            fanController: page.fanController
            theme: page.theme
            darkMode: page.darkMode
            uiScale: page.uiScale
        }
    }

    Component.onCompleted: {
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
        running: page.visible
        onTriggered: page.pushFanHistory()
    }
}
