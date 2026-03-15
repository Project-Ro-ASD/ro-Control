import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../components"

Item {
    id: page
    required property var cpuMonitor
    required property var gpuMonitor
    required property var ramMonitor
    required property var theme
    property bool darkMode: true

    function memorySummary(used, total, percent) {
        return qsTr("%1 / %2 MiB (%3%)").arg(used).arg(total).arg(percent)
    }

    ScrollView {
        anchors.fill: parent
        clip: true

        ColumnLayout {
            width: parent.width
            spacing: 18

            Rectangle {
                Layout.fillWidth: true
                radius: 28
                color: Qt.tint(page.theme.panel, page.darkMode ? "#1839a7ff" : "#121677ff")
                border.width: 1
                border.color: page.theme.border
                implicitHeight: headerColumn.implicitHeight + 28

                ColumnLayout {
                    id: headerColumn
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 12

                    Label {
                        text: qsTr("Live System Telemetry")
                        font.pixelSize: 30
                        font.bold: true
                        color: page.theme.text
                    }

                    Label {
                        text: qsTr("A compact overview of CPU, RAM and NVIDIA runtime state. Refresh manually at any time without leaving the dashboard.")
                        wrapMode: Text.Wrap
                        Layout.fillWidth: true
                        color: page.theme.textMuted
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 14

                        StatCard {
                            Layout.fillWidth: true
                            theme: page.theme
                            title: qsTr("CPU Load")
                            value: page.cpuMonitor.available ? qsTr("%1%").arg(page.cpuMonitor.usagePercent.toFixed(1)) : qsTr("Unavailable")
                            subtitle: page.cpuMonitor.temperatureC > 0 ? qsTr("Temperature: %1 C").arg(page.cpuMonitor.temperatureC) : qsTr("Temperature sensor not available")
                            accentColor: page.theme.accentB
                            emphasized: true
                        }

                        StatCard {
                            Layout.fillWidth: true
                            theme: page.theme
                            title: qsTr("GPU Load")
                            value: page.gpuMonitor.available ? qsTr("%1%").arg(page.gpuMonitor.utilizationPercent) : qsTr("Unavailable")
                            subtitle: page.gpuMonitor.available
                                      ? qsTr("VRAM: %1").arg(page.memorySummary(page.gpuMonitor.memoryUsedMiB, page.gpuMonitor.memoryTotalMiB, page.gpuMonitor.memoryUsagePercent))
                                      : qsTr("nvidia-smi output could not be read")
                            accentColor: page.theme.accentA
                        }

                        StatCard {
                            Layout.fillWidth: true
                            theme: page.theme
                            title: qsTr("RAM Usage")
                            value: page.ramMonitor.available ? qsTr("%1%").arg(page.ramMonitor.usagePercent) : qsTr("Unavailable")
                            subtitle: page.ramMonitor.available
                                      ? page.memorySummary(page.ramMonitor.usedMiB, page.ramMonitor.totalMiB, page.ramMonitor.usagePercent)
                                      : qsTr("Memory counters are unavailable")
                            accentColor: page.theme.success
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 18

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    radius: 26
                    color: page.theme.card
                    border.width: 1
                    border.color: page.theme.border
                    implicitHeight: cpuColumn.implicitHeight + 26

                    ColumnLayout {
                        id: cpuColumn
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 12

                        Label {
                            text: qsTr("CPU")
                            font.pixelSize: 21
                            font.bold: true
                            color: page.theme.text
                        }

                        ProgressBar {
                            Layout.fillWidth: true
                            from: 0
                            to: 100
                            value: page.cpuMonitor.usagePercent
                        }

                        Label {
                            text: page.cpuMonitor.available ? qsTr("Usage: %1%").arg(page.cpuMonitor.usagePercent.toFixed(1)) : qsTr("CPU data unavailable")
                            color: page.theme.textMuted
                        }

                        Label {
                            text: page.cpuMonitor.temperatureC > 0 ? qsTr("Temperature: %1 C").arg(page.cpuMonitor.temperatureC) : qsTr("No CPU temperature sensor value")
                            color: page.theme.textMuted
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    radius: 26
                    color: page.theme.card
                    border.width: 1
                    border.color: page.theme.border
                    implicitHeight: gpuColumn.implicitHeight + 26

                    ColumnLayout {
                        id: gpuColumn
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 12

                        Label {
                            text: qsTr("NVIDIA GPU")
                            font.pixelSize: 21
                            font.bold: true
                            color: page.theme.text
                        }

                        ProgressBar {
                            Layout.fillWidth: true
                            from: 0
                            to: 100
                            value: page.gpuMonitor.utilizationPercent
                            visible: page.gpuMonitor.available
                        }

                        Label {
                            text: page.gpuMonitor.available ? (page.gpuMonitor.gpuName.length > 0 ? page.gpuMonitor.gpuName : qsTr("Detected NVIDIA GPU")) : qsTr("NVIDIA GPU data unavailable")
                            wrapMode: Text.Wrap
                            Layout.fillWidth: true
                            color: page.theme.textMuted
                        }

                        Label {
                            text: qsTr("Load: %1%").arg(page.gpuMonitor.utilizationPercent)
                            visible: page.gpuMonitor.available
                            color: page.theme.textMuted
                        }

                        Label {
                            text: qsTr("VRAM: %1").arg(page.memorySummary(page.gpuMonitor.memoryUsedMiB, page.gpuMonitor.memoryTotalMiB, page.gpuMonitor.memoryUsagePercent))
                            visible: page.gpuMonitor.available && page.gpuMonitor.memoryTotalMiB > 0
                            color: page.theme.textMuted
                        }

                        Label {
                            text: qsTr("Temperature: %1 C").arg(page.gpuMonitor.temperatureC)
                            visible: page.gpuMonitor.available && page.gpuMonitor.temperatureC > 0
                            color: page.theme.textMuted
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    radius: 26
                    color: page.theme.card
                    border.width: 1
                    border.color: page.theme.border
                    implicitHeight: ramColumn.implicitHeight + 26

                    ColumnLayout {
                        id: ramColumn
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 12

                        Label {
                            text: qsTr("RAM")
                            font.pixelSize: 21
                            font.bold: true
                            color: page.theme.text
                        }

                        ProgressBar {
                            Layout.fillWidth: true
                            from: 0
                            to: 100
                            value: page.ramMonitor.usagePercent
                        }

                        Label {
                            text: page.ramMonitor.available ? qsTr("Usage: %1").arg(page.memorySummary(page.ramMonitor.usedMiB, page.ramMonitor.totalMiB, page.ramMonitor.usagePercent)) : qsTr("RAM data unavailable")
                            wrapMode: Text.Wrap
                            Layout.fillWidth: true
                            color: page.theme.textMuted
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                radius: 26
                color: page.theme.card
                border.width: 1
                border.color: page.theme.border
                implicitHeight: footerRow.implicitHeight + 24

                RowLayout {
                    id: footerRow
                    anchors.fill: parent
                    anchors.margins: 18
                    spacing: 14

                    Label {
                        text: qsTr("Update interval: %1 ms").arg(page.cpuMonitor.updateInterval)
                        color: page.theme.textMuted
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Button {
                        text: qsTr("Refresh Telemetry")
                        onClicked: {
                            page.cpuMonitor.refresh()
                            page.gpuMonitor.refresh()
                            page.ramMonitor.refresh()
                        }
                    }
                }
            }
        }
    }
}
