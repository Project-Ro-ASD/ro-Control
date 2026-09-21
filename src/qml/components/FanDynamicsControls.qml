import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

RowLayout {
    id: root
    Layout.fillWidth: true
    spacing: Math.round(12 * root.uiScale)
    visible: root.hasControllableFan || root.supportsBatteryFanSync

    property var fanController: null
    property var systemInfo: null
    property var theme: ({})
    property bool darkMode: false
    property real uiScale: 1.0

    property bool hasControllableFan: false
    property bool supportsBatteryFanSync: false

    readonly property color bgColor: theme && theme.card ? theme.card : (darkMode ? "#29233B" : "#FFFFFF")
    readonly property color borderColor: theme && theme.border ? theme.border : (darkMode ? "#4D436B" : "#CBD5E1")
    readonly property color textColor: theme && theme.text ? theme.text : (darkMode ? "#F8FAFC" : "#0F172A")
    readonly property color softTextColor: theme && theme.textSoft ? theme.textSoft : (darkMode ? "#94A3B8" : "#64748B")
    readonly property color accentColor: theme && theme.accentA ? theme.accentA : (darkMode ? "#818CF8" : "#4F46E5")

    Rectangle {
        Layout.fillWidth: true
        Layout.preferredWidth: 1
        visible: root.hasControllableFan
        implicitHeight: Math.round(58 * root.uiScale)
        radius: 10
        color: root.bgColor
        border.width: 1
        border.color: root.borderColor

        RowLayout {
            anchors.fill: parent
            anchors.margins: Math.round(12 * root.uiScale)
            spacing: 10

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 1

                Label {
                    text: qsTr("Response Smoothing")
                    color: root.textColor
                    font.pixelSize: Math.round(13 * root.uiScale)
                    font.weight: Font.DemiBold
                }

                Label {
                    text: !root.fanController ? ""
                          : (root.fanController.smoothingEnabled
                             ? qsTr("Active — GPU changes are rate-limited (%1°C hysteresis).").arg(root.fanController.hysteresisTempC)
                             : qsTr("Disabled — GPU changes apply immediately."))
                    color: root.softTextColor
                    font.pixelSize: Math.round(11 * root.uiScale)
                }
            }

            Switch {
                id: smoothingSwitch
                checked: root.fanController ? root.fanController.smoothingEnabled : false
                implicitWidth: Math.round(44 * root.uiScale)
                implicitHeight: Math.round(24 * root.uiScale)

                indicator: Rectangle {
                    implicitWidth: Math.round(44 * root.uiScale)
                    implicitHeight: Math.round(24 * root.uiScale)
                    radius: Math.round(12 * root.uiScale)
                    color: smoothingSwitch.checked ? root.accentColor : (root.darkMode ? "#342D4A" : "#CBD5E1")
                    border.width: 1
                    border.color: smoothingSwitch.checked ? root.accentColor : root.borderColor

                    Rectangle {
                        x: smoothingSwitch.checked ? (parent.width - width - 2) : 2
                        y: (parent.height - height) / 2
                        width: Math.round(20 * root.uiScale)
                        height: Math.round(20 * root.uiScale)
                        radius: Math.round(10 * root.uiScale)
                        color: "#FFFFFF"

                        Behavior on x { NumberAnimation { duration: 150; easing.type: Easing.InOutQuad } }
                    }
                }

                onToggled: {
                    if (root.fanController)
                        root.fanController.setSmoothingEnabled(checked);
                }
            }
        }
    }

    Rectangle {
        Layout.fillWidth: true
        Layout.preferredWidth: 1
        visible: root.supportsBatteryFanSync
        implicitHeight: Math.round(58 * root.uiScale)
        radius: 10
        color: root.bgColor
        border.width: 1
        border.color: root.borderColor

        RowLayout {
            anchors.fill: parent
            anchors.margins: Math.round(12 * root.uiScale)
            spacing: 10

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 1

                Label {
                    text: qsTr("Battery Profile Sync")
                    color: root.textColor
                    font.pixelSize: Math.round(13 * root.uiScale)
                    font.weight: Font.DemiBold
                }

                Label {
                    text: !root.fanController ? ""
                          : (!root.fanController.batteryProfileSyncEnabled
                             ? qsTr("Disabled — does not alter fan profiles on battery.")
                             : (root.systemInfo && root.systemInfo.onBattery
                                ? qsTr("Active — controllable GPU fan uses Silent on battery.")
                                : qsTr("Armed — uses Silent when battery power begins.")))
                    color: root.softTextColor
                    font.pixelSize: Math.round(11 * root.uiScale)
                }
            }

            Switch {
                id: batterySyncSwitch
                checked: root.fanController ? root.fanController.batteryProfileSyncEnabled : false
                implicitWidth: Math.round(44 * root.uiScale)
                implicitHeight: Math.round(24 * root.uiScale)

                indicator: Rectangle {
                    implicitWidth: Math.round(44 * root.uiScale)
                    implicitHeight: Math.round(24 * root.uiScale)
                    radius: Math.round(12 * root.uiScale)
                    color: batterySyncSwitch.checked ? root.accentColor : (root.darkMode ? "#342D4A" : "#CBD5E1")
                    border.width: 1
                    border.color: batterySyncSwitch.checked ? root.accentColor : root.borderColor

                    Rectangle {
                        x: batterySyncSwitch.checked ? (parent.width - width - 2) : 2
                        y: (parent.height - height) / 2
                        width: Math.round(20 * root.uiScale)
                        height: Math.round(20 * root.uiScale)
                        radius: Math.round(10 * root.uiScale)
                        color: "#FFFFFF"

                        Behavior on x { NumberAnimation { duration: 150; easing.type: Easing.InOutQuad } }
                    }
                }

                onToggled: {
                    if (root.fanController)
                        root.fanController.batteryProfileSyncEnabled = checked;
                }
            }
        }
    }
}
