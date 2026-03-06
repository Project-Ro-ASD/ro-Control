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
            text: "Surucu Yonetimi"
            font.pixelSize: 24
            font.bold: true
        }

        Label {
            text: "GPU: " + (nvidiaDetector.gpuFound ? nvidiaDetector.gpuName : "Tespit edilmedi")
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }

        Label {
            text: "Aktif surucu: " + nvidiaDetector.activeDriver
            wrapMode: Text.Wrap
        }

        Label {
            text: "Surucu versiyonu: " + (nvidiaDetector.driverVersion.length > 0 ? nvidiaDetector.driverVersion : "Yok")
        }

        Label {
            text: "Secure Boot: " + (nvidiaDetector.secureBootEnabled ? "Acik" : "Kapali/Bilinmiyor")
            color: nvidiaDetector.secureBootEnabled ? "#c43a3a" : "#2b8a3e"
            font.bold: true
        }

        Label {
            text: "Oturum altyapisi: " + nvidiaDetector.sessionType
            font.bold: true
        }

        Label {
            text: nvidiaDetector.waylandSession ? "Wayland icin nvidia-drm.modeset=1 parametresi otomatik uygulanir." : "X11 icin xorg-x11-drv-nvidia paketi kontrol edilip kurulur."
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
            text: "Lisans/sozlesme kosullarini kabul ediyorum"
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Button {
                text: "Kapali Kaynak Surucu Kur"
                enabled: !nvidiaInstaller.proprietaryAgreementRequired || eulaAccept.checked
                onClicked: nvidiaInstaller.installProprietary(eulaAccept.checked)
            }

            Button {
                text: "Acik Kaynak Surucu Kur (Nouveau)"
                onClicked: nvidiaInstaller.installOpenSource()
            }

            Button {
                text: "Deep Clean"
                onClicked: nvidiaInstaller.deepClean()
            }
        }

        RowLayout {
            spacing: 8

            Button {
                text: "Guncelleme Kontrol Et"
                onClicked: nvidiaUpdater.checkForUpdate()
            }

            Button {
                text: "Guncellemeyi Uygula"
                enabled: nvidiaUpdater.updateAvailable
                onClicked: nvidiaUpdater.applyUpdate()
            }

            Label {
                visible: nvidiaUpdater.updateAvailable
                text: "Yeni surum: " + nvidiaUpdater.latestVersion
                color: "#8a6500"
            }
        }

        RowLayout {
            spacing: 8

            Button {
                text: "Yeniden Tara"
                onClicked: {
                    nvidiaDetector.refresh();
                    nvidiaInstaller.refreshProprietaryAgreement();
                }
            }

            Label {
                text: "Mevcut nvidia surumu: " + nvidiaUpdater.currentVersion
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
