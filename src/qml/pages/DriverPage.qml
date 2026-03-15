import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../components"

Item {
    id: page
    required property var nvidiaDetector
    required property var nvidiaInstaller
    required property var nvidiaUpdater
    required property var theme
    property bool darkMode: true
    readonly property bool operationsBusy: nvidiaInstaller.busy || nvidiaUpdater.busy

    function appendLog(message) {
        const maxLines = 180
        const nextText = (logArea.text.length > 0 ? logArea.text + "\n" : "") + message
        const lines = nextText.split("\n")
        logArea.text = lines.length > maxLines ? lines.slice(lines.length - maxLines).join("\n") : nextText
        logArea.cursorPosition = logArea.text.length
    }

    ScrollView {
        anchors.fill: parent
        clip: true

        ColumnLayout {
            width: parent.width
            spacing: 18

            Rectangle {
                Layout.fillWidth: true
                radius: 28
                color: Qt.tint(page.theme.panel, page.darkMode ? "#22ff8a3d" : "#14f47b20")
                border.width: 1
                border.color: page.theme.border
                implicitHeight: heroLayout.implicitHeight + 28

                ColumnLayout {
                    id: heroLayout
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 16

                    Label {
                        text: qsTr("NVIDIA Driver Workspace")
                        font.pixelSize: 30
                        font.bold: true
                        color: page.theme.text
                    }

                    Label {
                        text: qsTr("Manage detection, installation, updates and version pinning from one place. The flow is tuned for Fedora systems that only need NVIDIA driver stack handling.")
                        wrapMode: Text.Wrap
                        Layout.fillWidth: true
                        color: page.theme.textMuted
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 14

                        StatCard {
                            Layout.fillWidth: true
                            theme: page.theme
                            title: qsTr("GPU State")
                            value: page.nvidiaDetector.gpuFound ? page.nvidiaDetector.gpuName : qsTr("Not Detected")
                            subtitle: qsTr("Active driver: %1").arg(page.nvidiaDetector.activeDriver)
                            accentColor: page.theme.accentB
                            emphasized: true
                        }

                        StatCard {
                            Layout.fillWidth: true
                            theme: page.theme
                            title: qsTr("Installed Version")
                            value: page.nvidiaUpdater.currentVersion.length > 0 ? page.nvidiaUpdater.currentVersion : qsTr("None")
                            subtitle: qsTr("Latest repo version: %1").arg(page.nvidiaUpdater.latestVersion.length > 0 ? page.nvidiaUpdater.latestVersion : qsTr("Unknown"))
                            accentColor: page.theme.accentA
                            busy: page.nvidiaUpdater.busy
                        }

                        StatCard {
                            Layout.fillWidth: true
                            theme: page.theme
                            title: qsTr("Session")
                            value: page.nvidiaDetector.sessionType.length > 0 ? page.nvidiaDetector.sessionType : qsTr("Unknown")
                            subtitle: page.nvidiaDetector.secureBootKnown
                                      ? qsTr("Secure Boot: %1").arg(page.nvidiaDetector.secureBootEnabled ? qsTr("Enabled") : qsTr("Disabled"))
                                      : qsTr("Secure Boot state could not be detected")
                            accentColor: page.nvidiaDetector.secureBootEnabled ? page.theme.danger : page.theme.success
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 18

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    radius: 26
                    color: page.theme.card
                    border.width: 1
                    border.color: page.theme.border
                    implicitHeight: installColumn.implicitHeight + 26

                    ColumnLayout {
                        id: installColumn
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 12

                        Label {
                            text: qsTr("Install & Recovery")
                            font.pixelSize: 21
                            font.bold: true
                            color: page.theme.text
                        }

                        Label {
                            text: page.nvidiaDetector.waylandSession
                                  ? qsTr("Wayland is active. The workflow will also enforce nvidia-drm.modeset=1 when required.")
                                  : qsTr("X11 is active. The workflow verifies X11 driver components together with the core NVIDIA stack.")
                            wrapMode: Text.Wrap
                            Layout.fillWidth: true
                            color: page.theme.textMuted
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            radius: 18
                            color: page.theme.cardMuted
                            border.width: 1
                            border.color: page.theme.border
                            visible: page.nvidiaInstaller.proprietaryAgreementRequired
                            implicitHeight: agreementColumn.implicitHeight + 18

                            ColumnLayout {
                                id: agreementColumn
                                anchors.fill: parent
                                anchors.margins: 14
                                spacing: 10

                                Label {
                                    text: page.nvidiaInstaller.proprietaryAgreementText
                                    wrapMode: Text.Wrap
                                    Layout.fillWidth: true
                                    color: page.theme.warning
                                }

                                CheckBox {
                                    id: eulaAccept
                                    text: qsTr("I accept the license/agreement terms")
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 10

                            Button {
                                Layout.fillWidth: true
                                text: qsTr("Install Proprietary")
                                enabled: (!page.nvidiaInstaller.proprietaryAgreementRequired || eulaAccept.checked) && !page.operationsBusy
                                onClicked: page.nvidiaInstaller.installProprietary(eulaAccept.checked)
                            }

                            Button {
                                Layout.fillWidth: true
                                text: qsTr("Install Nouveau")
                                enabled: !page.operationsBusy
                                onClicked: {
                                    page.appendLog(qsTr("Nouveau driver installation started..."))
                                    page.nvidiaInstaller.installOpenSource()
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 10

                            Button {
                                Layout.fillWidth: true
                                text: qsTr("Deep Clean")
                                enabled: !page.operationsBusy
                                onClicked: page.nvidiaInstaller.deepClean()
                            }

                            Button {
                                Layout.fillWidth: true
                                text: qsTr("Rescan")
                                enabled: !page.operationsBusy
                                onClicked: {
                                    page.appendLog(qsTr("Rescanning system..."))
                                    page.nvidiaDetector.refresh()
                                    page.nvidiaInstaller.refreshProprietaryAgreement()
                                    page.nvidiaUpdater.checkForUpdate()
                                    page.appendLog(qsTr("Rescan completed."))
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    radius: 26
                    color: page.theme.card
                    border.width: 1
                    border.color: page.theme.border
                    implicitHeight: updateColumn.implicitHeight + 26

                    ColumnLayout {
                        id: updateColumn
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 12

                        Label {
                            text: qsTr("Updates & Version Pinning")
                            font.pixelSize: 21
                            font.bold: true
                            color: page.theme.text
                        }

                        Label {
                            text: page.nvidiaUpdater.updateAvailable
                                  ? qsTr("A newer repository version is available. You can apply the latest package set or lock the system to a specific version.")
                                  : qsTr("The installed version is current, or no newer repository version has been found yet.")
                            wrapMode: Text.Wrap
                            Layout.fillWidth: true
                            color: page.theme.textMuted
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 10

                            Button {
                                Layout.fillWidth: true
                                text: qsTr("Check for Updates")
                                enabled: !page.operationsBusy
                                onClicked: {
                                    page.appendLog(qsTr("Update check requested..."))
                                    page.nvidiaUpdater.checkForUpdate()
                                }
                            }

                            Button {
                                Layout.fillWidth: true
                                text: qsTr("Apply Latest")
                                enabled: page.nvidiaUpdater.updateAvailable && !page.operationsBusy
                                onClicked: page.nvidiaUpdater.applyUpdate()
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            radius: 18
                            color: page.theme.cardMuted
                            implicitHeight: 52

                            Label {
                                anchors.fill: parent
                                anchors.margins: 14
                                verticalAlignment: Text.AlignVCenter
                                text: qsTr("Latest repo version: %1").arg(page.nvidiaUpdater.latestVersion.length > 0 ? page.nvidiaUpdater.latestVersion : qsTr("Unknown"))
                                color: page.nvidiaUpdater.updateAvailable ? page.theme.warning : page.theme.textMuted
                                font.bold: page.nvidiaUpdater.updateAvailable
                            }
                        }

                        Label {
                            text: qsTr("Available Versions")
                            font.bold: true
                            color: page.theme.text
                        }

                        ComboBox {
                            id: versionSelector
                            Layout.fillWidth: true
                            model: page.nvidiaUpdater.availableVersions
                            enabled: model.length > 0 && !page.operationsBusy
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 10

                            Button {
                                Layout.fillWidth: true
                                text: qsTr("Refresh Versions")
                                enabled: !page.operationsBusy
                                onClicked: {
                                    page.appendLog(qsTr("Refreshing repository version list..."))
                                    page.nvidiaUpdater.refreshAvailableVersions()
                                }
                            }

                            Button {
                                Layout.fillWidth: true
                                text: qsTr("Apply Selected")
                                enabled: versionSelector.currentIndex >= 0 && page.nvidiaUpdater.availableVersions.length > 0 && !page.operationsBusy
                                onClicked: {
                                    const selectedVersion = versionSelector.currentText
                                    page.appendLog(qsTr("Applying selected version: %1").arg(selectedVersion))
                                    page.nvidiaUpdater.applyVersion(selectedVersion)
                                }
                            }
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 18

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    radius: 26
                    color: page.theme.card
                    border.width: 1
                    border.color: page.theme.border
                    implicitHeight: verificationColumn.implicitHeight + 26

                    ColumnLayout {
                        id: verificationColumn
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 12

                        Label {
                            text: qsTr("Environment Verification")
                            font.pixelSize: 21
                            font.bold: true
                            color: page.theme.text
                        }

                        Label {
                            text: qsTr("Use this report to confirm that Fedora session state, Secure Boot information and package checks are aligned before changing drivers.")
                            wrapMode: Text.Wrap
                            Layout.fillWidth: true
                            color: page.theme.textMuted
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            radius: 18
                            color: page.theme.cardMuted
                            border.width: 1
                            border.color: page.theme.border
                            implicitHeight: verificationText.implicitHeight + 24

                            Label {
                                id: verificationText
                                anchors.fill: parent
                                anchors.margins: 14
                                wrapMode: Text.Wrap
                                text: page.nvidiaDetector.verificationReport
                                color: page.theme.textMuted
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    radius: 26
                    color: page.theme.card
                    border.width: 1
                    border.color: page.theme.border
                    implicitHeight: logColumn.implicitHeight + 26

                    ColumnLayout {
                        id: logColumn
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 12

                        RowLayout {
                            Layout.fillWidth: true

                            Label {
                                text: qsTr("Operation Log")
                                font.pixelSize: 21
                                font.bold: true
                                color: page.theme.text
                            }

                            Item {
                                Layout.fillWidth: true
                            }

                            Label {
                                text: page.operationsBusy ? qsTr("Running") : qsTr("Idle")
                                color: page.operationsBusy ? page.theme.warning : page.theme.success
                                font.bold: true
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            Layout.minimumHeight: 250
                            radius: 18
                            color: page.darkMode ? "#0d1523" : "#edf4ff"
                            border.width: 1
                            border.color: page.theme.border

                            ScrollView {
                                anchors.fill: parent
                                anchors.margins: 1
                                clip: true

                                TextArea {
                                    id: logArea
                                    readOnly: true
                                    wrapMode: Text.Wrap
                                    color: page.theme.text
                                    selectionColor: page.theme.accentB
                                    selectedTextColor: "#ffffff"
                                    background: null
                                    text: ""
                                    placeholderText: qsTr("Driver actions and backend progress messages will appear here.")
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Connections {
        target: page.nvidiaInstaller
        function onProgressMessage(message) {
            page.appendLog(message)
        }
        function onInstallFinished(success, message) {
            page.appendLog(message)
            page.nvidiaDetector.refresh()
            page.nvidiaUpdater.checkForUpdate()
            page.nvidiaInstaller.refreshProprietaryAgreement()
        }
        function onRemoveFinished(success, message) {
            page.appendLog(message)
            page.nvidiaDetector.refresh()
            page.nvidiaInstaller.refreshProprietaryAgreement()
        }
    }

    Connections {
        target: page.nvidiaUpdater
        function onProgressMessage(message) {
            page.appendLog(message)
        }
        function onUpdateFinished(success, message) {
            page.appendLog(message)
            page.nvidiaDetector.refresh()
            page.nvidiaUpdater.checkForUpdate()
        }
    }

    Component.onCompleted: {
        page.nvidiaDetector.refresh()
        page.nvidiaUpdater.checkForUpdate()
        page.nvidiaInstaller.refreshProprietaryAgreement()
    }
}
