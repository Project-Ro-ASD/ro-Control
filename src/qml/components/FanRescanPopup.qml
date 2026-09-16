import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
    id: popup
    required property var fanController
    property var theme: ({})
    property bool darkMode: false
    property real uiScale: 1.0

    // Step 1: "confirm", Step 2: "scanning", Step 3: "done"
    property string step: "confirm"
    property int scanProgress: 0
    property string currentPhaseText: qsTr("Initializing Hardware Probe...")
    property bool phase1Done: false
    property bool phase2Done: false
    property bool phase3Done: false
    property bool testRunning: false
    property int testRemainingSeconds: 4

    readonly property color bgColor: theme && theme.card ? theme.card : (popup.darkMode ? "#241E34" : "#FFFFFF")
    readonly property color cardColor: theme && theme.cardStrong ? theme.cardStrong : (popup.darkMode ? "#2E2742" : "#F8FAFC")
    readonly property color borderColor: theme && theme.border ? theme.border : (popup.darkMode ? "#43385E" : "#E2E8F0")
    readonly property color textColor: theme && theme.text ? theme.text : (popup.darkMode ? "#F8FAFC" : "#0F172A")
    readonly property color softTextColor: theme && theme.textSoft ? theme.textSoft : (popup.darkMode ? "#94A3B8" : "#64748B")
    readonly property color accentColor: theme && theme.accentA ? theme.accentA : (popup.darkMode ? "#818CF8" : "#4F46E5")
    readonly property color accentButtonText: "#FFFFFF"
    readonly property color successBg: theme && theme.successBg ? theme.successBg : (popup.darkMode ? "#143828" : "#ECFDF5")
    readonly property color successText: theme && theme.success ? theme.success : (popup.darkMode ? "#4ADE80" : "#059669")
    readonly property color warningBg: theme && theme.warningBg ? theme.warningBg : (popup.darkMode ? "#3A2E12" : "#FFFBEB")
    readonly property color warningText: theme && theme.warning ? theme.warning : (popup.darkMode ? "#FBBF24" : "#D97706")
    readonly property color infoBg: popup.darkMode ? "#1A233A" : "#EFF6FF"
    readonly property color infoText: popup.darkMode ? "#93C5FD" : "#2563EB"

    modal: true
    focus: true
    closePolicy: (step === "scanning" || testRunning) ? Popup.NoAutoClose : (Popup.CloseOnEscape | Popup.CloseOnPressOutside)
    width: Math.min(parent ? parent.width - 32 : 540, Math.round(540 * popup.uiScale))
    height: Math.min(parent ? parent.height - 40 : 620, contentColumn.implicitHeight + topPadding + bottomPadding)
    x: Math.round(((parent ? parent.width : 600) - width) / 2)
    y: Math.max(16, Math.round(((parent ? parent.height : 600) - height) / 2))
    padding: Math.round(18 * popup.uiScale)

    Behavior on height {
        NumberAnimation { duration: 180; easing.type: Easing.OutCubic }
    }
    Behavior on y {
        NumberAnimation { duration: 180; easing.type: Easing.OutCubic }
    }

    enter: Transition {
        ParallelAnimation {
            NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: 200; easing.type: Easing.OutCubic }
            NumberAnimation { property: "scale"; from: 0.95; to: 1.0; duration: 200; easing.type: Easing.OutBack }
        }
    }

    exit: Transition {
        ParallelAnimation {
            NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: 150; easing.type: Easing.InCubic }
            NumberAnimation { property: "scale"; from: 1.0; to: 0.96; duration: 150 }
        }
    }

    function openWizard() {
        step = "confirm";
        scanProgress = 0;
        phase1Done = false;
        phase2Done = false;
        phase3Done = false;
        testRunning = false;
        testRemainingSeconds = 4;
        testCountdownTimer.stop();
        currentPhaseText = qsTr("Initializing Hardware Probe...");
        popup.open();
    }

    function startScanProcess() {
        step = "scanning";
        scanProgress = 15;
        currentPhaseText = qsTr("Probing Linux HWMON & ACPI kernel thermal controllers...");
        if (popup.fanController)
            popup.fanController.runHardwareSetup();
        phase1Timer.start();
    }

    function triggerAcousticValidation() {
        if (testRunning || !popup.fanController)
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

    Timer {
        id: phase1Timer
        interval: 600
        repeat: false
        onTriggered: {
            popup.phase1Done = true;
            popup.scanProgress = 55;
            popup.currentPhaseText = qsTr("Querying NVIDIA NV-CONTROL & GPU fan tachometers...");
            phase2Timer.start();
        }
    }

    Timer {
        id: phase2Timer
        interval: 700
        repeat: false
        onTriggered: {
            popup.phase2Done = true;
            popup.scanProgress = 85;
            popup.currentPhaseText = qsTr("Calibrating zero-RPM thresholds & refreshing telemetry...");
            if (popup.fanController) {
                popup.fanController.refresh();
            }
            phase3Timer.start();
        }
    }

    Timer {
        id: phase3Timer
        interval: 500
        repeat: false
        onTriggered: {
            popup.phase3Done = true;
            popup.scanProgress = 100;
            popup.step = "done";
        }
    }

    background: Rectangle {
        radius: 14
        color: popup.bgColor
        border.width: 1
        border.color: popup.borderColor
    }

    contentItem: ColumnLayout {
        id: contentColumn
        spacing: Math.round(14 * popup.uiScale)
        width: parent.width

        // ---------------- HEADER ----------------
        RowLayout {
            Layout.fillWidth: true
            spacing: Math.round(12 * popup.uiScale)

            Rectangle {
                implicitWidth: Math.round(38 * popup.uiScale)
                implicitHeight: Math.round(38 * popup.uiScale)
                radius: Math.round(10 * popup.uiScale)
                color: popup.step === "done" ? popup.successBg : (popup.darkMode ? "#31284A" : "#EEF2FF")
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
                          ? qsTr("Enumerate cooling fans, calibrate PWM headers, and sync sensor registers")
                          : (popup.step === "scanning"
                             ? qsTr("Please wait while hardware sensors are probed...")
                             : qsTr("All system cooling devices have been synchronized"))
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
                    radius: Math.round(14 * popup.uiScale)
                    color: closeBtn.hovered ? (popup.darkMode ? "#3B3156" : "#E2E8F0") : "transparent"
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
            spacing: Math.round(12 * popup.uiScale)

            Rectangle {
                Layout.fillWidth: true
                radius: 10
                color: popup.cardColor
                border.width: 1
                border.color: popup.borderColor
                implicitHeight: confirmContentCol.implicitHeight + Math.round(24 * popup.uiScale)

                ColumnLayout {
                    id: confirmContentCol
                    anchors.fill: parent
                    anchors.margins: Math.round(12 * popup.uiScale)
                    spacing: Math.round(10 * popup.uiScale)

                    Label {
                        text: qsTr("What will this hardware setup wizard do?")
                        color: popup.textColor
                        font.pixelSize: Math.round(12 * popup.uiScale)
                        font.weight: Font.DemiBold
                    }

                    // Feature row 1
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10
                        Rectangle {
                            implicitWidth: Math.round(24 * popup.uiScale)
                            implicitHeight: Math.round(24 * popup.uiScale)
                            radius: 6
                            color: popup.darkMode ? "#222C42" : "#EFF6FF"
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
                        spacing: 10
                        Rectangle {
                            implicitWidth: Math.round(24 * popup.uiScale)
                            implicitHeight: Math.round(24 * popup.uiScale)
                            radius: 6
                            color: popup.darkMode ? "#2C2242" : "#F5F3FF"
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
                        spacing: 10
                        Rectangle {
                            implicitWidth: Math.round(24 * popup.uiScale)
                            implicitHeight: Math.round(24 * popup.uiScale)
                            radius: 6
                            color: popup.darkMode ? "#1E342B" : "#ECFDF5"
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
                radius: 8
                color: popup.infoBg
                implicitHeight: safeNoteCol.implicitHeight + Math.round(14 * popup.uiScale)

                RowLayout {
                    id: safeNoteCol
                    anchors.fill: parent
                    anchors.margins: Math.round(8 * popup.uiScale)
                    spacing: 8

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
                spacing: Math.round(8 * popup.uiScale)

                Item { Layout.fillWidth: true }

                Button {
                    id: cancelBtn
                    text: qsTr("Cancel")
                    implicitHeight: Math.round(34 * popup.uiScale)
                    implicitWidth: Math.round(85 * popup.uiScale)
                    hoverEnabled: true

                    background: Rectangle {
                        radius: 8
                        color: cancelBtn.down ? (popup.darkMode ? "#3B3156" : "#E2E8F0")
                                              : (cancelBtn.hovered ? (popup.darkMode ? "#342D4A" : "#F1F5F9")
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
                    Behavior on scale { NumberAnimation { duration: 100 } }

                    background: Rectangle {
                        radius: 8
                        color: startScanBtn.down ? (popup.darkMode ? "#4F46E5" : "#3730A3")
                                                 : (startScanBtn.hovered ? (popup.darkMode ? "#939BFA" : "#5B52E8")
                                                                         : popup.accentColor)
                    }

                    contentItem: RowLayout {
                        spacing: 6
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
            spacing: Math.round(12 * popup.uiScale)

            Rectangle {
                Layout.fillWidth: true
                radius: 10
                color: popup.cardColor
                border.width: 1
                border.color: popup.borderColor
                implicitHeight: scanningContentCol.implicitHeight + Math.round(24 * popup.uiScale)

                ColumnLayout {
                    id: scanningContentCol
                    anchors.fill: parent
                    anchors.margins: Math.round(14 * popup.uiScale)
                    spacing: Math.round(12 * popup.uiScale)

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

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
                        radius: 4
                        color: popup.darkMode ? "#1E2238" : "#E2E8F0"
                        clip: true

                        Rectangle {
                            anchors.left: parent.left
                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                            width: parent.width * (popup.scanProgress / 100.0)
                            radius: 4
                            color: popup.accentColor

                            Behavior on width {
                                NumberAnimation { duration: 300; easing.type: Easing.OutQuad }
                            }
                        }
                    }

                    // Checklist
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: Math.round(8 * popup.uiScale)

                        RowLayout {
                            spacing: 10
                            Rectangle {
                                implicitWidth: Math.round(18 * popup.uiScale)
                                implicitHeight: Math.round(18 * popup.uiScale)
                                radius: 4
                                color: popup.phase1Done ? popup.successBg : (popup.darkMode ? "#31284A" : "#EEF2FF")
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
                            spacing: 10
                            Rectangle {
                                implicitWidth: Math.round(18 * popup.uiScale)
                                implicitHeight: Math.round(18 * popup.uiScale)
                                radius: 4
                                color: popup.phase2Done ? popup.successBg : (popup.phase1Done ? (popup.darkMode ? "#31284A" : "#EEF2FF") : (popup.darkMode ? "#231F33" : "#F1F5F9"))
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
                            spacing: 10
                            Rectangle {
                                implicitWidth: Math.round(18 * popup.uiScale)
                                implicitHeight: Math.round(18 * popup.uiScale)
                                radius: 4
                                color: popup.phase3Done ? popup.successBg : (popup.phase2Done ? (popup.darkMode ? "#31284A" : "#EEF2FF") : (popup.darkMode ? "#231F33" : "#F1F5F9"))
                                Label {
                                    anchors.centerIn: parent
                                    text: popup.phase3Done ? "✓" : "3"
                                    color: popup.phase3Done ? popup.successText : (popup.phase2Done ? popup.accentColor : popup.softTextColor)
                                    font.pixelSize: Math.round(10 * popup.uiScale)
                                    font.weight: Font.Bold
                                }
                            }
                            Label {
                                text: qsTr("Telemetry Calibration & Sensor Sync")
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
            spacing: Math.round(10 * popup.uiScale)

            // Success Summary Banner
            Rectangle {
                Layout.fillWidth: true
                radius: 10
                color: popup.successBg
                border.width: 1
                border.color: popup.successText
                implicitHeight: doneLayout.implicitHeight + Math.round(16 * popup.uiScale)

                ColumnLayout {
                    id: doneLayout
                    anchors.fill: parent
                    anchors.margins: Math.round(10 * popup.uiScale)
                    spacing: 3

                    RowLayout {
                        spacing: 8
                        Label {
                            text: "✓"
                            color: popup.successText
                            font.pixelSize: Math.round(14 * popup.uiScale)
                            font.weight: Font.Bold
                        }
                        Label {
                            text: qsTr("Hardware Scan Successfully Completed!")
                            color: popup.successText
                            font.pixelSize: Math.round(12 * popup.uiScale)
                            font.weight: Font.Bold
                        }
                    }

                    Label {
                        text: qsTr("%1 Active cooling fan device(s) synchronized and calibrated.")
                              .arg(popup.fanController ? popup.fanController.systemFanCount : 0)
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
                spacing: Math.round(6 * popup.uiScale)
                model: popup.fanController ? popup.fanController.systemFans : []
                visible: popup.fanController && popup.fanController.systemFanCount > 0

                delegate: Rectangle {
                    required property var modelData
                    width: channelListView.width
                    height: Math.round(46 * popup.uiScale)
                    radius: 8
                    color: popup.cardColor
                    border.width: 1
                    border.color: popup.borderColor

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: Math.round(10 * popup.uiScale)
                        anchors.rightMargin: Math.round(10 * popup.uiScale)
                        spacing: Math.round(10 * popup.uiScale)

                        // Type Icon Badge
                        Rectangle {
                            Layout.alignment: Qt.AlignVCenter
                            implicitWidth: Math.round(28 * popup.uiScale)
                            implicitHeight: Math.round(28 * popup.uiScale)
                            radius: 6
                            color: popup.darkMode ? "#3B3156" : "#EEF2FF"

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
                            radius: 4
                            color: modelData.controllable ? popup.successBg : (popup.darkMode ? "#2E241E" : "#FFFBEB")
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
                radius: 8
                color: popup.cardColor
                border.width: 1
                border.color: popup.borderColor
                implicitHeight: emptyStateCol.implicitHeight + Math.round(20 * popup.uiScale)

                ColumnLayout {
                    id: emptyStateCol
                    anchors.fill: parent
                    anchors.margins: Math.round(12 * popup.uiScale)
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
                spacing: Math.round(8 * popup.uiScale)

                Button {
                    id: testPulseBtn
                    text: popup.testRunning ? qsTr("Testing Airflow... (%1s)").arg(popup.testRemainingSeconds) : qsTr("Quick Acoustic Test (4s)")
                    implicitHeight: Math.round(34 * popup.uiScale)
                    hoverEnabled: true

                    scale: down ? 0.98 : (hovered ? 1.01 : 1.0)
                    Behavior on scale { NumberAnimation { duration: 100 } }

                    background: Rectangle {
                        radius: 8
                        color: popup.testRunning
                               ? (popup.darkMode ? "#3A2E12" : "#FFFBEB")
                               : (testPulseBtn.hovered ? (popup.darkMode ? "#342D4A" : "#F1F5F9") : popup.cardColor)
                        border.width: 1
                        border.color: popup.testRunning ? popup.warningText : (testPulseBtn.hovered ? popup.accentColor : popup.borderColor)
                    }

                    contentItem: RowLayout {
                        spacing: 6
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
                }

                Item { Layout.fillWidth: true }

                Button {
                    id: finishBtn
                    text: qsTr("Done")
                    implicitHeight: Math.round(34 * popup.uiScale)
                    implicitWidth: Math.round(85 * popup.uiScale)
                    hoverEnabled: true

                    scale: down ? 0.98 : (hovered ? 1.01 : 1.0)
                    Behavior on scale { NumberAnimation { duration: 100 } }

                    background: Rectangle {
                        radius: 8
                        color: finishBtn.down ? (popup.darkMode ? "#4F46E5" : "#3730A3")
                                              : (finishBtn.hovered ? (popup.darkMode ? "#939BFA" : "#5B52E8")
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
