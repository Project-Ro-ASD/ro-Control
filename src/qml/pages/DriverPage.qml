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
    property bool operationRunning: nvidiaInstaller.busy || nvidiaUpdater.busy

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
        operationSource = source;
        operationDetail = message;
        operationPhase = classifyOperationPhase(message);
        operationRunning = running;
        bannerText = (operationPhase.length > 0 ? operationPhase + ": " : "") + message;
        bannerTone = tone;
    }

    function finishOperation(source, success, message) {
        setOperationState(source, message, success ? "success" : "error", false);
    }

    readonly property bool remoteDriverCatalogAvailable: nvidiaUpdater.availableVersions.length > 0
    readonly property bool canInstallLatestRemoteDriver: nvidiaDetector.gpuFound && remoteDriverCatalogAvailable
    readonly property bool driverInstalledLocally: nvidiaUpdater.currentVersion.length > 0

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
                    value: nvidiaDetector.driverVersion.length > 0 ? nvidiaDetector.driverVersion : qsTr("None")
                    subtitle: page.driverInstalledLocally
                              ? (nvidiaUpdater.updateAvailable
                                 ? qsTr("Latest available online: ") + nvidiaUpdater.latestVersion
                                 : qsTr("No pending online package update detected."))
                              : (page.remoteDriverCatalogAvailable
                                 ? qsTr("Latest driver found online: ") + nvidiaUpdater.latestVersion
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

                        Button {
                            Layout.fillWidth: true
                            text: qsTr("Install Proprietary")
                            enabled: !nvidiaInstaller.busy && (!nvidiaInstaller.proprietaryAgreementRequired || eulaAccept.checked)
                            onClicked: {
                                page.setOperationState(qsTr("Installer"), qsTr("Installing the proprietary NVIDIA driver (akmod-nvidia)..."), "info", true);
                                nvidiaInstaller.installProprietary(eulaAccept.checked);
                            }
                        }

                        Button {
                            Layout.fillWidth: true
                            text: qsTr("Install Nouveau")
                            enabled: !nvidiaInstaller.busy
                            onClicked: {
                                page.setOperationState(qsTr("Installer"), qsTr("Switching to the open-source driver..."), "info", true);
                                nvidiaInstaller.installOpenSource();
                            }
                        }

                        Button {
                            Layout.fillWidth: true
                            text: qsTr("Remove Driver")
                            enabled: !nvidiaInstaller.busy
                            onClicked: {
                                page.setOperationState(qsTr("Installer"), qsTr("Removing the NVIDIA driver..."), "info", true);
                                nvidiaInstaller.remove();
                            }
                        }

                        Button {
                            Layout.fillWidth: true
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

                        Button {
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
                            text: qsTr("Installed: ") + (nvidiaUpdater.currentVersion.length > 0 ? nvidiaUpdater.currentVersion : qsTr("None"))
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

                        Button {
                            text: qsTr("Check for Updates")
                            enabled: !nvidiaUpdater.busy && !nvidiaInstaller.busy
                            onClicked: {
                                page.setOperationState(qsTr("Updater"), qsTr("Searching the online NVIDIA package catalog..."), "info", true);
                                nvidiaUpdater.checkForUpdate();
                            }
                        }

                        Button {
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
                        visible: nvidiaUpdater.availableVersions.length > 0

                        ComboBox {
                            id: versionPicker
                            Layout.fillWidth: true
                            model: nvidiaUpdater.availableVersions
                        }

                        Button {
                            text: qsTr("Apply Selected")
                            enabled: !nvidiaUpdater.busy && !nvidiaInstaller.busy && versionPicker.currentIndex >= 0 && page.remoteDriverCatalogAvailable
                            onClicked: {
                                page.setOperationState(qsTr("Updater"), page.driverInstalledLocally
                                                       ? qsTr("Switching NVIDIA driver to selected online version: ") + versionPicker.currentText
                                                       : qsTr("Downloading and installing selected NVIDIA driver version: ") + versionPicker.currentText,
                                                       "info", true);
                                nvidiaUpdater.applyVersion(versionPicker.currentText);
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

                    Button {
                        text: qsTr("Clear Log")
                        onClicked: logArea.text = ""
                    }
                }
            }
        }
    }

    Connections {
        target: nvidiaInstaller

        function onProgressMessage(message) {
            logArea.append(message);
            page.setOperationState(qsTr("Installer"), message, "info", true);
        }

        function onInstallFinished(success, message) {
            logArea.append(message);
            page.finishOperation(qsTr("Installer"), success, message);
            nvidiaDetector.refresh();
            nvidiaUpdater.checkForUpdate();
            nvidiaInstaller.refreshProprietaryAgreement();
            eulaAccept.checked = false;
        }

        function onRemoveFinished(success, message) {
            logArea.append(message);
            page.finishOperation(qsTr("Installer"), success, message);
            nvidiaDetector.refresh();
            nvidiaUpdater.checkForUpdate();
            nvidiaInstaller.refreshProprietaryAgreement();
        }
    }

    Connections {
        target: nvidiaUpdater

        function onProgressMessage(message) {
            logArea.append(message);
            page.setOperationState(qsTr("Updater"), message, "info", true);
        }

        function onUpdateFinished(success, message) {
            logArea.append(message);
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
