import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components" as Components

Item {
    id: page
    required property var systemInfo
    property var cpuMonitor: null
    property var gpuMonitor: null
    property var ramMonitor: null
    property var nvidiaDetector: null
    property var powerController: null

    property var theme: ({})
    property bool darkMode: false
    property bool showAdvancedInfo: true
    property real uiScale: 1.0
    property bool reportCopied: false
    property string generatedReport: ""
    property int reportViewMode: 0
    property string reportFilterText: ""
    property string lastCopiedKey: ""
    property bool refreshBusy: false
    property bool reportRefreshPending: false
    property string actionFeedback: ""
    property bool actionFailed: false

    readonly property color bgColor: theme && theme.card ? theme.card : (page.darkMode ? "#29233B" : "#FFFFFF")
    readonly property color cardColor: theme && theme.cardStrong ? theme.cardStrong : (page.darkMode ? "#342D4A" : "#F1F5F9")
    readonly property color borderColor: theme && theme.border ? theme.border : (page.darkMode ? "#4D436B" : "#CBD5E1")
    readonly property color textColor: theme && theme.text ? theme.text : (page.darkMode ? "#F8FAFC" : "#0F172A")
    readonly property color softTextColor: theme && theme.textSoft ? theme.textSoft : (page.darkMode ? "#94A3B8" : "#64748B")
    readonly property color accentColor: theme && theme.accentA ? theme.accentA : (page.darkMode ? "#818CF8" : "#4F46E5")
    readonly property color infoBg: theme && theme.infoBg ? theme.infoBg : (page.darkMode ? "#1E2548" : "#EFF6FF")
    readonly property color successColor: theme && theme.success ? theme.success : (page.darkMode ? "#4ADE80" : "#059669")
    readonly property color warningColor: theme && theme.warning ? theme.warning : (page.darkMode ? "#FBBF24" : "#D97706")

    function deviceAndPowerSummary() {
        const dev = page.systemInfo && page.systemInfo.deviceType ? page.systemInfo.deviceType : "";
        const pwr = page.systemInfo && page.systemInfo.powerSource ? page.systemInfo.powerSource : "";
        return dev.length > 0 && pwr.length > 0 ? (dev + " • " + pwr) : (dev || pwr);
    }

    function nvidiaDriverSummary() {
        const ver = page.nvidiaDetector && page.nvidiaDetector.driverVersion ? page.nvidiaDetector.driverVersion : "";
        if (ver.length === 0)
            return "";
        const src = page.nvidiaDetector ? page.nvidiaDetector.installedDriverSourceLabel : "";
        return (src.length > 0 && src !== "None") ? (ver + " (" + src + ")") : ver;
    }

    function systemHealthSummary() {
        const items = [];
        if (page.nvidiaDetector && page.nvidiaDetector.driverVersion)
            items.push(qsTr("Driver: %1").arg(page.nvidiaDetector.driverVersion));
        if (page.gpuMonitor && page.gpuMonitor.available)
            items.push(qsTr("GPU telemetry: Available"));
        if (page.nvidiaDetector && page.nvidiaDetector.secureBootKnown)
            items.push(page.nvidiaDetector.secureBootEnabled ? qsTr("Secure Boot: On") : qsTr("Secure Boot: Off"));
        return items.length > 0 ? items.join(" • ") : qsTr("Live system information");
    }

    function platformSecuritySummary() {
        const virt = (page.systemInfo && page.systemInfo.virtualMachine) ? page.systemInfo.virtualizationType : qsTr("Bare Metal");
        const sb = (page.nvidiaDetector && page.nvidiaDetector.secureBootKnown)
                 ? (page.nvidiaDetector.secureBootEnabled ? qsTr("Secure Boot: On") : qsTr("Secure Boot: Off"))
                 : "";
        return sb.length > 0 ? (virt + " • " + sb) : virt;
    }

    function localizeGpuName(name) {
        if (!name || name.toString().trim().length === 0)
            return "";
        return page.systemInfo ? page.systemInfo.localizeGpuName(name) : name;
    }

    function diagnosticGpuName() {
        if (!page.nvidiaDetector)
            return "";
        var name = page.nvidiaDetector.gpuFound ? page.nvidiaDetector.gpuName : page.nvidiaDetector.displayAdapterName;
        return page.localizeGpuName(name);
    }

    // Cached card computation to eliminate repeated object allocations and delegate thrashing
    readonly property var hardwareItems: {
        const cards = [];
        function add(title, value) {
            if (value && value.toString().trim().length > 0)
                cards.push({ title: title, value: value });
        }
        add(qsTr("Graphics Card (GPU)"), page.diagnosticGpuName());
        add(qsTr("Processor (CPU)"), page.systemInfo ? page.systemInfo.cpuModel : "");
        add(qsTr("Motherboard"), page.systemInfo ? page.systemInfo.motherboardModel : "");
        add(qsTr("UEFI / BIOS"), page.systemInfo ? page.systemInfo.biosVersion : "");
        if (page.ramMonitor && page.ramMonitor.totalMiB > 0)
            add(qsTr("System Memory (RAM)"), (page.ramMonitor.totalMiB / 1024.0).toFixed(1) + " GB (" + page.ramMonitor.totalMiB + " MiB)");
        if (page.gpuMonitor && page.gpuMonitor.memoryTotalMiB > 0)
            add(qsTr("Video Memory (VRAM)"), (page.gpuMonitor.memoryTotalMiB / 1024.0).toFixed(1) + " GB (" + page.gpuMonitor.memoryTotalMiB + " MiB)");
        if (page.systemInfo && page.systemInfo.integratedGpuName && page.systemInfo.integratedGpuMemory)
            add(qsTr("Integrated Graphics Memory"), page.localizeGpuName(page.systemInfo.integratedGpuName) + " • " + page.systemInfo.integratedGpuMemory);
        add(qsTr("PCIe Link Interface"), page.gpuMonitor ? page.gpuMonitor.pcieLinkStatus : "");
        add(qsTr("Device & Power"), page.deviceAndPowerSummary());
        return cards;
    }

    readonly property var softwareItems: {
        const cards = [];
        function add(title, value) {
            if (value && value.toString().trim().length > 0)
                cards.push({ title: title, value: value });
        }
        add(qsTr("Operating System"), page.systemInfo ? page.systemInfo.osName : "");
        add(qsTr("Desktop Environment"), page.systemInfo ? page.systemInfo.desktopEnvironment : "");
        add(qsTr("Linux Kernel"), page.systemInfo ? page.systemInfo.kernelVersion : "");
        if (page.nvidiaDetector && page.nvidiaDetector.sessionType)
            add(qsTr("Display Server / Session"), page.nvidiaDetector.sessionType.charAt(0).toUpperCase() + page.nvidiaDetector.sessionType.slice(1));
        if (page.nvidiaDetector && page.nvidiaDetector.gpuFound && page.nvidiaDetector.driverVersion)
            add(qsTr("NVIDIA Driver"), page.nvidiaDriverSummary());
        add(qsTr("Graphics & Compute APIs"), page.systemInfo ? page.systemInfo.graphicsApiSummary : "");
        add(qsTr("Platform & Security"), page.platformSecuritySummary());
        return cards;
    }

    function hardwareCards() {
        return page.hardwareItems;
    }

    function softwareCards() {
        return page.softwareItems;
    }

    function diagnosticReportSections() {
        return [
            {
                title: qsTr("Operating System & Platform"),
                icon: "💻",
                items: [
                    { label: qsTr("Operating System"), value: page.systemInfo ? page.systemInfo.osName : "", icon: "🐧" },
                    { label: qsTr("Linux Kernel"), value: page.systemInfo ? page.systemInfo.kernelVersion : "", icon: "⚙️" },
                    { label: qsTr("Desktop Environment"), value: page.systemInfo ? page.systemInfo.desktopEnvironment : "", icon: "🖥️" },
                    { label: qsTr("Display Server / Session"), value: (page.nvidiaDetector && page.nvidiaDetector.sessionType) ? page.nvidiaDetector.sessionType.toUpperCase() : "", icon: "🪟" },
                    { label: qsTr("Platform Security"), value: page.platformSecuritySummary(), icon: "🛡️" }
                ]
            },
            {
                title: qsTr("Processor & Hardware"),
                icon: "⚡",
                items: [
                    { label: qsTr("Processor (CPU)"), value: page.systemInfo ? page.systemInfo.cpuModel : "", icon: "⚡" },
                    { label: qsTr("Motherboard"), value: page.systemInfo ? page.systemInfo.motherboardModel : "", icon: "🔌" },
                    { label: qsTr("UEFI / BIOS"), value: page.systemInfo ? page.systemInfo.biosVersion : "", icon: "💾" },
                    { label: qsTr("System Memory (RAM)"), value: (page.ramMonitor && page.ramMonitor.totalMiB > 0) ? ((page.ramMonitor.totalMiB / 1024.0).toFixed(1) + " GB (" + page.ramMonitor.totalMiB + " MiB)") : "", icon: "🧠" },
                    { label: qsTr("Device & Power"), value: page.deviceAndPowerSummary(), icon: "🔋" }
                ]
            },
            {
                title: qsTr("Graphics & Accelerators"),
                icon: "🎮",
                items: [
                    { label: qsTr("Dedicated GPU"), value: page.diagnosticGpuName(), icon: "🎮" },
                    { label: qsTr("NVIDIA Driver"), value: page.nvidiaDriverSummary(), icon: "⚙️" },
                    { label: qsTr("Video Memory (VRAM)"), value: (page.gpuMonitor && page.gpuMonitor.memoryTotalMiB > 0) ? ((page.gpuMonitor.memoryTotalMiB / 1024.0).toFixed(1) + " GB (" + page.gpuMonitor.memoryTotalMiB + " MiB)") : "", icon: "📼" },
                    { label: qsTr("PCIe Link Interface"), value: page.gpuMonitor ? page.gpuMonitor.pcieLinkStatus : "", icon: "🔗" },
                    { label: qsTr("Integrated GPU"), value: (page.systemInfo && page.systemInfo.integratedGpuName && page.systemInfo.integratedGpuMemory) ? (page.localizeGpuName(page.systemInfo.integratedGpuName) + " • " + page.systemInfo.integratedGpuMemory) : "", icon: "🎨" },
                    { label: qsTr("Graphics & Compute APIs"), value: page.systemInfo ? page.systemInfo.graphicsApiSummary : "", icon: "🚀" }
                ]
            }
        ];
    }

    function openDiagnosticReport() {
        if (!page.systemInfo)
            return;

        page.systemInfo.rescanHardware();
        if (page.cpuMonitor) page.cpuMonitor.refresh();
        if (page.ramMonitor) page.ramMonitor.refresh();
        if (page.gpuMonitor) {
            page.reportRefreshPending = true;
            page.gpuMonitor.requestRefresh();
            return;
        }
        page.finalizeDiagnosticReport();
    }

    function finalizeDiagnosticReport() {
        page.reportRefreshPending = false;

        const gpu = page.diagnosticGpuName();
        const drv = page.nvidiaDriverSummary();
        const vram = (page.gpuMonitor && page.gpuMonitor.memoryTotalMiB > 0)
                   ? ((page.gpuMonitor.memoryTotalMiB / 1024.0).toFixed(1) + " GB (" + page.gpuMonitor.memoryTotalMiB + " MiB)")
                   : "";
        const ram = (page.ramMonitor && page.ramMonitor.totalMiB > 0)
                  ? ((page.ramMonitor.totalMiB / 1024.0).toFixed(1) + " GB (" + page.ramMonitor.totalMiB + " MiB)")
                  : "";
        const pcie = page.gpuMonitor ? page.gpuMonitor.pcieLinkStatus : "";
        const sec = page.platformSecuritySummary();

        page.generatedReport = page.systemInfo.generateSystemReport(gpu, drv, vram, ram, pcie, sec, page.systemInfo.diagnosticReportFormat);
        if (page.systemInfo.diagnosticReportDestination === "clipboard") {
            page.reportCopied = page.systemInfo.copyToClipboard(page.generatedReport);
            if (page.reportCopied) {
                page.actionFailed = false;
                page.actionFeedback = qsTr("Diagnostic report copied to clipboard.");
                copiedFeedbackTimer.restart();
                return;
            }
            page.actionFailed = true;
            page.actionFeedback = qsTr("The report could not be copied. You can copy it manually from this preview.");
        }

        if (!diagnosticDialogLoader.item) {
            diagnosticDialogLoader.active = true;
        }
        if (diagnosticDialogLoader.item) {
            diagnosticDialogLoader.item.theme = page.theme;
            diagnosticDialogLoader.item.darkMode = page.darkMode;
            diagnosticDialogLoader.item.uiScale = page.uiScale;
            diagnosticDialogLoader.item.openWithData(page.generatedReport, page.diagnosticReportSections());
        }
    }

    function refreshSystemData() {
        if (page.refreshBusy || !page.systemInfo)
            return;
        page.refreshBusy = true;
        page.systemInfo.rescanHardware();
        if (page.cpuMonitor) page.cpuMonitor.refresh();
        if (page.gpuMonitor) {
            page.gpuMonitor.requestRefresh();
        } else {
            page.refreshBusy = false;
        }
        if (page.ramMonitor) page.ramMonitor.refresh();
        page.actionFailed = false;
        page.actionFeedback = qsTr("System information refreshed.");
        refreshFeedbackTimer.restart();
    }

    Connections {
        target: page.gpuMonitor
        function onTelemetryRefreshFinished() {
            if (page.reportRefreshPending)
                page.finalizeDiagnosticReport();
            page.refreshBusy = false;
        }
    }

    Timer {
        id: copiedFeedbackTimer
        interval: 3000
        repeat: false
        onTriggered: page.reportCopied = false
    }

    Timer {
        id: refreshFeedbackTimer
        interval: 3000
        repeat: false
        onTriggered: page.actionFeedback = ""
    }

    ScrollView {
        id: pageScroll
        anchors.fill: parent
        clip: true
        contentWidth: availableWidth

        ColumnLayout {
            width: pageScroll.availableWidth
            spacing: Math.round(14 * page.uiScale)

            // Health Status Bar
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 52
                radius: 12
                color: page.cardColor
                border.width: 1
                border.color: page.borderColor

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 12

                    Label {
                        text: qsTr("System health")
                        color: page.textColor
                        font.weight: Font.DemiBold
                    }

                    Label {
                        Layout.fillWidth: true
                        text: page.systemHealthSummary()
                        color: page.softTextColor
                        elide: Text.ElideRight
                    }

                    Components.ActionButton {
                        text: page.refreshBusy ? qsTr("Refreshing…") : qsTr("Refresh")
                        enabled: !page.refreshBusy
                        theme: page.theme
                        compact: true
                        uiScale: page.uiScale
                        onClicked: page.refreshSystemData()
                    }
                }
            }

            // Feedback Banner
            Rectangle {
                Layout.fillWidth: true
                visible: page.actionFeedback.length > 0
                implicitHeight: actionFeedbackLabel.implicitHeight + Math.round(18 * page.uiScale)
                radius: 9
                color: page.actionFailed ? (page.darkMode ? "#3A2E12" : "#FFFBEB") : page.infoBg
                border.width: 1
                border.color: page.actionFailed ? page.warningColor : page.accentColor

                Label {
                    id: actionFeedbackLabel
                    anchors.fill: parent
                    anchors.margins: Math.round(9 * page.uiScale)
                    text: page.actionFeedback
                    color: page.actionFailed ? page.warningColor : page.textColor
                    font.pixelSize: Math.round(12 * page.uiScale)
                    wrapMode: Text.WordWrap
                }
            }

            // Section 1: Hardware Specifications
            Rectangle {
                Layout.fillWidth: true
                radius: 14
                color: page.cardColor
                border.width: 1
                border.color: page.borderColor
                implicitHeight: hwSectionLayout.implicitHeight + Math.round(24 * page.uiScale)

                ColumnLayout {
                    id: hwSectionLayout
                    anchors.fill: parent
                    anchors.margins: Math.round(14 * page.uiScale)
                    spacing: Math.round(12 * page.uiScale)

                    Label {
                        text: qsTr("Hardware Specifications")
                        color: page.textColor
                        font.pixelSize: Math.round(16 * page.uiScale)
                        font.weight: Font.DemiBold
                        Layout.fillWidth: true
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        columns: width > Math.round(1080 * page.uiScale) ? 4 : (width > Math.round(780 * page.uiScale) ? 3 : (width > Math.round(520 * page.uiScale) ? 2 : 1))
                        columnSpacing: Math.round(8 * page.uiScale)
                        rowSpacing: Math.round(8 * page.uiScale)

                        Repeater {
                            model: page.hardwareItems

                            delegate: Rectangle {
                                required property var modelData
                                Layout.fillWidth: true
                                implicitHeight: Math.round(64 * page.uiScale)
                                radius: 10
                                color: page.bgColor
                                border.width: 1
                                border.color: page.borderColor

                                Column {
                                    anchors.fill: parent
                                    anchors.margins: Math.round(8 * page.uiScale)
                                    spacing: Math.round(3 * page.uiScale)

                                    Label {
                                        width: parent.width
                                        text: modelData.title
                                        color: page.softTextColor
                                        font.pixelSize: Math.round(11 * page.uiScale)
                                        font.weight: Font.DemiBold
                                        elide: Text.ElideRight
                                    }

                                    Label {
                                        width: parent.width
                                        text: modelData.value
                                        color: page.textColor
                                        font.pixelSize: Math.round(13 * page.uiScale)
                                        font.weight: Font.DemiBold
                                        elide: Text.ElideRight
                                    }
                                }
                            }
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        visible: page.hardwareItems.length === 0
                        text: qsTr("No readable hardware details are currently exposed by this system.")
                        color: page.softTextColor
                        wrapMode: Text.WordWrap
                    }
                }
            }

            // Section 2: Operating System & Software Stack
            Rectangle {
                Layout.fillWidth: true
                radius: 14
                color: page.cardColor
                border.width: 1
                border.color: page.borderColor
                implicitHeight: osSectionLayout.implicitHeight + Math.round(24 * page.uiScale)

                ColumnLayout {
                    id: osSectionLayout
                    anchors.fill: parent
                    anchors.margins: Math.round(14 * page.uiScale)
                    spacing: Math.round(12 * page.uiScale)

                    Label {
                        text: qsTr("Operating System & Software Stack")
                        color: page.textColor
                        font.pixelSize: Math.round(16 * page.uiScale)
                        font.weight: Font.DemiBold
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        columns: width > Math.round(1080 * page.uiScale) ? 4 : (width > Math.round(780 * page.uiScale) ? 3 : (width > Math.round(520 * page.uiScale) ? 2 : 1))
                        columnSpacing: Math.round(8 * page.uiScale)
                        rowSpacing: Math.round(8 * page.uiScale)

                        Repeater {
                            model: page.softwareItems

                            delegate: Rectangle {
                                required property var modelData
                                Layout.fillWidth: true
                                implicitHeight: Math.round(64 * page.uiScale)
                                radius: 10
                                color: page.bgColor
                                border.width: 1
                                border.color: page.borderColor

                                Column {
                                    anchors.fill: parent
                                    anchors.margins: Math.round(8 * page.uiScale)
                                    spacing: Math.round(3 * page.uiScale)

                                    Label {
                                        width: parent.width
                                        text: modelData.title
                                        color: page.softTextColor
                                        font.pixelSize: Math.round(11 * page.uiScale)
                                        font.weight: Font.DemiBold
                                        elide: Text.ElideRight
                                    }

                                    Label {
                                        width: parent.width
                                        text: modelData.value
                                        color: page.textColor
                                        font.pixelSize: Math.round(13 * page.uiScale)
                                        font.weight: Font.DemiBold
                                        elide: Text.ElideRight
                                    }
                                }
                            }
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        visible: page.softwareItems.length === 0
                        text: qsTr("Software and platform details are temporarily unavailable.")
                        color: page.softTextColor
                        wrapMode: Text.WordWrap
                    }
                }
            }

            // Section 3: Diagnostic Actions & Firmware Control
            Rectangle {
                Layout.fillWidth: true
                radius: 14
                color: page.cardColor
                border.width: 1
                border.color: page.borderColor
                implicitHeight: actionsSectionLayout.implicitHeight + Math.round(24 * page.uiScale)

                ColumnLayout {
                    id: actionsSectionLayout
                    anchors.fill: parent
                    anchors.margins: Math.round(14 * page.uiScale)
                    spacing: Math.round(12 * page.uiScale)

                    Label {
                        text: qsTr("Diagnostics & System Controls")
                        color: page.textColor
                        font.pixelSize: Math.round(16 * page.uiScale)
                        font.weight: Font.DemiBold
                    }

                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Generate a shareable system report or restart directly into firmware setup.")
                        color: page.softTextColor
                        font.pixelSize: Math.round(12 * page.uiScale)
                        wrapMode: Text.WordWrap
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        columns: width > Math.round(680 * page.uiScale) ? 2 : 1
                        columnSpacing: Math.round(10 * page.uiScale)
                        rowSpacing: Math.round(10 * page.uiScale)

                        Button {
                            id: diagnosticActionBtn
                            Layout.fillWidth: true
                            implicitHeight: Math.round(88 * page.uiScale)
                            hoverEnabled: true

                            scale: !enabled ? 1.0 : (down ? 0.985 : (hovered ? 1.01 : 1.0))
                            Behavior on scale { NumberAnimation { duration: 120; easing.type: Easing.OutQuad } }

                            background: Rectangle {
                                radius: 10
                                color: diagnosticActionBtn.hovered
                                       ? (page.darkMode ? Qt.tint(page.bgColor, Qt.rgba(page.accentColor.r, page.accentColor.g, page.accentColor.b, 0.12))
                                                        : Qt.tint(page.bgColor, Qt.rgba(page.accentColor.r, page.accentColor.g, page.accentColor.b, 0.06)))
                                       : page.bgColor
                                border.width: diagnosticActionBtn.hovered ? 1.5 : 1
                                border.color: diagnosticActionBtn.hovered ? page.accentColor : page.borderColor
                            }

                            contentItem: RowLayout {
                                spacing: Math.round(12 * page.uiScale)

                                Rectangle {
                                    Layout.preferredWidth: Math.round(38 * page.uiScale)
                                    Layout.preferredHeight: Math.round(38 * page.uiScale)
                                    radius: 8
                                    color: page.darkMode ? "#312E81" : "#E0E7FF"

                                    Label {
                                        anchors.centerIn: parent
                                        text: "▤"
                                        color: page.accentColor
                                        font.pixelSize: Math.round(19 * page.uiScale)
                                        font.weight: Font.Bold
                                    }
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 3
                                    Label {
                                        text: qsTr("Diagnostic Report")
                                        color: page.textColor
                                        font.pixelSize: Math.round(13 * page.uiScale)
                                        font.weight: Font.DemiBold
                                    }
                                    Label {
                                        Layout.fillWidth: true
                                        text: qsTr("Preview, format, and share live system details")
                                        color: page.softTextColor
                                        font.pixelSize: Math.round(11 * page.uiScale)
                                        elide: Text.ElideRight
                                    }
                                }

                                Rectangle {
                                    implicitHeight: Math.round(32 * page.uiScale)
                                    implicitWidth: openBtnRow.implicitWidth + Math.round(20 * page.uiScale)
                                    radius: 6
                                    color: diagnosticActionBtn.down
                                           ? Qt.darker(page.accentColor, 1.1)
                                           : (diagnosticActionBtn.hovered ? page.accentColor : (page.darkMode ? "#312E81" : "#EEF2FF"))
                                    border.width: 1
                                    border.color: page.accentColor

                                    RowLayout {
                                        id: openBtnRow
                                        anchors.centerIn: parent
                                        spacing: Math.round(4 * page.uiScale)

                                        Label {
                                            text: qsTr("Open")
                                            color: diagnosticActionBtn.hovered ? "#FFFFFF" : page.accentColor
                                            font.pixelSize: Math.round(12 * page.uiScale)
                                            font.weight: Font.DemiBold
                                        }

                                        Label {
                                            text: "↗"
                                            color: diagnosticActionBtn.hovered ? "#FFFFFF" : page.accentColor
                                            font.pixelSize: Math.round(11 * page.uiScale)
                                            font.weight: Font.Bold
                                        }
                                    }
                                }
                            }

                            onClicked: page.openDiagnosticReport()
                        }

                        Button {
                            id: rebootFirmwareBtn
                            Layout.fillWidth: true
                            implicitHeight: Math.round(88 * page.uiScale)
                            hoverEnabled: true

                            scale: !enabled ? 1.0 : (down ? 0.985 : (hovered ? 1.01 : 1.0))
                            Behavior on scale { NumberAnimation { duration: 120; easing.type: Easing.OutQuad } }

                            background: Rectangle {
                                radius: 10
                                color: rebootFirmwareBtn.hovered
                                       ? (page.darkMode ? Qt.tint(page.bgColor, Qt.rgba(page.warningColor.r, page.warningColor.g, page.warningColor.b, 0.12))
                                                        : Qt.tint(page.bgColor, Qt.rgba(page.warningColor.r, page.warningColor.g, page.warningColor.b, 0.06)))
                                       : page.bgColor
                                border.width: rebootFirmwareBtn.hovered ? 1.5 : 1
                                border.color: rebootFirmwareBtn.hovered ? page.warningColor : page.borderColor
                            }

                            contentItem: RowLayout {
                                spacing: Math.round(12 * page.uiScale)

                                Rectangle {
                                    Layout.preferredWidth: Math.round(38 * page.uiScale)
                                    Layout.preferredHeight: Math.round(38 * page.uiScale)
                                    radius: 8
                                    color: page.darkMode ? "#3A2E12" : "#FFFBEB"
                                    Label {
                                        anchors.centerIn: parent
                                        text: "↻"
                                        color: page.warningColor
                                        font.pixelSize: Math.round(20 * page.uiScale)
                                        font.weight: Font.Bold
                                    }
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 3
                                    Label {
                                        text: qsTr("UEFI / BIOS Firmware")
                                        color: page.textColor
                                        font.pixelSize: Math.round(13 * page.uiScale)
                                        font.weight: Font.DemiBold
                                    }
                                    Label {
                                        Layout.fillWidth: true
                                        text: qsTr("Restart directly into firmware setup")
                                        color: page.softTextColor
                                        font.pixelSize: Math.round(11 * page.uiScale)
                                        elide: Text.ElideRight
                                    }
                                }

                                Rectangle {
                                    implicitHeight: Math.round(32 * page.uiScale)
                                    implicitWidth: restartBtnRow.implicitWidth + Math.round(20 * page.uiScale)
                                    radius: 6
                                    color: rebootFirmwareBtn.down
                                           ? Qt.darker(page.warningColor, 1.1)
                                           : (rebootFirmwareBtn.hovered ? page.warningColor : (page.darkMode ? "#3A2E12" : "#FFFBEB"))
                                    border.width: 1
                                    border.color: page.warningColor

                                    RowLayout {
                                        id: restartBtnRow
                                        anchors.centerIn: parent
                                        spacing: Math.round(4 * page.uiScale)

                                        Label {
                                            text: qsTr("Restart")
                                            color: rebootFirmwareBtn.hovered ? "#FFFFFF" : page.warningColor
                                            font.pixelSize: Math.round(12 * page.uiScale)
                                            font.weight: Font.DemiBold
                                        }

                                        Label {
                                            text: "↻"
                                            color: rebootFirmwareBtn.hovered ? "#FFFFFF" : page.warningColor
                                            font.pixelSize: Math.round(13 * page.uiScale)
                                            font.weight: Font.Bold
                                        }
                                    }
                                }
                            }

                            onClicked: {
                                if (!rebootDialogLoader.item) {
                                    rebootDialogLoader.active = true;
                                }
                                if (rebootDialogLoader.item) {
                                    rebootDialogLoader.item.theme = page.theme;
                                    rebootDialogLoader.item.darkMode = page.darkMode;
                                    rebootDialogLoader.item.uiScale = page.uiScale;
                                    rebootDialogLoader.item.open();
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Lazy dialog loaders - instantiated only when opened
    Loader {
        id: diagnosticDialogLoader
        active: false
        source: "../components/SystemDiagnosticDialog.qml"
        onLoaded: {
            item.systemInfo = page.systemInfo;
            item.theme = page.theme;
            item.darkMode = page.darkMode;
            item.uiScale = page.uiScale;
            item.copySuccess.connect(function(msg) {
                page.actionFailed = false;
                page.actionFeedback = msg;
                copiedFeedbackTimer.restart();
            });
            item.copyFailed.connect(function(msg) {
                page.actionFailed = true;
                page.actionFeedback = msg;
                copiedFeedbackTimer.restart();
            });
            item.requestRefresh.connect(function() {
                page.openDiagnosticReport();
            });
        }
    }

    Loader {
        id: rebootDialogLoader
        active: false
        source: "../components/SystemRebootDialog.qml"
        onLoaded: {
            item.systemInfo = page.systemInfo;
            item.theme = page.theme;
            item.darkMode = page.darkMode;
            item.uiScale = page.uiScale;
            item.rebootFailed.connect(function(msg) {
                page.actionFailed = true;
                page.actionFeedback = msg;
                refreshFeedbackTimer.restart();
            });
        }
    }

    Component.onCompleted: {
        if (page.systemInfo)
            page.systemInfo.refresh();
    }
}
