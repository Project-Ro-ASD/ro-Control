import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: page

    required property var theme
    property bool darkMode: false
    property bool compactMode: false
    property bool showAdvancedInfo: true

    property string bannerText: ""
    property string bannerTone: "info"
    property string operationSource: ""
    property string operationPhase: ""
    property string operationDetail: ""
    property bool operationActive: false
    property double operationStartedAt: 0
    property double lastLogAt: 0
    property int operationElapsedSeconds: 0
    readonly property string nvidiaLicenseUrl: "https://www.nvidia.com/en-us/drivers/nvidia-license/"
    readonly property bool backendBusy: nvidiaInstaller.busy || nvidiaUpdater.busy
    readonly property bool operationRunning: page.operationActive || page.backendBusy
    readonly property bool remoteDriverCatalogAvailable: nvidiaUpdater.availableVersions.length > 0
    readonly property bool canInstallLatestRemoteDriver: nvidiaDetector.gpuFound && remoteDriverCatalogAvailable
    readonly property bool driverInstalledLocally: nvidiaDetector.driverVersion.length > 0 || nvidiaUpdater.currentVersion.length > 0
    readonly property string installedVersionLabel: nvidiaDetector.driverVersion.length > 0 ? nvidiaDetector.driverVersion : nvidiaUpdater.currentVersion
    readonly property string latestVersionLabel: cleanVersionLabel(nvidiaUpdater.latestVersion)
    readonly property var availableVersionOptions: buildAvailableVersionOptions(nvidiaUpdater.availableVersions)
    readonly property string recommendedTitle: driverInstalledLocally
                                               ? qsTr("Driver installed and ready")
                                               : remoteDriverCatalogAvailable
                                                 ? qsTr("Recommended package available")
                                                 : qsTr("Preparing recommendation")
    readonly property string recommendedVersion: latestVersionLabel.length > 0
                                                 ? latestVersionLabel
                                                 : (installedVersionLabel.length > 0 ? installedVersionLabel : qsTr("Waiting for scan"))
    readonly property string detectedHardwareLabel: nvidiaDetector.gpuName.length > 0
                                                    ? nvidiaDetector.gpuName
                                                    : (nvidiaDetector.displayAdapterName.length > 0
                                                       ? nvidiaDetector.displayAdapterName
                                                       : qsTr("Hardware information unavailable"))

    function classifyOperationPhase(message) {
        const lowered = message.toLowerCase();
        if (lowered.indexOf("rpm fusion") >= 0 || lowered.indexOf("repository") >= 0)
            return qsTr("Repository Setup");
        if (lowered.indexOf("install") >= 0 || lowered.indexOf("remove") >= 0 || lowered.indexOf("deep clean") >= 0)
            return qsTr("Package Transaction");
        if (lowered.indexOf("kernel") >= 0 || lowered.indexOf("akmods") >= 0 || lowered.indexOf("dracut") >= 0)
            return qsTr("Kernel Integration");
        if (lowered.indexOf("wayland") >= 0 || lowered.indexOf("x11") >= 0 || lowered.indexOf("session") >= 0)
            return qsTr("Session Finalization");
        if (lowered.indexOf("update") >= 0 || lowered.indexOf("version") >= 0)
            return qsTr("Update Check");
        return qsTr("General");
    }

    function setOperationState(source, message, tone, running) {
        if (running && !operationRunning)
            operationStartedAt = Date.now();
        if (!running)
            operationStartedAt = 0;
        operationSource = source;
        operationDetail = message;
        operationPhase = classifyOperationPhase(message);
        operationActive = running;
        operationElapsedSeconds = operationRunning && operationStartedAt > 0
                                  ? Math.max(0, Math.floor((Date.now() - operationStartedAt) / 1000))
                                  : 0;
        bannerText = (operationPhase.length > 0 ? operationPhase + ": " : "") + message;
        bannerTone = tone;
    }

    function finishOperation(source, success, message) {
        setOperationState(source, message, success ? "success" : "error", false);
    }

    function formatTimestamp(epochMs) {
        if (epochMs <= 0)
            return "--:--:--";
        const stamp = new Date(epochMs);
        return Qt.formatTime(stamp, "HH:mm:ss");
    }

    function formatDuration(totalSeconds) {
        const seconds = Math.max(0, totalSeconds);
        const hours = Math.floor(seconds / 3600);
        const minutes = Math.floor((seconds % 3600) / 60);
        const remainingSeconds = seconds % 60;

        function pad(value) {
            return value < 10 ? "0" + value : value.toString();
        }

        if (hours > 0)
            return hours + ":" + pad(minutes) + ":" + pad(remainingSeconds);
        return minutes + ":" + pad(remainingSeconds);
    }

    function cleanVersionLabel(rawVersion) {
        let normalized = (rawVersion || "").trim();
        if (normalized.length === 0)
            return "";

        const epochIndex = normalized.indexOf(":");
        if (epochIndex >= 0)
            normalized = normalized.substring(epochIndex + 1);

        const releaseMatch = normalized.match(/^([0-9]+(?:\.[0-9]+)+)/);
        if (releaseMatch && releaseMatch.length > 1)
            return releaseMatch[1];

        const hyphenIndex = normalized.indexOf("-");
        if (hyphenIndex > 0)
            return normalized.substring(0, hyphenIndex);

        return normalized;
    }

    function compareVersionLabels(leftVersion, rightVersion) {
        const leftParts = cleanVersionLabel(leftVersion).split(".");
        const rightParts = cleanVersionLabel(rightVersion).split(".");
        const maxLength = Math.max(leftParts.length, rightParts.length);

        for (let i = 0; i < maxLength; ++i) {
            const leftValue = i < leftParts.length ? parseInt(leftParts[i], 10) : 0;
            const rightValue = i < rightParts.length ? parseInt(rightParts[i], 10) : 0;

            if (leftValue > rightValue)
                return -1;
            if (leftValue < rightValue)
                return 1;
        }

        return 0;
    }

    function buildVersionTitle(displayVersion, isInstalled, isLatest) {
        const tags = [];
        if (isInstalled)
            tags.push(qsTr("Installed"));
        if (isLatest)
            tags.push(qsTr("Latest"));

        return tags.length > 0
                ? displayVersion + " (" + tags.join(", ") + ")"
                : displayVersion;
    }

    function buildAvailableVersionOptions(rawVersions) {
        const options = [];
        const seenLabels = {};
        const installedVersionLabel = cleanVersionLabel(nvidiaUpdater.currentVersion);
        const latestVersionLabel = cleanVersionLabel(nvidiaUpdater.latestVersion);

        for (let i = 0; i < rawVersions.length; ++i) {
            const rawVersion = rawVersions[i];
            const displayVersion = cleanVersionLabel(rawVersion);
            if (displayVersion.length === 0 || seenLabels[displayVersion])
                continue;

            seenLabels[displayVersion] = true;
            const isInstalled = installedVersionLabel.length > 0 && displayVersion === installedVersionLabel;
            const isLatest = latestVersionLabel.length > 0 && displayVersion === latestVersionLabel;
            options.push({
                rawVersion: rawVersion,
                displayVersion: displayVersion,
                versionTitle: buildVersionTitle(displayVersion, isInstalled, isLatest),
                isInstalled: isInstalled,
                isLatest: isLatest
            });
        }

        options.sort(function(left, right) {
            return compareVersionLabels(left.displayVersion, right.displayVersion);
        });

        return options;
    }

    function appendLog(source, message) {
        const now = Date.now();
        lastLogAt = now;
        logArea.append("[" + formatTimestamp(now) + "] " + source + ": " + message);
        logArea.cursorPosition = logArea.length;
    }

    Timer {
        interval: 1000
        repeat: true
        running: page.operationRunning
        onTriggered: {
            if (page.operationStartedAt > 0)
                page.operationElapsedSeconds = Math.max(0, Math.floor((Date.now() - page.operationStartedAt) / 1000));
        }
    }

    ScrollView {
        id: pageScroll
        anchors.fill: parent
        clip: true
        contentWidth: availableWidth

        ColumnLayout {
            width: pageScroll.availableWidth
            spacing: page.compactMode ? 16 : 22

            StatusBanner {
                Layout.fillWidth: true
                theme: page.theme
                tone: page.bannerTone
                text: page.bannerText
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
                spacing: page.compactMode ? 14 : 18

                Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("Select Installation Type")
                    color: page.theme.text
                    font.pixelSize: page.compactMode ? 34 : 40
                    font.weight: Font.Bold
                }

                Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("Optimized for your hardware")
                    color: page.theme.textSoft
                    font.pixelSize: 17
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.maximumWidth: 1080
                Layout.alignment: Qt.AlignHCenter
                radius: 26
                color: page.theme.card
                border.width: 1
                border.color: page.theme.border
                implicitHeight: expressColumn.implicitHeight + 42
                layer.enabled: true

                ColumnLayout {
                    id: expressColumn
                    x: 30
                    y: 26
                    width: parent.width - 60
                    spacing: 18

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 18

                        Rectangle {
                            width: 46
                            height: 46
                            radius: 15
                            color: page.theme.success

                            Label {
                                anchors.centerIn: parent
                                text: "OK"
                                color: "#ffffff"
                                font.pixelSize: 14
                                font.weight: Font.Bold
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 6

                            Label {
                                text: qsTr("Express Install")
                                color: page.theme.text
                                font.pixelSize: 24
                                font.weight: Font.Bold
                            }

                            Label {
                                Layout.fillWidth: true
                                wrapMode: Text.Wrap
                                color: page.theme.textSoft
                                font.pixelSize: 15
                                text: qsTr("Automatically installs the recommended driver version with optimal settings")
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        radius: 18
                        color: page.theme.successBg
                        border.width: 1
                        border.color: Qt.tint(page.theme.success, "#55ffffff")
                        implicitHeight: 44

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 18
                            anchors.rightMargin: 18
                            spacing: 10

                            Rectangle {
                                width: 18
                                height: 18
                                radius: 9
                                color: page.theme.success
                            }

                            Label {
                                Layout.fillWidth: true
                                text: "nvidia-" + page.recommendedVersion + " • "
                                      + (nvidiaDetector.gpuFound ? qsTr("Verified Compatible") : page.detectedHardwareLabel)
                                color: page.theme.success
                                font.pixelSize: 14
                                font.weight: Font.Bold
                                elide: Text.ElideRight
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        ActionButton {
                            Layout.preferredWidth: 220
                            theme: page.theme
                            tone: "primary"
                            text: page.driverInstalledLocally ? qsTr("Reinstall Recommended") : qsTr("Install Recommended")
                            enabled: !page.backendBusy
                                     && (!nvidiaInstaller.proprietaryAgreementRequired || eulaAccept.checked)
                                     && (page.remoteDriverCatalogAvailable || page.driverInstalledLocally)
                            onClicked: {
                                page.setOperationState(qsTr("Installer"), qsTr("Installing the proprietary NVIDIA driver (akmod-nvidia)..."), "info", true);
                                nvidiaInstaller.installProprietary(eulaAccept.checked);
                            }
                        }

                        ActionButton {
                            theme: page.theme
                            text: qsTr("Rescan System")
                            enabled: !page.backendBusy
                            onClicked: {
                                nvidiaDetector.refresh();
                                nvidiaInstaller.refreshProprietaryAgreement();
                                nvidiaUpdater.refreshAvailableVersions();
                                nvidiaUpdater.checkForUpdate();
                            }
                        }

                        Item {
                            Layout.fillWidth: true
                        }

                        BusyIndicator {
                            running: page.backendBusy
                            visible: running
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.maximumWidth: 1080
                Layout.alignment: Qt.AlignHCenter
                radius: 26
                color: page.theme.card
                border.width: 1
                border.color: page.theme.border
                implicitHeight: customColumn.implicitHeight + 42

                ColumnLayout {
                    id: customColumn
                    x: 30
                    y: 26
                    width: parent.width - 60
                    spacing: 18

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 18

                        Rectangle {
                            width: 46
                            height: 46
                            radius: 15
                            color: page.theme.accentA

                            Label {
                                anchors.centerIn: parent
                                text: "EX"
                                color: "#ffffff"
                                font.pixelSize: 14
                                font.weight: Font.Bold
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 6

                            Label {
                                text: qsTr("Custom Install")
                                color: page.theme.text
                                font.pixelSize: 24
                                font.weight: Font.Bold
                            }

                            Label {
                                Layout.fillWidth: true
                                wrapMode: Text.Wrap
                                color: page.theme.textSoft
                                font.pixelSize: 15
                                text: qsTr("Advanced options to choose specific driver version and kernel module type")
                            }
                        }
                    }

                    Flow {
                        Layout.fillWidth: true
                        spacing: 8

                        InfoBadge {
                            text: qsTr("Expert Mode")
                            backgroundColor: page.theme.cardStrong
                            foregroundColor: page.theme.textMuted
                        }

                        InfoBadge {
                            text: qsTr("Installed: ") + (page.installedVersionLabel.length > 0 ? page.installedVersionLabel : qsTr("None"))
                            backgroundColor: page.theme.cardStrong
                            foregroundColor: page.theme.textMuted
                        }

                        InfoBadge {
                            text: page.driverInstalledLocally
                                  ? (nvidiaUpdater.updateAvailable ? qsTr("Update Available") : qsTr("Up to Date"))
                                  : (page.remoteDriverCatalogAvailable ? qsTr("Catalog Ready") : qsTr("Catalog Loading"))
                            backgroundColor: page.driverInstalledLocally
                                             ? (nvidiaUpdater.updateAvailable ? page.theme.warningBg : page.theme.successBg)
                                             : page.theme.infoBg
                            foregroundColor: page.theme.text
                        }
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        columns: width > 760 ? 3 : 1
                        columnSpacing: 12
                        rowSpacing: 12

                        ActionButton {
                            Layout.fillWidth: true
                            theme: page.theme
                            text: qsTr("Install Open Modules")
                            enabled: !nvidiaInstaller.busy
                            onClicked: {
                                page.setOperationState(qsTr("Installer"), qsTr("Switching to NVIDIA open kernel modules..."), "info", true);
                                nvidiaInstaller.installOpenSource();
                            }
                        }

                        ActionButton {
                            Layout.fillWidth: true
                            theme: page.theme
                            tone: "primary"
                            text: page.driverInstalledLocally ? qsTr("Apply Latest") : qsTr("Install Latest")
                            enabled: !nvidiaUpdater.busy && !nvidiaInstaller.busy && (nvidiaUpdater.updateAvailable || page.canInstallLatestRemoteDriver)
                            onClicked: {
                                page.setOperationState(qsTr("Updater"), page.driverInstalledLocally
                                                       ? qsTr("Updating NVIDIA driver to the latest online version...")
                                                       : qsTr("Downloading and installing the latest online NVIDIA driver..."),
                                                       "info", true);
                                nvidiaUpdater.applyUpdate();
                            }
                        }

                        ActionButton {
                            Layout.fillWidth: true
                            theme: page.theme
                            tone: "danger"
                            text: qsTr("Remove Driver")
                            enabled: !nvidiaInstaller.busy
                            onClicked: {
                                page.setOperationState(qsTr("Installer"), qsTr("Removing the NVIDIA driver..."), "info", true);
                                nvidiaInstaller.remove();
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12
                        visible: page.availableVersionOptions.length > 0

                        ComboBox {
                            id: versionPicker
                            Layout.fillWidth: true
                            model: page.availableVersionOptions
                            textRole: "versionTitle"
                        }

                        ActionButton {
                            theme: page.theme
                            text: qsTr("Apply Selected")
                            enabled: !nvidiaUpdater.busy && !nvidiaInstaller.busy && versionPicker.currentIndex >= 0 && page.remoteDriverCatalogAvailable
                            onClicked: {
                                page.setOperationState(qsTr("Updater"), page.driverInstalledLocally
                                                       ? qsTr("Switching NVIDIA driver to selected online version: ") + versionPicker.currentText
                                                       : qsTr("Downloading and installing selected NVIDIA driver version: ") + versionPicker.currentText,
                                                       "info", true);
                                nvidiaUpdater.applyVersion(page.availableVersionOptions[versionPicker.currentIndex].rawVersion);
                            }
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.maximumWidth: 1080
                Layout.alignment: Qt.AlignHCenter
                radius: 26
                color: Qt.tint(page.theme.warningBg, page.darkMode ? "#11ffffff" : "#22ffffff")
                border.width: 1
                border.color: Qt.tint(page.theme.warning, "#55ffffff")
                implicitHeight: warningColumn.implicitHeight + 42

                ColumnLayout {
                    id: warningColumn
                    x: 30
                    y: 26
                    width: parent.width - 60
                    spacing: 12

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 16

                        Rectangle {
                            width: 48
                            height: 48
                            radius: 16
                            color: page.theme.warningBg

                            Label {
                                anchors.centerIn: parent
                                text: "!"
                                color: page.theme.warning
                                font.pixelSize: 24
                                font.weight: Font.Bold
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            Label {
                                text: nvidiaDetector.secureBootEnabled ? qsTr("Secure Boot Detected") : qsTr("System Readiness")
                                color: page.theme.warning
                                font.pixelSize: 18
                                font.weight: Font.Bold
                            }

                            Label {
                                Layout.fillWidth: true
                                wrapMode: Text.Wrap
                                color: page.theme.textMuted
                                font.pixelSize: 14
                                text: nvidiaDetector.secureBootEnabled
                                      ? qsTr("You may need to sign the kernel modules or disable Secure Boot in BIOS to use NVIDIA proprietary drivers.")
                                      : qsTr("No Secure Boot blocker is currently detected. You can continue with the recommended installation path.")
                            }
                        }
                    }
                }
            }

            GridLayout {
                Layout.fillWidth: true
                Layout.maximumWidth: 1080
                Layout.alignment: Qt.AlignHCenter
                columns: width > 780 ? 2 : 1
                columnSpacing: 16
                rowSpacing: 16

                SectionPanel {
                    Layout.fillWidth: true
                    theme: page.theme
                    title: qsTr("Hardware Summary")
                    subtitle: qsTr("Live system checks behind the guided install flow.")

                    Flow {
                        Layout.fillWidth: true
                        spacing: 8

                        InfoBadge {
                            text: nvidiaDetector.gpuFound ? qsTr("GPU Ready") : qsTr("GPU Missing")
                            backgroundColor: nvidiaDetector.gpuFound ? page.theme.successBg : page.theme.dangerBg
                            foregroundColor: page.theme.text
                        }

                        InfoBadge {
                            text: nvidiaDetector.driverLoaded ? qsTr("Kernel Module Loaded") : qsTr("Kernel Module Missing")
                            backgroundColor: nvidiaDetector.driverLoaded ? page.theme.successBg : page.theme.warningBg
                            foregroundColor: page.theme.text
                        }

                        InfoBadge {
                            text: nvidiaDetector.waylandSession ? qsTr("Wayland Session") : qsTr("X11 / Other Session")
                            backgroundColor: page.theme.infoBg
                            foregroundColor: page.theme.text
                        }

                        InfoBadge {
                            text: nvidiaDetector.nouveauActive ? qsTr("Fallback Driver Active") : qsTr("Fallback Driver Inactive")
                            backgroundColor: nvidiaDetector.nouveauActive ? page.theme.warningBg : page.theme.cardStrong
                            foregroundColor: page.theme.text
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        wrapMode: Text.Wrap
                        color: page.theme.textSoft
                        text: qsTr("GPU: %1\nActive driver: %2\nInstalled version: %3")
                              .arg(nvidiaDetector.gpuName.length > 0
                                   ? nvidiaDetector.gpuName
                                   : (nvidiaDetector.displayAdapterName.length > 0
                                      ? nvidiaDetector.displayAdapterName
                                      : qsTr("Unavailable")))
                              .arg(nvidiaDetector.activeDriver.length > 0 ? nvidiaDetector.activeDriver : qsTr("Unknown"))
                              .arg(page.installedVersionLabel.length > 0 ? page.installedVersionLabel : qsTr("None"))
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        color: page.theme.cardStrong
                        border.width: 1
                        border.color: page.theme.border
                        radius: 16
                        implicitHeight: verificationText.implicitHeight + 24
                        visible: page.showAdvancedInfo

                        Label {
                            id: verificationText
                            x: 12
                            y: 12
                            width: parent.width - 24
                            text: nvidiaDetector.verificationReport
                            wrapMode: Text.Wrap
                            color: page.theme.text
                        }
                    }
                }

                SectionPanel {
                    Layout.fillWidth: true
                    theme: page.theme
                    title: qsTr("Activity Log")
                    subtitle: qsTr("Installer and updater output is streamed here in real time.")

                    Flow {
                        Layout.fillWidth: true
                        spacing: 8
                        visible: page.operationDetail.length > 0

                        InfoBadge {
                            text: qsTr("Source: ") + (page.operationSource.length > 0 ? page.operationSource : qsTr("Idle"))
                            backgroundColor: page.theme.cardStrong
                            foregroundColor: page.theme.text
                        }

                        InfoBadge {
                            text: qsTr("Phase: ") + (page.operationPhase.length > 0 ? page.operationPhase : qsTr("Idle"))
                            backgroundColor: page.operationRunning ? page.theme.infoBg : page.theme.cardStrong
                            foregroundColor: page.theme.text
                        }

                        InfoBadge {
                            text: qsTr("Elapsed: ") + page.formatDuration(page.operationElapsedSeconds)
                            backgroundColor: page.theme.cardStrong
                            foregroundColor: page.theme.text
                            visible: page.operationRunning || page.operationElapsedSeconds > 0
                        }

                        InfoBadge {
                            text: qsTr("Last Log: ") + page.formatTimestamp(page.lastLogAt)
                            backgroundColor: page.theme.cardStrong
                            foregroundColor: page.theme.text
                            visible: page.lastLogAt > 0
                        }
                    }

                    StatusBanner {
                        Layout.fillWidth: true
                        theme: page.theme
                        tone: "warning"
                        text: nvidiaInstaller.proprietaryAgreementRequired ? nvidiaInstaller.proprietaryAgreementText : ""
                    }

                    CheckBox {
                        id: eulaAccept
                        visible: nvidiaInstaller.proprietaryAgreementRequired
                        text: qsTr("I reviewed the NVIDIA license terms")
                    }

                    Label {
                        Layout.fillWidth: true
                        visible: nvidiaInstaller.proprietaryAgreementRequired
                        textFormat: Text.RichText
                        wrapMode: Text.Wrap
                        color: page.theme.textSoft
                        text: qsTr("Official NVIDIA license: <a href=\"%1\">%1</a>").arg(page.nvidiaLicenseUrl)
                        onLinkActivated: function(link) { Qt.openUrlExternally(link) }
                    }

                    TextArea {
                        id: logArea
                        Layout.fillWidth: true
                        Layout.preferredHeight: 220
                        readOnly: true
                        wrapMode: Text.Wrap
                        color: page.theme.text
                        selectedTextColor: "#ffffff"
                        selectionColor: page.theme.accentA
                        background: Rectangle {
                            radius: 16
                            color: page.theme.cardStrong
                            border.width: 1
                            border.color: page.theme.border
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        ActionButton {
                            theme: page.theme
                            text: qsTr("Check for Updates")
                            enabled: !nvidiaUpdater.busy && !nvidiaInstaller.busy
                            onClicked: {
                                page.setOperationState(qsTr("Updater"), qsTr("Searching the online NVIDIA package catalog..."), "info", true);
                                nvidiaUpdater.checkForUpdate();
                            }
                        }

                        ActionButton {
                            theme: page.theme
                            text: qsTr("Deep Clean")
                            enabled: !nvidiaInstaller.busy
                            onClicked: {
                                page.setOperationState(qsTr("Installer"), qsTr("Cleaning legacy driver leftovers..."), "info", true);
                                nvidiaInstaller.deepClean();
                            }
                        }

                        Item {
                            Layout.fillWidth: true
                        }

                        ActionButton {
                            theme: page.theme
                            text: qsTr("Clear Log")
                            onClicked: {
                                logArea.text = "";
                                page.lastLogAt = 0;
                            }
                        }
                    }
                }
            }
        }
    }

    Connections {
        target: nvidiaInstaller

        function onProgressMessage(message) {
            page.appendLog(qsTr("Installer"), message);
            page.setOperationState(qsTr("Installer"), message, "info", true);
        }

        function onInstallFinished(success, message) {
            page.appendLog(qsTr("Installer"), message);
            page.finishOperation(qsTr("Installer"), success, message);
            nvidiaDetector.refresh();
            nvidiaUpdater.checkForUpdate();
            nvidiaInstaller.refreshProprietaryAgreement();
            eulaAccept.checked = false;
        }

        function onRemoveFinished(success, message) {
            page.appendLog(qsTr("Installer"), message);
            page.finishOperation(qsTr("Installer"), success, message);
            nvidiaDetector.refresh();
            nvidiaUpdater.checkForUpdate();
            nvidiaInstaller.refreshProprietaryAgreement();
        }
    }

    Connections {
        target: nvidiaUpdater

        function onProgressMessage(message) {
            page.appendLog(qsTr("Updater"), message);
            page.setOperationState(qsTr("Updater"), message, "info", true);
        }

        function onUpdateFinished(success, message) {
            page.appendLog(qsTr("Updater"), message);
            page.finishOperation(qsTr("Updater"), success, message);
            nvidiaDetector.refresh();
            nvidiaUpdater.checkForUpdate();
        }
    }

    Component.onCompleted: {
        nvidiaDetector.refresh();
        nvidiaUpdater.checkForUpdate();
        nvidiaUpdater.refreshAvailableVersions();
        nvidiaInstaller.refreshProprietaryAgreement();
    }
}
