import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "." as Components

Popup {
    id: currentDriverPopup
    modal: true
    focus: true

    property var theme: ({})
    property bool darkMode: false
    property real uiScale: 1.0

    signal reinstallRequested()

    readonly property color textColor: theme && theme.text ? theme.text : (darkMode ? "#F8FAFC" : "#0F172A")
    readonly property color softTextColor: theme && theme.textSoft ? theme.textSoft : (darkMode ? "#94A3B8" : "#64748B")
    readonly property color borderColor: theme && theme.border ? theme.border : (darkMode ? "#4D436B" : "#CBD5E1")
    readonly property color bgColor: theme && theme.card ? theme.card : (darkMode ? "#29233B" : "#FFFFFF")

    width: Math.min((parent ? parent.width : 600) - 40, Math.round(520 * uiScale))
    height: Math.min((parent ? parent.height : 800) - 40, currentDriverContent.implicitHeight + topPadding + bottomPadding)
    x: Math.round(((parent ? parent.width : 600) - width) / 2)
    y: Math.round(((parent ? parent.height : 800) - height) / 2)
    padding: 14
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    background: Rectangle {
        radius: 12
        color: currentDriverPopup.bgColor
        border.width: 1
        border.color: currentDriverPopup.borderColor
    }

    contentItem: ScrollView {
        id: currentDriverScroll
        clip: true
        contentWidth: availableWidth

        ColumnLayout {
            id: currentDriverContent
            width: currentDriverScroll.availableWidth
            spacing: 10

            Label {
                Layout.fillWidth: true
                text: qsTr("Driver Is Already Current")
                color: currentDriverPopup.textColor
                font.pixelSize: Math.round(18 * currentDriverPopup.uiScale)
                font.weight: Font.DemiBold
            }

            Label {
                Layout.fillWidth: true
                text: qsTr("The installed NVIDIA driver already matches the latest version available from the configured driver sources. Reinstall only if you want to rebuild the driver packages and kernel module.")
                color: currentDriverPopup.softTextColor
                wrapMode: Text.Wrap
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Components.ModernDialogButton {
                    Layout.fillWidth: true
                    text: qsTr("Cancel")
                    tone: "neutral"
                    theme: currentDriverPopup.theme
                    darkMode: currentDriverPopup.darkMode
                    uiScale: currentDriverPopup.uiScale
                    onClicked: currentDriverPopup.close()
                }

                Components.ModernDialogButton {
                    Layout.fillWidth: true
                    text: qsTr("Reinstall Anyway")
                    tone: "primary"
                    theme: currentDriverPopup.theme
                    darkMode: currentDriverPopup.darkMode
                    uiScale: currentDriverPopup.uiScale
                    onClicked: {
                        currentDriverPopup.close();
                        currentDriverPopup.reinstallRequested();
                    }
                }
            }
        }
    }
}
