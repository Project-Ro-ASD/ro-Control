import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 12

        Label {
            text: "Sistem Izleme"
            font.pixelSize: 24
            font.bold: true
        }

        Rectangle {
            Layout.fillWidth: true
            radius: 10
            border.width: 1
            border.color: "#5f6b86"
            color: "transparent"
            implicitHeight: cpuCol.implicitHeight + 18

            ColumnLayout {
                id: cpuCol
                anchors.fill: parent
                anchors.margins: 10
                spacing: 6

                Label {
                    text: "CPU"
                    font.bold: true
                }

                Label {
                    text: cpuMonitor.available ? "Kullanim: " + cpuMonitor.usagePercent.toFixed(1) + "%" : "CPU verisi alinamiyor"
                }

                ProgressBar {
                    from: 0
                    to: 100
                    value: cpuMonitor.usagePercent
                    Layout.fillWidth: true
                }

                Label {
                    text: "Sicaklik: " + cpuMonitor.temperatureC + " C"
                    visible: cpuMonitor.temperatureC > 0
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            radius: 10
            border.width: 1
            border.color: "#5f6b86"
            color: "transparent"
            implicitHeight: gpuCol.implicitHeight + 18

            ColumnLayout {
                id: gpuCol
                anchors.fill: parent
                anchors.margins: 10
                spacing: 6

                Label {
                    text: "GPU (NVIDIA)"
                    font.bold: true
                }

                Label {
                    text: gpuMonitor.available ? (gpuMonitor.gpuName.length > 0 ? gpuMonitor.gpuName : "NVIDIA GPU") : "nvidia-smi ile veri alinamadi"
                }

                Label {
                    text: "Yuk: " + gpuMonitor.utilizationPercent + "%"
                    visible: gpuMonitor.available
                }

                ProgressBar {
                    from: 0
                    to: 100
                    value: gpuMonitor.utilizationPercent
                    Layout.fillWidth: true
                    visible: gpuMonitor.available
                }

                Label {
                    text: "VRAM: " + gpuMonitor.memoryUsedMiB + " / " + gpuMonitor.memoryTotalMiB + " MiB (" + gpuMonitor.memoryUsagePercent + "%)"
                    visible: gpuMonitor.available && gpuMonitor.memoryTotalMiB > 0
                }

                Label {
                    text: "Sicaklik: " + gpuMonitor.temperatureC + " C"
                    visible: gpuMonitor.available && gpuMonitor.temperatureC > 0
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            radius: 10
            border.width: 1
            border.color: "#5f6b86"
            color: "transparent"
            implicitHeight: ramCol.implicitHeight + 18

            ColumnLayout {
                id: ramCol
                anchors.fill: parent
                anchors.margins: 10
                spacing: 6

                Label {
                    text: "RAM"
                    font.bold: true
                }

                Label {
                    text: ramMonitor.available ? "Kullanim: " + ramMonitor.usedMiB + " / " + ramMonitor.totalMiB + " MiB (" + ramMonitor.usagePercent + "%)" : "RAM verisi alinamiyor"
                }

                ProgressBar {
                    from: 0
                    to: 100
                    value: ramMonitor.usagePercent
                    Layout.fillWidth: true
                }
            }
        }

        RowLayout {
            spacing: 8

            Button {
                text: "Yenile"
                onClicked: {
                    cpuMonitor.refresh();
                    gpuMonitor.refresh();
                    ramMonitor.refresh();
                }
            }

            Label {
                text: "Guncelleme araligi: " + cpuMonitor.updateInterval + " ms"
                color: "#6d7384"
            }
        }
    }
}
