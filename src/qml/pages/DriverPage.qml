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
    readonly property bool wideLayout: width >= 1240

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

    Timer {
        interval: 1000
        repeat: true
        running: page.operationRunning
        onTriggered: {
            if (page.operationStartedAt > 0)
                page.operationElapsedSeconds = Math.max(0, Math.floor((Date.now() - page.operationStartedAt) / 1000));
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: page.compactMode ? 14 : 16

        StatusBanner {
            Layout.fillWidth: true
            theme: page.theme
            tone: page.bannerTone
            text: page.bannerText
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 6

            Label {
                text: qsTr("Select Installation Type")
                color: page.theme.text
                font.pixelSize: 28
                font.weight: Font.DemiBold
            }

            Label {
                text: qsTr("Optimized for your hardware")
                color: page.theme.textSoft
                font.pixelSize: 15
                font.weight: Font.Medium
            }
        }

        GridLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            columns: page.wideLayout ? 2 : 1
            columnSpacing: 16
            rowSpacing: 16

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: 26
                color: page.theme.card
                border.width: 1
                border.color: page.theme.border
                implicitHeight: expressColumn.implicitHeight + 34
                layer.enabled: true

                ColumnLayout {
                    id: expressColumn
                    x: 24
                    y: 20
                    width: parent.width - 48
                    spacing: 14

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 14

                        Rectangle {
                            width: 42
                            height: 42
                            radius: 14
                            color: page.theme.success

                            Label {
                                anchors.centerIn: parent
                                text: "OK"
                                color: "#ffffff"
                                font.pixelSize: 13
                                font.weight: Font.DemiBold
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            Label {
                                text: qsTr("Express Install")
                                color: page.theme.text
                                font.pixelSize: 20
                                font.weight: Font.DemiBold
                            }

                            Label {
                                Layout.fillWidth: true
                                wrapMode: Text.Wrap
                                color: page.theme.textSoft
                                font.pixelSize: 14
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
                        implicitHeight: 40

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 14
                            anchors.rightMargin: 14
                            spacing: 8

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
                                font.pixelSize: 13
                                font.weight: Font.DemiBold
                                elide: Text.ElideRight
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        ActionButton {
                            Layout.preferredWidth: 210
                            theme: page.theme
                            tone: "primary"
                            text: page.driverInstalledLocally ? qsTr("Reinstall Recommended") : qsTr("Install Recommended")
                            enabled: !page.backendBusy
                                     && (page.remoteDriverCatalogAvailable || page.driverInstalledLocally)
                            onClicked: {
                                page.setOperationState(qsTr("Installer"), qsTr("Installing the proprietary NVIDIA driver (akmod-nvidia)..."), "info", true);
                                nvidiaInstaller.installProprietary(true);
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
                Layout.fillHeight: true
                radius: 26
                color: page.theme.card
                border.width: 1
                border.color: page.theme.border
                implicitHeight: customColumn.implicitHeight + 34

                ColumnLayout {
                    id: customColumn
                    x: 24
                    y: 20
                    width: parent.width - 48
                    spacing: 14

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 14

                        Rectangle {
                            width: 42
                            height: 42
                            radius: 14
                            color: page.theme.accentA

                            Label {
                                anchors.centerIn: parent
                                text: "EX"
                                color: "#ffffff"
                                font.pixelSize: 13
                                font.weight: Font.DemiBold
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            Label {
                                text: qsTr("Custom Install")
                                color: page.theme.text
                                font.pixelSize: 20
                                font.weight: Font.DemiBold
                            }

                            Label {
                                Layout.fillWidth: true
                                wrapMode: Text.Wrap
                                color: page.theme.textSoft
                                font.pixelSize: 14
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
                        columns: 1
                        columnSpacing: 10
                        rowSpacing: 10

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
                Layout.columnSpan: page.wideLayout ? 2 : 1
                radius: 26
                color: Qt.tint(page.theme.warningBg, page.darkMode ? "#11ffffff" : "#22ffffff")
                border.width: 1
                border.color: Qt.tint(page.theme.warning, "#55ffffff")
                implicitHeight: warningColumn.implicitHeight + 30

                ColumnLayout {
                    id: warningColumn
                    x: 24
                    y: 18
                    width: parent.width - 48
                    spacing: 10

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        Rectangle {
                            width: 40
                            height: 40
                            radius: 14
                            color: page.theme.warningBg

                            Label {
                                anchors.centerIn: parent
                                text: "!"
                                color: page.theme.warning
                                font.pixelSize: 20
                                font.weight: Font.DemiBold
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            Label {
                                text: nvidiaDetector.secureBootEnabled ? qsTr("Secure Boot Detected") : qsTr("System Readiness")
                                color: page.theme.warning
                                font.pixelSize: 16
                                font.weight: Font.DemiBold
                            }

                            Label {
                                Layout.fillWidth: true
                                wrapMode: Text.Wrap
                                color: page.theme.textMuted
                                font.pixelSize: 13
                                text: nvidiaDetector.secureBootEnabled
                                      ? qsTr("You may need to sign the kernel modules or disable Secure Boot in BIOS to use NVIDIA proprietary drivers.")
                                      : qsTr("No Secure Boot blocker is currently detected. You can continue with the recommended installation path.")
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
            page.setOperationState(qsTr("Installer"), message, "info", true);
        }

        function onInstallFinished(success, message) {
            page.finishOperation(qsTr("Installer"), success, message);
            nvidiaDetector.refresh();
            nvidiaUpdater.checkForUpdate();
            nvidiaInstaller.refreshProprietaryAgreement();
        }

        function onRemoveFinished(success, message) {
            page.finishOperation(qsTr("Installer"), success, message);
            nvidiaDetector.refresh();
            nvidiaUpdater.checkForUpdate();
            nvidiaInstaller.refreshProprietaryAgreement();
        }
    }

    Connections {
        target: nvidiaUpdater

        function onProgressMessage(message) {
            page.setOperationState(qsTr("Updater"), message, "info", true);
        }

        function onUpdateFinished(success, message) {
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
