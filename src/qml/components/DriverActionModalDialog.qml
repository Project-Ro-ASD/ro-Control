import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "." as Components

Popup {
    id: driverActionModalPopup
    property string actionKey: ""
    property string actionTitle: ""
    property string actionSubtitle: ""
    property color actionAccentColor: "#10B981"
    property string actionDescription: ""
    property var actionPoints: []
    property string actionWarning: ""
    property string actionConfirmText: qsTr("Proceed")
    property string actionConfirmTone: "primary"
    property bool secureBootEnabled: false
    property bool secureBootAcknowledged: false

    property var theme: ({})
    property bool darkMode: false
    property real uiScale: 1.0

    readonly property bool secureBootAcknowledgementRequired: secureBootEnabled
                                                        && (actionKey === "closed" || actionKey === "open" || actionKey === "update")

    signal actionConfirmed(string key)

    readonly property color textColor: theme && theme.text ? theme.text : (darkMode ? "#F8FAFC" : "#0F172A")
    readonly property color softTextColor: theme && theme.textSoft ? theme.textSoft : (darkMode ? "#94A3B8" : "#64748B")
    readonly property color borderColor: theme && theme.border ? theme.border : (darkMode ? "#4D436B" : "#CBD5E1")
    readonly property color bgColor: theme && theme.card ? theme.card : (darkMode ? "#29233B" : "#FFFFFF")
    readonly property color warningBg: theme && theme.warningBg ? theme.warningBg : (darkMode ? "#3A2E12" : "#FFFBEB")
    readonly property color warningColor: theme && theme.warning ? theme.warning : (darkMode ? "#FBBF24" : "#D97706")

    modal: true
    focus: true
    width: Math.min((parent ? parent.width : 650) - 40, Math.round(580 * uiScale))
    height: Math.min((parent ? parent.height : 800) - 40, driverActionContent.implicitHeight + topPadding + bottomPadding)
    x: Math.round(((parent ? parent.width : 650) - width) / 2)
    y: Math.round(((parent ? parent.height : 800) - height) / 2)
    padding: Math.round(18 * uiScale)
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    onOpened: secureBootAcknowledged = false

    background: Rectangle {
        radius: Math.round(14 * driverActionModalPopup.uiScale)
        color: driverActionModalPopup.bgColor
        border.width: 1
        border.color: driverActionModalPopup.borderColor
    }

    contentItem: ScrollView {
        id: driverActionScroll
        clip: true
        contentWidth: availableWidth

        ColumnLayout {
            id: driverActionContent
            width: driverActionScroll.availableWidth
            spacing: Math.round(12 * driverActionModalPopup.uiScale)

            RowLayout {
                Layout.fillWidth: true
                spacing: Math.round(10 * driverActionModalPopup.uiScale)

                Rectangle {
                    width: Math.round(4 * driverActionModalPopup.uiScale)
                    height: Math.round(28 * driverActionModalPopup.uiScale)
                    radius: 2
                    color: driverActionModalPopup.actionAccentColor
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    Label {
                        Layout.fillWidth: true
                        text: driverActionModalPopup.actionTitle
                        color: driverActionModalPopup.textColor
                        font.pixelSize: Math.round(17 * driverActionModalPopup.uiScale)
                        font.weight: Font.DemiBold
                    }

                    Label {
                        Layout.fillWidth: true
                        text: driverActionModalPopup.actionSubtitle
                        color: driverActionModalPopup.softTextColor
                        font.pixelSize: Math.round(11 * driverActionModalPopup.uiScale)
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: driverActionModalPopup.borderColor
            }

            CheckBox {
                visible: driverActionModalPopup.secureBootAcknowledgementRequired
                Layout.fillWidth: true
                text: qsTr("I have completed the required akmods MOK key enrollment and understand that the NVIDIA module will not load without it.")
                checked: driverActionModalPopup.secureBootAcknowledged
                onToggled: driverActionModalPopup.secureBootAcknowledged = checked
            }

            Label {
                Layout.fillWidth: true
                text: driverActionModalPopup.actionDescription
                color: driverActionModalPopup.textColor
                font.pixelSize: Math.round(13 * driverActionModalPopup.uiScale)
                wrapMode: Text.Wrap
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: Math.round(6 * driverActionModalPopup.uiScale)

                Repeater {
                    model: driverActionModalPopup.actionPoints

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Math.round(8 * driverActionModalPopup.uiScale)

                        Label {
                            text: "•"
                            color: driverActionModalPopup.actionAccentColor
                            font.pixelSize: Math.round(14 * driverActionModalPopup.uiScale)
                            font.weight: Font.Bold
                            Layout.alignment: Qt.AlignTop
                        }

                        Label {
                            Layout.fillWidth: true
                            text: modelData
                            color: driverActionModalPopup.softTextColor
                            font.pixelSize: Math.round(12 * driverActionModalPopup.uiScale)
                            wrapMode: Text.Wrap
                        }
                    }
                }
            }

            Rectangle {
                visible: driverActionModalPopup.actionWarning.length > 0
                Layout.fillWidth: true
                radius: 10
                color: driverActionModalPopup.warningBg
                border.width: 1
                border.color: driverActionModalPopup.warningColor
                implicitHeight: warningCol.implicitHeight + Math.round(20 * driverActionModalPopup.uiScale)

                RowLayout {
                    id: warningCol
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.margins: Math.round(12 * driverActionModalPopup.uiScale)
                    spacing: Math.round(10 * driverActionModalPopup.uiScale)

                    Label {
                        text: "⚠️"
                        font.pixelSize: Math.round(16 * driverActionModalPopup.uiScale)
                        Layout.alignment: Qt.AlignTop
                    }

                    Label {
                        Layout.fillWidth: true
                        text: driverActionModalPopup.actionWarning
                        color: driverActionModalPopup.warningColor
                        font.pixelSize: Math.round(12 * driverActionModalPopup.uiScale)
                        font.weight: Font.Medium
                        wrapMode: Text.Wrap
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: Math.round(10 * driverActionModalPopup.uiScale)

                Components.ModernDialogButton {
                    Layout.fillWidth: true
                    text: qsTr("Cancel")
                    tone: "neutral"
                    theme: driverActionModalPopup.theme
                    darkMode: driverActionModalPopup.darkMode
                    uiScale: driverActionModalPopup.uiScale
                    onClicked: driverActionModalPopup.close()
                }

                Components.ModernDialogButton {
                    Layout.fillWidth: true
                    text: driverActionModalPopup.actionConfirmText
                    tone: driverActionModalPopup.actionConfirmTone
                    theme: driverActionModalPopup.theme
                    darkMode: driverActionModalPopup.darkMode
                    uiScale: driverActionModalPopup.uiScale
                    enabled: !driverActionModalPopup.secureBootAcknowledgementRequired || driverActionModalPopup.secureBootAcknowledged
                    onClicked: {
                        const key = driverActionModalPopup.actionKey;
                        driverActionModalPopup.close();
                        driverActionModalPopup.actionConfirmed(key);
                    }
                }
            }
        }
    }
}
