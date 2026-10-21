import QtQuick
import "../style" as Style
import QtQuick.Controls
import QtQuick.Layouts

Popup {
    id: popup
    Style.Theme { id: fallbackTheme; darkMode: popup.darkMode }
    required property var fanController
    property var theme: fallbackTheme
    property bool darkMode: false
    property real uiScale: 1.0

    // Step 1: "confirm", Step 2: "scanning", Step 3: "done"
    property string step: "confirm"
    property int scanProgress: 0
    property string currentPhaseText: qsTr("Initializing Hardware Probe...")
    property bool phase1Done: false
    property bool phase2Done: false
    property bool phase3Done: false
    property int scannedChannelCount: 0
    property string scannedCapability: ""
    property bool testRunning: false
    property int testRemainingSeconds: 4
    readonly property int controllableFanCount: {
        var fans = popup.fanController ? popup.fanController.systemFans : [];
        var count = 0;
        for (var i = 0; i < fans.length; ++i) {
            if (fans[i].controllable)
                ++count;
        }
        return count;
    }

    readonly property color bgColor: theme.card
    readonly property color cardColor: theme && theme.cardStrong ? theme.cardStrong : (popup.darkMode ? theme.cardStrong : theme.text)
    readonly property color borderColor: theme && theme.border ? theme.border : (popup.darkMode ? theme.border : theme.cardStrong)
    readonly property color textColor: theme.text
    readonly property color softTextColor: theme && theme.textSoft ? theme.textSoft : (theme.textMuted)
    readonly property color accentColor: theme.accentA
    readonly property color accentButtonText: theme.onAccent
    readonly property color successBg: theme.successBg
    readonly property color successText: theme.success
    readonly property color warningBg: theme.warningBg
    readonly property color warningText: theme.warning
    readonly property color infoBg: theme.infoBg
    readonly property color infoText: theme.accentA

    modal: true
    focus: true
    closePolicy: (step === "scanning" || testRunning) ? Popup.NoAutoClose : (Popup.CloseOnEscape | Popup.CloseOnPressOutside)
    width: Math.min(parent ? parent.width - 32 : 540, Math.round(540 * popup.uiScale))
    height: Math.min(parent ? parent.height - 40 : 620, contentColumn.implicitHeight + topPadding + bottomPadding)
    x: Math.round(((parent ? parent.width : 600) - width) / 2)
    y: Math.max(16, Math.round(((parent ? parent.height : 600) - height) / 2))
    padding: Math.round(Style.RoTokens.spaceLg * popup.uiScale)

    Behavior on height {
        NumberAnimation { duration: Style.Motion.fastMs; easing.type: Easing.OutCubic }
    }
    Behavior on y {
        NumberAnimation { duration: Style.Motion.fastMs; easing.type: Easing.OutCubic }
    }

    enter: Transition {
        ParallelAnimation {
            NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: Style.Motion.fastMs; easing.type: Easing.OutCubic }
            NumberAnimation { property: "scale"; from: 0.95; to: 1.0; duration: Style.Motion.fastMs; easing.type: Easing.OutCubic }
        }
    }

    exit: Transition {
        ParallelAnimation {
            NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: Style.Motion.fastMs; easing.type: Easing.InCubic }
            NumberAnimation { property: "scale"; from: 1.0; to: 0.96; duration: Style.Motion.fastMs }
        }
    }

    function openWizard() {
        step = "confirm";
        scanProgress = 0;
        phase1Done = false;
        phase2Done = false;
        phase3Done = false;
        scannedChannelCount = 0;
        scannedCapability = "";
        testRunning = false;
        testRemainingSeconds = 4;
        testCountdownTimer.stop();
        currentPhaseText = qsTr("Initializing Hardware Probe...");
        popup.open();
    }

    function startScanProcess() {
        step = "scanning";
        scanProgress = 5;
        currentPhaseText = qsTr("Preparing a read-only hardware probe...");
        Qt.callLater(function() {
            if (popup.step !== "scanning")
                return;

            popup.scanProgress = 35;
            popup.currentPhaseText = qsTr("Probing Linux HWMON, ACPI, and NVIDIA fan interfaces...");
            var result = popup.fanController ? popup.fanController.runHardwareSetup() : null;
            popup.scannedChannelCount = result && result.channelCount !== undefined
                    ? Number(result.channelCount)
                    : (popup.fanController ? popup.fanController.systemFanCount : 0);
            popup.scannedCapability = result && result.capability !== undefined
                    ? String(result.capability) : "";
            popup.phase1Done = true;
            popup.phase2Done = true;
            popup.phase3Done = true;
            popup.scanProgress = 100;
            popup.currentPhaseText = popup.scannedChannelCount > 0
                    ? qsTr("Scan complete: %1 readable fan channel(s) found.").arg(popup.scannedChannelCount)
                    : qsTr("Scan complete: no readable fan channels were exposed.");
            popup.step = "done";
        });
    }

    function triggerAcousticValidation() {
        if (testRunning || !popup.fanController || popup.controllableFanCount === 0)
            return;
        testRunning = true;
        testRemainingSeconds = 4;
        
        var fans = popup.fanController.systemFans || [];
        for (var i = 0; i < fans.length; ++i) {
            if (fans[i].controllable) {
                popup.fanController.testFanSpeedForFan(fans[i].id, 100);
            }
        }
        testCountdownTimer.start();
    }

    function stopAcousticValidation() {
        testCountdownTimer.stop();
        testRunning = false;
        if (popup.fanController) {
            var fans = popup.fanController.systemFans || [];
            for (var i = 0; i < fans.length; ++i) {
                if (fans[i].controllable) {
                    popup.fanController.restoreFanControlForFan(fans[i].id);
                }
            }
            popup.fanController.refresh();
        }
    }

    Timer {
        id: testCountdownTimer
        interval: 1000
        repeat: true
        onTriggered: {
            popup.testRemainingSeconds -= 1;
            if (popup.testRemainingSeconds <= 0) {
                popup.stopAcousticValidation();
            }
        }
    }

    background: Rectangle {
        radius: Style.RoTokens.radiusMd
        color: popup.bgColor
        border.width: 1
        border.color: popup.borderColor
    }

    contentItem: ScrollView {
        id: contentScroll
        clip: true
        contentWidth: availableWidth

        ColumnLayout {
            id: contentColumn
            width: contentScroll.availableWidth
            spacing: Math.round(Style.RoTokens.spaceMd * popup.uiScale)

        // ---------------- HEADER ----------------
        RowLayout {
            Layout.fillWidth: true
            spacing: Math.round(Style.RoTokens.spaceMd * popup.uiScale)

            Rectangle {
                implicitWidth: Math.round(38 * popup.uiScale)
                implicitHeight: Math.round(38 * popup.uiScale)
                radius: Math.round(Style.RoTokens.radiusSm * popup.uiScale)
                color: popup.step === "done" ? popup.successBg : (popup.darkMode ? theme.cardStrong : theme.accentC)
                border.width: 1
                border.color: popup.step === "done" ? popup.successText : popup.accentColor

                Label {
                    anchors.centerIn: parent
                    text: popup.step === "done" ? "✓" : (popup.step === "scanning" ? "⚡" : "⚙")
                    color: popup.step === "done" ? popup.successText : popup.accentColor
                    font.pixelSize: Math.round(16 * popup.uiScale)
                    font.weight: Font.Bold
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Label {
                    text: popup.step === "confirm"
                          ? qsTr("Fan Setup & Discovery Wizard")
                          : (popup.step === "scanning"
                             ? qsTr("Scanning Thermal & Fan Controllers...")
                             : qsTr("Hardware Discovery Complete"))
                    color: popup.textColor
                    font.pixelSize: Math.round(15 * popup.uiScale)
                    font.weight: Font.DemiBold
                }

                Label {
                    text: popup.step === "confirm"
                          ? qsTr("Enumerate cooling fans and refresh exposed sensor telemetry")
                          : (popup.step === "scanning"
                             ? qsTr("Please wait while hardware sensors are probed...")
                             : (popup.scannedChannelCount > 0
                                ? qsTr("Detected cooling telemetry is ready to review")
                                : qsTr("No readable fan telemetry was exposed by this system")))
                    color: popup.softTextColor
                    font.pixelSize: Math.round(11 * popup.uiScale)
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
            }

            ToolButton {
                id: closeBtn
                visible: popup.step !== "scanning" && !popup.testRunning
                Layout.alignment: Qt.AlignTop | Qt.AlignRight
                implicitWidth: Math.round(28 * popup.uiScale)
                implicitHeight: Math.round(28 * popup.uiScale)
                hoverEnabled: true

                background: Rectangle {
                    radius: Math.round(Style.RoTokens.radiusMd * popup.uiScale)
                    color: closeBtn.hovered ? (popup.darkMode ? theme.hover : theme.cardStrong) : "transparent"
                }

                contentItem: Text {
                    text: "✕"
                    color: closeBtn.hovered ? popup.textColor : popup.softTextColor
                    font.pixelSize: Math.round(13 * popup.uiScale)
                    font.weight: Font.DemiBold
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                ToolTip {
                    visible: closeBtn.hovered
                    text: qsTr("Close")
                    delay: 400
                }

                onClicked: {
                    if (popup.testRunning)
                        popup.stopAcousticValidation();
                    popup.close();
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 1
            color: popup.borderColor
            opacity: 0.6
        }

        // ================= STEP 1: CONFIRMATION =================
        ColumnLayout {
            visible: popup.step === "confirm"
            Layout.fillWidth: true
            spacing: Math.round(Style.RoTokens.spaceMd * popup.uiScale)

            Rectangle {
                Layout.fillWidth: true
                radius: Style.RoTokens.radiusSm
                color: popup.cardColor
                border.width: 1
                border.color: popup.borderColor
                implicitHeight: confirmContentCol.implicitHeight + Math.round(24 * popup.uiScale)

                ColumnLayout {
                    id: confirmContentCol
                    anchors.fill: parent
                    anchors.margins: Math.round(Style.RoTokens.spaceMd * popup.uiScale)
                    spacing: Math.round(Style.RoTokens.spaceMd * popup.uiScale)

                    Label {
                        text: qsTr("What will this hardware setup wizard do?")
                        color: popup.textColor
                        font.pixelSize: Math.round(12 * popup.uiScale)
                        font.weight: Font.DemiBold
                    }

                    // Feature row 1
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Style.RoTokens.spaceMd
                        Rectangle {
                            implicitWidth: Math.round(24 * popup.uiScale)
                            implicitHeight: Math.round(24 * popup.uiScale)
                            radius: Style.RoTokens.radiusSm
                            color: popup.darkMode ? theme.cardStrong : theme.infoBg
                            Label {
                                anchors.centerIn: parent
                                text: "🌡️"
                                font.pixelSize: Math.round(11 * popup.uiScale)
                            }
                        }
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 1
                            Label {
                                text: qsTr("Kernel & HWMON Thermal Zones")
                                color: popup.textColor
                                font.pixelSize: Math.round(11 * popup.uiScale)
                                font.weight: Font.DemiBold
                            }
                            Label {
                                text: qsTr("Scans /sys/class/thermal and motherboard hardware monitoring chips")
                                color: popup.softTextColor
                                font.pixelSize: Math.round(10 * popup.uiScale)
                            }
                        }
                    }

                    // Feature row 2
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Style.RoTokens.spaceMd
                        Rectangle {
                            implicitWidth: Math.round(24 * popup.uiScale)
                            implicitHeight: Math.round(24 * popup.uiScale)
                            radius: Style.RoTokens.radiusSm
                            color: popup.darkMode ? theme.hover : theme.accentC
                            Label {
                                anchors.centerIn: parent
                                text: "⚡"
                                font.pixelSize: Math.round(11 * popup.uiScale)
                            }
                        }
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 1
                            Label {
                                text: qsTr("NVIDIA NV-CONTROL & Fan Tachometers")
                                color: popup.textColor
                                font.pixelSize: Math.round(11 * popup.uiScale)
                                font.weight: Font.DemiBold
                            }
                            Label {
                                text: qsTr("Probes dedicated GPU cooling channels and PWM control registers")
                                color: popup.softTextColor
                                font.pixelSize: Math.round(10 * popup.uiScale)
                            }
                        }
                    }

                    // Feature row 3
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Style.RoTokens.spaceMd
                        Rectangle {
                            implicitWidth: Math.round(24 * popup.uiScale)
                            implicitHeight: Math.round(24 * popup.uiScale)
                            radius: Style.RoTokens.radiusSm
                            color: theme.successBg
                            Label {
                                anchors.centerIn: parent
                                text: "❄️"
                                font.pixelSize: Math.round(11 * popup.uiScale)
                            }
                        }
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 1
                            Label {
                                text: qsTr("Profile & Zero-dB Calibration")
                                color: popup.textColor
                                font.pixelSize: Math.round(11 * popup.uiScale)
                                font.weight: Font.DemiBold
                            }
                            Label {
                                text: qsTr("Recalibrates zero-RPM stops, baseline curves, and safety overrides")
                                color: popup.softTextColor
                                font.pixelSize: Math.round(10 * popup.uiScale)
                            }
                        }
                    }
                }
            }

            // Safe Operation Note
            Rectangle {
                Layout.fillWidth: true
                radius: Style.RoTokens.radiusSm
                color: popup.infoBg
                implicitHeight: safeNoteCol.implicitHeight + Math.round(14 * popup.uiScale)

                RowLayout {
                    id: safeNoteCol
                    anchors.fill: parent
                    anchors.margins: Math.round(Style.RoTokens.spaceSm * popup.uiScale)
                    spacing: Style.RoTokens.spaceSm

                    Label {
                        text: "ℹ️"
                        font.pixelSize: Math.round(12 * popup.uiScale)
                    }
                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Non-intrusive probe: Fan speeds will not be interrupted during this scan.")
                        color: popup.infoText
                        font.pixelSize: Math.round(10 * popup.uiScale)
                        wrapMode: Text.WordWrap
                    }
                }
            }

            // Actions
            RowLayout {
                Layout.fillWidth: true
                spacing: Math.round(Style.RoTokens.spaceSm * popup.uiScale)

                Item { Layout.fillWidth: true }

                Button {
                    id: cancelBtn
                    text: qsTr("Cancel")
                    implicitHeight: Math.round(34 * popup.uiScale)
                    implicitWidth: Math.round(85 * popup.uiScale)
                    hoverEnabled: true

                    background: Rectangle {
                        radius: Style.RoTokens.radiusSm
                        color: cancelBtn.down ? (popup.darkMode ? theme.hover : theme.cardStrong)
                                              : (cancelBtn.hovered ? (theme.cardStrong)
                                                                   : popup.cardColor)
                        border.width: 1
                        border.color: cancelBtn.hovered ? popup.accentColor : popup.borderColor
                    }

                    contentItem: Text {
                        text: cancelBtn.text
                        color: cancelBtn.hovered ? popup.accentColor : popup.textColor
                        font.pixelSize: Math.round(11 * popup.uiScale)
                        font.weight: Font.DemiBold
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: popup.close()
                }

                Button {
                    id: startScanBtn
                    text: qsTr("Run Setup Wizard")
                    implicitHeight: Math.round(34 * popup.uiScale)
                    hoverEnabled: true

                    scale: down ? 0.98 : (hovered ? 1.01 : 1.0)
                    Behavior on scale { NumberAnimation { duration: Style.Motion.fastMs } }

                    background: Rectangle {
                        radius: Style.RoTokens.radiusSm
                        color: startScanBtn.down ? (theme.accentA)
                                                 : (startScanBtn.hovered ? (theme.accentA)
                                                                         : popup.accentColor)
                    }

                    contentItem: RowLayout {
                        spacing: Style.RoTokens.spaceSm
                        anchors.centerIn: parent
                        Label {
                            text: "🔍"
                            font.pixelSize: Math.round(11 * popup.uiScale)
                        }
                        Text {
                            text: startScanBtn.text
                            color: popup.accentButtonText
                            font.pixelSize: Math.round(11 * popup.uiScale)
                            font.weight: Font.Bold
                        }
                    }

                    onClicked: popup.startScanProcess()
                }
            }
        }

        // ================= STEP 2: SCANNING =================
        ColumnLayout {
            visible: popup.step === "scanning"
            Layout.fillWidth: true
            spacing: Math.round(Style.RoTokens.spaceMd * popup.uiScale)

            Rectangle {
                Layout.fillWidth: true
                radius: Style.RoTokens.radiusSm
                color: popup.cardColor
                border.width: 1
                border.color: popup.borderColor
                implicitHeight: scanningContentCol.implicitHeight + Math.round(24 * popup.uiScale)

                ColumnLayout {
                    id: scanningContentCol
                    anchors.fill: parent
                    anchors.margins: Math.round(Style.RoTokens.spaceMd * popup.uiScale)
                    spacing: Math.round(Style.RoTokens.spaceMd * popup.uiScale)

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Style.RoTokens.spaceSm

                        Label {
                            text: popup.currentPhaseText
                            color: popup.textColor
                            font.pixelSize: Math.round(12 * popup.uiScale)
                            font.weight: Font.DemiBold
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }

                        Label {
                            text: popup.scanProgress + "%"
                            color: popup.accentColor
                            font.pixelSize: Math.round(12 * popup.uiScale)
                            font.weight: Font.Bold
                        }
                    }

                    // Progress Bar
                    Rectangle {
                        Layout.fillWidth: true
                        implicitHeight: Math.round(8 * popup.uiScale)
                        radius: Style.RoTokens.radiusXs
                        color: theme.cardStrong
                        clip: true

                        Rectangle {
                            anchors.left: parent.left
                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                            width: parent.width * (popup.scanProgress / 100.0)
                            radius: Style.RoTokens.radiusXs
                            color: popup.accentColor

                            Behavior on width {
                                NumberAnimation { duration: Style.Motion.fastMs; easing.type: Easing.OutQuad }
                            }
                        }
                    }

                    // Checklist
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: Math.round(Style.RoTokens.spaceSm * popup.uiScale)

                        RowLayout {
                            spacing: Style.RoTokens.spaceMd
                            Rectangle {
                                implicitWidth: Math.round(18 * popup.uiScale)
                                implicitHeight: Math.round(18 * popup.uiScale)
                                radius: Style.RoTokens.radiusXs
                                color: popup.phase1Done ? popup.successBg : (popup.darkMode ? theme.cardStrong : theme.accentC)
                                Label {
                                    anchors.centerIn: parent
                                    text: popup.phase1Done ? "✓" : "1"
                                    color: popup.phase1Done ? popup.successText : popup.accentColor
                                    font.pixelSize: Math.round(10 * popup.uiScale)
                                    font.weight: Font.Bold
                                }
                            }
                            Label {
                                text: qsTr("Linux HWMON & ACPI Controllers")
                                color: popup.phase1Done ? popup.textColor : popup.softTextColor
                                font.pixelSize: Math.round(11 * popup.uiScale)
                                font.weight: popup.phase1Done ? Font.DemiBold : Font.Normal
                            }
                        }

                        RowLayout {
                            spacing: Style.RoTokens.spaceMd
                            Rectangle {
                                implicitWidth: Math.round(18 * popup.uiScale)
                                implicitHeight: Math.round(18 * popup.uiScale)
                                radius: Style.RoTokens.radiusXs
                                color: popup.phase2Done ? popup.successBg : (popup.phase1Done ? (popup.darkMode ? theme.cardStrong : theme.accentC) : (theme.cardStrong))
                                Label {
                                    anchors.centerIn: parent
                                    text: popup.phase2Done ? "✓" : "2"
                                    color: popup.phase2Done ? popup.successText : (popup.phase1Done ? popup.accentColor : popup.softTextColor)
                                    font.pixelSize: Math.round(10 * popup.uiScale)
                                    font.weight: Font.Bold
                                }
                            }
                            Label {
                                text: qsTr("NVIDIA GPU NV-CONTROL Interfaces")
                                color: popup.phase2Done ? popup.textColor : popup.softTextColor
                                font.pixelSize: Math.round(11 * popup.uiScale)
                                font.weight: popup.phase2Done ? Font.DemiBold : Font.Normal
                            }
                        }

                        RowLayout {
                            spacing: Style.RoTokens.spaceMd
                            Rectangle {
                                implicitWidth: Math.round(18 * popup.uiScale)
                                implicitHeight: Math.round(18 * popup.uiScale)
                                radius: Style.RoTokens.radiusXs
                                color: popup.phase3Done ? popup.successBg : (popup.phase2Done ? (popup.darkMode ? theme.cardStrong : theme.accentC) : (theme.cardStrong))
                                Label {
                                    anchors.centerIn: parent
                                    text: popup.phase3Done ? "✓" : "3"
                                    color: popup.phase3Done ? popup.successText : (popup.phase2Done ? popup.accentColor : popup.softTextColor)
                                    font.pixelSize: Math.round(10 * popup.uiScale)
                                    font.weight: Font.Bold
                                }
                            }
                            Label {
                                text: qsTr("Telemetry Refresh & Sensor Sync")
                                color: popup.phase3Done ? popup.textColor : popup.softTextColor
                                font.pixelSize: Math.round(11 * popup.uiScale)
                                font.weight: popup.phase3Done ? Font.DemiBold : Font.Normal
                            }
                        }
                    }
                }
            }
        }

        // ================= STEP 3: RESULTS =================
        ColumnLayout {
            visible: popup.step === "done"
            Layout.fillWidth: true
            spacing: Math.round(Style.RoTokens.spaceMd * popup.uiScale)

            // Success Summary Banner
            Rectangle {
                Layout.fillWidth: true
                radius: Style.RoTokens.radiusSm
                color: popup.scannedChannelCount > 0 ? popup.successBg : popup.warningBg
                border.width: 1
                border.color: popup.scannedChannelCount > 0 ? popup.successText : popup.warningText
                implicitHeight: doneLayout.implicitHeight + Math.round(16 * popup.uiScale)

                ColumnLayout {
                    id: doneLayout
                    anchors.fill: parent
                    anchors.margins: Math.round(Style.RoTokens.spaceMd * popup.uiScale)
                    spacing: 3

                    RowLayout {
                        spacing: Style.RoTokens.spaceSm
                        Label {
                            text: popup.scannedChannelCount > 0 ? "✓" : "!"
                            color: popup.scannedChannelCount > 0 ? popup.successText : popup.warningText
                            font.pixelSize: Math.round(14 * popup.uiScale)
                            font.weight: Font.Bold
                        }
                        Label {
                            text: popup.scannedChannelCount > 0
                                  ? qsTr("Hardware Scan Completed")
                                  : qsTr("Scan Completed Without Readable Fan Channels")
                            color: popup.scannedChannelCount > 0 ? popup.successText : popup.warningText
                            font.pixelSize: Math.round(12 * popup.uiScale)
                            font.weight: Font.Bold
                        }
                    }

                    Label {
                        text: popup.scannedChannelCount > 0
                              ? qsTr("%1 cooling channel(s) reported live telemetry. %2")
                                    .arg(popup.scannedChannelCount)
                                    .arg(popup.scannedCapability)
                              : qsTr("The scan is complete, but firmware or the driver did not expose readable fan telemetry.")
                        color: popup.textColor
                        font.pixelSize: Math.round(10 * popup.uiScale)
                    }
                }
            }

            // Discovered Channels Section
            RowLayout {
                Layout.fillWidth: true
                Label {
                    text: qsTr("Discovered Cooling Channels")
                    color: popup.textColor
                    font.pixelSize: Math.round(12 * popup.uiScale)
                    font.weight: Font.DemiBold
                }
                Item { Layout.fillWidth: true }
                Label {
                    text: (popup.fanController ? popup.fanController.systemFanCount : 0) + " " + qsTr("found")
                    color: popup.softTextColor
                    font.pixelSize: Math.round(10 * popup.uiScale)
                }
            }

            // List of Channels (ListView avoids ScrollView layout loop / zero-width collapse)
            ListView {
                id: channelListView
                Layout.fillWidth: true
                Layout.preferredHeight: Math.min(180, count * Math.round(52 * popup.uiScale))
                implicitHeight: Layout.preferredHeight
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                spacing: Math.round(Style.RoTokens.spaceSm * popup.uiScale)
                model: popup.fanController ? popup.fanController.systemFans : []
                visible: popup.fanController && popup.fanController.systemFanCount > 0

                delegate: Rectangle {
                    required property var modelData
                    width: channelListView.width
                    height: Math.round(46 * popup.uiScale)
                    radius: Style.RoTokens.radiusSm
                    color: popup.cardColor
                    border.width: 1
                    border.color: popup.borderColor

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: Math.round(Style.RoTokens.spaceMd * popup.uiScale)
                        anchors.rightMargin: Math.round(Style.RoTokens.spaceMd * popup.uiScale)
                        spacing: Math.round(Style.RoTokens.spaceMd * popup.uiScale)

                        // Type Icon Badge
                        Rectangle {
                            Layout.alignment: Qt.AlignVCenter
                            implicitWidth: Math.round(28 * popup.uiScale)
                            implicitHeight: Math.round(28 * popup.uiScale)
                            radius: Style.RoTokens.radiusSm
                            color: popup.darkMode ? theme.hover : theme.accentC

                            Label {
                                anchors.centerIn: parent
                                text: modelData.type === "CPU" ? "▦" : (modelData.type === "GPU" ? "▰" : "✣")
                                color: popup.accentColor
                                font.pixelSize: Math.round(12 * popup.uiScale)
                                font.weight: Font.Bold
                            }
                        }

                        // Fan Name & RPM/Speed
                        ColumnLayout {
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignVCenter
                            spacing: 1

                            Label {
                                text: modelData.name || qsTr("Fan Channel")
                                color: popup.textColor
                                font.pixelSize: Math.round(12 * popup.uiScale)
                                font.weight: Font.DemiBold
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }

                            Label {
                                text: modelData.rpm > 0 ? qsTr("%1 RPM • %2%").arg(modelData.rpm).arg(modelData.speedPercent || 0)
                                                        : qsTr("0 RPM • Zero-dB / Passive")
                                color: popup.softTextColor
                                font.pixelSize: Math.round(10 * popup.uiScale)
                            }
                        }

                        // Status Badge
                        Rectangle {
                            Layout.alignment: Qt.AlignVCenter
                            implicitHeight: Math.round(22 * popup.uiScale)
                            implicitWidth: statusBadgeText.implicitWidth + Math.round(14 * popup.uiScale)
                            radius: Style.RoTokens.radiusXs
                            color: modelData.controllable ? popup.successBg : (theme.warningBg)
                            border.width: 1
                            border.color: modelData.controllable ? popup.successText : popup.warningText

                            Label {
                                id: statusBadgeText
                                anchors.centerIn: parent
                                text: modelData.controllable ? qsTr("Controllable") : qsTr("Monitored")
                                color: modelData.controllable ? popup.successText : popup.warningText
                                font.pixelSize: Math.round(9 * popup.uiScale)
                                font.weight: Font.DemiBold
                            }
                        }
                    }
                }
            }

            // Fallback state if 0 fans
            Rectangle {
                Layout.fillWidth: true
                visible: !popup.fanController || popup.fanController.systemFanCount === 0
                radius: Style.RoTokens.radiusSm
                color: popup.cardColor
                border.width: 1
                border.color: popup.borderColor
                implicitHeight: emptyStateCol.implicitHeight + Math.round(20 * popup.uiScale)

                ColumnLayout {
                    id: emptyStateCol
                    anchors.fill: parent
                    anchors.margins: Math.round(Style.RoTokens.spaceMd * popup.uiScale)
                    spacing: 3

                    Label {
                        text: qsTr("ACPI Standard Thermal Mode")
                        color: popup.textColor
                        font.pixelSize: Math.round(11 * popup.uiScale)
                        font.weight: Font.DemiBold
                    }

                    Label {
                        text: qsTr("Direct PWM fan tachometers are not exposed by the hardware. Thermal management is handled automatically via motherboard ACPI profiles.")
                        color: popup.softTextColor
                        font.pixelSize: Math.round(10 * popup.uiScale)
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }
                }
            }

            // Bottom Action Bar
            RowLayout {
                Layout.fillWidth: true
                spacing: Math.round(Style.RoTokens.spaceSm * popup.uiScale)

                Button {
                    id: testPulseBtn
                    text: popup.testRunning ? qsTr("Testing Airflow... (%1s)").arg(popup.testRemainingSeconds) : qsTr("Quick Acoustic Test (4s)")
                    enabled: popup.testRunning || popup.controllableFanCount > 0
                    implicitHeight: Math.round(34 * popup.uiScale)
                    hoverEnabled: true
                    opacity: enabled ? 1.0 : 0.55

                    scale: down ? 0.98 : (hovered ? 1.01 : 1.0)
                    Behavior on scale { NumberAnimation { duration: Style.Motion.fastMs } }

                    background: Rectangle {
                        radius: Style.RoTokens.radiusSm
                        color: popup.testRunning
                               ? (theme.warningBg)
                               : (testPulseBtn.hovered ? (theme.cardStrong) : popup.cardColor)
                        border.width: 1
                        border.color: popup.testRunning ? popup.warningText : (testPulseBtn.hovered ? popup.accentColor : popup.borderColor)
                    }

                    contentItem: RowLayout {
                        spacing: Style.RoTokens.spaceSm
                        anchors.centerIn: parent
                        Label {
                            text: popup.testRunning ? "⚡" : "🔊"
                            font.pixelSize: Math.round(11 * popup.uiScale)
                        }
                        Text {
                            text: testPulseBtn.text
                            color: popup.testRunning ? popup.warningText : (testPulseBtn.hovered ? popup.accentColor : popup.textColor)
                            font.pixelSize: Math.round(11 * popup.uiScale)
                            font.weight: Font.DemiBold
                        }
                    }

                    onClicked: {
                        if (popup.testRunning)
                            popup.stopAcousticValidation();
                        else
                            popup.triggerAcousticValidation();
                    }

                    ToolTip {
                        visible: testPulseBtn.hovered && !testPulseBtn.enabled
                        text: qsTr("A readable fan channel was found, but this hardware does not expose manual fan control.")
                        delay: 400
                    }
                }

                Item { Layout.fillWidth: true }

                Button {
                    id: finishBtn
                    text: qsTr("Done")
                    implicitHeight: Math.round(34 * popup.uiScale)
                    implicitWidth: Math.round(85 * popup.uiScale)
                    hoverEnabled: true

                    scale: down ? 0.98 : (hovered ? 1.01 : 1.0)
                    Behavior on scale { NumberAnimation { duration: Style.Motion.fastMs } }

                    background: Rectangle {
                        radius: Style.RoTokens.radiusSm
                        color: finishBtn.down ? (theme.accentA)
                                              : (finishBtn.hovered ? (theme.accentA)
                                                                   : popup.accentColor)
                    }

                    contentItem: Text {
                        text: finishBtn.text
                        color: popup.accentButtonText
                        font.pixelSize: Math.round(11 * popup.uiScale)
                        font.weight: Font.Bold
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        if (popup.testRunning)
                            popup.stopAcousticValidation();
                        popup.close();
                    }
                }
            }
        }
    }

}

}
