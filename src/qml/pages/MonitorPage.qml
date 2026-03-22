import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: page

    required property var theme
    property bool darkMode: false
    property bool compactMode: false
    property bool showAdvancedInfo: true

    ScrollView {
        anchors.fill: parent
        clip: true
        contentWidth: availableWidth

        ColumnLayout {
            width: parent.availableWidth
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
                              ? qsTr("Temperature: ") + cpuMonitor.temperatureC + " C"
                              : qsTr("CPU telemetry is currently unavailable.")
                    accentColor: page.theme.accentA
                    emphasized: cpuMonitor.available && cpuMonitor.usagePercent >= 85
                }

                StatCard {
                    Layout.fillWidth: true
                    theme: page.theme
                    title: qsTr("GPU Load")
                    value: gpuMonitor.available ? gpuMonitor.utilizationPercent + "%" : qsTr("Unavailable")
                    subtitle: gpuMonitor.available
                              ? (gpuMonitor.gpuName.length > 0 ? gpuMonitor.gpuName : qsTr("NVIDIA GPU"))
                              : qsTr("nvidia-smi did not return live GPU telemetry.")
                    accentColor: page.theme.accentB
                    emphasized: gpuMonitor.available && gpuMonitor.temperatureC >= 80
                }

                StatCard {
                    Layout.fillWidth: true
                    theme: page.theme
                    title: qsTr("Memory Usage")
                    value: ramMonitor.available ? ramMonitor.usagePercent + "%" : qsTr("Unavailable")
                    subtitle: ramMonitor.available
                              ? qsTr("Used: ") + ramMonitor.usedMiB + " / " + ramMonitor.totalMiB + " MiB"
                              : qsTr("RAM telemetry is currently unavailable.")
                    accentColor: page.theme.accentC
                    emphasized: ramMonitor.available && ramMonitor.usagePercent >= 85
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
                            visible: gpuMonitor.available
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
                            text: gpuMonitor.available ? qsTr("GPU Online") : qsTr("GPU Telemetry Missing")
                            backgroundColor: gpuMonitor.available ? page.theme.successBg : page.theme.warningBg
                            foregroundColor: page.theme.text
                        }

                        InfoBadge {
                            text: ramMonitor.available && ramMonitor.usagePercent >= 85 ? qsTr("Memory Pressure") : qsTr("Memory Stable")
                            backgroundColor: ramMonitor.available && ramMonitor.usagePercent >= 85 ? page.theme.warningBg : page.theme.successBg
                            foregroundColor: page.theme.text
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        wrapMode: Text.Wrap
                        color: page.theme.textSoft
                        text: gpuMonitor.available
                              ? qsTr("GPU temperature: ") + gpuMonitor.temperatureC + " C, VRAM " + gpuMonitor.memoryUsedMiB + " / " + gpuMonitor.memoryTotalMiB + " MiB."
                              : qsTr("GPU metrics are unavailable. Check driver installation and nvidia-smi accessibility.")
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
                            text: cpuMonitor.temperatureC > 0 ? cpuMonitor.temperatureC + " C" : qsTr("Unknown")
                            color: page.theme.text
                        }

                        Label {
                            text: qsTr("GPU Temperature")
                            color: page.theme.textMuted
                        }

                        Label {
                            text: gpuMonitor.available && gpuMonitor.temperatureC > 0 ? gpuMonitor.temperatureC + " C" : qsTr("Unknown")
                            color: page.theme.text
                        }

                        Label {
                            text: qsTr("VRAM")
                            color: page.theme.textMuted
                        }

                        Label {
                            text: gpuMonitor.available ? gpuMonitor.memoryUsedMiB + " / " + gpuMonitor.memoryTotalMiB + " MiB" : qsTr("Unknown")
                            color: page.theme.text
                        }

                        Label {
                            text: qsTr("RAM Footprint")
                            color: page.theme.textMuted
                        }

                        Label {
                            text: ramMonitor.available ? ramMonitor.usedMiB + " / " + ramMonitor.totalMiB + " MiB" : qsTr("Unknown")
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
                            text: gpuMonitor.available ? qsTr("NVIDIA Path OK") : qsTr("Check NVIDIA Path")
                            backgroundColor: gpuMonitor.available ? page.theme.successBg : page.theme.warningBg
                            foregroundColor: page.theme.text
                        }
                    }
                }
            }
        }
    }
}
