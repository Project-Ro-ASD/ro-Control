import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "." as Components

Popup {
    id: sourceSwitchBlockedPopup
    property string requestedTarget: "closed"
    modal: true
    focus: true

    property var theme: ({})
    property bool darkMode: false
    property real uiScale: 1.0

    property bool canManageDriverStack: false
    property bool driverInstalledLocally: false
    property bool operationRunning: false

    signal deepCleanRequested()

    readonly property color textColor: theme && theme.text ? theme.text : (darkMode ? "#F8FAFC" : "#0F172A")
    readonly property color softTextColor: theme && theme.textSoft ? theme.textSoft : (darkMode ? "#94A3B8" : "#64748B")
    readonly property color borderColor: theme && theme.border ? theme.border : (darkMode ? "#4D436B" : "#CBD5E1")
    readonly property color bgColor: theme && theme.card ? theme.card : (darkMode ? "#29233B" : "#FFFFFF")

    width: Math.min((parent ? parent.width : 600) - 40, Math.round(540 * uiScale))
    height: Math.min((parent ? parent.height : 800) - 40, sourceSwitchContent.implicitHeight + topPadding + bottomPadding)
    x: Math.round(((parent ? parent.width : 600) - width) / 2)
    y: Math.round(((parent ? parent.height : 800) - height) / 2)
    padding: 14
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    background: Rectangle {
        radius: 12
        color: sourceSwitchBlockedPopup.bgColor
        border.width: 1
        border.color: sourceSwitchBlockedPopup.borderColor
    }

    contentItem: ScrollView {
        id: sourceSwitchScroll
        clip: true
        contentWidth: availableWidth

        ColumnLayout {
            id: sourceSwitchContent
            width: sourceSwitchScroll.availableWidth
            spacing: 10

            Label {
                Layout.fillWidth: true
                text: qsTr("Deep Clean Required")
                color: sourceSwitchBlockedPopup.textColor
                font.pixelSize: Math.round(18 * sourceSwitchBlockedPopup.uiScale)
                font.weight: Font.DemiBold
            }

            Label {
                Layout.fillWidth: true
                text: sourceSwitchBlockedPopup.requestedTarget === "closed"
                      ? qsTr("NVIDIA Open Kernel Modules are currently detected. Run Deep Clean before installing the proprietary NVIDIA module.")
                      : qsTr("The proprietary NVIDIA module is currently detected. Run Deep Clean before installing NVIDIA Open Kernel Modules.")
                color: sourceSwitchBlockedPopup.softTextColor
                wrapMode: Text.Wrap
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Components.ModernDialogButton {
                    Layout.fillWidth: true
                    text: qsTr("Cancel")
                    tone: "neutral"
                    theme: sourceSwitchBlockedPopup.theme
                    darkMode: sourceSwitchBlockedPopup.darkMode
                    uiScale: sourceSwitchBlockedPopup.uiScale
                    onClicked: sourceSwitchBlockedPopup.close()
                }

                Components.ModernDialogButton {
                    Layout.fillWidth: true
                    text: qsTr("Deep Clean")
                    tone: "primary"
                    theme: sourceSwitchBlockedPopup.theme
                    darkMode: sourceSwitchBlockedPopup.darkMode
                    uiScale: sourceSwitchBlockedPopup.uiScale
                    enabled: sourceSwitchBlockedPopup.canManageDriverStack && sourceSwitchBlockedPopup.driverInstalledLocally && !sourceSwitchBlockedPopup.operationRunning
                    onClicked: {
                        sourceSwitchBlockedPopup.close();
                        sourceSwitchBlockedPopup.deepCleanRequested();
                    }
                }
            }
        }
    }
}
