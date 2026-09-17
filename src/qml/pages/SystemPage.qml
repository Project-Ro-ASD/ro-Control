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

    Timer {
        id: itemCopiedTimer
        interval: 1500
        repeat: false
        onTriggered: page.lastCopiedKey = ""
    }

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

    function hardwareCards() {
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

    function softwareCards() {
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

    function diagnosticReportSections() {
        const query = page.reportFilterText.trim().toLowerCase();
        const rawSections = [
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

        const filteredSections = [];
        for (let s = 0; s < rawSections.length; s++) {
            const sec = rawSections[s];
            const validItems = [];
            for (let i = 0; i < sec.items.length; i++) {
                const itm = sec.items[i];
                if (!itm.value || itm.value.toString().trim().length === 0)
                    continue;
                if (query.length === 0 ||
                    itm.label.toLowerCase().indexOf(query) !== -1 ||
                    itm.value.toString().toLowerCase().indexOf(query) !== -1 ||
                    sec.title.toLowerCase().indexOf(query) !== -1) {
                    validItems.push(itm);
                }
            }
            if (validItems.length > 0) {
                filteredSections.push({ title: sec.title, icon: sec.icon, items: validItems });
            }
        }
        return filteredSections;
    }

    function openDiagnosticReport() {
        if (!page.systemInfo)
            return;

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
            if (page.reportCopied)
                copiedFeedbackTimer.restart();
        }
        diagnosticReportDialog.open();
    }

    Timer {
        id: copiedFeedbackTimer
        interval: 3000
        repeat: false
        onTriggered: page.reportCopied = false
    }

    ScrollView {
        id: pageScroll
        anchors.fill: parent
        clip: true
        contentWidth: availableWidth

        ColumnLayout {
            width: pageScroll.availableWidth
            spacing: Math.round(14 * page.uiScale)

            Rectangle { Layout.fillWidth: true; implicitHeight: 52; radius: 12; color: page.cardColor; border.width: 1; border.color: page.borderColor
                RowLayout { anchors.fill: parent; anchors.margins: 12; spacing: 18
                    Label { text: qsTr("System health"); color: page.textColor; font.weight: Font.DemiBold }
                    Label { Layout.fillWidth: true; text: page.systemHealthSummary(); color: page.softTextColor; elide: Text.ElideRight }
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
                            model: page.hardwareCards()

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
                            model: page.softwareCards()

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

                            onClicked: rebootConfirmDialog.open()
                        }
                    }
                }
            }
        }
    }

    Dialog {
        id: diagnosticReportDialog
        modal: true
        anchors.centerIn: parent
        width: Math.min(page.width * 0.94, Math.round(880 * page.uiScale))
        height: Math.min(page.height * 0.90, Math.round(700 * page.uiScale))
        padding: 0
        header: null
        footer: null

        enter: Transition {
            NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: 180; easing.type: Easing.OutQuad }
            NumberAnimation { property: "scale"; from: 0.96; to: 1.0; duration: 180; easing.type: Easing.OutQuad }
        }
        exit: Transition {
            NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: 140; easing.type: Easing.InQuad }
            NumberAnimation { property: "scale"; from: 1.0; to: 0.96; duration: 140; easing.type: Easing.InQuad }
        }

        background: Rectangle {
            radius: 16
            color: page.cardColor
            border.width: 1
            border.color: page.borderColor
        }

        contentItem: ColumnLayout {
            spacing: 0

            // Header Bar
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: Math.round(64 * page.uiScale)
                color: page.darkMode ? "#2E2640" : "#F8FAFC"
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
                    color: page.borderColor
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: Math.round(20 * page.uiScale)
                    anchors.rightMargin: Math.round(16 * page.uiScale)
                    spacing: Math.round(12 * page.uiScale)

                    Rectangle {
                        implicitWidth: Math.round(36 * page.uiScale)
                        implicitHeight: Math.round(36 * page.uiScale)
                        radius: 8
                        color: page.darkMode ? "#312E81" : "#E0E7FF"
                        Label {
                            anchors.centerIn: parent
                            text: "▤"
                            color: page.accentColor
                            font.pixelSize: Math.round(18 * page.uiScale)
                            font.weight: Font.Bold
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2
                        Label {
                            text: qsTr("System Diagnostic Report")
                            color: page.textColor
                            font.pixelSize: Math.round(16 * page.uiScale)
                            font.weight: Font.DemiBold
                        }
                        Label {
                            text: qsTr("System hardware, kernel, driver and security telemetry snapshot")
                            color: page.softTextColor
                            font.pixelSize: Math.round(11 * page.uiScale)
                        }
                    }

                    ToolButton {
                        id: closeDiagnosticReportButton
                        text: "✕"
                        implicitWidth: Math.round(32 * page.uiScale)
                        implicitHeight: Math.round(32 * page.uiScale)
                        hoverEnabled: true
                        background: Rectangle {
                            radius: 8
                            color: closeDiagnosticReportButton.hovered ? (page.darkMode ? "#43385E" : "#E2E8F0") : "transparent"
                        }
                        contentItem: Text {
                            text: "✕"
                            color: closeDiagnosticReportButton.hovered ? page.textColor : page.softTextColor
                            font.pixelSize: Math.round(14 * page.uiScale)
                            font.weight: Font.Bold
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        onClicked: diagnosticReportDialog.close()
                    }
                }
            }

            // Controls Toolbar (Segmented View Switcher & Context Controls)
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: controlsRow.implicitHeight + Math.round(20 * page.uiScale)
                color: "transparent"

                RowLayout {
                    id: controlsRow
                    anchors.fill: parent
                    anchors.margins: Math.round(16 * page.uiScale)
                    spacing: Math.round(14 * page.uiScale)

                    // View Mode Segmented Switcher
                    Rectangle {
                        implicitHeight: Math.round(34 * page.uiScale)
                        implicitWidth: Math.round(260 * page.uiScale)
                        radius: 8
                        color: page.darkMode ? "#241E34" : "#E2E8F0"
                        border.width: 1
                        border.color: page.borderColor

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 2
                            spacing: 2

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                radius: 6
                                color: page.reportViewMode === 0 ? (page.darkMode ? "#3E355B" : "#FFFFFF") : "transparent"
                                border.width: page.reportViewMode === 0 ? 1 : 0
                                border.color: page.reportViewMode === 0 ? (page.darkMode ? "#5B4E85" : "#CBD5E1") : "transparent"

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: page.reportViewMode = 0
                                }
                                Label {
                                    anchors.centerIn: parent
                                    text: qsTr("Overview Cards")
                                    color: page.reportViewMode === 0 ? page.textColor : page.softTextColor
                                    font.pixelSize: Math.round(12 * page.uiScale)
                                    font.weight: page.reportViewMode === 0 ? Font.DemiBold : Font.Normal
                                }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                radius: 6
                                color: page.reportViewMode === 1 ? (page.darkMode ? "#3E355B" : "#FFFFFF") : "transparent"
                                border.width: page.reportViewMode === 1 ? 1 : 0
                                border.color: page.reportViewMode === 1 ? (page.darkMode ? "#5B4E85" : "#CBD5E1") : "transparent"

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: page.reportViewMode = 1
                                }
                                Label {
                                    anchors.centerIn: parent
                                    text: qsTr("Code / Export")
                                    color: page.reportViewMode === 1 ? page.textColor : page.softTextColor
                                    font.pixelSize: Math.round(12 * page.uiScale)
                                    font.weight: page.reportViewMode === 1 ? Font.DemiBold : Font.Normal
                                }
                            }
                        }
                    }

                    Item { Layout.fillWidth: true }

                    // Overview Cards View Toolbar: Live Search Filter
                    Rectangle {
                        visible: page.reportViewMode === 0
                        implicitHeight: Math.round(34 * page.uiScale)
                        implicitWidth: Math.min(controlsRow.width * 0.45, Math.round(240 * page.uiScale))
                        radius: 8
                        color: page.bgColor
                        border.width: 1
                        border.color: filterInput.activeFocus ? page.accentColor : page.borderColor

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: Math.round(10 * page.uiScale)
                            anchors.rightMargin: Math.round(8 * page.uiScale)
                            spacing: 6

                            Label {
                                text: "🔍"
                                font.pixelSize: Math.round(12 * page.uiScale)
                                color: page.softTextColor
                            }

                            TextInput {
                                id: filterInput
                                Layout.fillWidth: true
                                text: page.reportFilterText
                                color: page.textColor
                                font.pixelSize: Math.round(12 * page.uiScale)
                                verticalAlignment: TextInput.AlignVCenter
                                onTextChanged: page.reportFilterText = text

                                Text {
                                    text: qsTr("Filter properties...")
                                    color: page.softTextColor
                                    font.pixelSize: Math.round(12 * page.uiScale)
                                    visible: !filterInput.text && !filterInput.activeFocus
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }

                            ToolButton {
                                visible: page.reportFilterText.length > 0
                                text: "✕"
                                implicitWidth: Math.round(20 * page.uiScale)
                                implicitHeight: Math.round(20 * page.uiScale)
                                background: null
                                contentItem: Text {
                                    text: "✕"
                                    color: page.softTextColor
                                    font.pixelSize: Math.round(11 * page.uiScale)
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                onClicked: {
                                    page.reportFilterText = "";
                                    filterInput.text = "";
                                }
                            }
                        }
                    }

                    // Code / Export View Toolbar: Format & Action Dropdowns
                    RowLayout {
                        visible: page.reportViewMode === 1
                        spacing: Math.round(14 * page.uiScale)

                        // Format Dropdown Selector
                        RowLayout {
                            spacing: Math.round(8 * page.uiScale)
                            Label {
                                text: qsTr("Format:")
                                color: page.softTextColor
                                font.pixelSize: Math.round(12 * page.uiScale)
                                font.weight: Font.DemiBold
                            }

                            Rectangle {
                                id: formatSelectorButton
                                implicitHeight: Math.round(32 * page.uiScale)
                                implicitWidth: formatBtnRow.implicitWidth + Math.round(20 * page.uiScale)
                                radius: 8
                                color: formatMouse.containsMouse ? (page.darkMode ? "#342D4A" : "#E2E8F0") : page.bgColor
                                border.width: 1
                                border.color: (formatMouse.containsMouse || formatPopup.visible) ? page.accentColor : page.borderColor

                                MouseArea {
                                    id: formatMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: formatPopup.open()
                                }

                                RowLayout {
                                    id: formatBtnRow
                                    anchors.centerIn: parent
                                    spacing: 6

                                    Label {
                                        text: {
                                            const fmt = page.systemInfo ? page.systemInfo.diagnosticReportFormat : "markdown";
                                            if (fmt === "json") return "JSON";
                                            if (fmt === "plain") return qsTr("Plain Text");
                                            return "Markdown";
                                        }
                                        color: page.textColor
                                        font.pixelSize: Math.round(12 * page.uiScale)
                                        font.weight: Font.DemiBold
                                    }

                                    Label {
                                        text: "▾"
                                        color: page.accentColor
                                        font.pixelSize: Math.round(11 * page.uiScale)
                                        font.weight: Font.Bold
                                    }
                                }

                                Popup {
                                    id: formatPopup
                                    y: formatSelectorButton.height + 4
                                    width: Math.round(150 * page.uiScale)
                                    padding: 4
                                    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

                                    background: Rectangle {
                                        radius: 10
                                        color: page.bgColor
                                        border.width: 1
                                        border.color: page.borderColor
                                    }

                                    contentItem: ColumnLayout {
                                        spacing: 2
                                        Repeater {
                                            model: [
                                                { id: "markdown", label: "Markdown" },
                                                { id: "plain", label: qsTr("Plain Text") },
                                                { id: "json", label: "JSON" }
                                            ]

                                            delegate: AbstractButton {
                                                id: fmtItemBtn
                                                required property var modelData
                                                Layout.fillWidth: true
                                                implicitHeight: Math.round(32 * page.uiScale)
                                                hoverEnabled: true
                                                readonly property bool isSelected: page.systemInfo && page.systemInfo.diagnosticReportFormat === fmtItemBtn.modelData.id

                                                background: Rectangle {
                                                    radius: 6
                                                    color: fmtItemBtn.hovered
                                                           ? (page.darkMode ? "#43385E" : "#E2E8F0")
                                                           : (fmtItemBtn.isSelected ? (page.darkMode ? "#342D4A" : "#F1F5F9") : "transparent")
                                                }

                                                contentItem: RowLayout {
                                                    anchors.fill: parent
                                                    anchors.leftMargin: Math.round(10 * page.uiScale)
                                                    anchors.rightMargin: Math.round(10 * page.uiScale)
                                                    spacing: 6

                                                    Label {
                                                        Layout.fillWidth: true
                                                        text: fmtItemBtn.modelData.label
                                                        color: fmtItemBtn.isSelected ? page.accentColor : (fmtItemBtn.hovered ? page.textColor : page.softTextColor)
                                                        font.pixelSize: Math.round(12 * page.uiScale)
                                                        font.weight: fmtItemBtn.isSelected ? Font.Bold : Font.Normal
                                                    }

                                                    Label {
                                                        visible: fmtItemBtn.isSelected
                                                        text: "✓"
                                                        color: page.accentColor
                                                        font.pixelSize: Math.round(12 * page.uiScale)
                                                        font.weight: Font.Bold
                                                    }
                                                }

                                                onClicked: {
                                                    if (page.systemInfo) {
                                                        page.systemInfo.setDiagnosticReportFormat(fmtItemBtn.modelData.id);
                                                        page.openDiagnosticReport();
                                                    }
                                                    formatPopup.close();
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        // Destination / Auto-Action Dropdown Selector
                        RowLayout {
                            spacing: Math.round(8 * page.uiScale)
                            Label {
                                text: qsTr("Action:")
                                color: page.softTextColor
                                font.pixelSize: Math.round(12 * page.uiScale)
                                font.weight: Font.DemiBold
                            }

                            Rectangle {
                                id: actionSelectorButton
                                implicitHeight: Math.round(32 * page.uiScale)
                                implicitWidth: actionBtnRow.implicitWidth + Math.round(20 * page.uiScale)
                                radius: 8
                                color: actionMouse.containsMouse ? (page.darkMode ? "#342D4A" : "#E2E8F0") : page.bgColor
                                border.width: 1
                                border.color: (actionMouse.containsMouse || actionPopup.visible) ? page.accentColor : page.borderColor

                                MouseArea {
                                    id: actionMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: actionPopup.open()
                                }

                                RowLayout {
                                    id: actionBtnRow
                                    anchors.centerIn: parent
                                    spacing: 6

                                    Label {
                                        text: {
                                            const dest = page.systemInfo ? page.systemInfo.diagnosticReportDestination : "preview";
                                            if (dest === "clipboard") return qsTr("Copy on Open");
                                            return qsTr("Preview");
                                        }
                                        color: page.textColor
                                        font.pixelSize: Math.round(12 * page.uiScale)
                                        font.weight: Font.DemiBold
                                    }

                                    Label {
                                        text: "▾"
                                        color: page.accentColor
                                        font.pixelSize: Math.round(11 * page.uiScale)
                                        font.weight: Font.Bold
                                    }
                                }

                                Popup {
                                    id: actionPopup
                                    y: actionSelectorButton.height + 4
                                    width: Math.round(160 * page.uiScale)
                                    padding: 4
                                    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

                                    background: Rectangle {
                                        radius: 10
                                        color: page.bgColor
                                        border.width: 1
                                        border.color: page.borderColor
                                    }

                                    contentItem: ColumnLayout {
                                        spacing: 2
                                        Repeater {
                                            model: [
                                                { id: "preview", label: qsTr("Preview") },
                                                { id: "clipboard", label: qsTr("Copy on Open") }
                                            ]

                                            delegate: AbstractButton {
                                                id: actItemBtn
                                                required property var modelData
                                                Layout.fillWidth: true
                                                implicitHeight: Math.round(32 * page.uiScale)
                                                hoverEnabled: true
                                                readonly property bool isSelected: page.systemInfo && page.systemInfo.diagnosticReportDestination === actItemBtn.modelData.id

                                                background: Rectangle {
                                                    radius: 6
                                                    color: actItemBtn.hovered
                                                           ? (page.darkMode ? "#43385E" : "#E2E8F0")
                                                           : (actItemBtn.isSelected ? (page.darkMode ? "#342D4A" : "#F1F5F9") : "transparent")
                                                }

                                                contentItem: RowLayout {
                                                    anchors.fill: parent
                                                    anchors.leftMargin: Math.round(10 * page.uiScale)
                                                    anchors.rightMargin: Math.round(10 * page.uiScale)
                                                    spacing: 6

                                                    Label {
                                                        Layout.fillWidth: true
                                                        text: actItemBtn.modelData.label
                                                        color: actItemBtn.isSelected ? page.accentColor : (actItemBtn.hovered ? page.textColor : page.softTextColor)
                                                        font.pixelSize: Math.round(12 * page.uiScale)
                                                        font.weight: actItemBtn.isSelected ? Font.Bold : Font.Normal
                                                    }

                                                    Label {
                                                        visible: actItemBtn.isSelected
                                                        text: "✓"
                                                        color: page.accentColor
                                                        font.pixelSize: Math.round(12 * page.uiScale)
                                                        font.weight: Font.Bold
                                                    }
                                                }

                                                onClicked: {
                                                    if (page.systemInfo)
                                                        page.systemInfo.setDiagnosticReportDestination(actItemBtn.modelData.id);
                                                    actionPopup.close();
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Main Content Area
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.leftMargin: Math.round(16 * page.uiScale)
                Layout.rightMargin: Math.round(16 * page.uiScale)

                // View 0: Interactive System Snapshot Cards
                ScrollView {
                    id: cardsScrollView
                    visible: page.reportViewMode === 0
                    anchors.fill: parent
                    clip: true

                    ColumnLayout {
                        width: cardsScrollView.availableWidth
                        spacing: Math.round(16 * page.uiScale)

                        Repeater {
                            model: page.diagnosticReportSections()

                            delegate: Rectangle {
                                id: sectionCard
                                required property var modelData
                                Layout.fillWidth: true
                                radius: 12
                                color: page.bgColor
                                border.width: 1
                                border.color: page.borderColor
                                implicitHeight: secColumn.implicitHeight + Math.round(24 * page.uiScale)

                                ColumnLayout {
                                    id: secColumn
                                    anchors.fill: parent
                                    anchors.margins: Math.round(12 * page.uiScale)
                                    spacing: Math.round(10 * page.uiScale)

                                    // Section Header with Category Badge
                                    RowLayout {
                                        Layout.fillWidth: true
                                        spacing: Math.round(8 * page.uiScale)

                                        Label {
                                            text: sectionCard.modelData.icon
                                            font.pixelSize: Math.round(15 * page.uiScale)
                                        }

                                        Label {
                                            text: sectionCard.modelData.title
                                            color: page.textColor
                                            font.pixelSize: Math.round(14 * page.uiScale)
                                            font.weight: Font.DemiBold
                                            Layout.fillWidth: true
                                        }

                                        Rectangle {
                                            implicitHeight: Math.round(20 * page.uiScale)
                                            implicitWidth: secCountLabel.implicitWidth + Math.round(12 * page.uiScale)
                                            radius: 10
                                            color: page.darkMode ? "#342D4A" : "#E2E8F0"
                                            Label {
                                                id: secCountLabel
                                                anchors.centerIn: parent
                                                text: sectionCard.modelData.items.length + " " + qsTr("items")
                                                color: page.softTextColor
                                                font.pixelSize: Math.round(10 * page.uiScale)
                                                font.weight: Font.DemiBold
                                            }
                                        }
                                    }

                                    // Grid of Parameter Cards
                                    GridLayout {
                                        Layout.fillWidth: true
                                        columns: width > 520 ? 2 : 1
                                        columnSpacing: Math.round(8 * page.uiScale)
                                        rowSpacing: Math.round(8 * page.uiScale)

                                        Repeater {
                                            model: sectionCard.modelData.items

                                            delegate: Rectangle {
                                                id: itemTile
                                                required property var modelData
                                                Layout.fillWidth: true
                                                implicitHeight: Math.round(62 * page.uiScale)
                                                radius: 8
                                                color: tileMouse.containsMouse
                                                       ? (page.darkMode ? "#383050" : "#F8FAFC")
                                                       : (page.darkMode ? "#2E2742" : "#F1F5F9")
                                                border.width: 1
                                                border.color: tileMouse.containsMouse ? page.accentColor : page.borderColor


                                                MouseArea {
                                                    id: tileMouse
                                                    anchors.fill: parent
                                                    hoverEnabled: true
                                                }

                                                RowLayout {
                                                    anchors.fill: parent
                                                    anchors.leftMargin: Math.round(12 * page.uiScale)
                                                    anchors.rightMargin: Math.round(12 * page.uiScale)
                                                    spacing: Math.round(10 * page.uiScale)

                                                    Label {
                                                        text: itemTile.modelData.icon || "•"
                                                        font.pixelSize: Math.round(16 * page.uiScale)
                                                    }

                                                    ColumnLayout {
                                                        Layout.fillWidth: true
                                                        spacing: 2

                                                        Label {
                                                            Layout.fillWidth: true
                                                            text: itemTile.modelData.label
                                                            color: page.softTextColor
                                                            font.pixelSize: Math.round(11 * page.uiScale)
                                                            font.weight: Font.DemiBold
                                                            elide: Text.ElideRight
                                                        }

                                                        Label {
                                                            Layout.fillWidth: true
                                                            text: itemTile.modelData.value
                                                            color: page.textColor
                                                            font.pixelSize: Math.round(13 * page.uiScale)
                                                            font.weight: Font.DemiBold
                                                            elide: Text.ElideRight
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        // Empty Filter State
                        Rectangle {
                            visible: page.diagnosticReportSections().length === 0
                            Layout.fillWidth: true
                            implicitHeight: Math.round(160 * page.uiScale)
                            radius: 12
                            color: page.bgColor
                            border.width: 1
                            border.color: page.borderColor

                            ColumnLayout {
                                anchors.centerIn: parent
                                spacing: Math.round(8 * page.uiScale)

                                Label {
                                    Layout.alignment: Qt.AlignHCenter
                                    text: "🔍"
                                    font.pixelSize: Math.round(24 * page.uiScale)
                                }

                                Label {
                                    Layout.alignment: Qt.AlignHCenter
                                    text: qsTr("No matching properties found")
                                    color: page.textColor
                                    font.pixelSize: Math.round(14 * page.uiScale)
                                    font.weight: Font.DemiBold
                                }

                                Label {
                                    Layout.alignment: Qt.AlignHCenter
                                    text: qsTr("Try a different search term or clear the filter.")
                                    color: page.softTextColor
                                    font.pixelSize: Math.round(12 * page.uiScale)
                                }

                                Button {
                                    Layout.alignment: Qt.AlignHCenter
                                    text: qsTr("Clear Filter")
                                    onClicked: {
                                        page.reportFilterText = "";
                                        filterInput.text = "";
                                    }
                                }
                            }
                        }
                    }
                }

                // View 1: Formatted Code / Export Viewer
                Rectangle {
                    visible: page.reportViewMode === 1
                    anchors.fill: parent
                    radius: 10
                    color: page.bgColor
                    border.width: 1
                    border.color: page.borderColor

                    ScrollView {
                        anchors.fill: parent
                        anchors.margins: Math.round(10 * page.uiScale)
                        clip: true

                        TextArea {
                            id: diagnosticReportText
                            text: page.generatedReport
                            readOnly: true
                            selectByMouse: true
                            textFormat: {
                                const fmt = page.systemInfo ? page.systemInfo.diagnosticReportFormat : "markdown";
                                return fmt === "markdown" ? TextEdit.MarkdownText : TextEdit.PlainText;
                            }
                            wrapMode: {
                                const fmt = page.systemInfo ? page.systemInfo.diagnosticReportFormat : "markdown";
                                return fmt === "json" ? TextEdit.NoWrap : TextEdit.Wrap;
                            }
                            color: page.textColor
                            font.family: {
                                const fmt = page.systemInfo ? page.systemInfo.diagnosticReportFormat : "markdown";
                                return fmt === "json" ? "monospace" : ""
                            }
                            font.pixelSize: Math.round(13 * page.uiScale)
                            selectedTextColor: "#FFFFFF"
                            selectionColor: page.accentColor
                            padding: Math.round(8 * page.uiScale)
                            background: null
                        }
                    }
                }
            }

            // Bottom Footer Bar
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: Math.round(64 * page.uiScale)
                color: "transparent"

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: Math.round(16 * page.uiScale)
                    spacing: Math.round(12 * page.uiScale)

                    // Copied Feedback Badge
                    Rectangle {
                        visible: page.reportCopied
                        implicitHeight: Math.round(32 * page.uiScale)
                        implicitWidth: copyFeedbackRow.implicitWidth + Math.round(16 * page.uiScale)
                        radius: 8
                        color: page.darkMode ? "#143828" : "#ECFDF5"
                        border.width: 1
                        border.color: page.successColor

                        RowLayout {
                            id: copyFeedbackRow
                            anchors.centerIn: parent
                            spacing: 6
                            Label {
                                text: "✓"
                                color: page.successColor
                                font.weight: Font.Bold
                                font.pixelSize: Math.round(13 * page.uiScale)
                            }
                            Label {
                                text: qsTr("Copied to clipboard!")
                                color: page.successColor
                                font.pixelSize: Math.round(12 * page.uiScale)
                                font.weight: Font.DemiBold
                            }
                        }
                    }

                    Item { Layout.fillWidth: true }

                    // Close Button
                    Button {
                        id: closeReportBtn
                        text: qsTr("Close")
                        implicitHeight: Math.round(38 * page.uiScale)
                        implicitWidth: Math.round(90 * page.uiScale)
                        hoverEnabled: true

                        background: Rectangle {
                            radius: 8
                            color: closeReportBtn.hovered
                                   ? (page.darkMode ? "#3B3156" : "#E2E8F0")
                                   : page.bgColor
                            border.width: 1
                            border.color: closeReportBtn.hovered ? page.accentColor : page.borderColor
                        }

                        contentItem: Label {
                            text: closeReportBtn.text
                            color: closeReportBtn.hovered ? page.textColor : page.softTextColor
                            font.pixelSize: Math.round(13 * page.uiScale)
                            font.weight: Font.DemiBold
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: diagnosticReportDialog.close()
                    }

                    // Copy to Clipboard Primary Button
                    Button {
                        id: copyReportBtn
                        text: qsTr("Copy Full Report")
                        implicitHeight: Math.round(38 * page.uiScale)
                        implicitWidth: copyLabel.implicitWidth + Math.round(28 * page.uiScale)
                        hoverEnabled: true

                        background: Rectangle {
                            radius: 8
                            color: copyReportBtn.down ? Qt.darker(page.accentColor, 1.15)
                                                      : (copyReportBtn.hovered ? Qt.lighter(page.accentColor, 1.1) : page.accentColor)
                            border.width: 1
                            border.color: page.accentColor
                        }

                        contentItem: Label {
                            id: copyLabel
                            text: copyReportBtn.text
                            color: "#FFFFFF"
                            font.pixelSize: Math.round(13 * page.uiScale)
                            font.weight: Font.Bold
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            page.reportCopied = page.systemInfo && page.systemInfo.copyToClipboard(page.generatedReport);
                            if (page.reportCopied)
                                copiedFeedbackTimer.restart();
                        }
                    }
                }
            }
        }
    }

    Dialog {
        id: rebootConfirmDialog
        modal: true
        focus: true
        anchors.centerIn: parent
        width: Math.max(0, Math.min(page.width - Math.round(24 * page.uiScale), Math.round(440 * page.uiScale)))
        padding: 0
        header: null
        footer: null

        background: Rectangle {
            radius: 16
            color: page.cardColor
            border.width: 1
            border.color: page.borderColor
        }

        contentItem: ColumnLayout {
            spacing: 0

            // Header
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: Math.round(60 * page.uiScale)
                color: page.darkMode ? "#3A2E12" : "#FFFBEB"
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
                    color: page.darkMode ? "#4D3D18" : "#FDE68A"
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: Math.round(16 * page.uiScale)
                    spacing: Math.round(10 * page.uiScale)

                    Label {
                        text: "⚠️"
                        font.pixelSize: Math.round(18 * page.uiScale)
                    }

                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Reboot to UEFI / BIOS")
                        color: page.textColor
                        font.pixelSize: Math.round(15 * page.uiScale)
                        font.weight: Font.Bold
                    }
                }
            }

            // Body Content
            ColumnLayout {
                Layout.fillWidth: true
                Layout.margins: Math.round(18 * page.uiScale)
                spacing: Math.round(10 * page.uiScale)

                Label {
                    Layout.fillWidth: true
                    text: qsTr("Your system will restart immediately and boot directly into the UEFI / BIOS firmware setup utility.")
                    color: page.textColor
                    font.pixelSize: Math.round(13 * page.uiScale)
                    wrapMode: Text.WordWrap
                }

                Label {
                    Layout.fillWidth: true
                    text: qsTr("Make sure any unsaved work in other applications is saved before continuing.")
                    color: page.warningColor
                    font.pixelSize: Math.round(12 * page.uiScale)
                    font.weight: Font.DemiBold
                    wrapMode: Text.WordWrap
                }
            }

            // Footer
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: firmwareButtons.columns === 1
                                ? Math.round(104 * page.uiScale)
                                : Math.round(58 * page.uiScale)
                color: "transparent"

                GridLayout {
                    id: firmwareButtons
                    anchors.fill: parent
                    anchors.leftMargin: Math.round(18 * page.uiScale)
                    anchors.rightMargin: Math.round(18 * page.uiScale)
                    anchors.bottomMargin: Math.round(14 * page.uiScale)
                    columnSpacing: Math.round(10 * page.uiScale)
                    rowSpacing: Math.round(8 * page.uiScale)
                    columns: rebootConfirmDialog.width < Math.round(340 * page.uiScale) ? 1 : 3

                    Item {
                        visible: firmwareButtons.columns > 1
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }

                    Button {
                        id: cancelRebootBtn
                        text: qsTr("Cancel")
                        implicitHeight: Math.round(36 * page.uiScale)
                        implicitWidth: cancelRebootLabel.implicitWidth + Math.round(32 * page.uiScale)
                        Layout.fillWidth: firmwareButtons.columns === 1
                        hoverEnabled: true

                        background: Rectangle {
                            radius: 8
                            color: cancelRebootBtn.down
                                   ? (page.darkMode ? "#342A4E" : "#CBD5E1")
                                   : (cancelRebootBtn.hovered ? (page.darkMode ? "#3B3156" : "#E2E8F0") : page.bgColor)
                            border.width: 1
                            border.color: cancelRebootBtn.hovered ? page.accentColor : page.borderColor
                        }

                        contentItem: Label {
                            id: cancelRebootLabel
                            text: cancelRebootBtn.text
                            color: cancelRebootBtn.hovered ? page.textColor : page.softTextColor
                            font.pixelSize: Math.round(13 * page.uiScale)
                            font.weight: Font.DemiBold
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: rebootConfirmDialog.close()
                    }

                    Button {
                        id: confirmRebootBtn
                        text: qsTr("Restart Now ↻")
                        implicitHeight: Math.round(36 * page.uiScale)
                        implicitWidth: confirmRebootLabel.implicitWidth + Math.round(32 * page.uiScale)
                        Layout.fillWidth: firmwareButtons.columns === 1
                        hoverEnabled: true

                        background: Rectangle {
                            radius: 8
                            color: confirmRebootBtn.down
                                   ? Qt.darker(page.warningColor, 1.15)
                                   : (confirmRebootBtn.hovered ? Qt.lighter(page.warningColor, 1.1) : page.warningColor)
                        }

                        contentItem: Label {
                            id: confirmRebootLabel
                            text: confirmRebootBtn.text
                            color: "#FFFFFF"
                            font.pixelSize: Math.round(13 * page.uiScale)
                            font.weight: Font.Bold
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            rebootConfirmDialog.close();
                            if (page.systemInfo)
                                page.systemInfo.requestRebootToFirmware();
                        }
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        if (page.systemInfo)
            page.systemInfo.refresh();
    }
}
