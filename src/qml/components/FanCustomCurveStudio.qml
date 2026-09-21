import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    Layout.fillWidth: true
    radius: 12
    color: bgColor
    border.width: 1
    border.color: borderColor
    implicitHeight: customSummaryLayout.implicitHeight + Math.round(24 * root.uiScale)

    property var fanController: null
    property var theme: ({})
    property bool darkMode: false
    property real uiScale: 1.0

    signal openStudioRequested()
    signal applyPresetRequested(string presetId)

    readonly property color bgColor: theme && theme.card ? theme.card : (darkMode ? "#29233B" : "#FFFFFF")
    readonly property color cardColor: theme && theme.cardStrong ? theme.cardStrong : (darkMode ? "#342D4A" : "#F1F5F9")
    readonly property color borderColor: theme && theme.border ? theme.border : (darkMode ? "#4D436B" : "#CBD5E1")
    readonly property color textColor: theme && theme.text ? theme.text : (darkMode ? "#F8FAFC" : "#0F172A")
    readonly property color softTextColor: theme && theme.textSoft ? theme.textSoft : (darkMode ? "#94A3B8" : "#64748B")
    readonly property color accentColor: theme && theme.accentA ? theme.accentA : (darkMode ? "#818CF8" : "#4F46E5")
    readonly property color warningText: theme && theme.warning ? theme.warning : (darkMode ? "#FBBF24" : "#D97706")

    ColumnLayout {
        id: customSummaryLayout
        anchors.fill: parent
        anchors.margins: Math.round(14 * root.uiScale)
        spacing: Math.round(12 * root.uiScale)

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 1

                Label {
                    text: qsTr("Custom Fan Curve Dynamics & Control Points")
                    color: root.textColor
                    font.pixelSize: Math.round(14 * root.uiScale)
                    font.weight: Font.DemiBold
                }

                Label {
                    text: qsTr("Multi-point linear temperature ramp curve mapped to cooling PWM controllers.")
                    color: root.softTextColor
                    font.pixelSize: Math.round(11 * root.uiScale)
                }
            }

            Button {
                id: configureStudioBtn
                text: qsTr("Open Curve Studio & Live Tuner ↗")
                implicitHeight: Math.round(34 * root.uiScale)
                hoverEnabled: true

                background: Rectangle {
                    radius: 8
                    color: configureStudioBtn.hovered ? (root.darkMode ? "#3B3156" : "#EDE9FE") : (root.darkMode ? "#2E2442" : "#F3E8FF")
                    border.width: 1
                    border.color: root.accentColor
                }

                contentItem: Text {
                    text: configureStudioBtn.text
                    color: root.accentColor
                    font.pixelSize: Math.round(11 * root.uiScale)
                    font.weight: Font.DemiBold
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: root.openStudioRequested()
            }
        }

        // 4 Equal Sized Control Point Cards
        GridLayout {
            Layout.fillWidth: true
            columns: width > 640 ? 4 : 2
            columnSpacing: Math.round(8 * root.uiScale)
            rowSpacing: Math.round(8 * root.uiScale)

            Repeater {
                model: 4

                delegate: Rectangle {
                    id: ptCard
                    required property int index
                    readonly property var ptData: {
                        var pts = root.fanController ? root.fanController.customCurvePoints : [];
                        if (pts && pts.length > ptCard.index)
                            return pts[ptCard.index];
                        return { temp: 40 + ptCard.index * 15, speed: 30 + ptCard.index * 20 };
                    }

                    Layout.fillWidth: true
                    implicitHeight: Math.round(64 * root.uiScale)
                    radius: 8
                    color: root.cardColor
                    border.width: 1
                    border.color: root.borderColor

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: Math.round(8 * root.uiScale)
                        spacing: 4

                        RowLayout {
                            Layout.fillWidth: true

                            Rectangle {
                                implicitWidth: Math.round(20 * root.uiScale)
                                implicitHeight: Math.round(18 * root.uiScale)
                                radius: 4
                                color: root.darkMode ? "#221C30" : "#E2E8F0"

                                Label {
                                    anchors.centerIn: parent
                                    text: "#" + (ptCard.index + 1)
                                    color: root.accentColor
                                    font.pixelSize: Math.round(10 * root.uiScale)
                                    font.weight: Font.Bold
                                }
                            }

                            Label {
                                text: (ptCard.ptData.temp || 0) + " °C"
                                color: root.textColor
                                font.pixelSize: Math.round(12 * root.uiScale)
                                font.weight: Font.DemiBold
                            }

                            Item { Layout.fillWidth: true }

                            Label {
                                text: (ptCard.ptData.speed || 0) + "%"
                                color: root.accentColor
                                font.pixelSize: Math.round(13 * root.uiScale)
                                font.weight: Font.Bold
                            }
                        }

                        // Mini Speed Progress Bar
                        Rectangle {
                            Layout.fillWidth: true
                            implicitHeight: Math.round(6 * root.uiScale)
                            radius: 3
                            color: root.darkMode ? "#1E2238" : "#E2E8F0"
                            clip: true

                            Rectangle {
                                anchors.left: parent.left
                                anchors.top: parent.top
                                anchors.bottom: parent.bottom
                                width: parent.width * Math.min(1.0, Math.max(0.0, (ptCard.ptData.speed || 0) / 100.0))
                                radius: 3
                                color: {
                                    var s = ptCard.ptData.speed || 0;
                                    if (s > 80) return root.warningText;
                                    if (s > 50) return root.accentColor;
                                    return root.darkMode ? "#34D399" : "#10B981";
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
            spacing: Math.round(8 * root.uiScale)

            Label {
                text: qsTr("Curve Presets:")
                color: root.softTextColor
                font.pixelSize: Math.round(11 * root.uiScale)
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
                    implicitHeight: Math.round(28 * root.uiScale)
                    leftPadding: Math.round(10 * root.uiScale)
                    rightPadding: Math.round(10 * root.uiScale)
                    hoverEnabled: true

                    background: Rectangle {
                        radius: 6
                        color: presetBtn.hovered ? (root.darkMode ? "#3B3156" : "#E2E8F0") : root.cardColor
                        border.width: 1
                        border.color: presetBtn.hovered ? root.accentColor : root.borderColor
                    }

                    contentItem: Text {
                        text: presetBtn.text
                        color: presetBtn.hovered ? root.accentColor : root.textColor
                        font.pixelSize: Math.round(11 * root.uiScale)
                        font.weight: Font.DemiBold
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: root.applyPresetRequested(presetBtn.modelData.id)
                }
            }
        }
    }
}
