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
    readonly property bool backendBusy: nvidiaInstaller.busy || nvidiaUpdater.busy
    readonly property bool operationRunning: page.operationActive || page.backendBusy

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

    readonly property bool remoteDriverCatalogAvailable: nvidiaUpdater.availableVersions.length > 0
    readonly property bool canInstallLatestRemoteDriver: nvidiaDetector.gpuFound && remoteDriverCatalogAvailable
    readonly property bool driverInstalledLocally: nvidiaDetector.driverVersion.length > 0 || nvidiaUpdater.currentVersion.length > 0
    readonly property string installedVersionLabel: nvidiaDetector.driverVersion.length > 0 ? nvidiaDetector.driverVersion : nvidiaUpdater.currentVersion
    readonly property string latestVersionLabel: cleanVersionLabel(nvidiaUpdater.latestVersion)
    readonly property var availableVersionOptions: buildAvailableVersionOptions(nvidiaUpdater.availableVersions)

    ScrollView {
        id: pageScroll
        anchors.fill: parent
        clip: true
        contentWidth: availableWidth

        ColumnLayout {
            width: pageScroll.availableWidth
            spacing: page.compactMode ? 12 : 16

            StatusBanner {
                Layout.fillWidth: true
                theme: page.theme
                tone: page.bannerTone
                text: page.bannerText
            }

            GridLayout {
                Layout.fillWidth: true
                columns: width > 980 ? 4 : 2
                columnSpacing: 12
                rowSpacing: 12

                StatCard {
                    Layout.fillWidth: true
                    theme: page.theme
                    title: qsTr("GPU Detection")
                    value: nvidiaDetector.gpuFound ? qsTr("Detected") : qsTr("Missing")
                    subtitle: nvidiaDetector.gpuFound ? nvidiaDetector.gpuName : qsTr("No NVIDIA GPU was detected on this system.")
                    accentColor: page.theme.accentA
                    emphasized: nvidiaDetector.gpuFound
                }

                StatCard {
                    Layout.fillWidth: true
                    theme: page.theme
                    title: qsTr("Active Driver")
                    value: nvidiaDetector.activeDriver
                    subtitle: qsTr("Session: ") + (nvidiaDetector.sessionType.length > 0 ? nvidiaDetector.sessionType : qsTr("Unknown"))
                    accentColor: page.theme.accentB
                }

                StatCard {
                    Layout.fillWidth: true
                    theme: page.theme
                    title: qsTr("Installed Version")
                    value: page.installedVersionLabel.length > 0 ? page.installedVersionLabel : qsTr("None")
                    subtitle: page.driverInstalledLocally
                              ? (nvidiaUpdater.updateAvailable
                                 ? qsTr("Latest available online: ") + page.latestVersionLabel
                                 : qsTr("No pending online package update detected."))
                              : (page.remoteDriverCatalogAvailable
                                 ? qsTr("Latest driver found online: ") + page.latestVersionLabel
                                 : qsTr("No online driver catalog has been loaded yet."))
                    accentColor: page.theme.accentC
                    busy: page.operationRunning
                }

                StatCard {
                    Layout.fillWidth: true
                    theme: page.theme
                    title: qsTr("Secure Boot")
                    value: nvidiaDetector.secureBootEnabled ? qsTr("Enabled") : qsTr("Disabled / Unknown")
                    subtitle: nvidiaDetector.secureBootEnabled
                              ? qsTr("Unsigned kernel modules may require manual signing.")
                              : qsTr("No Secure Boot blocker is currently detected.")
                    accentColor: nvidiaDetector.secureBootEnabled ? page.theme.warning : page.theme.success
                    emphasized: nvidiaDetector.secureBootEnabled
                }
            }

            GridLayout {
                Layout.fillWidth: true
                columns: width > 980 ? 2 : 1
                columnSpacing: 16
                rowSpacing: 16

                SectionPanel {
                    Layout.fillWidth: true
                    theme: page.theme
                    title: qsTr("Verification")
                    subtitle: qsTr("Review driver prerequisites before changing packages.")

                    Flow {
                        Layout.fillWidth: true
                        spacing: 8

                        InfoBadge {
                            text: nvidiaDetector.gpuFound ? qsTr("GPU Ready") : qsTr("GPU Missing")
                            backgroundColor: nvidiaDetector.gpuFound ? page.theme.successBg : page.theme.dangerBg
                            foregroundColor: page.theme.text
                        }

                        InfoBadge {
                            text: nvidiaDetector.waylandSession ? qsTr("Wayland Session") : qsTr("X11 / Other Session")
                            backgroundColor: page.theme.infoBg
                            foregroundColor: page.theme.text
                        }

                        InfoBadge {
                            text: nvidiaDetector.nouveauActive ? qsTr("Nouveau Active") : qsTr("Nouveau Inactive")
                            backgroundColor: nvidiaDetector.nouveauActive ? page.theme.warningBg : page.theme.cardStrong
                            foregroundColor: page.theme.text
                        }

                        InfoBadge {
                            text: nvidiaDetector.driverLoaded ? qsTr("Kernel Module Loaded") : qsTr("Kernel Module Missing")
                            backgroundColor: nvidiaDetector.driverLoaded ? page.theme.successBg : page.theme.warningBg
                            foregroundColor: page.theme.text
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        wrapMode: Text.Wrap
                        color: page.theme.textSoft
                        text: nvidiaDetector.waylandSession
                              ? qsTr("Wayland sessions automatically need the nvidia-drm.modeset=1 kernel argument.")
                              : qsTr("X11 sessions require matching userspace packages after install or update.")
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
                    title: qsTr("Driver Actions")
                    subtitle: qsTr("Use guided actions to install, switch or remove the current stack.")

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
                            text: page.operationRunning ? qsTr("Running") : qsTr("Idle")
                            backgroundColor: page.operationRunning ? page.theme.warningBg : page.theme.successBg
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
                        text: qsTr("I accept the detected license / agreement terms")
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        columns: width > 460 ? 2 : 1
                        columnSpacing: 10
                        rowSpacing: 10

                        ActionButton {
                            Layout.fillWidth: true
                            theme: page.theme
                            tone: "primary"
                            text: qsTr("Install Proprietary")
                            enabled: !nvidiaInstaller.busy && (!nvidiaInstaller.proprietaryAgreementRequired || eulaAccept.checked)
                            onClicked: {
                                page.setOperationState(qsTr("Installer"), qsTr("Installing the proprietary NVIDIA driver (akmod-nvidia)..."), "info", true);
                                nvidiaInstaller.installProprietary(eulaAccept.checked);
                            }
                        }

                        ActionButton {
                            Layout.fillWidth: true
                            theme: page.theme
                            text: qsTr("Install Nouveau")
                            enabled: !nvidiaInstaller.busy
                            onClicked: {
                                page.setOperationState(qsTr("Installer"), qsTr("Switching to the open-source driver..."), "info", true);
                                nvidiaInstaller.installOpenSource();
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

                        ActionButton {
                            Layout.fillWidth: true
                            theme: page.theme
                            text: qsTr("Deep Clean")
                            enabled: !nvidiaInstaller.busy
                            onClicked: {
                                page.setOperationState(qsTr("Installer"), qsTr("Cleaning legacy driver leftovers..."), "info", true);
                                nvidiaInstaller.deepClean();
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        ActionButton {
                            theme: page.theme
                            text: qsTr("Rescan System")
                            enabled: !nvidiaInstaller.busy && !nvidiaUpdater.busy
                            onClicked: {
                                nvidiaDetector.refresh();
                                nvidiaInstaller.refreshProprietaryAgreement();
                                nvidiaUpdater.refreshAvailableVersions();
                            }
                        }

                        Item {
                            Layout.fillWidth: true
                        }

                        BusyIndicator {
                            running: nvidiaInstaller.busy || nvidiaUpdater.busy
                            visible: running
                        }
                    }
                }

                SectionPanel {
                    Layout.fillWidth: true
                    theme: page.theme
                    title: qsTr("Update Center")
                    subtitle: qsTr("Search the online package catalog, then download and install a matching driver build.")

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        InfoBadge {
                            text: qsTr("Installed: ") + (page.installedVersionLabel.length > 0 ? page.installedVersionLabel : qsTr("None"))
                            backgroundColor: page.theme.cardStrong
                            foregroundColor: page.theme.text
                        }

                        InfoBadge {
                            text: page.driverInstalledLocally
                                  ? (nvidiaUpdater.updateAvailable ? qsTr("Update Available") : qsTr("Up to Date"))
                                  : (page.remoteDriverCatalogAvailable ? qsTr("Remote Driver Available") : qsTr("Catalog Not Ready"))
                            backgroundColor: page.driverInstalledLocally
                                             ? (nvidiaUpdater.updateAvailable ? page.theme.warningBg : page.theme.successBg)
                                             : (page.remoteDriverCatalogAvailable ? page.theme.successBg : page.theme.warningBg)
                            foregroundColor: page.theme.text
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10

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
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10
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

                    Label {
                        Layout.fillWidth: true
                        wrapMode: Text.Wrap
                        color: page.theme.textSoft
                        text: nvidiaUpdater.availableVersions.length > 0
                              ? qsTr("Online repository versions loaded: ") + nvidiaUpdater.availableVersions.length
                              : qsTr("No online repository version list has been loaded yet.")
                    }
                }

                SectionPanel {
                    Layout.fillWidth: true
                    theme: page.theme
                    title: qsTr("Activity Log")
                    subtitle: qsTr("Operation output is streamed here in real time.")

                    TextArea {
                        id: logArea
                        Layout.fillWidth: true
                        Layout.preferredHeight: 240
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

                    ActionButton {
                        theme: page.theme
                        text: qsTr("Clear Log")
                        onClicked: {
                            logArea.text = ""
                            page.lastLogAt = 0
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
