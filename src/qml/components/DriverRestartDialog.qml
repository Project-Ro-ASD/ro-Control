import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "." as Components

Popup {
    id: restartPopup
    modal: true
    focus: true

    property var theme: ({})
    property bool darkMode: false
    property real uiScale: 1.0
    property bool secureBootEnabled: false

    signal restartRequested()
    signal openMokGuideRequested()

    readonly property color textColor: theme && theme.text ? theme.text : (darkMode ? "#F8FAFC" : "#0F172A")
    readonly property color softTextColor: theme && theme.textSoft ? theme.textSoft : (darkMode ? "#94A3B8" : "#64748B")
    readonly property color borderColor: theme && theme.border ? theme.border : (darkMode ? "#4D436B" : "#CBD5E1")
    readonly property color bgColor: theme && theme.card ? theme.card : (darkMode ? "#29233B" : "#FFFFFF")

    width: Math.min((parent ? parent.width : 600) - 40, Math.round(520 * uiScale))
    height: Math.min((parent ? parent.height : 800) - 40, restartContent.implicitHeight + topPadding + bottomPadding)
    x: Math.round(((parent ? parent.width : 600) - width) / 2)
    y: Math.round(((parent ? parent.height : 800) - height) / 2)
    padding: 14
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    background: Rectangle {
        radius: 12
        color: restartPopup.bgColor
        border.width: 1
        border.color: restartPopup.borderColor
    }

    contentItem: ScrollView {
        id: restartScroll
        clip: true
        contentWidth: availableWidth

        ColumnLayout {
            id: restartContent
            width: restartScroll.availableWidth
            spacing: 10

            Label {
                Layout.fillWidth: true
                text: qsTr("Restart Computer")
                color: restartPopup.textColor
                font.pixelSize: Math.round(18 * restartPopup.uiScale)
                font.weight: Font.DemiBold
            }

            Label {
                Layout.fillWidth: true
                text: qsTr("A driver operation has completed and the computer must restart before the new graphics stack is active.")
                color: restartPopup.softTextColor
                wrapMode: Text.Wrap
            }

            Rectangle {
                visible: restartPopup.secureBootEnabled
                Layout.fillWidth: true
                radius: 8
                color: restartPopup.darkMode ? "#2E2442" : "#F3E8FF"
                border.width: 1
                border.color: restartPopup.darkMode ? "#7C3AED" : "#C084FC"
                implicitHeight: mokNoticeCol.implicitHeight + Math.round(14 * restartPopup.uiScale)

                RowLayout {
                    id: mokNoticeCol
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.margins: Math.round(10 * restartPopup.uiScale)
                    spacing: Math.round(8 * restartPopup.uiScale)

                    Label {
                        text: "🔐"
                        font.pixelSize: Math.round(14 * restartPopup.uiScale)
                    }

                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Secure Boot active: If prompted on restart, complete the one-time MOK enrollment to authenticate the driver.")
                        color: restartPopup.darkMode ? "#E9D5FF" : "#581C87"
                        font.pixelSize: Math.round(11 * restartPopup.uiScale)
                        wrapMode: Text.Wrap
                    }

                    Components.ModernMiniButton {
                        text: qsTr("MOK Guide")
                        tone: "neutral"
                        theme: restartPopup.theme
                        darkMode: restartPopup.darkMode
                        uiScale: restartPopup.uiScale
                        onClicked: {
                            restartPopup.close();
                            restartPopup.openMokGuideRequested();
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Components.ModernDialogButton {
                    Layout.fillWidth: true
                    text: qsTr("Cancel")
                    tone: "neutral"
                    theme: restartPopup.theme
                    darkMode: restartPopup.darkMode
                    uiScale: restartPopup.uiScale
                    onClicked: restartPopup.close()
                }

                Components.ModernDialogButton {
                    Layout.fillWidth: true
                    text: qsTr("Restart Now")
                    tone: "danger"
                    theme: restartPopup.theme
                    darkMode: restartPopup.darkMode
                    uiScale: restartPopup.uiScale
                    onClicked: {
                        restartPopup.close();
                        restartPopup.restartRequested();
                    }
                }
            }
        }
    }
}
