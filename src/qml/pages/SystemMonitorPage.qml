import QtQuick
import "../style" as Style
import QtQuick.Controls
import QtQuick.Layouts
import "../components" as Components

Item {
    id: page
    Style.Theme { id: fallbackTheme; darkMode: page.darkMode }
    required property var processMonitor
    required property var diskMonitor
    required property var networkMonitor
    property var uiPreferences: null

    property var theme: fallbackTheme
    property bool darkMode: false
    property bool showAdvancedInfo: true
    property real uiScale: 1.0

    property int processSortMode: 0 // 0 = CPU%, 1 = Memory%, 2 = PID
    readonly property int processLimit: (uiPreferences && uiPreferences.processListLimit)
                                        ? uiPreferences.processListLimit
                                        : 50
    property int pendingKillPid: -1
    property string pendingKillName: ""
    property string actionFeedback: ""
    property bool actionFailed: false

    readonly property color bgColor: theme.card
    readonly property color cardColor: theme.cardStrong
    readonly property color borderColor: theme && theme.border ? theme.border : (page.darkMode ? theme.border : theme.textMuted)
    readonly property color textColor: theme.text
    readonly property color softTextColor: theme && theme.textSoft ? theme.textSoft : theme.textMuted
    readonly property color accentColor: theme.accentA
    readonly property color infoBg: theme.infoBg
    readonly property color successColor: theme.success
    readonly property color warningColor: theme.warning
    readonly property color dangerColor: theme.danger

    Timer {
        id: feedbackTimer
        interval: 4000
        repeat: false
        onTriggered: page.actionFeedback = ""
    }

    property var cachedProcesses: []

    function updateProcesses() {
        if (!page.processMonitor) {
            page.cachedProcesses = [];
            return;
        }

        let list = page.processMonitor.allProcesses;
        if (!Array.isArray(list)) {
            page.cachedProcesses = [];
            return;
        }

        if (page.processSortMode === 0) {
            list = list.slice().sort(function(a, b) { return (b.cpuPercent || 0) - (a.cpuPercent || 0); });
        } else if (page.processSortMode === 1) {
            list = list.slice().sort(function(a, b) { return (b.memoryBytes || 0) - (a.memoryBytes || 0); });
        } else {
            list = list.slice().sort(function(a, b) { return (a.pid || 0) - (b.pid || 0); });
        }

        page.cachedProcesses = list.slice(0, page.processLimit);
    }

    Connections {
        target: page.processMonitor
        function onProcessesChanged() { page.updateProcesses(); }
    }
    onProcessSortModeChanged: page.updateProcesses()
    Component.onCompleted: page.updateProcesses()

    function formatBytes(bytes) {
        if (bytes === undefined || bytes === null)
            return "—";

        var b = Number(bytes);
        if (b < 1024)
            return b + " B";
        if (b < 1024 * 1024)
            return (b / 1024).toFixed(1) + " KiB";
        if (b < 1024 * 1024 * 1024)
            return (b / (1024 * 1024)).toFixed(1) + " MiB";
        return (b / (1024 * 1024 * 1024)).toFixed(2) + " GiB";
    }

    function sortLabel() {
        switch (page.processSortMode) {
        case 0: return qsTr("CPU %");
        case 1: return qsTr("Memory");
        default: return qsTr("PID");
        }
    }

    function requestKillConfirmation(pid, name) {
        page.pendingKillPid = pid;
        page.pendingKillName = name && name.length > 0 ? name : qsTr("process");
        killConfirmDialog.open();
    }

    function performKill() {
        if (!page.processMonitor || page.pendingKillPid <= 0)
            return;

        const ok = page.processMonitor.killProcess(page.pendingKillPid, 15);
        page.actionFailed = !ok;
        page.actionFeedback = ok
            ? qsTr("SIGTERM sent to PID %1 (%2).").arg(page.pendingKillPid).arg(page.pendingKillName)
            : qsTr("Failed to terminate PID %1. See status message.").arg(page.pendingKillPid);
        feedbackTimer.restart();
        page.pendingKillPid = -1;
        page.pendingKillName = "";
    }

    function performRenice(pid, niceValue) {
        if (!page.processMonitor)
            return;

        const ok = page.processMonitor.setProcessPriority(pid, niceValue);
        page.actionFailed = !ok;
        page.actionFeedback = ok
            ? qsTr("PID %1 reniced to %2.").arg(pid).arg(niceValue)
            : qsTr("Failed to renice PID %1. See status message.").arg(pid);
        feedbackTimer.restart();
    }

    ScrollView {
        id: pageScroll
        anchors.fill: parent
        clip: true
        contentWidth: availableWidth

        ColumnLayout {
            width: pageScroll.availableWidth
            spacing: Style.RoTokens.spaceMd

            // ─── System Overview Cards ─────────────────────────────
            Components.SectionPanel {
                theme: page.theme
                Layout.fillWidth: true
                title: qsTr("System Resources")
                subtitle: qsTr("Live overview of CPU, memory, disk, and network")

                GridLayout {
                    Layout.fillWidth: true
                    columns: width > 900 ? 4 : (width > 520 ? 2 : 1)
                    columnSpacing: Style.RoTokens.spaceSm
                    rowSpacing: Style.RoTokens.spaceSm

                    Components.StatCard {
                        theme: page.theme
                        Layout.fillWidth: true
                        title: qsTr("Processes")
                        value: page.processMonitor && page.processMonitor.available
                               ? String(page.processMonitor.processCount)
                               : "—"
                        subtitle: page.processMonitor && page.processMonitor.available
                                  ? qsTr("running") : qsTr("unavailable")
                        accentColor: page.accentColor
                    }

                    Components.StatCard {
                        theme: page.theme
                        Layout.fillWidth: true
                        title: qsTr("Disks")
                        value: page.diskMonitor && page.diskMonitor.available
                               ? String(page.diskMonitor.diskCount)
                               : "—"
                        subtitle: page.diskMonitor && page.diskMonitor.available
                                  ? qsTr("mounted block devices")
                                  : qsTr("unavailable")
                        accentColor: page.accentColor
                    }

                    Components.StatCard {
                        theme: page.theme
                        Layout.fillWidth: true
                        title: qsTr("Network")
                        value: page.networkMonitor && page.networkMonitor.available
                               ? String(page.networkMonitor.interfaceCount)
                               : "—"
                        subtitle: page.networkMonitor && page.networkMonitor.available
                                  ? qsTr("active interfaces")
                                  : qsTr("unavailable")
                        accentColor: page.accentColor
                    }

                    Components.StatCard {
                        theme: page.theme
                        Layout.fillWidth: true
                        title: qsTr("Throughput")
                        value: page.networkMonitor && page.networkMonitor.available
                               ? page.formatBytes(page.networkMonitor.totalRxBytesPerSec + page.networkMonitor.totalTxBytesPerSec) + "/s"
                               : "—"
                        subtitle: qsTr("combined rx+tx")
                        accentColor: page.accentColor
                    }
                }
            }

            // ─── Action Feedback Banner ────────────────────────────
            Rectangle {
                Layout.fillWidth: true
                visible: page.actionFeedback.length > 0
                implicitHeight: feedbackLabel.implicitHeight + 18
                radius: Style.RoTokens.radiusSm
                color: page.actionFailed ? theme.warningBg : page.infoBg
                border.width: 1
                border.color: page.actionFailed ? page.warningColor : page.accentColor

                Label {
                    id: feedbackLabel
                    anchors.fill: parent
                    anchors.margins: 9
                    text: page.actionFeedback
                    color: page.actionFailed ? page.warningColor : page.textColor
                    wrapMode: Text.WordWrap
                }
            }

            // ─── Process List ──────────────────────────────────────
            Components.SectionPanel {
                theme: page.theme
                Layout.fillWidth: true
                title: qsTr("Processes")
                subtitle: qsTr("Top processes by CPU usage, memory footprint or PID. Right-click to manage.")

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: Style.RoTokens.spaceSm

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Style.RoTokens.spaceSm

                        Label {
                            text: qsTr("Sort by:")
                            color: page.softTextColor
                        }

                        RowLayout {
                            spacing: Style.RoTokens.spaceXs

                            RadioButton {
                                text: qsTr("CPU %")
                                checked: page.processSortMode === 0
                                onClicked: page.processSortMode = 0
                            }
                            RadioButton {
                                text: qsTr("Memory")
                                checked: page.processSortMode === 1
                                onClicked: page.processSortMode = 1
                            }
                            RadioButton {
                                text: qsTr("PID")
                                checked: page.processSortMode === 2
                                onClicked: page.processSortMode = 2
                            }
                        }

                        Item { Layout.fillWidth: true }

                        Label {
                            text: qsTr("Showing %1 of %2 processes")
                                  .arg(Math.min(page.processLimit, page.cachedProcesses.length))
                                  .arg(page.processMonitor && page.processMonitor.available
                                       ? page.processMonitor.processCount
                                       : 0)
                            color: page.softTextColor
                        }

                        Button {
                            text: qsTr("Refresh")
                            enabled: page.processMonitor && page.processMonitor.available
                            onClicked: if (page.processMonitor) page.processMonitor.refresh()
                        }
                    }

                    // Header row
                    Rectangle {
                        Layout.fillWidth: true
                        implicitHeight: 28
                        radius: Style.RoTokens.radiusSm
                        color: page.bgColor
                        border.width: 1
                        border.color: page.borderColor

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 12

                            Label { text: qsTr("PID");     Layout.preferredWidth: 60;  color: page.softTextColor; font.weight: Font.DemiBold }
                            Label { text: qsTr("NAME");    Layout.fillWidth: true;  color: page.softTextColor; font.weight: Font.DemiBold }
                            Label { text: qsTr("CPU %");  Layout.preferredWidth: 72;  horizontalAlignment: Text.AlignRight; color: page.softTextColor; font.weight: Font.DemiBold }
                            Label { text: qsTr("MEM");     Layout.preferredWidth: 84;  horizontalAlignment: Text.AlignRight; color: page.softTextColor; font.weight: Font.DemiBold }
                            Label { text: qsTr("STATE");   Layout.preferredWidth: 48;  horizontalAlignment: Text.AlignHCenter; color: page.softTextColor; font.weight: Font.DemiBold }
                            Item   { Layout.preferredWidth: 90 }
                        }
                    }

                    Repeater {
                        model: page.cachedProcesses

                        delegate: Rectangle {
                            required property var modelData
                            Layout.fillWidth: true
                            implicitHeight: 36
                            radius: Style.RoTokens.radiusSm
                            color: index % 2 === 0 ? "transparent" : page.bgColor
                            border.width: 0

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 8
                                spacing: 12

                                Label {
                                    Layout.preferredWidth: 60
                                    text: String(modelData.pid || 0)
                                    color: page.textColor
                                    font.family: "monospace"
                                }
                                Label {
                                    Layout.fillWidth: true
                                    text: modelData.name || qsTr("unknown")
                                    color: page.textColor
                                    elide: Text.ElideRight
                                }
                                Label {
                                    Layout.preferredWidth: 72
                                    text: (modelData.cpuPercent || 0).toFixed(1)
                                    horizontalAlignment: Text.AlignRight
                                    color: (modelData.cpuPercent || 0) > 50 ? page.dangerColor : page.textColor
                                    font.family: "monospace"
                                }
                                Label {
                                    Layout.preferredWidth: 84
                                    text: page.formatBytes(modelData.memoryBytes || 0)
                                    horizontalAlignment: Text.AlignRight
                                    color: page.textColor
                                    font.family: "monospace"
                                }
                                Label {
                                    Layout.preferredWidth: 48
                                    text: modelData.state || "?"
                                    horizontalAlignment: Text.AlignHCenter
                                    color: page.softTextColor
                                    font.family: "monospace"
                                }
                                RowLayout {
                                    Layout.preferredWidth: 90
                                    spacing: 4

                                    Button {
                                        text: qsTr("Kill")
                                        Layout.preferredWidth: 42
                                        onClicked: page.requestKillConfirmation(modelData.pid, modelData.name)
                                    }
                                    Button {
                                        text: qsTr("+")
                                        Layout.preferredWidth: 36
                                        ToolTip.text: qsTr("Increase nice by 1")
                                        onClicked: page.performRenice(modelData.pid, (modelData.nice || 0) + 1)
                                    }
                                }
                            }
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        visible: !page.processMonitor || !page.processMonitor.available
                        text: qsTr("Process telemetry is not available. The /proc filesystem is not mounted or readable.")
                        color: page.softTextColor
                        wrapMode: Text.WordWrap
                    }
                }
            }

            // ─── Disks ─────────────────────────────────────────────
            Components.SectionPanel {
                theme: page.theme
                Layout.fillWidth: true
                title: qsTr("Disks")
                subtitle: qsTr("Block device I/O counters and filesystem usage")

                Repeater {
                    model: page.diskMonitor && page.diskMonitor.available ? page.diskMonitor.diskStats : []

                    delegate: Rectangle {
                        required property var modelData
                        Layout.fillWidth: true
                        implicitHeight: 56
                        radius: Style.RoTokens.radiusSm
                        color: page.bgColor
                        border.width: 1
                        border.color: page.borderColor

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 12

                            Label {
                                Layout.preferredWidth: 160
                                text: modelData.device || ""
                                color: page.textColor
                                font.family: "monospace"
                                elide: Text.ElideRight
                            }
                            Label {
                                Layout.fillWidth: true
                                text: (modelData.mountPoint || "") + "  •  " + (modelData.fsType || "")
                                color: page.softTextColor
                                elide: Text.ElideRight
                            }
                            Label {
                                Layout.preferredWidth: 140
                                text: page.formatBytes(Number(modelData.usedBytes || 0))
                                      + " / " + page.formatBytes(Number(modelData.totalBytes || 0))
                                horizontalAlignment: Text.AlignRight
                                color: page.textColor
                                font.family: "monospace"
                            }
                            Label {
                                Layout.preferredWidth: 60
                                text: (Number(modelData.usagePercent || 0)).toFixed(1) + "%"
                                horizontalAlignment: Text.AlignRight
                                color: (modelData.usagePercent || 0) > 90 ? page.dangerColor : page.textColor
                                font.family: "monospace"
                            }
                        }
                    }
                }

                Label {
                    Layout.fillWidth: true
                    visible: !page.diskMonitor || !page.diskMonitor.available || page.diskMonitor.diskStats.length === 0
                    text: qsTr("No block devices to display.")
                    color: page.softTextColor
                    wrapMode: Text.WordWrap
                }
            }

            // ─── Network Interfaces ────────────────────────────────
            Components.SectionPanel {
                theme: page.theme
                Layout.fillWidth: true
                title: qsTr("Network Interfaces")
                subtitle: qsTr("Per-interface receive/transmit throughput and error counters")

                Repeater {
                    model: page.networkMonitor && page.networkMonitor.available ? page.networkMonitor.interfaces : []

                    delegate: Rectangle {
                        required property var modelData
                        Layout.fillWidth: true
                        implicitHeight: 56
                        radius: Style.RoTokens.radiusSm
                        color: page.bgColor
                        border.width: 1
                        border.color: page.borderColor

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 12

                            Label {
                                Layout.preferredWidth: 140
                                text: modelData.name || ""
                                color: page.textColor
                                font.family: "monospace"
                                elide: Text.ElideRight
                            }
                            Label {
                                Layout.fillWidth: true
                                text: qsTr("rx") + ": " + page.formatBytes(modelData.rxBytesPerSec || 0) + "/s    "
                                      + qsTr("tx") + ": " + page.formatBytes(modelData.txBytesPerSec || 0) + "/s"
                                horizontalAlignment: Text.AlignRight
                                color: page.textColor
                                font.family: "monospace"
                            }
                            Label {
                                Layout.preferredWidth: 180
                                text: qsTr("err rx/tx") + ": " + (modelData.rxErrors || 0) + "/" + (modelData.txErrors || 0)
                                      + "    " + qsTr("drop") + ": " + (modelData.rxDropped || 0) + "/" + (modelData.txDropped || 0)
                                horizontalAlignment: Text.AlignRight
                                color: (modelData.rxErrors || 0) + (modelData.txErrors || 0) > 0 ? page.warningColor : page.softTextColor
                                font.family: "monospace"
                                font.pixelSize: 11
                            }
                        }
                    }
                }

                Label {
                    Layout.fillWidth: true
                    visible: !page.networkMonitor || !page.networkMonitor.available || page.networkMonitor.interfaces.length === 0
                    text: qsTr("No network interfaces to display.")
                    color: page.softTextColor
                    wrapMode: Text.WordWrap
                }
            }

            Item { height: 24 }
        }
    }

    // ─── Kill Confirmation Popup ──────────────────────────────────
    Popup {
        id: killConfirmDialog
        modal: true
        focus: true
        anchors.centerIn: parent
        width: Math.min(parent ? parent.width - 40 : 420, Math.round(420 * page.uiScale))
        padding: Math.round(Style.RoTokens.spaceLg * page.uiScale)
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            radius: Style.RoTokens.radiusMd
            color: page.cardColor
            border.width: 1
            border.color: page.borderColor
        }

        contentItem: ColumnLayout {
            spacing: Style.RoTokens.spaceMd

            Label {
                Layout.fillWidth: true
                text: qsTr("Terminate process?")
                color: page.textColor
                font.pixelSize: Math.round(17 * page.uiScale)
                font.weight: Font.DemiBold
            }
            Label {
                Layout.fillWidth: true
                text: qsTr("Send SIGTERM to PID %1 (%2)? Unsaved work in that process will be lost.")
                      .arg(page.pendingKillPid).arg(page.pendingKillName)
                color: page.softTextColor
                wrapMode: Text.WordWrap
                font.pixelSize: Math.round(12 * page.uiScale)
            }
            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
                Button {
                    text: qsTr("Cancel")
                    onClicked: killConfirmDialog.close()
                }
                Button {
                    text: qsTr("End process")
                    enabled: page.pendingKillPid > 0
                    onClicked: {
                        page.performKill();
                        killConfirmDialog.close();
                    }
                }
            }
        }
    }
}