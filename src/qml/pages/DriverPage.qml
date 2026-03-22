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

    ScrollView {
        anchors.fill: parent
        clip: true
        contentWidth: availableWidth

        ColumnLayout {
            width: parent.availableWidth
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
                    subtitle: nvidiaUpdater.updateAvailable ? qsTr("Latest available: ") + nvidiaUpdater.latestVersion : qsTr("No pending package update detected.")
                    accentColor: page.theme.accentC
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
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.margins: 12
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
                            onClicked: nvidiaInstaller.installProprietary(eulaAccept.checked)
                        }

                        Button {
                            Layout.fillWidth: true
                            text: qsTr("Install Nouveau")
                            enabled: !nvidiaInstaller.busy
                            onClicked: nvidiaInstaller.installOpenSource()
                        }

                        Button {
                            Layout.fillWidth: true
                            text: qsTr("Remove Driver")
                            enabled: !nvidiaInstaller.busy
                            onClicked: nvidiaInstaller.remove()
                        }

                        Button {
                            Layout.fillWidth: true
                            text: qsTr("Deep Clean")
                            enabled: !nvidiaInstaller.busy
                            onClicked: nvidiaInstaller.deepClean()
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
                    subtitle: qsTr("Check the repository version and pin a specific build when required.")

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        InfoBadge {
                            text: qsTr("Installed: ") + (nvidiaUpdater.currentVersion.length > 0 ? nvidiaUpdater.currentVersion : qsTr("None"))
                            backgroundColor: page.theme.cardStrong
                            foregroundColor: page.theme.text
                        }

                        InfoBadge {
                            text: nvidiaUpdater.updateAvailable ? qsTr("Update Available") : qsTr("Up to Date")
                            backgroundColor: nvidiaUpdater.updateAvailable ? page.theme.warningBg : page.theme.successBg
                            foregroundColor: page.theme.text
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10

                        Button {
                            text: qsTr("Check for Updates")
                            enabled: !nvidiaUpdater.busy && !nvidiaInstaller.busy
                            onClicked: nvidiaUpdater.checkForUpdate()
                        }

                        Button {
                            text: qsTr("Apply Latest")
                            enabled: !nvidiaUpdater.busy && !nvidiaInstaller.busy && nvidiaUpdater.updateAvailable
                            onClicked: nvidiaUpdater.applyUpdate()
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
                            enabled: !nvidiaUpdater.busy && !nvidiaInstaller.busy && versionPicker.currentIndex >= 0
                            onClicked: nvidiaUpdater.applyVersion(versionPicker.currentText)
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        wrapMode: Text.Wrap
                        color: page.theme.textSoft
                        text: nvidiaUpdater.availableVersions.length > 0
                              ? qsTr("Repository versions loaded: ") + nvidiaUpdater.availableVersions.length
                              : qsTr("No repository version list has been loaded yet.")
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
            page.bannerText = message;
            page.bannerTone = "info";
        }

        function onInstallFinished(success, message) {
            logArea.append(message);
            page.bannerText = message;
            page.bannerTone = success ? "success" : "error";
            nvidiaDetector.refresh();
            nvidiaUpdater.checkForUpdate();
            nvidiaInstaller.refreshProprietaryAgreement();
            eulaAccept.checked = false;
        }

        function onRemoveFinished(success, message) {
            logArea.append(message);
            page.bannerText = message;
            page.bannerTone = success ? "success" : "error";
            nvidiaDetector.refresh();
            nvidiaUpdater.checkForUpdate();
            nvidiaInstaller.refreshProprietaryAgreement();
        }
    }

    Connections {
        target: nvidiaUpdater

        function onProgressMessage(message) {
            logArea.append(message);
            page.bannerText = message;
            page.bannerTone = "info";
        }

        function onUpdateFinished(success, message) {
            logArea.append(message);
            page.bannerText = message;
            page.bannerTone = success ? "success" : "error";
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
