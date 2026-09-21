import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: perfCard
    Layout.fillWidth: true
    radius: 14
    color: theme && theme.cardStrong ? theme.cardStrong : (darkMode ? "#342D4A" : "#F1F5F9")
    border.width: 1
    border.color: theme && theme.border ? theme.border : (darkMode ? "#4D436B" : "#CBD5E1")
    implicitHeight: gpuPerfLayout.implicitHeight + 24

    property var theme: ({})
    property bool darkMode: false
    property real uiScale: 1.0

    property var gpuMonitor: null
    property var systemInfo: null

    readonly property color bgColor: theme && theme.card ? theme.card : (darkMode ? "#29233B" : "#FFFFFF")
    readonly property color borderColor: theme && theme.border ? theme.border : (darkMode ? "#4D436B" : "#CBD5E1")
    readonly property color textColor: theme && theme.text ? theme.text : (darkMode ? "#F8FAFC" : "#0F172A")
    readonly property color softTextColor: theme && theme.textSoft ? theme.textSoft : (darkMode ? "#94A3B8" : "#64748B")

    function formatTemp(value) {
        if (value > 0)
            return value + " °C";
        if (systemInfo && systemInfo.virtualMachine)
            return qsTr("VM sensor unavailable");
        return qsTr("Unavailable");
    }

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
                color: perfCard.textColor
                font.pixelSize: Math.round(18 * perfCard.uiScale)
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
                        value: (perfCard.gpuMonitor && perfCard.gpuMonitor.graphicsClockMHz > 0)
                               ? (perfCard.gpuMonitor.graphicsClockMHz + " MHz • " + perfCard.gpuMonitor.memoryClockMHz + " MHz")
                               : qsTr("Dynamic Clock")
                    },
                    {
                        title: qsTr("Power Draw / TDP Limit"),
                        value: (perfCard.gpuMonitor && perfCard.gpuMonitor.powerDrawW > 0)
                               ? (perfCard.gpuMonitor.powerDrawW.toFixed(1) + " W / " + perfCard.gpuMonitor.powerLimitW.toFixed(0) + " W")
                               : qsTr("Dynamic Power")
                    },
                    {
                        title: qsTr("VRAM Allocation"),
                        value: (perfCard.gpuMonitor && perfCard.gpuMonitor.memoryTotalMiB > 0)
                               ? (perfCard.gpuMonitor.memoryUsedMiB + " / " + perfCard.gpuMonitor.memoryTotalMiB + " MiB (" + perfCard.gpuMonitor.memoryUsagePercent + "%)")
                               : qsTr("Unavailable")
                    },
                    {
                        title: qsTr("Thermals & Hotspot"),
                        value: (perfCard.gpuMonitor && perfCard.gpuMonitor.temperatureC > 0)
                               ? (qsTr("Core: %1°C").arg(perfCard.gpuMonitor.temperatureC) +
                                  (perfCard.gpuMonitor.hotspotTemperatureC > 0 ? qsTr(" • Hotspot: %1°C").arg(perfCard.gpuMonitor.hotspotTemperatureC) : "") +
                                  (perfCard.gpuMonitor.memoryTemperatureC > 0 ? qsTr(" • VRAM: %1°C").arg(perfCard.gpuMonitor.memoryTemperatureC) : ""))
                               : (perfCard.gpuMonitor && perfCard.gpuMonitor.temperatureC > 0 ? perfCard.formatTemp(perfCard.gpuMonitor.temperatureC) : qsTr("Unavailable"))
                    }
                ]

                delegate: Rectangle {
                    required property var modelData
                    Layout.fillWidth: true
                    implicitHeight: Math.round(64 * perfCard.uiScale)
                    radius: 10
                    color: perfCard.bgColor
                    border.width: 1
                    border.color: perfCard.borderColor

                    Column {
                        anchors.fill: parent
                        anchors.margins: Math.round(8 * perfCard.uiScale)
                        spacing: Math.round(3 * perfCard.uiScale)

                        Label {
                            width: parent.width
                            text: modelData.title
                            color: perfCard.softTextColor
                            font.pixelSize: Math.round(11 * perfCard.uiScale)
                            font.weight: Font.DemiBold
                            elide: Text.ElideRight
                        }

                        Label {
                            width: parent.width
                            text: modelData.value
                            color: perfCard.textColor
                            font.pixelSize: Math.round(13 * perfCard.uiScale)
                            font.weight: Font.DemiBold
                            elide: Text.ElideRight
                        }
                    }
                }
            }
        }
    }
}
