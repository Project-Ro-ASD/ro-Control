import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: page
    required property var nvidiaDetector
    required property var nvidiaInstaller
    required property var nvidiaUpdater

    function appendLog(message) {
        const maxLines = 200
        const nextText = (logArea.text.length > 0 ? logArea.text + "\n" : "") + message
        const lines = nextText.split("\n")
        logArea.text = lines.length > maxLines ? lines.slice(lines.length - maxLines).join("\n") : nextText
        logArea.cursorPosition = logArea.text.length
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 12

        Label {
            text: qsTr("Driver Management")
            font.pixelSize: 24
            font.bold: true
        }

        Label {
            text: qsTr("GPU: ") + (page.nvidiaDetector.gpuFound ? page.nvidiaDetector.gpuName : qsTr("Not detected"))
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }

        Label {
            text: qsTr("Active driver: ") + page.nvidiaDetector.activeDriver
            wrapMode: Text.Wrap
        }

        Label {
            text: qsTr("Driver version: ") + (page.nvidiaDetector.driverVersion.length > 0 ? page.nvidiaDetector.driverVersion : qsTr("None"))
        }

        Label {
            text: qsTr("Secure Boot: ") + (page.nvidiaDetector.secureBootKnown ? (page.nvidiaDetector.secureBootEnabled ? qsTr("Enabled") : qsTr("Disabled")) : qsTr("Unknown"))
            color: !page.nvidiaDetector.secureBootKnown ? "#8a6500" : (page.nvidiaDetector.secureBootEnabled ? "#c43a3a" : "#2b8a3e")
            font.bold: true
        }

        Label {
            text: qsTr("Session type: ") + page.nvidiaDetector.sessionType
            font.bold: true
        }

        Label {
            text: page.nvidiaDetector.waylandSession ? qsTr("For Wayland, the nvidia-drm.modeset=1 parameter is applied automatically.") : qsTr("For X11, the xorg-x11-drv-nvidia package is checked and installed.")
            wrapMode: Text.Wrap
            Layout.fillWidth: true
            color: "#6d7384"
        }

        Rectangle {
            Layout.fillWidth: true
            border.width: 1
            border.color: "#5f6b86"
            color: "transparent"
            radius: 8
            implicitHeight: verificationText.implicitHeight + 20

            Label {
                id: verificationText
                anchors.fill: parent
                anchors.margins: 10
                text: page.nvidiaDetector.verificationReport
                wrapMode: Text.Wrap
            }
        }

        Label {
            visible: page.nvidiaInstaller.proprietaryAgreementRequired
            text: page.nvidiaInstaller.proprietaryAgreementText
            color: "#8a6500"
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }

        CheckBox {
            id: eulaAccept
            visible: page.nvidiaInstaller.proprietaryAgreementRequired
            text: qsTr("I accept the license/agreement terms")
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Button {
                text: qsTr("Install Proprietary Driver")
                enabled: (!page.nvidiaInstaller.proprietaryAgreementRequired || eulaAccept.checked) && !page.nvidiaInstaller.busy && !page.nvidiaUpdater.busy
                onClicked: page.nvidiaInstaller.installProprietary(eulaAccept.checked)
            }

            Button {
                text: qsTr("Install Nouveau Driver")
                enabled: !page.nvidiaInstaller.busy && !page.nvidiaUpdater.busy
                onClicked: {
                    page.appendLog(qsTr("Nouveau driver installation started..."));
                    page.nvidiaInstaller.installOpenSource();
                }
            }

            Button {
                text: qsTr("Deep Clean")
                enabled: !page.nvidiaInstaller.busy && !page.nvidiaUpdater.busy
                onClicked: page.nvidiaInstaller.deepClean()
            }
        }

        RowLayout {
            spacing: 8

            // TR: Manuel kontrol butonu, sonucu log alanina yazar.
            // EN: Manual check button writes status into the on-screen log.
            Button {
                text: qsTr("Check for Updates")
                enabled: !page.nvidiaInstaller.busy && !page.nvidiaUpdater.busy
                onClicked: {
                    page.appendLog(qsTr("Update check requested..."));
                    page.nvidiaUpdater.checkForUpdate();
                }
            }

            Button {
                text: qsTr("Apply Latest Update")
                enabled: page.nvidiaUpdater.updateAvailable && !page.nvidiaInstaller.busy && !page.nvidiaUpdater.busy
                onClicked: page.nvidiaUpdater.applyUpdate()
            }

            Label {
                visible: page.nvidiaUpdater.updateAvailable
                text: qsTr("New version: ") + page.nvidiaUpdater.latestVersion
                color: "#8a6500"
            }
        }

        Rectangle {
            Layout.fillWidth: true
            border.width: 1
            border.color: "#5f6b86"
            color: "transparent"
            radius: 8
            implicitHeight: versionColumn.implicitHeight + 20

            ColumnLayout {
                id: versionColumn
                anchors.fill: parent
                anchors.margins: 10
                spacing: 8

                Label {
                    text: qsTr("Apply Specific Version")
                    font.bold: true
                }

                Label {
                    text: page.nvidiaUpdater.availableVersions.length > 0
                          ? qsTr("Repository versions were listed. The selected version can be installed or synced.")
                          : qsTr("Repository version list is not loaded yet or no version was found.")
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                    color: "#6d7384"
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    ComboBox {
                        id: versionSelector
                        Layout.fillWidth: true
                        model: page.nvidiaUpdater.availableVersions
                        enabled: model.length > 0 && !page.nvidiaInstaller.busy && !page.nvidiaUpdater.busy
                    }

                    Button {
                        text: qsTr("Refresh Versions")
                        enabled: !page.nvidiaInstaller.busy && !page.nvidiaUpdater.busy
                        onClicked: {
                            page.appendLog(qsTr("Refreshing repository version list..."));
                            page.nvidiaUpdater.refreshAvailableVersions();
                        }
                    }

                    Button {
                        text: qsTr("Apply Selected Version")
                        enabled: versionSelector.currentIndex >= 0 && page.nvidiaUpdater.availableVersions.length > 0 && !page.nvidiaInstaller.busy && !page.nvidiaUpdater.busy
                        onClicked: {
                            const selectedVersion = versionSelector.currentText;
                            page.appendLog(qsTr("Applying selected version: ") + selectedVersion);
                            page.nvidiaUpdater.applyVersion(selectedVersion);
                        }
                    }
                }
            }
        }

        RowLayout {
            spacing: 8

            // TR: Yeniden Tara; detector + lisans durumu + update kontrolunu tazeler.
            // EN: Rescan refreshes detector state, agreement state, and update check.
            Button {
                text: qsTr("Rescan")
                enabled: !page.nvidiaInstaller.busy && !page.nvidiaUpdater.busy
                onClicked: {
                    page.appendLog(qsTr("Rescanning system..."));
                    page.nvidiaDetector.refresh();
                    page.nvidiaInstaller.refreshProprietaryAgreement();
                    page.nvidiaUpdater.checkForUpdate();
                    page.appendLog(qsTr("Rescan completed."));
                }
            }

            Label {
                text: qsTr("Installed NVIDIA version: ") + page.nvidiaUpdater.currentVersion
                visible: page.nvidiaUpdater.currentVersion.length > 0
            }
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            TextArea {
                id: logArea
                readOnly: true
                wrapMode: Text.Wrap
                text: ""
            }
        }
    }

    Connections {
        target: page.nvidiaInstaller
        function onProgressMessage(message) {
            page.appendLog(message);
        }
        function onInstallFinished(success, message) {
            page.appendLog(message);
            page.nvidiaDetector.refresh();
            page.nvidiaUpdater.checkForUpdate();
            page.nvidiaInstaller.refreshProprietaryAgreement();
        }
        function onRemoveFinished(success, message) {
            page.appendLog(message);
            page.nvidiaDetector.refresh();
            page.nvidiaInstaller.refreshProprietaryAgreement();
        }
    }

    Connections {
        target: page.nvidiaUpdater
        // TR: Updater backend mesajlarini canli log olarak UI'ye aktar.
        // EN: Stream updater backend messages into the live UI log.
        function onProgressMessage(message) {
            page.appendLog(message);
        }
        function onUpdateFinished(success, message) {
            page.appendLog(message);
            page.nvidiaDetector.refresh();
            page.nvidiaUpdater.checkForUpdate();
        }
    }

    Component.onCompleted: {
        page.nvidiaDetector.refresh();
        page.nvidiaUpdater.checkForUpdate();
        page.nvidiaInstaller.refreshProprietaryAgreement();
    }
}
