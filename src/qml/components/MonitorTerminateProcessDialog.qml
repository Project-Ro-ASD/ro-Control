import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
    id: terminateProcessPopup
    modal: true
    focus: true
    anchors.centerIn: parent
    width: Math.min(parent ? parent.width - 40 : 420, Math.round(420 * uiScale))
    padding: Math.round(18 * uiScale)
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    property var theme: ({})
    property bool darkMode: false
    property real uiScale: 1.0
    property var gpuMonitor: null
    property int targetPid: -1
    property string targetName: ""
    property string terminationError: ""

    signal processTerminated()

    readonly property color cardColor: theme && theme.cardStrong ? theme.cardStrong : (darkMode ? "#342D4A" : "#F1F5F9")
    readonly property color borderColor: theme && theme.border ? theme.border : (darkMode ? "#4D436B" : "#CBD5E1")
    readonly property color textColor: theme && theme.text ? theme.text : (darkMode ? "#F8FAFC" : "#0F172A")
    readonly property color softTextColor: theme && theme.textSoft ? theme.textSoft : (darkMode ? "#94A3B8" : "#64748B")

    background: Rectangle {
        radius: 12
        color: terminateProcessPopup.cardColor
        border.width: 1
        border.color: terminateProcessPopup.borderColor
    }

    contentItem: ColumnLayout {
        spacing: 12
        Label {
            Layout.fillWidth: true
            text: qsTr("End GPU process?")
            color: terminateProcessPopup.textColor
            font.pixelSize: Math.round(17 * terminateProcessPopup.uiScale)
            font.weight: Font.DemiBold
        }
        Label {
            Layout.fillWidth: true
            text: qsTr("%1 (PID %2) will be terminated. Unsaved work may be lost.")
                  .arg(terminateProcessPopup.targetName).arg(terminateProcessPopup.targetPid)
            color: terminateProcessPopup.softTextColor
            wrapMode: Text.WordWrap
            font.pixelSize: Math.round(12 * terminateProcessPopup.uiScale)
        }
        Label {
            visible: terminateProcessPopup.terminationError.length > 0
            Layout.fillWidth: true
            text: terminateProcessPopup.terminationError
            color: terminateProcessPopup.theme && terminateProcessPopup.theme.danger ? terminateProcessPopup.theme.danger : "#DC2626"
            wrapMode: Text.WordWrap
            font.pixelSize: Math.round(11 * terminateProcessPopup.uiScale)
        }
        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            ActionButton {
                text: qsTr("Cancel")
                theme: terminateProcessPopup.theme
                compact: true
                uiScale: terminateProcessPopup.uiScale
                onClicked: terminateProcessPopup.close()
            }
            ActionButton {
                text: qsTr("End process")
                enabled: terminateProcessPopup.targetPid > 0
                theme: terminateProcessPopup.theme
                tone: "danger"
                compact: true
                uiScale: terminateProcessPopup.uiScale
                onClicked: {
                    terminateProcessPopup.terminationError = "";
                    if (terminateProcessPopup.gpuMonitor) {
                        const success = terminateProcessPopup.gpuMonitor.killProcess(terminateProcessPopup.targetPid);
                        if (!success) {
                            terminateProcessPopup.terminationError = terminateProcessPopup.gpuMonitor.statusMessage;
                        } else {
                            terminateProcessPopup.processTerminated();
                            terminateProcessPopup.close();
                        }
                    }
                }
            }
        }
    }
}
