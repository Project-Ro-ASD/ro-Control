import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "." as Components

Popup {
    id: licensePopup
    modal: true
    focus: true

    property var theme: ({})
    property bool darkMode: false
    property real uiScale: 1.0
    property string licenseText: ""

    signal accepted()
    signal rejected()

    readonly property color textColor: theme && theme.text ? theme.text : (darkMode ? "#F8FAFC" : "#0F172A")
    readonly property color borderColor: theme && theme.border ? theme.border : (darkMode ? "#4D436B" : "#CBD5E1")
    readonly property color cardColor: theme && theme.cardStrong ? theme.cardStrong : (darkMode ? "#342D4A" : "#F1F5F9")
    readonly property color bgColor: theme && theme.card ? theme.card : (darkMode ? "#29233B" : "#FFFFFF")
    readonly property color infoBg: theme && theme.infoBg ? theme.infoBg : (darkMode ? "#1E2548" : "#EFF6FF")
    readonly property color dangerColor: theme && theme.danger ? theme.danger : (darkMode ? "#F87171" : "#EF4444")

    width: Math.min((parent ? parent.width : 700) - 40, Math.round(640 * uiScale))
    height: Math.min((parent ? parent.height : 800) - 80, Math.round(500 * uiScale))
    x: Math.round(((parent ? parent.width : 700) - width) / 2)
    y: Math.round(((parent ? parent.height : 800) - height) / 2)
    padding: 14
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    background: Rectangle {
        radius: 12
        color: licensePopup.bgColor
        border.width: 1
        border.color: licensePopup.borderColor
    }

    contentItem: ColumnLayout {
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Label {
                Layout.fillWidth: true
                text: qsTr("NVIDIA License Review")
                color: licensePopup.textColor
                font.pixelSize: Math.round(18 * licensePopup.uiScale)
                font.weight: Font.DemiBold
            }

            ToolButton {
                id: closeLicenseButton
                implicitWidth: Math.round(34 * licensePopup.uiScale)
                implicitHeight: Math.round(34 * licensePopup.uiScale)
                icon.name: "window-close"
                icon.width: Math.round(16 * licensePopup.uiScale)
                icon.height: Math.round(16 * licensePopup.uiScale)
                icon.color: licensePopup.textColor
                display: AbstractButton.IconOnly

                ToolTip {
                    id: closeTip
                    visible: closeLicenseButton.hovered
                    text: qsTr("Close")
                    delay: 300
                    timeout: 5000
                    topPadding: Math.round(6 * licensePopup.uiScale)
                    bottomPadding: Math.round(6 * licensePopup.uiScale)
                    leftPadding: Math.round(12 * licensePopup.uiScale)
                    rightPadding: Math.round(12 * licensePopup.uiScale)

                    contentItem: Label {
                        text: closeTip.text
                        color: licensePopup.textColor
                        font.pixelSize: Math.round(11 * licensePopup.uiScale)
                        font.weight: Font.Medium
                    }

                    background: Rectangle {
                        radius: 8
                        color: licensePopup.darkMode ? "#241E34" : "#FFFFFF"
                        border.width: 1
                        border.color: licensePopup.darkMode ? "#4D436B" : "#CBD5E1"

                        Rectangle {
                            anchors.left: parent.left
                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                            anchors.margins: 3
                            width: 3
                            radius: 1.5
                            color: licensePopup.dangerColor
                        }
                    }
                }
                onClicked: {
                    licensePopup.close();
                    licensePopup.rejected();
                }

                background: Rectangle {
                    radius: width / 2
                    color: closeLicenseButton.down ? licensePopup.infoBg : licensePopup.bgColor
                    border.width: 1
                    border.color: licensePopup.borderColor
                }
            }
        }

        TextArea {
            Layout.fillWidth: true
            Layout.fillHeight: true
            readOnly: true
            wrapMode: Text.Wrap
            text: licensePopup.licenseText
            color: licensePopup.textColor
            background: Rectangle {
                radius: 8
                color: licensePopup.cardColor
                border.width: 1
                border.color: licensePopup.borderColor
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Components.ModernDialogButton {
                Layout.fillWidth: true
                text: qsTr("Reject")
                tone: "neutral"
                theme: licensePopup.theme
                darkMode: licensePopup.darkMode
                uiScale: licensePopup.uiScale
                onClicked: {
                    licensePopup.close();
                    licensePopup.rejected();
                }
            }

            Components.ModernDialogButton {
                Layout.fillWidth: true
                text: qsTr("Accept")
                tone: "success"
                theme: licensePopup.theme
                darkMode: licensePopup.darkMode
                uiScale: licensePopup.uiScale
                onClicked: {
                    licensePopup.close();
                    licensePopup.accepted();
                }
            }
        }
    }
}
