import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "." as Components

Rectangle {
    id: root
    Layout.fillWidth: true
    radius: 10
    color: cardColor
    border.width: 1
    border.color: borderColor
    implicitHeight: Math.round((root.activityExpanded ? 320 : 78) * root.uiScale)
    Layout.preferredHeight: implicitHeight
    Layout.maximumHeight: Math.round(340 * root.uiScale)

    property var theme: ({})
    property bool darkMode: false
    property real uiScale: 1.0

    property bool activityExpanded: false
    property bool activityFollowTail: true
    property bool operationRunning: false
    property string lastOperationText: ""
    property string lastOperationTone: "info"

    readonly property string logText: activityLog.text

    signal cancelRequested()

    readonly property color textColor: theme && theme.text ? theme.text : (darkMode ? "#F8FAFC" : "#0F172A")
    readonly property color softTextColor: theme && theme.textSoft ? theme.textSoft : (darkMode ? "#94A3B8" : "#64748B")
    readonly property color borderColor: theme && theme.border ? theme.border : (darkMode ? "#4D436B" : "#CBD5E1")
    readonly property color cardColor: theme && theme.cardStrong ? theme.cardStrong : (darkMode ? "#342D4A" : "#F1F5F9")
    readonly property color bgColor: theme && theme.card ? theme.card : (darkMode ? "#29233B" : "#FFFFFF")
    readonly property color successBg: theme && theme.successBg ? theme.successBg : (darkMode ? "#143828" : "#ECFDF5")
    readonly property color accentColor: theme && theme.accentA ? theme.accentA : (darkMode ? "#818CF8" : "#4F46E5")

    onOperationRunningChanged: {
        if (operationRunning)
            activityExpanded = true;
    }

    function appendLog(source, message) {
        const prefix = source && source.length > 0 ? source : qsTr("System");
        const nextLine = "[" + Qt.formatTime(new Date(), "HH:mm:ss") + "] " + prefix + ": " + message;
        const shouldFollow = root.activityFollowTail && activityLog.selectedText.length === 0;
        activityLog.text = activityLog.text.length > 0 ? activityLog.text + "\n" + nextLine : nextLine;
        if (shouldFollow)
            activityLog.cursorPosition = activityLog.length;
    }

    function resumeActivityFollow() {
        activityFollowTail = true;
        activityLog.deselect();
        activityLog.cursorPosition = activityLog.length;
        activityLog.forceActiveFocus();
    }

    function clearLog() {
        activityLog.text = "";
        activityFollowTail = true;
    }

    ColumnLayout {
        id: activityLayout
        anchors.fill: parent
        anchors.margins: 12
        spacing: 7

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Label {
                Layout.fillWidth: true
                text: qsTr("Activity")
                color: root.textColor
                font.pixelSize: Math.round(16 * root.uiScale)
                font.weight: Font.DemiBold
            }

            Rectangle {
                Layout.preferredWidth: liveStatusRow.implicitWidth + Math.round(18 * root.uiScale)
                Layout.preferredHeight: Math.round(26 * root.uiScale)
                radius: 7
                color: root.activityFollowTail ? root.successBg : root.bgColor
                border.width: 1
                border.color: root.activityFollowTail ? (root.theme && root.theme.success ? root.theme.success : root.borderColor)
                                                      : root.borderColor

                RowLayout {
                    id: liveStatusRow
                    anchors.centerIn: parent
                    spacing: 6

                    Rectangle {
                        implicitWidth: Math.round(7 * root.uiScale)
                        implicitHeight: Math.round(7 * root.uiScale)
                        radius: Math.round(3.5 * root.uiScale)
                        color: root.activityFollowTail ? (root.theme && root.theme.success ? root.theme.success : "#22C55E")
                                                       : (root.theme && root.theme.warning ? root.theme.warning : "#F59E0B")

                        SequentialAnimation on opacity {
                            running: root.activityFollowTail && root.operationRunning
                            loops: Animation.Infinite
                            NumberAnimation { from: 1.0; to: 0.25; duration: 550; easing.type: Easing.InOutQuad }
                            NumberAnimation { from: 0.25; to: 1.0; duration: 550; easing.type: Easing.InOutQuad }
                        }
                    }

                    Label {
                        text: root.activityFollowTail ? qsTr("Live") : qsTr("Reading")
                        color: root.activityFollowTail ? (root.theme && root.theme.success ? root.theme.success : root.textColor)
                                                       : root.softTextColor
                        font.pixelSize: Math.round(11 * root.uiScale)
                        font.weight: Font.DemiBold
                    }
                }
            }

            Components.ModernMiniButton {
                text: root.activityExpanded ? qsTr("Collapse") : qsTr("View log")
                tone: "neutral"
                theme: root.theme
                darkMode: root.darkMode
                uiScale: root.uiScale
                onClicked: root.activityExpanded = !root.activityExpanded
            }
        }

        Label {
            visible: !root.activityExpanded
            Layout.fillWidth: true
            text: root.operationRunning
                  ? qsTr("Operation is running. Open the log to follow progress.")
                  : (activityLog.text.length > 0
                     ? qsTr("Latest activity is available. Open the log to review it.")
                     : qsTr("No active driver operation."))
            color: root.softTextColor
            font.pixelSize: Math.round(11 * root.uiScale)
            elide: Text.ElideRight
        }

        Components.StatusBanner {
            visible: root.activityExpanded && root.lastOperationText.length > 0
            Layout.fillWidth: true
            theme: root.theme
            tone: root.lastOperationTone
            text: root.lastOperationText
        }

        ScrollView {
            visible: root.activityExpanded
            id: activityScroll
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            ScrollBar.vertical.policy: ScrollBar.AlwaysOn
            ScrollBar.horizontal.policy: ScrollBar.AsNeeded
            background: Rectangle {
                radius: 8
                color: root.darkMode ? "#181424" : "#F8FAFC"
                border.width: 1
                border.color: root.borderColor
            }

            Label {
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.margins: Math.round(12 * root.uiScale)
                visible: activityLog.text.length === 0
                text: qsTr("> Ready. Awaiting driver tasks, updates, or kernel operations...")
                color: root.softTextColor
                opacity: 0.65
                font.family: (Qt.platform.os === "osx") ? "Menlo" : (Qt.platform.os === "windows" ? "Consolas" : "Monospace")
                font.pixelSize: Math.round(11 * root.uiScale)
            }

            TextArea {
                id: activityLog
                width: activityScroll.availableWidth
                readOnly: true
                selectByMouse: true
                persistentSelection: true
                wrapMode: Text.Wrap
                textFormat: TextEdit.PlainText
                color: root.textColor
                selectedTextColor: root.bgColor
                selectionColor: root.accentColor
                font.family: (Qt.platform.os === "osx") ? "Menlo" : (Qt.platform.os === "windows" ? "Consolas" : "Monospace")
                font.pixelSize: Math.round(12 * root.uiScale)
                padding: Math.round(10 * root.uiScale)
                background: null

                Keys.onPressed: function(event) {
                    if (event.key === Qt.Key_PageUp || event.key === Qt.Key_Up || event.key === Qt.Key_Home)
                        root.activityFollowTail = false;
                }

                TapHandler {
                    onTapped: root.activityFollowTail = false
                }

                WheelHandler {
                    onWheel: root.activityFollowTail = false
                }
            }
        }

        RowLayout {
            visible: root.activityExpanded
            Layout.fillWidth: true
            spacing: 8

            Label {
                Layout.fillWidth: true
                text: root.operationRunning
                      ? qsTr("Command output is being captured.")
                      : (root.activityFollowTail ? qsTr("Following output") : qsTr("Paused for reading"))
                color: root.softTextColor
                font.pixelSize: Math.round(11 * root.uiScale)
            }

            Components.ModernMiniButton {
                id: copyBtn
                text: copiedTimer.running ? qsTr("Copied ✓") : qsTr("Copy")
                tone: copiedTimer.running ? "success" : "neutral"
                enabled: activityLog.text.length > 0
                theme: root.theme
                darkMode: root.darkMode
                uiScale: root.uiScale
                onClicked: {
                    activityLog.selectAll();
                    activityLog.copy();
                    activityLog.deselect();
                    copiedTimer.restart();
                }

                Timer {
                    id: copiedTimer
                    interval: 1500
                    repeat: false
                }
            }

            Components.ModernMiniButton {
                text: qsTr("Follow")
                enabled: !root.activityFollowTail
                tone: "neutral"
                theme: root.theme
                darkMode: root.darkMode
                uiScale: root.uiScale
                onClicked: root.resumeActivityFollow()
            }

            Components.ModernMiniButton {
                text: qsTr("Cancel")
                enabled: root.operationRunning
                tone: "danger"
                theme: root.theme
                darkMode: root.darkMode
                uiScale: root.uiScale
                onClicked: root.cancelRequested()
            }

            Components.ModernMiniButton {
                text: qsTr("Clear")
                tone: "neutral"
                theme: root.theme
                darkMode: root.darkMode
                uiScale: root.uiScale
                onClicked: root.clearLog()
            }
        }
    }
}
