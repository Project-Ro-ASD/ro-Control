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

    readonly property bool nvidiaGpuDetected: page.nvidiaDetector && page.nvidiaDetector.gpuFound
    readonly property bool gpuTelemetryAvailable: page.nvidiaGpuDetected && page.gpuMonitor && page.gpuMonitor.available
    readonly property bool cpuTelemetryAvailable: page.cpuMonitor && page.cpuMonitor.available
    readonly property bool ramTelemetryAvailable: page.ramMonitor && page.ramMonitor.available

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

    onVisibleChanged: {
        if (page.visible) {
            page.pushTelemetryHistory();
        }
    }

    ScrollView {
        id: pageScroll
        anchors.fill: parent
        clip: true
        contentWidth: availableWidth

        ColumnLayout {
            width: pageScroll.availableWidth
            spacing: Math.round(12 * page.uiScale)

            Components.MonitorTelemetrySummary {
                Layout.fillWidth: true
                theme: page.theme
                darkMode: page.darkMode
                uiScale: page.uiScale
                systemInfo: page.systemInfo
                cpuMonitor: page.cpuMonitor
                gpuMonitor: page.gpuMonitor
                ramMonitor: page.ramMonitor
                nvidiaDetector: page.nvidiaDetector
                cpuUsageHistory: page.cpuUsageHistory
                gpuLoadHistory: page.gpuLoadHistory
                ramUsageHistory: page.ramUsageHistory
                telemetryRefreshAnimating: page.telemetryRefreshAnimating
                onRefreshClicked: page.refreshTelemetry()
            }

            Components.MonitorGpuPerfCard {
                visible: page.showAdvancedInfo && page.gpuTelemetryAvailable
                theme: page.theme
                darkMode: page.darkMode
                uiScale: page.uiScale
                gpuMonitor: page.gpuMonitor
                systemInfo: page.systemInfo
            }

            Components.MonitorPowerControls {
                visible: page.powerController !== null && (page.powerController.supported
                                                           || page.powerController.systemPowerProfileSupported)
                theme: page.theme
                darkMode: page.darkMode
                uiScale: page.uiScale
                powerController: page.powerController
            }

            Components.MonitorProcessList {
                visible: page.showAdvancedInfo && page.gpuTelemetryAvailable
                theme: page.theme
                darkMode: page.darkMode
                uiScale: page.uiScale
                gpuMonitor: page.gpuMonitor
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
        running: page.visible && Qt.application.state === Qt.ApplicationActive
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
