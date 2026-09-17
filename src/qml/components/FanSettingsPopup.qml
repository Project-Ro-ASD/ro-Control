import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
    id: popup
    required property var fanController
    property var theme: ({})
    property bool darkMode: false
    property real uiScale: 1.0

    property var currentFan: null
    property string activeMode: "auto"
    property int activeManualSpeed: 50
    property int activeThermalThreshold: 85
    property var activeCurvePoints: []
    property bool saveFeedbackVisible: false
    property bool testingFanActive: false
    property bool fanTestFailed: false
    property bool editingFanName: false
    property string pendingFanName: ""
    property real fanAngle: 0
    property string operationError: ""
    readonly property bool hasHardwareControl: currentFan && currentFan.controllable === true
    readonly property bool compactLayout: width < Math.round(520 * popup.uiScale)

    readonly property color bgColor: theme && theme.card ? theme.card : (popup.darkMode ? "#241E34" : "#FFFFFF")
    readonly property color cardColor: theme && theme.cardStrong ? theme.cardStrong : (popup.darkMode ? "#2E2742" : "#F8FAFC")
    readonly property color borderColor: theme && theme.border ? theme.border : (popup.darkMode ? "#43385E" : "#E2E8F0")
    readonly property color textColor: theme && theme.text ? theme.text : (popup.darkMode ? "#F8FAFC" : "#0F172A")
    readonly property color softTextColor: theme && theme.textSoft ? theme.textSoft : (popup.darkMode ? "#94A3B8" : "#64748B")
    readonly property color accentColor: theme && theme.accentA ? theme.accentA : (popup.darkMode ? "#818CF8" : "#4F46E5")
    readonly property color accentButtonText: "#FFFFFF"
    readonly property color infoBg: theme && theme.infoBg ? theme.infoBg : (popup.darkMode ? "#1E2548" : "#EFF6FF")
    readonly property color infoText: popup.darkMode ? "#93C5FD" : "#2563EB"
    readonly property color warningBg: theme && theme.warningBg ? theme.warningBg : (popup.darkMode ? "#3A2E12" : "#FFFBEB")
    readonly property color warningText: theme && theme.warning ? theme.warning : (popup.darkMode ? "#FBBF24" : "#D97706")
    readonly property color successBg: theme && theme.successBg ? theme.successBg : (popup.darkMode ? "#143828" : "#ECFDF5")
    readonly property color successText: theme && theme.success ? theme.success : (popup.darkMode ? "#4ADE80" : "#059669")

    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    width: Math.max(0, Math.min(parent ? parent.width - 24 : 580, Math.round(580 * popup.uiScale)))
    height: Math.max(0, Math.min(parent ? parent.height - 24 : 560, Math.round(560 * popup.uiScale)))
    x: Math.round(((parent ? parent.width : 600) - width) / 2)
    y: Math.round(((parent ? parent.height : 600) - height) / 2)
    padding: Math.round(16 * popup.uiScale)

    enter: Transition {
        ParallelAnimation {
            NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: 200; easing.type: Easing.OutCubic }
            NumberAnimation { property: "scale"; from: 0.94; to: 1.0; duration: 200; easing.type: Easing.OutBack }
        }
    }

    exit: Transition {
        ParallelAnimation {
            NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: 150; easing.type: Easing.InCubic }
            NumberAnimation { property: "scale"; from: 1.0; to: 0.96; duration: 150 }
        }
    }

    function openForFan(fanData) {
        if (!fanData)
            return;
        currentFan = fanData;
        activeMode = fanData.mode || "auto";
        activeManualSpeed = fanData.manualSpeedPercent !== undefined ? fanData.manualSpeedPercent : (fanData.speedPercent || 50);
        activeThermalThreshold = fanData.thermalThresholdC || 85;
        testingFanActive = false;
        fanTestFailed = false;
        operationError = "";
        editingFanName = false;
        pendingFanName = fanData.name || "";
        
        var pts = [];
        if (fanData.customCurvePoints && fanData.customCurvePoints.length > 0) {
            for (var i = 0; i < fanData.customCurvePoints.length; ++i) {
                pts.push({
                    temp: fanData.customCurvePoints[i].temp || (40 + i * 15),
                    speed: fanData.customCurvePoints[i].speed || (30 + i * 20)
                });
            }
        } else {
            pts = [
                { temp: 40, speed: 30 },
                { temp: 55, speed: 50 },
                { temp: 70, speed: 70 },
                { temp: 85, speed: 100 }
            ];
        }
        activeCurvePoints = pts;
        saveFeedbackVisible = false;
        popup.open();
    }

    function syncLiveFanData() {
        if (!popup.opened || !currentFan || !popup.fanController)
            return;
        var list = popup.fanController.systemFans;
        for (var i = 0; i < list.length; ++i) {
            if (list[i].id === currentFan.id) {
                currentFan = list[i];
                break;
            }
        }
    }

    Connections {
        target: popup.fanController
        enabled: popup.opened
        function onSystemFansChanged() {
            if (popup.opened) popup.syncLiveFanData();
        }
        function onCurrentFanSpeedPercentChanged() {
            if (popup.opened) popup.syncLiveFanData();
        }
        function onCurrentRpmChanged() {
            if (popup.opened) popup.syncLiveFanData();
        }
    }

    function modeDescription(mode) {
        switch (mode) {
        case "silent":
            return qsTr("Acoustic priority profile. Minimizes fan noise and delays speed ramp-up for quiet operation.");
        case "balanced":
            return qsTr("Dynamically balances thermal dissipation and acoustic comfort based on workload.");
        case "performance":
            return qsTr("Aggressive cooling profile providing maximum sustained airflow for heavy loads.");
        case "manual":
            return qsTr("Locked fixed fan speed percentage set directly by the manual slider.");
        case "custom":
            return qsTr("Multi-point custom temperature-to-speed fan curve with smooth interpolation.");
        case "auto":
        default:
            return qsTr("Automatic cooling curve managed natively by hardware VBIOS / BIOS thermal controllers.");
        }
    }

    function applyAllSettings() {
        if (!currentFan || !popup.fanController || !hasHardwareControl)
            return;
        var fanId = currentFan.id;
        var success = popup.fanController.setFanModeForFan(fanId, activeMode);
        success = popup.fanController.setManualSpeedForFan(fanId, activeManualSpeed) && success;
        success = popup.fanController.setThermalThresholdForFan(fanId, activeThermalThreshold) && success;
        
        for (var i = 0; i < activeCurvePoints.length; ++i) {
            success = popup.fanController.setCustomCurvePointForFan(fanId, i, activeCurvePoints[i].temp, activeCurvePoints[i].speed) && success;
        }

        success = popup.fanController.applyFanConfiguration(fanId) && success;
        if (success) {
            operationError = "";
            saveFeedbackVisible = true;
            feedbackTimer.restart();
        } else {
            saveFeedbackVisible = false;
            operationError = popup.fanController.statusMessage || qsTr("The fan controller rejected this change.");
        }
    }

    function resetToAutoMode() {
        if (!currentFan || !popup.fanController || !hasHardwareControl)
            return;
        activeMode = "auto";
        if (popup.fanController.resetFanToAuto(currentFan.id)) {
            operationError = "";
            saveFeedbackVisible = true;
            feedbackTimer.restart();
        } else {
            operationError = popup.fanController.statusMessage || qsTr("The automatic fan mode could not be restored.");
        }
    }

    function triggerQuickTest100() {
        if (!currentFan || !popup.fanController)
            return;
        fanTestFailed = !popup.fanController.testFanSpeedForFan(currentFan.id, 100);
        if (fanTestFailed)
            return;
        testingFanActive = true;
        testDurationTimer.restart();
    }

    function saveFanName() {
        if (!currentFan || !popup.fanController)
            return;
        if (popup.fanController.setFanDisplayName(currentFan.id, pendingFanName)) {
            currentFan = popup.fanController.getFanConfig(currentFan.id);
            editingFanName = false;
        }
    }

    Timer {
        id: testDurationTimer
        interval: 6000
        repeat: false
        onTriggered: {
            popup.testingFanActive = false;
            if (popup.currentFan && popup.fanController) {
                popup.fanController.restoreFanControlForFan(popup.currentFan.id);
            }
        }
    }

    onClosed: {
        if (testingFanActive) {
            testDurationTimer.stop();
            testingFanActive = false;
            if (currentFan && fanController)
                fanController.restoreFanControlForFan(currentFan.id);
        }
    }

    Timer {
        id: feedbackTimer
        interval: 2500
        repeat: false
        onTriggered: popup.saveFeedbackVisible = false
    }

    background: Rectangle {
        radius: 16
        color: popup.bgColor
        border.width: 1
        border.color: popup.borderColor
    }

    contentItem: ColumnLayout {
        width: popup.availableWidth
        spacing: Math.round(12 * popup.uiScale)

        // Pinned Header Bar (Always visible at top)
        RowLayout {
            Layout.fillWidth: true
            Layout.preferredWidth: popup.availableWidth
            spacing: 12

            Rectangle {
                visible: !popup.compactLayout
                implicitWidth: Math.round(36 * popup.uiScale)
                implicitHeight: Math.round(36 * popup.uiScale)
                radius: Math.round(18 * popup.uiScale)
                color: popup.darkMode ? "#3B3156" : "#EEF2FF"
                border.width: 1
                border.color: popup.accentColor

                Label {
                    id: spinningFanLabel
                    anchors.centerIn: parent
                    text: !popup.currentFan ? "✣"
                          : (popup.currentFan.type === "CPU" ? "▦"
                             : (popup.currentFan.type === "GPU" ? "▰" : "✣"))
                    color: popup.accentColor
                    font.pixelSize: Math.round(17 * popup.uiScale)
                    font.weight: Font.Bold
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    Label {
                        visible: !popup.editingFanName
                        text: popup.currentFan ? popup.currentFan.name : qsTr("Fan Settings & Dynamics")
                        color: popup.textColor
                        font.pixelSize: Math.round(16 * popup.uiScale)
                        font.weight: Font.DemiBold
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                    }

                    TextField {
                        id: fanNameField
                        visible: popup.editingFanName
                        Layout.fillWidth: true
                        text: popup.pendingFanName
                        maximumLength: 64
                        selectByMouse: true
                        color: popup.textColor
                        font.pixelSize: Math.round(16 * popup.uiScale)
                        font.weight: Font.DemiBold
                        background: Rectangle {
                            radius: 6
                            color: popup.cardColor
                            border.width: 1
                            border.color: fanNameField.activeFocus ? popup.accentColor : popup.borderColor
                        }
                        onTextEdited: popup.pendingFanName = text
                        onAccepted: popup.saveFanName()
                    }

                    ToolButton {
                        id: renameFanButton
                        visible: popup.currentFan
                        text: popup.editingFanName ? "✓" : "✎"
                        implicitWidth: Math.round(30 * popup.uiScale)
                        implicitHeight: Math.round(30 * popup.uiScale)
                        hoverEnabled: true
                        background: Rectangle {
                            radius: width / 2
                            color: renameFanButton.hovered
                                   ? (popup.darkMode ? "#3B3156" : "#EEF2FF")
                                   : "transparent"
                            border.width: renameFanButton.hovered ? 1 : 0
                            border.color: popup.accentColor
                        }
                        contentItem: Text {
                            text: renameFanButton.text
                            color: renameFanButton.hovered ? popup.accentColor : popup.softTextColor
                            font.pixelSize: Math.round(14 * popup.uiScale)
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        onClicked: {
                            if (popup.editingFanName)
                                popup.saveFanName();
                            else {
                                popup.editingFanName = true;
                                fanNameField.forceActiveFocus();
                                fanNameField.selectAll();
                            }
                        }
                        ToolTip.visible: hovered
                        ToolTip.text: popup.editingFanName ? qsTr("Save name") : qsTr("Rename fan")
                    }

                    Rectangle {
                        visible: !popup.compactLayout
                        implicitWidth: fanTypeBadge.implicitWidth + 12
                        implicitHeight: Math.round(20 * popup.uiScale)
                        radius: 4
                        color: popup.darkMode ? "#342D4A" : "#E2E8F0"

                        Label {
                            id: fanTypeBadge
                            anchors.centerIn: parent
                            text: popup.currentFan ? (popup.currentFan.type || "SYS") : "FAN"
                            color: popup.accentColor
                            font.pixelSize: Math.round(10 * popup.uiScale)
                            font.weight: Font.Bold
                        }
                    }
                }

                Label {
                    Layout.fillWidth: true
                    text: popup.currentFan ? (qsTr("Hardware Channel: %1 • Interface: %2").arg(popup.currentFan.id).arg(popup.currentFan.channel || "PWM")) : ""
                    color: popup.softTextColor
                    font.pixelSize: Math.round(11 * popup.uiScale)
                    elide: Text.ElideMiddle
                }
            }

            Item {
                Layout.fillWidth: true
            }

            ToolButton {
                id: closeBtn
                Layout.alignment: Qt.AlignTop | Qt.AlignRight
                Layout.preferredWidth: Math.round(32 * popup.uiScale)
                Layout.fillWidth: false
                implicitWidth: Math.round(32 * popup.uiScale)
                implicitHeight: Math.round(32 * popup.uiScale)
                hoverEnabled: true

                background: Rectangle {
                    radius: Math.round(16 * popup.uiScale)
                    color: closeBtn.hovered ? (popup.darkMode ? "#3B3156" : "#E2E8F0") : "transparent"
                }

                contentItem: Text {
                    text: "✕"
                    color: closeBtn.hovered ? popup.textColor : popup.softTextColor
                    font.pixelSize: Math.round(15 * popup.uiScale)
                    font.weight: Font.DemiBold
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                ToolTip {
                    visible: closeBtn.hovered
                    text: qsTr("Close")
                    delay: 400
                }

                onClicked: popup.close()
            }
        }

        ScrollView {
            id: popupScroll
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: availableWidth

            ColumnLayout {
                id: popupScrollLayout
                width: popupScroll.availableWidth
                spacing: Math.round(12 * popup.uiScale)

                Rectangle {
                    Layout.fillWidth: true
                    visible: popup.currentFan && !popup.hasHardwareControl
                    radius: 8
                    color: popup.infoBg
                    border.width: 1
                    border.color: popup.accentColor
                    implicitHeight: readOnlyNotice.implicitHeight + Math.round(16 * popup.uiScale)

                    Label {
                        id: readOnlyNotice
                        anchors.fill: parent
                        anchors.margins: Math.round(8 * popup.uiScale)
                        text: qsTr("This channel provides telemetry only. Its BIOS, firmware, or driver owns fan control, so ro-Control will not present simulated settings.")
                        color: popup.infoText
                        font.pixelSize: Math.round(11 * popup.uiScale)
                        wrapMode: Text.WordWrap
                    }
                }

                // Live Dynamic Telemetry Cards (Speed %, RPM, Temperature)
                Rectangle {
                    Layout.fillWidth: true
                    radius: 12
                    color: popup.cardColor
                    border.width: 1
                    border.color: popup.borderColor
                    implicitHeight: liveMetricsRow.implicitHeight + 20

                    GridLayout {
                        id: liveMetricsRow
                        anchors.fill: parent
                        anchors.margins: 12
                        columnSpacing: 12
                        rowSpacing: 10
                        columns: popup.compactLayout ? 1 : 5

                        // Live Speed Gauge
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            RowLayout {
                                Layout.fillWidth: true
                                Label {
                                    text: qsTr("LIVE SPEED")
                                    color: popup.softTextColor
                                    font.pixelSize: Math.round(10 * popup.uiScale)
                                    font.weight: Font.DemiBold
                                }
                                Item { Layout.fillWidth: true }
                                Label {
                                    text: (popup.currentFan && popup.currentFan.speedPercent !== undefined ? popup.currentFan.speedPercent : 0) + "%"
                                    color: popup.textColor
                                    font.pixelSize: Math.round(18 * popup.uiScale)
                                    font.weight: Font.Bold
                                }
                            }

                            // Dynamic Capsule Progress Bar
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
                                    width: {
                                        var spd = popup.currentFan ? (popup.currentFan.speedPercent || 0) : 0;
                                        return parent.width * Math.min(1.0, Math.max(0.0, spd / 100.0));
                                    }
                                    radius: 4
                                    color: {
                                        var s = popup.currentFan ? (popup.currentFan.speedPercent || 0) : 0;
                                        if (s > 80) return popup.warningText;
                                        if (s > 50) return popup.accentColor;
                                        return popup.darkMode ? "#34D399" : "#10B981";
                                    }

                                    Behavior on width { NumberAnimation { duration: 250; easing.type: Easing.OutQuad } }
                                }
                            }
                        }

                        Rectangle {
                            visible: !popup.compactLayout
                            implicitWidth: 1
                            Layout.fillHeight: true
                            color: popup.borderColor
                        }

                        // Live RPM Readout
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            RowLayout {
                                Layout.fillWidth: true
                                Label {
                                    text: qsTr("TACHOMETER")
                                    color: popup.softTextColor
                                    font.pixelSize: Math.round(10 * popup.uiScale)
                                    font.weight: Font.DemiBold
                                }
                                Item { Layout.fillWidth: true }
                                Label {
                                    text: {
                                        if (!popup.currentFan) return qsTr("--");
                                        if (popup.currentFan.rpm > 0) return popup.currentFan.rpm + " RPM";
                                        return qsTr("0 RPM");
                                    }
                                    color: popup.textColor
                                    font.pixelSize: Math.round(18 * popup.uiScale)
                                    font.weight: Font.Bold
                                }
                            }

                            Label {
                                text: (popup.currentFan && (popup.currentFan.isZeroRpm || popup.currentFan.rpm === 0))
                                      ? qsTr("Silent Zero-RPM Active")
                                      : qsTr("Active Airflow Cooling")
                                color: (popup.currentFan && (popup.currentFan.isZeroRpm || popup.currentFan.rpm === 0))
                                       ? (popup.darkMode ? "#34D399" : "#10B981")
                                       : popup.softTextColor
                                font.pixelSize: Math.round(10 * popup.uiScale)
                                font.weight: Font.Medium
                            }
                        }

                        Rectangle {
                            visible: !popup.compactLayout
                            implicitWidth: 1
                            Layout.fillHeight: true
                            color: popup.borderColor
                        }

                        // Temperature
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            RowLayout {
                                Layout.fillWidth: true
                                Label {
                                    text: qsTr("TEMPERATURE")
                                    color: popup.softTextColor
                                    font.pixelSize: Math.round(10 * popup.uiScale)
                                    font.weight: Font.DemiBold
                                }
                                Item { Layout.fillWidth: true }
                                Label {
                                    text: (popup.currentFan && popup.currentFan.temperatureC > 0)
                                          ? (popup.currentFan.temperatureC + " °C")
                                          : qsTr("--")
                                    color: {
                                        var t = popup.currentFan ? popup.currentFan.temperatureC : 0;
                                        if (t >= 80) return popup.warningText;
                                        return popup.textColor;
                                    }
                                    font.pixelSize: Math.round(18 * popup.uiScale)
                                    font.weight: Font.Bold
                                }
                            }

                            Label {
                                text: {
                                    var t = popup.currentFan ? popup.currentFan.temperatureC : 0;
                                    if (t >= 80) return qsTr("Thermal Load Elevated");
                                    if (t >= 60) return qsTr("Moderate Thermals");
                                    return qsTr("Optimal Thermal State");
                                }
                                color: popup.softTextColor
                                font.pixelSize: Math.round(10 * popup.uiScale)
                            }
                        }
                    }
                }

                // Profile Selector Row
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 6

                    Label {
                        text: qsTr("Select Optimization Profile")
                        color: popup.textColor
                        font.pixelSize: Math.round(13 * popup.uiScale)
                        font.weight: Font.DemiBold
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        columns: width > 540 ? 6 : (width > 360 ? 3 : 2)
                        columnSpacing: 6
                        rowSpacing: 6

                        Repeater {
                            model: [
                                { mode: "auto", label: qsTr("Auto") },
                                { mode: "silent", label: qsTr("Silent") },
                                { mode: "balanced", label: qsTr("Balanced") },
                                { mode: "performance", label: qsTr("Performance") },
                                { mode: "manual", label: qsTr("Manual") },
                                { mode: "custom", label: qsTr("Custom") }
                            ]

                            delegate: Button {
                                id: modeBtn
                                required property var modelData
                                Layout.fillWidth: true
                                implicitHeight: Math.round(36 * popup.uiScale)
                                hoverEnabled: true

                                background: Rectangle {
                                    radius: 8
                                    color: popup.activeMode === modeBtn.modelData.mode ? popup.accentColor
                                           : (modeBtn.hovered ? (popup.darkMode ? "#3B3156" : "#F1F5F9") : popup.cardColor)
                                    border.width: 1
                                    border.color: popup.activeMode === modeBtn.modelData.mode ? popup.accentColor : popup.borderColor

                                }

                                contentItem: Text {
                                    text: modeBtn.modelData.label
                                    color: popup.activeMode === modeBtn.modelData.mode ? popup.accentButtonText : popup.textColor
                                    font.pixelSize: Math.round(12 * popup.uiScale)
                                    font.weight: popup.activeMode === modeBtn.modelData.mode ? Font.Bold : Font.DemiBold
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }

                                enabled: popup.hasHardwareControl
                                onClicked: popup.activeMode = modeBtn.modelData.mode
                            }
                        }
                    }
                }

                // Dynamic Mode Configuration Section
                Rectangle {
                    Layout.fillWidth: true
                    radius: 12
                    color: popup.cardColor
                    border.width: 1
                    border.color: popup.borderColor
                    implicitHeight: configAreaLayout.implicitHeight + 24

                    ColumnLayout {
                        id: configAreaLayout
                        enabled: popup.hasHardwareControl
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 12

                        // Manual Slider View
                        ColumnLayout {
                            visible: popup.activeMode === "manual"
                            Layout.fillWidth: true
                            spacing: 10

                            RowLayout {
                                Layout.fillWidth: true

                                Label {
                                    text: qsTr("Manual Fixed Fan Speed Target")
                                    color: popup.textColor
                                    font.pixelSize: Math.round(13 * popup.uiScale)
                                    font.weight: Font.DemiBold
                                }

                                Item { Layout.fillWidth: true }

                                Rectangle {
                                    implicitWidth: Math.round(60 * popup.uiScale)
                                    implicitHeight: Math.round(26 * popup.uiScale)
                                    radius: 6
                                    color: popup.accentColor

                                    Label {
                                        anchors.centerIn: parent
                                        text: popup.activeManualSpeed + "%"
                                        color: popup.accentButtonText
                                        font.pixelSize: Math.round(13 * popup.uiScale)
                                        font.weight: Font.Bold
                                    }
                                }
                            }

                            Slider {
                                id: manualPopupSlider
                                Layout.fillWidth: true
                                from: 0
                                to: 100
                                stepSize: 1
                                value: popup.activeManualSpeed
                                onMoved: popup.activeManualSpeed = Math.round(value)

                                background: Rectangle {
                                    x: manualPopupSlider.leftPadding
                                    y: manualPopupSlider.topPadding + manualPopupSlider.availableHeight / 2 - height / 2
                                    implicitHeight: 8
                                    width: manualPopupSlider.availableWidth
                                    height: implicitHeight
                                    radius: 4
                                    color: popup.darkMode ? "#1E2238" : "#E2E8F0"

                                    Rectangle {
                                        width: manualPopupSlider.visualPosition * parent.width
                                        height: parent.height
                                        color: popup.accentColor
                                        radius: 4
                                    }
                                }

                                handle: Rectangle {
                                    x: manualPopupSlider.leftPadding + manualPopupSlider.visualPosition * (manualPopupSlider.availableWidth - width)
                                    y: manualPopupSlider.topPadding + manualPopupSlider.availableHeight / 2 - height / 2
                                    implicitWidth: 20
                                    implicitHeight: 20
                                    radius: 10
                                    color: "#FFFFFF"
                                    border.color: popup.accentColor
                                    border.width: 2.5
                                }
                            }

                            GridLayout {
                                Layout.fillWidth: true
                                columnSpacing: 8
                                rowSpacing: 8
                                columns: popup.compactLayout ? 2 : 6

                                Label {
                                    Layout.columnSpan: popup.compactLayout ? 2 : 1
                                    Layout.fillWidth: true
                                    text: qsTr("Quick Speed Presets:")
                                    color: popup.softTextColor
                                    font.pixelSize: Math.round(11 * popup.uiScale)
                                }

                                Repeater {
                                    model: [0, 30, 50, 75, 100]

                                    delegate: Button {
                                        id: presetBtn
                                        required property int modelData
                                        text: presetBtn.modelData === 0 ? qsTr("0% (Stop)") : (presetBtn.modelData + "%")
                                        implicitHeight: Math.round(28 * popup.uiScale)
                                        Layout.fillWidth: true
                                        hoverEnabled: true

                                        background: Rectangle {
                                            radius: 6
                                            color: popup.activeManualSpeed === presetBtn.modelData ? popup.accentColor : popup.bgColor
                                            border.width: 1
                                            border.color: popup.borderColor
                                        }

                                        contentItem: Text {
                                            text: presetBtn.text
                                            color: popup.activeManualSpeed === presetBtn.modelData ? popup.accentButtonText : popup.textColor
                                            font.pixelSize: Math.round(11 * popup.uiScale)
                                            font.weight: Font.DemiBold
                                            horizontalAlignment: Text.AlignHCenter
                                            verticalAlignment: Text.AlignVCenter
                                        }

                                        onClicked: {
                                            popup.activeManualSpeed = modelData;
                                            manualPopupSlider.value = modelData;
                                        }
                                    }
                                }
                            }
                        }

                        // Custom Curve View
                        ColumnLayout {
                            visible: popup.activeMode === "custom"
                            Layout.fillWidth: true
                            spacing: 10

                            RowLayout {
                                Layout.fillWidth: true

                                Label {
                                    text: qsTr("Interactive Custom Fan Curve & Presets")
                                    color: popup.textColor
                                    font.pixelSize: Math.round(13 * popup.uiScale)
                                    font.weight: Font.DemiBold
                                    Layout.fillWidth: true
                                }

                                Button {
                                    id: resetBaselineButton
                                    text: qsTr("Reset curve")
                                    implicitWidth: resetBaselineContent.implicitWidth + Math.round(20 * popup.uiScale)
                                    implicitHeight: Math.round(30 * popup.uiScale)
                                    hoverEnabled: true

                                    background: Rectangle {
                                        radius: Math.round(8 * popup.uiScale)
                                        color: resetBaselineButton.down
                                               ? (popup.darkMode ? "#40375B" : "#E0E7FF")
                                               : (resetBaselineButton.hovered
                                                  ? (popup.darkMode ? "#372F50" : "#EEF2FF")
                                                  : "transparent")
                                        border.width: 1
                                        border.color: resetBaselineButton.hovered ? popup.accentColor : popup.borderColor
                                    }

                                    contentItem: RowLayout {
                                        id: resetBaselineContent
                                        spacing: Math.round(6 * popup.uiScale)

                                        Label {
                                            text: "↺"
                                            color: resetBaselineButton.hovered ? popup.accentColor : popup.softTextColor
                                            font.pixelSize: Math.round(15 * popup.uiScale)
                                            font.weight: Font.Bold
                                        }

                                        Label {
                                            text: resetBaselineButton.text
                                            color: resetBaselineButton.hovered ? popup.accentColor : popup.textColor
                                            font.pixelSize: Math.round(11 * popup.uiScale)
                                            font.weight: Font.DemiBold
                                        }
                                    }

                                    ToolTip.visible: hovered
                                    ToolTip.delay: 400
                                    ToolTip.text: qsTr("Restore the default balanced fan curve")
                                    onClicked: {
                                        popup.activeCurvePoints = [
                                            { temp: 40, speed: 30 },
                                            { temp: 55, speed: 50 },
                                            { temp: 70, speed: 70 },
                                            { temp: 85, speed: 100 }
                                        ];
                                    }
                                }
                            }

                            // Interactive Presets Buttons
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: Math.round(6 * popup.uiScale)

                                Label {
                                    text: qsTr("Presets:")
                                    color: popup.softTextColor
                                    font.pixelSize: Math.round(11 * popup.uiScale)
                                    font.weight: Font.DemiBold
                                }

                                Repeater {
                                    model: [
                                        {
                                            id: "stealth",
                                            name: qsTr("Zero-dB Stealth"),
                                            points: [
                                                { temp: 45, speed: 0 },
                                                { temp: 55, speed: 30 },
                                                { temp: 68, speed: 55 },
                                                { temp: 80, speed: 85 }
                                            ]
                                        },
                                        {
                                            id: "balanced",
                                            name: qsTr("Balanced"),
                                            points: [
                                                { temp: 40, speed: 30 },
                                                { temp: 55, speed: 45 },
                                                { temp: 68, speed: 65 },
                                                { temp: 85, speed: 100 }
                                            ]
                                        },
                                        {
                                            id: "aggressive",
                                            name: qsTr("Aggressive"),
                                            points: [
                                                { temp: 35, speed: 50 },
                                                { temp: 50, speed: 70 },
                                                { temp: 65, speed: 85 },
                                                { temp: 82, speed: 100 }
                                            ]
                                        },
                                        {
                                            id: "stepped",
                                            name: qsTr("Stepped"),
                                            points: [
                                                { temp: 40, speed: 30 },
                                                { temp: 55, speed: 30 },
                                                { temp: 70, speed: 65 },
                                                { temp: 85, speed: 100 }
                                            ]
                                        }
                                    ]

                                    delegate: Button {
                                        id: presetPopupBtn
                                        required property var modelData
                                        text: presetPopupBtn.modelData.name
                                        implicitHeight: Math.round(26 * popup.uiScale)
                                        leftPadding: Math.round(10 * popup.uiScale)
                                        rightPadding: Math.round(10 * popup.uiScale)
                                        hoverEnabled: true

                                        background: Rectangle {
                                            radius: 6
                                            color: presetPopupBtn.hovered ? (popup.darkMode ? "#3B3156" : "#E2E8F0") : popup.bgColor
                                            border.width: 1
                                            border.color: presetPopupBtn.hovered ? popup.accentColor : popup.borderColor
                                        }

                                        contentItem: Text {
                                            text: presetPopupBtn.text
                                            color: presetPopupBtn.hovered ? popup.accentColor : popup.textColor
                                            font.pixelSize: Math.round(11 * popup.uiScale)
                                            font.weight: Font.DemiBold
                                            horizontalAlignment: Text.AlignHCenter
                                            verticalAlignment: Text.AlignVCenter
                                        }

                                        onClicked: {
                                            popup.activeCurvePoints = presetPopupBtn.modelData.points.slice();
                                        }
                                    }
                                }
                            }

                            // Dynamic Live Canvas Graph with Live Temperature Crosshair
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: Math.round(120 * popup.uiScale)
                                radius: 10
                                color: popup.bgColor
                                border.width: 1
                                border.color: popup.borderColor
                                clip: true

                                Canvas {
                                    id: curveCanvas
                                    anchors.fill: parent
                                    anchors.margins: 8

                                    property var pts: popup.activeCurvePoints
                                    property int liveTemp: (popup.currentFan && popup.currentFan.temperatureC > 0)
                                                           ? popup.currentFan.temperatureC : 0

                                    renderTarget: Canvas.Image
                                    renderStrategy: Canvas.Immediate
                                    visible: popup.opened && popup.activeMode === "custom"

                                    onPtsChanged: if (visible) requestPaint()
                                    onLiveTempChanged: if (visible) requestPaint()

                                    onPaint: {
                                        var ctx = getContext("2d");
                                        ctx.reset();
                                        var w = width;
                                        var h = height;
                                        var padL = 28;
                                        var padR = 14;
                                        var padT = 10;
                                        var padB = 18;
                                        var graphW = w - padL - padR;
                                        var graphH = h - padT - padB;

                                        // Grid Background Lines
                                        ctx.strokeStyle = popup.darkMode ? "rgba(255,255,255,0.07)" : "rgba(0,0,0,0.06)";
                                        ctx.lineWidth = 1;
                                        for (var g = 0; g <= 4; ++g) {
                                            var gy = padT + (graphH * g / 4);
                                            ctx.beginPath();
                                            ctx.moveTo(padL, gy);
                                            ctx.lineTo(padL + graphW, gy);
                                            ctx.stroke();
                                        }

                                        // Axis labels
                                        ctx.fillStyle = popup.softTextColor;
                                        ctx.font = "10px sans-serif";
                                        ctx.fillText("100%", 2, padT + 8);
                                        ctx.fillText("50%", 6, padT + (graphH / 2) + 3);
                                        ctx.fillText("0%", 10, padT + graphH);
                                        ctx.fillText("20°C", padL, h - 2);
                                        ctx.fillText("60°C", padL + (graphW / 2) - 12, h - 2);
                                        ctx.fillText("100°C", padL + graphW - 26, h - 2);

                                        if (!pts || pts.length === 0)
                                            return;

                                        function mapX(temp) {
                                            var clamped = Math.max(20, Math.min(100, temp));
                                            return padL + (graphW * (clamped - 20) / 80);
                                        }
                                        function mapY(speed) {
                                            var clamped = Math.max(0, Math.min(100, speed));
                                            return padT + graphH - (graphH * clamped / 100);
                                        }

                                        // Area Gradient Under Curve
                                        ctx.beginPath();
                                        ctx.moveTo(mapX(20), mapY(pts[0].speed));
                                        for (var i = 0; i < pts.length; ++i) {
                                            ctx.lineTo(mapX(pts[i].temp), mapY(pts[i].speed));
                                        }
                                        ctx.lineTo(mapX(100), mapY(pts[pts.length - 1].speed));
                                        ctx.lineTo(padL + graphW, padT + graphH);
                                        ctx.lineTo(padL, padT + graphH);
                                        ctx.closePath();

                                        var grad = ctx.createLinearGradient(0, padT, 0, padT + graphH);
                                        grad.addColorStop(0, popup.darkMode ? "rgba(129, 140, 248, 0.35)" : "rgba(79, 70, 229, 0.25)");
                                        grad.addColorStop(1, popup.darkMode ? "rgba(129, 140, 248, 0.02)" : "rgba(79, 70, 229, 0.02)");
                                        ctx.fillStyle = grad;
                                        ctx.fill();

                                        // Curve Stroke
                                        ctx.beginPath();
                                        ctx.strokeStyle = popup.accentColor;
                                        ctx.lineWidth = 2.5;
                                        ctx.moveTo(mapX(20), mapY(pts[0].speed));
                                        for (var k = 0; k < pts.length; ++k) {
                                            ctx.lineTo(mapX(pts[k].temp), mapY(pts[k].speed));
                                        }
                                        ctx.lineTo(mapX(100), mapY(pts[pts.length - 1].speed));
                                        ctx.stroke();

                                        // Control Point Dots
                                        for (var j = 0; j < pts.length; ++j) {
                                            var px = mapX(pts[j].temp);
                                            var py = mapY(pts[j].speed);

                                            ctx.beginPath();
                                            ctx.fillStyle = "#FFFFFF";
                                            ctx.arc(px, py, 5, 0, Math.PI * 2);
                                            ctx.fill();

                                            ctx.beginPath();
                                            ctx.strokeStyle = popup.accentColor;
                                            ctx.lineWidth = 2;
                                            ctx.arc(px, py, 5, 0, Math.PI * 2);
                                            ctx.stroke();
                                        }

                                        // Live Temperature Marker
                                        if (liveTemp >= 20 && liveTemp <= 100) {
                                            var liveX = mapX(liveTemp);
                                            ctx.beginPath();
                                            ctx.strokeStyle = popup.warningText;
                                            ctx.lineWidth = 1.8;
                                            ctx.setLineDash([4, 3]);
                                            ctx.moveTo(liveX, padT);
                                            ctx.lineTo(liveX, padT + graphH);
                                            ctx.stroke();
                                            ctx.setLineDash([]);

                                            // Live marker dot
                                            ctx.beginPath();
                                            ctx.fillStyle = popup.warningText;
                                            ctx.arc(liveX, padT + 4, 3.5, 0, Math.PI * 2);
                                            ctx.fill();
                                        }
                                    }
                                }
                            }

                            // Point Controls Grid
                            GridLayout {
                                Layout.fillWidth: true
                                columns: width > 500 ? 4 : 2
                                columnSpacing: 6
                                rowSpacing: 6

                                Repeater {
                                    model: 4

                                    delegate: Rectangle {
                                        id: curvePtBox
                                        required property int index
                                        readonly property var ptData: (popup.activeCurvePoints && popup.activeCurvePoints.length > curvePtBox.index)
                                                                      ? popup.activeCurvePoints[curvePtBox.index]
                                                                      : ({ temp: 40 + curvePtBox.index * 15, speed: 30 + curvePtBox.index * 20 })
                                        Layout.fillWidth: true
                                        implicitHeight: Math.round(62 * popup.uiScale)
                                        radius: 8
                                        color: popup.bgColor
                                        border.width: 1
                                        border.color: popup.borderColor

                                        ColumnLayout {
                                            anchors.fill: parent
                                            anchors.margins: 8
                                            spacing: 2

                                            RowLayout {
                                                Layout.fillWidth: true
                                                Label {
                                                    text: qsTr("Point %1 (%2°C)").arg(curvePtBox.index + 1).arg(curvePtBox.ptData.temp)
                                                    color: popup.textColor
                                                    font.pixelSize: Math.round(11 * popup.uiScale)
                                                    font.weight: Font.Bold
                                                }
                                                Item { Layout.fillWidth: true }
                                                Label {
                                                    text: (Math.round(ptSlider.value)) + "%"
                                                    color: popup.accentColor
                                                    font.pixelSize: Math.round(11 * popup.uiScale)
                                                    font.weight: Font.Bold
                                                }
                                            }

                                            Slider {
                                                id: ptSlider
                                                Layout.fillWidth: true
                                                from: 0
                                                to: 100
                                                stepSize: 5
                                                value: curvePtBox.ptData.speed
                                                onMoved: {
                                                    if (popup.activeCurvePoints && popup.activeCurvePoints.length > curvePtBox.index) {
                                                        popup.activeCurvePoints[curvePtBox.index].speed = Math.round(value);
                                                        curveCanvas.requestPaint();
                                                    }
                                                }

                                                background: Rectangle {
                                                    x: ptSlider.leftPadding
                                                    y: ptSlider.topPadding + ptSlider.availableHeight / 2 - height / 2
                                                    implicitHeight: 4
                                                    width: ptSlider.availableWidth
                                                    height: implicitHeight
                                                    radius: 2
                                                    color: popup.darkMode ? "#1E2238" : "#E2E8F0"

                                                    Rectangle {
                                                        width: ptSlider.visualPosition * parent.width
                                                        height: parent.height
                                                        color: popup.accentColor
                                                        radius: 2
                                                    }
                                                }

                                                handle: Rectangle {
                                                    x: ptSlider.leftPadding + ptSlider.visualPosition * (ptSlider.availableWidth - width)
                                                    y: ptSlider.topPadding + ptSlider.availableHeight / 2 - height / 2
                                                    implicitWidth: 14
                                                    implicitHeight: 14
                                                    radius: 7
                                                    color: "#FFFFFF"
                                                    border.color: popup.accentColor
                                                    border.width: 2
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        // Presets View Description for built-in modes
                        ColumnLayout {
                            visible: popup.activeMode !== "manual" && popup.activeMode !== "custom"
                            Layout.fillWidth: true
                            spacing: 4

                            Label {
                                text: qsTr("Profile Cooling Dynamics")
                                color: popup.textColor
                                font.pixelSize: Math.round(12 * popup.uiScale)
                                font.weight: Font.DemiBold
                            }

                            Label {
                                text: {
                                    if (popup.activeMode === "silent")
                                        return qsTr("Acoustic priority: Fans remain in Zero-dB silent state under 45°C, ramping gently to 50% at 68°C and 100% at 85°C.");
                                    if (popup.activeMode === "performance")
                                        return qsTr("Aggressive cooling: 45% minimum speed floor, ramping rapidly to 80% at 65°C and 100% at 82°C for heavy compute/gaming.");
                                    if (popup.activeMode === "balanced")
                                        return qsTr("Optimized baseline: 30% speed floor, dynamically balancing acoustic comfort and thermal dissipation.");
                                    return qsTr("Native automatic curve dynamically controlled by hardware thermals and firmware.");
                                }
                                color: popup.softTextColor
                                font.pixelSize: Math.round(11 * popup.uiScale)
                                wrapMode: Text.Wrap
                                Layout.fillWidth: true
                            }
                        }

                        // Thermal Emergency Safety Guard
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            RowLayout {
                                Layout.fillWidth: true

                                Label {
                                    text: qsTr("Emergency 100% Thermal Guard Threshold")
                                    color: popup.textColor
                                    font.pixelSize: Math.round(12 * popup.uiScale)
                                    font.weight: Font.DemiBold
                                }

                                Item { Layout.fillWidth: true }

                                Label {
                                    text: popup.activeThermalThreshold + " °C"
                                    color: popup.warningText
                                    font.pixelSize: Math.round(12 * popup.uiScale)
                                    font.weight: Font.Bold
                                }
                            }

                            Slider {
                                id: guardSlider
                                Layout.fillWidth: true
                                from: 65
                                to: 100
                                stepSize: 1
                                value: popup.activeThermalThreshold
                                onMoved: popup.activeThermalThreshold = Math.round(value)

                                background: Rectangle {
                                    x: guardSlider.leftPadding
                                    y: guardSlider.topPadding + guardSlider.availableHeight / 2 - height / 2
                                    implicitHeight: 6
                                    width: guardSlider.availableWidth
                                    height: implicitHeight
                                    radius: 3
                                    color: popup.darkMode ? "#1E2238" : "#E2E8F0"

                                    Rectangle {
                                        width: guardSlider.visualPosition * parent.width
                                        height: parent.height
                                        color: popup.warningText
                                        radius: 3
                                    }
                                }

                                handle: Rectangle {
                                    x: guardSlider.leftPadding + guardSlider.visualPosition * (guardSlider.availableWidth - width)
                                    y: guardSlider.topPadding + guardSlider.availableHeight / 2 - height / 2
                                    implicitWidth: 16
                                    implicitHeight: 16
                                    radius: 8
                                    color: "#FFFFFF"
                                    border.color: popup.warningText
                                    border.width: 2
                                }
                            }
                        }
                    }
                }

                // Success / Feedback Banner
                Rectangle {
                    visible: popup.saveFeedbackVisible
                    Layout.fillWidth: true
                    radius: 8
                    color: popup.successBg
                    border.width: 1
                    border.color: popup.successText
                    implicitHeight: 34

                    RowLayout {
                        anchors.centerIn: parent
                        spacing: 8
                        Label {
                            text: "✓"
                            color: popup.successText
                            font.pixelSize: Math.round(14 * popup.uiScale)
                            font.weight: Font.Bold
                        }
                        Label {
                            text: qsTr("Fan configuration applied and saved successfully!")
                            color: popup.successText
                            font.pixelSize: Math.round(12 * popup.uiScale)
                            font.weight: Font.DemiBold
                        }
                    }
                }

            }
        }

        // Pinned Action Buttons Footer (Always visible at bottom)
        GridLayout {
            Layout.fillWidth: true
            columnSpacing: 8
            rowSpacing: 8
            columns: popup.compactLayout ? 1 : 4

            Button {
                id: testBtn
                visible: popup.currentFan && popup.currentFan.id === "gpu_0"
                text: popup.testingFanActive ? qsTr("Testing (100%)...") : qsTr("Quick Test 100%")
                implicitHeight: Math.round(36 * popup.uiScale)
                Layout.fillWidth: popup.compactLayout
                hoverEnabled: true
                enabled: popup.fanController && popup.fanController.controlSupported

                background: Rectangle {
                    radius: 8
                    color: popup.testingFanActive ? (popup.darkMode ? "#581C87" : "#F3E8FF")
                           : (testBtn.hovered ? (popup.darkMode ? "#3B3156" : "#E2E8F0") : popup.cardColor)
                    border.width: 1
                    border.color: popup.testingFanActive ? popup.warningText : popup.borderColor
                }

                contentItem: Text {
                    text: testBtn.text
                    color: popup.testingFanActive ? popup.warningText : popup.textColor
                    font.pixelSize: Math.round(11 * popup.uiScale)
                    font.weight: Font.DemiBold
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: popup.triggerQuickTest100()
            }

            Label {
                visible: testBtn.visible && !testBtn.enabled
                Layout.fillWidth: popup.compactLayout
                text: qsTr("Direct fan control unavailable")
                color: popup.softTextColor
                font.pixelSize: Math.round(10 * popup.uiScale)
            }

            Label {
                visible: popup.fanTestFailed
                Layout.fillWidth: true
                Layout.columnSpan: popup.compactLayout ? 1 : 4
                text: qsTr("GPU fan test could not start. Enable NVIDIA Coolbits / fan control first.")
                color: popup.warningText
                font.pixelSize: Math.round(11 * popup.uiScale)
                wrapMode: Text.WordWrap
            }

            Label {
                visible: popup.operationError.length > 0
                Layout.fillWidth: true
                Layout.columnSpan: popup.compactLayout ? 1 : 4
                text: popup.operationError
                color: popup.warningText
                font.pixelSize: Math.round(11 * popup.uiScale)
                wrapMode: Text.WordWrap
            }

            Button {
                id: resetToAutoBtn
                text: qsTr("Reset to Auto")
                implicitHeight: Math.round(36 * popup.uiScale)
                Layout.fillWidth: popup.compactLayout
                hoverEnabled: true
                enabled: popup.hasHardwareControl

                background: Rectangle {
                    radius: 8
                    color: resetToAutoBtn.down ? (popup.darkMode ? "#3B3156" : "#E2E8F0")
                                               : (resetToAutoBtn.hovered ? (popup.darkMode ? "#342D4A" : "#F1F5F9")
                                                                        : popup.cardColor)
                    border.width: 1
                    border.color: resetToAutoBtn.hovered ? popup.accentColor : popup.borderColor
                }

                contentItem: Text {
                    text: resetToAutoBtn.text
                    color: resetToAutoBtn.hovered ? popup.accentColor : popup.textColor
                    font.pixelSize: Math.round(11 * popup.uiScale)
                    font.weight: Font.DemiBold
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: popup.resetToAutoMode()
            }

            Button {
                id: applyBtn
                text: qsTr("Apply & Save Settings")
                implicitHeight: Math.round(36 * popup.uiScale)
                Layout.fillWidth: popup.compactLayout
                hoverEnabled: true
                enabled: popup.hasHardwareControl

                background: Rectangle {
                    radius: 8
                    color: applyBtn.down ? (popup.darkMode ? "#4F46E5" : "#3730A3")
                                         : (applyBtn.hovered ? (popup.darkMode ? "#939BFA" : "#5B52E8")
                                                             : popup.accentColor)
                }

                contentItem: Text {
                    text: applyBtn.text
                    color: popup.accentButtonText
                    font.pixelSize: Math.round(12 * popup.uiScale)
                    font.weight: Font.Bold
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: popup.applyAllSettings()
            }
        }
    }
}
