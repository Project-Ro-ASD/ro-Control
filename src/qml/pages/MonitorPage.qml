import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: page

    required property var theme
    property bool darkMode: false
    property bool compactMode: false
    property bool showAdvancedInfo: true
    readonly property bool cpuTemperatureAvailable: cpuMonitor.temperatureC > 0
    readonly property bool gpuTelemetryAvailable: gpuMonitor.available
    readonly property bool gpuTemperatureAvailable: gpuMonitor.temperatureC > 0
    readonly property bool gpuMemoryAvailable: gpuMonitor.memoryTotalMiB > 0
    readonly property bool ramTelemetryAvailable: ramMonitor.available || ramMonitor.totalMiB > 0
    readonly property bool gpuDetected: nvidiaDetector.gpuFound
    readonly property bool gpuDriverActive: nvidiaDetector.driverLoaded || nvidiaDetector.nouveauActive
    readonly property color monitorBarColor: "#34c99a"

    function formatTemperature(value) {
        return value > 0 ? value + "\u00b0C" : qsTr("Unavailable");
    }

    function formatMemoryUsage(usedMiB, totalMiB) {
        if (totalMiB <= 0)
            return qsTr("Unavailable");

        const usedGiB = Math.round((usedMiB / 1024.0) * 10) / 10;
        const totalGiB = Math.round((totalMiB / 1024.0) * 10) / 10;
        return usedGiB + " / " + totalGiB + " GB";
    }

    function formatMemoryTotal(totalMiB) {
        if (totalMiB <= 0)
            return qsTr("Unavailable");
        const totalGiB = Math.round(totalMiB / 1024.0);
        return totalGiB + " GB";
    }

    function driverLabel() {
        if (nvidiaDetector.driverVersion.length > 0)
            return "nvidia-" + nvidiaDetector.driverVersion;
        if (nvidiaUpdater.currentVersion.length > 0)
            return "nvidia-" + nvidiaUpdater.currentVersion;
        return qsTr("Not installed");
    }

    function gpuLoadValueText() {
        if (page.gpuTelemetryAvailable)
            return gpuMonitor.utilizationPercent + "%";
        if (page.gpuDetected)
            return qsTr("No Live Data");
        return qsTr("Unavailable");
    }

    function gpuSummaryText() {
        if (page.gpuTelemetryAvailable)
            return qsTr("GPU telemetry active");
        if (!page.gpuDetected)
            return qsTr("No NVIDIA GPU detected");
        if (!page.gpuDriverActive)
            return qsTr("Driver inactive");
        return qsTr("Telemetry unavailable");
    }

    function progressValue(percentValue) {
        return Math.max(0, Math.min(100, percentValue)) / 100.0;
    }

    component InfoTile: Rectangle {
        id: infoTile
        required property string title
        required property string value
        required property string markerText
        required property color markerColor

        radius: 24
        color: page.theme.card
        border.width: 1
        border.color: page.theme.border
        implicitHeight: 126

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 28
            anchors.rightMargin: 28
            spacing: 20

            Rectangle {
                width: 62
                height: 62
                radius: 22
                color: infoTile.markerColor

                Label {
                    anchors.centerIn: parent
                    text: infoTile.markerText
                    color: "#ffffff"
                    font.pixelSize: 24
                    font.weight: Font.Bold
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 6

                Label {
                    text: infoTile.title
                    color: page.theme.textSoft
                    font.pixelSize: 16
                    font.weight: Font.Bold
                }

                Label {
                    Layout.fillWidth: true
                    text: infoTile.value
                    color: page.theme.text
                    font.pixelSize: 22
                    font.weight: Font.Bold
                    wrapMode: Text.Wrap
                }
            }
        }
    }

    component MetricRow: Item {
        id: metricRow
        required property string title
        required property string subtitle
        required property string valueText
        required property string markerText
        required property color markerColor
        required property real progress

        implicitHeight: 108

        ColumnLayout {
            anchors.fill: parent
            spacing: 14

            RowLayout {
                Layout.fillWidth: true
                spacing: 16

                Rectangle {
                    width: 56
                    height: 56
                    radius: 20
                    color: metricRow.markerColor

                    Label {
                        anchors.centerIn: parent
                        text: metricRow.markerText
                        color: "#ffffff"
                        font.pixelSize: 22
                        font.weight: Font.Bold
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    Label {
                        text: metricRow.title
                        color: page.theme.text
                        font.pixelSize: 18
                        font.weight: Font.Bold
                    }

                    Label {
                        text: metricRow.subtitle
                        color: page.theme.textSoft
                        font.pixelSize: 14
                    }
                }

                Rectangle {
                    radius: 18
                    color: page.theme.cardStrong
                    implicitWidth: metricValue.implicitWidth + 28
                    implicitHeight: 56

                    Label {
                        id: metricValue
                        anchors.centerIn: parent
                        text: metricRow.valueText
                        color: page.theme.text
                        font.pixelSize: 20
                        font.weight: Font.Bold
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 14
                radius: 7
                color: page.theme.cardStrong

                Rectangle {
                    width: Math.max(16, parent.width * metricRow.progress)
                    height: parent.height
                    radius: 7
                    color: page.monitorBarColor
                }
            }
        }
    }

    ScrollView {
        id: pageScroll
        anchors.fill: parent
        clip: true
        contentWidth: availableWidth

        ColumnLayout {
            width: pageScroll.availableWidth
            spacing: page.compactMode ? 16 : 22

            Label {
                Layout.leftMargin: 16
                text: qsTr("System Information")
                color: page.theme.text
                font.pixelSize: 36
                font.weight: Font.Bold
            }

            GridLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16
                columns: width > 980 ? 2 : 1
                columnSpacing: 22
                rowSpacing: 22

                InfoTile {
                    Layout.fillWidth: true
                    title: qsTr("OS")
                    value: systemInfo.desktopEnvironment.length > 0
                           ? systemInfo.osName + " (" + systemInfo.desktopEnvironment + ")"
                           : (systemInfo.osName.length > 0 ? systemInfo.osName : qsTr("Unavailable"))
                    markerText: "OS"
                    markerColor: "#1da1f2"
                }

                InfoTile {
                    Layout.fillWidth: true
                    title: qsTr("Kernel")
                    value: systemInfo.kernelVersion.length > 0 ? systemInfo.kernelVersion : qsTr("Kernel info unavailable")
                    markerText: "K"
                    markerColor: "#df4be0"
                }

                InfoTile {
                    Layout.fillWidth: true
                    title: qsTr("CPU")
                    value: systemInfo.cpuModel.length > 0 ? systemInfo.cpuModel : qsTr("CPU telemetry unavailable")
                    markerText: "CPU"
                    markerColor: "#ff6a13"
                }

                InfoTile {
                    Layout.fillWidth: true
                    title: qsTr("RAM")
                    value: page.ramTelemetryAvailable ? page.formatMemoryTotal(ramMonitor.totalMiB) : qsTr("RAM unavailable")
                    markerText: "RAM"
                    markerColor: "#16c65f"
                }

                InfoTile {
                    Layout.fillWidth: true
                    title: qsTr("GPU")
                    value: nvidiaDetector.gpuName.length > 0
                           ? nvidiaDetector.gpuName
                           : (nvidiaDetector.displayAdapterName.length > 0 ? nvidiaDetector.displayAdapterName : qsTr("Unavailable"))
                    markerText: "GPU"
                    markerColor: "#6a6fff"
                }

                InfoTile {
                    Layout.fillWidth: true
                    title: qsTr("Driver")
                    value: page.driverLabel()
                    markerText: "DRV"
                    markerColor: "#ff9800"
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16

                Label {
                    text: qsTr("GPU Status")
                    color: page.theme.text
                    font.pixelSize: 26
                    font.weight: Font.Bold
                }

                Item {
                    Layout.fillWidth: true
                }

                Rectangle {
                    radius: 20
                    color: page.theme.successBg
                    border.width: 1
                    border.color: Qt.tint(page.theme.success, "#55ffffff")
                    implicitWidth: 122
                    implicitHeight: 48

                    RowLayout {
                        anchors.centerIn: parent
                        spacing: 10

                        Rectangle {
                            width: 12
                            height: 12
                            radius: 6
                            color: page.theme.success
                        }

                        Label {
                            text: page.gpuTelemetryAvailable ? qsTr("Active") : qsTr("Standby")
                            color: page.theme.success
                            font.pixelSize: 16
                            font.weight: Font.Bold
                        }
                    }
                }
            }

            SectionPanel {
                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16
                theme: page.theme
                title: ""
                subtitle: ""

                MetricRow {
                    Layout.fillWidth: true
                    title: qsTr("Temperature")
                    subtitle: qsTr("Real-time monitoring")
                    valueText: page.formatTemperature(gpuMonitor.temperatureC)
                    markerText: "T"
                    markerColor: "#1da1f2"
                    progress: page.progressValue(page.gpuTemperatureAvailable ? gpuMonitor.temperatureC : 0)
                }

                MetricRow {
                    Layout.fillWidth: true
                    title: qsTr("GPU Load")
                    subtitle: qsTr("Real-time monitoring")
                    valueText: page.gpuLoadValueText()
                    markerText: "G"
                    markerColor: "#00c46a"
                    progress: page.progressValue(page.gpuTelemetryAvailable ? gpuMonitor.utilizationPercent : 0)
                }

                MetricRow {
                    Layout.fillWidth: true
                    title: qsTr("VRAM Usage")
                    subtitle: qsTr("Real-time monitoring")
                    valueText: page.gpuMemoryAvailable ? page.formatMemoryUsage(gpuMonitor.memoryUsedMiB, gpuMonitor.memoryTotalMiB) : qsTr("Unavailable")
                    markerText: "V"
                    markerColor: "#d84ef0"
                    progress: page.progressValue(page.gpuMemoryAvailable ? gpuMonitor.memoryUsagePercent : 0)
                }
            }

            Label {
                Layout.leftMargin: 16
                text: qsTr("System Resources")
                color: page.theme.text
                font.pixelSize: 26
                font.weight: Font.Bold
            }

            SectionPanel {
                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16
                theme: page.theme
                title: ""
                subtitle: ""

                MetricRow {
                    Layout.fillWidth: true
                    title: qsTr("CPU Usage")
                    subtitle: qsTr("Real-time monitoring")
                    valueText: cpuMonitor.available ? Math.round(cpuMonitor.usagePercent) + "%" : qsTr("Unavailable")
                    markerText: "C"
                    markerColor: "#ff6a13"
                    progress: page.progressValue(cpuMonitor.available ? cpuMonitor.usagePercent : 0)
                }

                MetricRow {
                    Layout.fillWidth: true
                    title: qsTr("RAM Usage")
                    subtitle: qsTr("Real-time monitoring")
                    valueText: page.ramTelemetryAvailable ? page.formatMemoryUsage(ramMonitor.usedMiB, ramMonitor.totalMiB) : qsTr("Unavailable")
                    markerText: "R"
                    markerColor: "#9247f6"
                    progress: page.progressValue(page.ramTelemetryAvailable ? ramMonitor.usagePercent : 0)
                }
            }

            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 10

                Rectangle {
                    width: 14
                    height: 14
                    radius: 7
                    color: page.theme.textSoft
                }

                Label {
                    text: qsTr("Updating every %1 seconds").arg(Math.max(1, Math.round(cpuMonitor.updateInterval / 1000)))
                    color: page.theme.textSoft
                    font.pixelSize: 14
                    font.weight: Font.Medium
                }
            }
        }
    }

    Component.onCompleted: {
        systemInfo.refresh();
        cpuMonitor.start();
        gpuMonitor.start();
        ramMonitor.start();
        cpuMonitor.refresh();
        gpuMonitor.refresh();
        ramMonitor.refresh();
    }
}
