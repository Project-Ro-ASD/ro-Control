import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: page

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
            text: qsTr("GPU: ") + (nvidiaDetector.gpuFound ? nvidiaDetector.gpuName : qsTr("Not detected"))
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }

        Label {
            text: qsTr("Active driver: ") + nvidiaDetector.activeDriver
            wrapMode: Text.Wrap
        }

        Label {
            text: qsTr("Driver version: ") + (nvidiaDetector.driverVersion.length > 0 ? nvidiaDetector.driverVersion : qsTr("None"))
        }

        Label {
            text: qsTr("Secure Boot: ") + (nvidiaDetector.secureBootEnabled ? qsTr("Enabled") : qsTr("Disabled / Unknown"))
            color: nvidiaDetector.secureBootEnabled ? "#c43a3a" : "#2b8a3e"
            font.bold: true
        }

        Label {
            text: qsTr("Session type: ") + nvidiaDetector.sessionType
            font.bold: true
        }

        Label {
            text: nvidiaDetector.waylandSession
                  ? qsTr("For Wayland sessions, nvidia-drm.modeset=1 is applied automatically.")
                  : qsTr("For X11 sessions, the xorg-x11-drv-nvidia package is verified and installed.")
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
                text: nvidiaDetector.verificationReport
                wrapMode: Text.Wrap
            }
        }

        Label {
            visible: nvidiaInstaller.proprietaryAgreementRequired
            text: nvidiaInstaller.proprietaryAgreementText
            color: "#8a6500"
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }

        CheckBox {
            id: eulaAccept
            visible: nvidiaInstaller.proprietaryAgreementRequired
            text: qsTr("I accept the license / agreement terms")
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Button {
                text: qsTr("Install Proprietary Driver")
                enabled: !nvidiaInstaller.proprietaryAgreementRequired || eulaAccept.checked
                onClicked: nvidiaInstaller.installProprietary(eulaAccept.checked)
            }

            Button {
                text: qsTr("Install Open-Source Driver (Nouveau)")
                onClicked: nvidiaInstaller.installOpenSource()
            }

            Button {
                text: qsTr("Deep Clean")
                onClicked: nvidiaInstaller.deepClean()
            }
        }

        RowLayout {
            spacing: 8

            Button {
                text: qsTr("Check for Updates")
                onClicked: nvidiaUpdater.checkForUpdate()
            }

            Button {
                text: qsTr("Apply Update")
                enabled: nvidiaUpdater.updateAvailable
                onClicked: nvidiaUpdater.applyUpdate()
            }

            Label {
                visible: nvidiaUpdater.updateAvailable
                text: qsTr("Latest version: ") + nvidiaUpdater.latestVersion
                color: "#8a6500"
            }
        }

        RowLayout {
            spacing: 8

            Button {
                text: qsTr("Rescan")
                onClicked: {
                    nvidiaDetector.refresh();
                    nvidiaInstaller.refreshProprietaryAgreement();
                }
            }

            Label {
                text: qsTr("Installed NVIDIA version: ") + nvidiaUpdater.currentVersion
                visible: nvidiaUpdater.currentVersion.length > 0
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
        target: nvidiaInstaller
        function onProgressMessage(message) {
            logArea.append(message);
        }
        function onInstallFinished(success, message) {
            logArea.append(message);
            nvidiaDetector.refresh();
            nvidiaUpdater.checkForUpdate();
            nvidiaInstaller.refreshProprietaryAgreement();
        }
        function onRemoveFinished(success, message) {
            logArea.append(message);
            nvidiaDetector.refresh();
            nvidiaInstaller.refreshProprietaryAgreement();
        }
    }

    Connections {
        target: nvidiaUpdater
        function onProgressMessage(message) {
            logArea.append(message);
        }
        function onUpdateFinished(success, message) {
            logArea.append(message);
            nvidiaDetector.refresh();
            nvidiaUpdater.checkForUpdate();
        }
    }

    Component.onCompleted: {
        nvidiaDetector.refresh();
        nvidiaUpdater.checkForUpdate();
        nvidiaInstaller.refreshProprietaryAgreement();
    }
}
