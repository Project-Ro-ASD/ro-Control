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

    function formatTemperature(value) {
        return value > 0 ? value + " C" : qsTr("Unavailable");
    }

    function formatMemoryUsage(usedMiB, totalMiB) {
        return totalMiB > 0 ? usedMiB + " / " + totalMiB + " MiB" : qsTr("Unavailable");
    }

    function gpuLoadValueText() {
        if (page.gpuTelemetryAvailable)
            return gpuMonitor.utilizationPercent + "%";
        if (page.gpuDetected)
            return qsTr("No Live Data");
        return qsTr("Unavailable");
    }

    function gpuSubtitleText() {
        if (page.gpuTelemetryAvailable)
            return gpuMonitor.gpuName.length > 0 ? gpuMonitor.gpuName : qsTr("NVIDIA GPU");
        if (!page.gpuDetected)
            return qsTr("No NVIDIA GPU was detected on this system.");
        if (!page.gpuDriverActive)
            return qsTr("GPU detected, but no active driver is loaded.");
        return qsTr("Live GPU telemetry is unavailable. Check nvidia-smi and driver access.");
    }

    function gpuSummaryText() {
        if (page.gpuTelemetryAvailable)
            return qsTr("GPU temperature: ") + page.formatTemperature(gpuMonitor.temperatureC) + qsTr(", VRAM ") + page.formatMemoryUsage(gpuMonitor.memoryUsedMiB, gpuMonitor.memoryTotalMiB) + ".";
        if (!page.gpuDetected)
            return qsTr("No NVIDIA GPU is currently detected on this system.");
        if (!page.gpuDriverActive)
            return qsTr("GPU telemetry is unavailable because the NVIDIA driver is not active.");
        return qsTr("GPU metrics are unavailable. Check driver installation and nvidia-smi accessibility.");
    }

    ScrollView {
        id: pageScroll
        anchors.fill: parent
        clip: true
        contentWidth: availableWidth

        ColumnLayout {
            width: pageScroll.availableWidth
            spacing: page.compactMode ? 12 : 16

            GridLayout {
                Layout.fillWidth: true
                columns: width > 980 ? 3 : 1
                columnSpacing: 14
                rowSpacing: 14

                StatCard {
                    Layout.fillWidth: true
                    theme: page.theme
                    title: qsTr("CPU Load")
                    value: cpuMonitor.available ? cpuMonitor.usagePercent.toFixed(1) + "%" : qsTr("Unavailable")
                    subtitle: cpuMonitor.available
                              ? qsTr("Temperature: ") + page.formatTemperature(cpuMonitor.temperatureC)
                              : qsTr("CPU telemetry is currently unavailable.")
                    accentColor: page.theme.accentA
                    emphasized: cpuMonitor.available && cpuMonitor.usagePercent >= 85
                }

                StatCard {
                    Layout.fillWidth: true
                    theme: page.theme
                    title: qsTr("GPU Load")
                    value: page.gpuLoadValueText()
                    subtitle: page.gpuSubtitleText()
                    accentColor: page.theme.accentB
                    emphasized: page.gpuTemperatureAvailable && gpuMonitor.temperatureC >= 80
                }

                StatCard {
                    Layout.fillWidth: true
                    theme: page.theme
                    title: qsTr("Memory Usage")
                    value: page.ramTelemetryAvailable ? ramMonitor.usagePercent + "%" : qsTr("Unavailable")
                    subtitle: page.ramTelemetryAvailable
                              ? qsTr("Used: ") + page.formatMemoryUsage(ramMonitor.usedMiB, ramMonitor.totalMiB)
                              : qsTr("RAM telemetry is currently unavailable.")
                    accentColor: page.theme.accentC
                    emphasized: page.ramTelemetryAvailable && ramMonitor.usagePercent >= 85
                }
            }

            GridLayout {
                Layout.fillWidth: true
                columns: width > 980 ? 2 : 1
                columnSpacing: 16
                rowSpacing: 16

                SectionPanel {
                    Layout.fillWidth: true
                    theme: page.theme
                    title: qsTr("Live Resource Curves")
                    subtitle: qsTr("Quick pulse view for the most important machine resources.")

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 10

                        Label {
                            text: qsTr("CPU")
                            color: page.theme.textMuted
                            font.bold: true
                        }

                        ProgressBar {
                            Layout.fillWidth: true
                            from: 0
                            to: 100
                            value: cpuMonitor.usagePercent
                        }

                        Label {
                            text: qsTr("GPU")
                            color: page.theme.textMuted
                            font.bold: true
                        }

                        ProgressBar {
                            Layout.fillWidth: true
                            from: 0
                            to: 100
                            value: gpuMonitor.utilizationPercent
                        }

                        Label {
                            text: qsTr("RAM")
                            color: page.theme.textMuted
                            font.bold: true
                        }

                        ProgressBar {
                            Layout.fillWidth: true
                            from: 0
                            to: 100
                            value: ramMonitor.usagePercent
                        }
                    }
                }

                SectionPanel {
                    Layout.fillWidth: true
                    theme: page.theme
                    title: qsTr("Health Summary")
                    subtitle: qsTr("Fast interpretation of the raw telemetry values.")

                    Flow {
                        Layout.fillWidth: true
                        spacing: 8

                        InfoBadge {
                            text: cpuMonitor.available && cpuMonitor.usagePercent >= 85 ? qsTr("CPU Busy") : qsTr("CPU Stable")
                            backgroundColor: cpuMonitor.available && cpuMonitor.usagePercent >= 85 ? page.theme.warningBg : page.theme.successBg
                            foregroundColor: page.theme.text
                        }

                        InfoBadge {
                            text: page.gpuTelemetryAvailable ? qsTr("GPU Online") : qsTr("GPU Telemetry Missing")
                            backgroundColor: page.gpuTelemetryAvailable ? page.theme.successBg : page.theme.warningBg
                            foregroundColor: page.theme.text
                        }

                        InfoBadge {
                            text: page.ramTelemetryAvailable && ramMonitor.usagePercent >= 85 ? qsTr("Memory Pressure") : qsTr("Memory Stable")
                            backgroundColor: page.ramTelemetryAvailable && ramMonitor.usagePercent >= 85 ? page.theme.warningBg : page.theme.successBg
                            foregroundColor: page.theme.text
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        wrapMode: Text.Wrap
                        color: page.theme.textSoft
                        text: page.gpuSummaryText()
                    }

                    Label {
                        Layout.fillWidth: true
                        wrapMode: Text.Wrap
                        color: page.theme.textSoft
                        visible: page.showAdvancedInfo
                        text: qsTr("Refresh interval: ") + cpuMonitor.updateInterval + " ms"
                    }
                }

                SectionPanel {
                    Layout.fillWidth: true
                    theme: page.theme
                    title: qsTr("Detailed Signals")
                    subtitle: qsTr("Expanded raw values for support and diagnostics.")
                    visible: page.showAdvancedInfo

                    GridLayout {
                        Layout.fillWidth: true
                        columns: 2
                        columnSpacing: 10
                        rowSpacing: 8

                        Label {
                            text: qsTr("CPU Temperature")
                            color: page.theme.textMuted
                        }

                        Label {
                            text: page.formatTemperature(cpuMonitor.temperatureC)
                            color: page.theme.text
                        }

                        Label {
                            text: qsTr("GPU Temperature")
                            color: page.theme.textMuted
                        }

                        Label {
                            text: page.gpuTelemetryAvailable ? page.formatTemperature(gpuMonitor.temperatureC) : qsTr("Unknown")
                            color: page.theme.text
                        }

                        Label {
                            text: qsTr("VRAM")
                            color: page.theme.textMuted
                        }

                        Label {
                            text: page.gpuTelemetryAvailable ? page.formatMemoryUsage(gpuMonitor.memoryUsedMiB, gpuMonitor.memoryTotalMiB) : qsTr("Unknown")
                            color: page.theme.text
                        }

                        Label {
                            text: qsTr("RAM Footprint")
                            color: page.theme.textMuted
                        }

                        Label {
                            text: page.ramTelemetryAvailable ? page.formatMemoryUsage(ramMonitor.usedMiB, ramMonitor.totalMiB) : qsTr("Unknown")
                            color: page.theme.text
                        }
                    }
                }

                SectionPanel {
                    Layout.fillWidth: true
                    theme: page.theme
                    title: qsTr("Actions")
                    subtitle: qsTr("Trigger a manual refresh when you need a fresh sample.")

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10

                        Button {
                            text: qsTr("Refresh Telemetry")
                            onClicked: {
                                cpuMonitor.refresh();
                                gpuMonitor.refresh();
                                ramMonitor.refresh();
                            }
                        }

                        Item {
                            Layout.fillWidth: true
                        }

                        InfoBadge {
                            text: page.gpuTelemetryAvailable ? qsTr("NVIDIA Path OK") : qsTr("Check NVIDIA Path")
                            backgroundColor: page.gpuTelemetryAvailable ? page.theme.successBg : page.theme.warningBg
                            foregroundColor: page.theme.text
                        }
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        cpuMonitor.start();
        gpuMonitor.start();
        ramMonitor.start();
        cpuMonitor.refresh();
        gpuMonitor.refresh();
        ramMonitor.refresh();
    }
}
