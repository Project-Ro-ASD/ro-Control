import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root
    property var systemInfo: null
    property var theme: ({})
    property bool darkMode: false
    property real uiScale: 1.0

    signal rebootRequested()
    signal rebootFailed(string message)

    modal: true
    focus: true
    anchors.centerIn: parent
    width: Math.max(0, Math.min(parent ? parent.width - Math.round(24 * uiScale) : 440, Math.round(440 * uiScale)))
    padding: 0
    header: null
    footer: null

    readonly property color bgColor: theme && theme.card ? theme.card : (darkMode ? "#29233B" : "#FFFFFF")
    readonly property color cardColor: theme && theme.cardStrong ? theme.cardStrong : (darkMode ? "#342D4A" : "#F1F5F9")
    readonly property color borderColor: theme && theme.border ? theme.border : (darkMode ? "#4D436B" : "#CBD5E1")
    readonly property color textColor: theme && theme.text ? theme.text : (darkMode ? "#F8FAFC" : "#0F172A")
    readonly property color softTextColor: theme && theme.textSoft ? theme.textSoft : (darkMode ? "#94A3B8" : "#64748B")
    readonly property color accentColor: theme && theme.accentA ? theme.accentA : (darkMode ? "#818CF8" : "#4F46E5")
    readonly property color warningColor: theme && theme.warning ? theme.warning : (darkMode ? "#FBBF24" : "#D97706")

    background: Rectangle {
        radius: 16
        color: root.cardColor
        border.width: 1
        border.color: root.borderColor
    }

    contentItem: ColumnLayout {
        spacing: 0

        // Header
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: Math.round(60 * root.uiScale)
            color: root.darkMode ? "#3A2E12" : "#FFFBEB"
            radius: 16
            Rectangle {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                height: 16
                color: parent.color
            }
            Rectangle {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                height: 1
                color: root.darkMode ? "#4D3D18" : "#FDE68A"
            }

            RowLayout {
                anchors.fill: parent
                anchors.margins: Math.round(16 * root.uiScale)
                spacing: Math.round(10 * root.uiScale)

                Label {
                    text: "⚠️"
                    font.pixelSize: Math.round(18 * root.uiScale)
                }

                Label {
                    Layout.fillWidth: true
                    text: qsTr("Reboot to UEFI / BIOS")
                    color: root.textColor
                    font.pixelSize: Math.round(15 * root.uiScale)
                    font.weight: Font.Bold
                }
            }
        }

        // Body Content
        ColumnLayout {
            Layout.fillWidth: true
            Layout.margins: Math.round(18 * root.uiScale)
            spacing: Math.round(10 * root.uiScale)

            Label {
                Layout.fillWidth: true
                text: qsTr("Your system will restart immediately and boot directly into the UEFI / BIOS firmware setup utility.")
                color: root.textColor
                font.pixelSize: Math.round(13 * root.uiScale)
                wrapMode: Text.WordWrap
            }

            Label {
                Layout.fillWidth: true
                text: qsTr("Make sure any unsaved work in other applications is saved before continuing.")
                color: root.warningColor
                font.pixelSize: Math.round(12 * root.uiScale)
                font.weight: Font.DemiBold
                wrapMode: Text.WordWrap
            }
        }

        // Footer
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: firmwareButtons.columns === 1
                            ? Math.round(104 * root.uiScale)
                            : Math.round(58 * root.uiScale)
            color: "transparent"

            GridLayout {
                id: firmwareButtons
                anchors.fill: parent
                anchors.leftMargin: Math.round(18 * root.uiScale)
                anchors.rightMargin: Math.round(18 * root.uiScale)
                anchors.bottomMargin: Math.round(14 * root.uiScale)
                columnSpacing: Math.round(10 * root.uiScale)
                rowSpacing: Math.round(8 * root.uiScale)
                columns: root.width < Math.round(340 * root.uiScale) ? 1 : 3

                Item {
                    visible: firmwareButtons.columns > 1
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                Button {
                    id: cancelRebootBtn
                    text: qsTr("Cancel")
                    implicitHeight: Math.round(36 * root.uiScale)
                    implicitWidth: cancelRebootLabel.implicitWidth + Math.round(32 * root.uiScale)
                    Layout.fillWidth: firmwareButtons.columns === 1
                    hoverEnabled: true

                    background: Rectangle {
                        radius: 8
                        color: cancelRebootBtn.down
                               ? (root.darkMode ? "#342A4E" : "#CBD5E1")
                               : (cancelRebootBtn.hovered ? (root.darkMode ? "#3B3156" : "#E2E8F0") : root.bgColor)
                        border.width: 1
                        border.color: cancelRebootBtn.hovered ? root.accentColor : root.borderColor
                    }

                    contentItem: Label {
                        id: cancelRebootLabel
                        text: cancelRebootBtn.text
                        color: cancelRebootBtn.hovered ? root.textColor : root.softTextColor
                        font.pixelSize: Math.round(13 * root.uiScale)
                        font.weight: Font.DemiBold
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: root.close()
                }

                Button {
                    id: confirmRebootBtn
                    text: qsTr("Restart Now ↻")
                    implicitHeight: Math.round(36 * root.uiScale)
                    implicitWidth: confirmRebootLabel.implicitWidth + Math.round(32 * root.uiScale)
                    Layout.fillWidth: firmwareButtons.columns === 1
                    hoverEnabled: true

                    background: Rectangle {
                        radius: 8
                        color: confirmRebootBtn.down
                               ? Qt.darker(root.warningColor, 1.15)
                               : (confirmRebootBtn.hovered ? Qt.lighter(root.warningColor, 1.1) : root.warningColor)
                    }

                    contentItem: Label {
                        id: confirmRebootLabel
                        text: confirmRebootBtn.text
                        color: "#FFFFFF"
                        font.pixelSize: Math.round(13 * root.uiScale)
                        font.weight: Font.Bold
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        root.close();
                        root.rebootRequested();
                        if (root.systemInfo) {
                            const started = root.systemInfo.requestRebootToFirmware();
                            if (!started) {
                                root.rebootFailed(qsTr("Could not start a firmware reboot. Your system may not support this action or authorization was denied."));
                            }
                        }
                    }
                }
            }
        }
    }
}
