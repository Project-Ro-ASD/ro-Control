import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: page
    required property var nvidiaDetector
    required property var nvidiaInstaller
    required property var nvidiaUpdater

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
            text: "GPU: " + (page.nvidiaDetector.gpuFound ? page.nvidiaDetector.gpuName : "Tespit edilmedi")
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }

        Label {
            text: "Aktif surucu: " + page.nvidiaDetector.activeDriver
            wrapMode: Text.Wrap
        }

        Label {
            text: "Surucu versiyonu: " + (page.nvidiaDetector.driverVersion.length > 0 ? page.nvidiaDetector.driverVersion : "Yok")
        }

        Label {
            text: "Secure Boot: " + (page.nvidiaDetector.secureBootKnown ? (page.nvidiaDetector.secureBootEnabled ? "Acik" : "Kapali") : "Bilinmiyor")
            color: !page.nvidiaDetector.secureBootKnown ? "#8a6500" : (page.nvidiaDetector.secureBootEnabled ? "#c43a3a" : "#2b8a3e")
            font.bold: true
        }

        Label {
            text: "Oturum altyapisi: " + page.nvidiaDetector.sessionType
            font.bold: true
        }

        Label {
            text: page.nvidiaDetector.waylandSession ? "Wayland icin nvidia-drm.modeset=1 parametresi otomatik uygulanir." : "X11 icin xorg-x11-drv-nvidia paketi kontrol edilip kurulur."
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
            text: "Lisans/sozlesme kosullarini kabul ediyorum"
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Button {
                text: "Kapali Kaynak Surucu Kur"
                enabled: !page.nvidiaInstaller.proprietaryAgreementRequired || eulaAccept.checked
                onClicked: page.nvidiaInstaller.installProprietary(eulaAccept.checked)
            }

            Button {
                text: "Nouveau Surucusu Kur"
                onClicked: {
                    logArea.append("Nouveau surucusu kurulumu baslatildi...");
                    page.nvidiaInstaller.installOpenSource();
                }
            }

            Button {
                text: "Deep Clean"
                onClicked: page.nvidiaInstaller.deepClean()
            }
        }

        RowLayout {
            spacing: 8

            // TR: Manuel kontrol butonu, sonucu log alanina yazar.
            // EN: Manual check button writes status into the on-screen log.
            Button {
                text: "Guncelleme Kontrol Et"
                onClicked: {
                    logArea.append("Guncelleme kontrolu istendi...");
                    page.nvidiaUpdater.checkForUpdate();
                }
            }

            Button {
                text: "Guncellemeyi Uygula"
                enabled: page.nvidiaUpdater.updateAvailable
                onClicked: page.nvidiaUpdater.applyUpdate()
            }

            Label {
                visible: page.nvidiaUpdater.updateAvailable
                text: "Yeni surum: " + page.nvidiaUpdater.latestVersion
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
                    text: "Belirli Surum Uygula"
                    font.bold: true
                }

                Label {
                    text: page.nvidiaUpdater.availableVersions.length > 0
                          ? "Repo surumleri listelendi. Secilen surum kurulabilir/guncellenebilir."
                          : "Repo surum listesi henuz yuklenmedi veya surum bulunamadi."
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
                        enabled: model.length > 0
                    }

                    Button {
                        text: "Surumleri Yenile"
                        onClicked: {
                            logArea.append("Repo surum listesi yenileniyor...");
                            page.nvidiaUpdater.refreshAvailableVersions();
                        }
                    }

                    Button {
                        text: "Secili Surumu Uygula"
                        enabled: versionSelector.currentIndex >= 0 && page.nvidiaUpdater.availableVersions.length > 0
                        onClicked: {
                            const selectedVersion = versionSelector.currentText;
                            logArea.append("Secilen surum uygulanacak: " + selectedVersion);
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
                text: "Yeniden Tara"
                onClicked: {
                    logArea.append("Sistem yeniden taraniyor...");
                    page.nvidiaDetector.refresh();
                    page.nvidiaInstaller.refreshProprietaryAgreement();
                    page.nvidiaUpdater.checkForUpdate();
                    logArea.append("Yeniden tarama tamamlandi.");
                }
            }

            Label {
                text: "Mevcut nvidia surumu: " + page.nvidiaUpdater.currentVersion
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
            logArea.append(message);
        }
        function onInstallFinished(success, message) {
            logArea.append(message);
            page.nvidiaDetector.refresh();
            page.nvidiaUpdater.checkForUpdate();
            page.nvidiaInstaller.refreshProprietaryAgreement();
        }
        function onRemoveFinished(success, message) {
            logArea.append(message);
            page.nvidiaDetector.refresh();
            page.nvidiaInstaller.refreshProprietaryAgreement();
        }
    }

    Connections {
        target: page.nvidiaUpdater
        // TR: Updater backend mesajlarini canli log olarak UI'ye aktar.
        // EN: Stream updater backend messages into the live UI log.
        function onProgressMessage(message) {
            logArea.append(message);
        }
        function onUpdateFinished(success, message) {
            logArea.append(message);
            page.nvidiaDetector.refresh();
            page.nvidiaUpdater.checkForUpdate();
        }
    }

    Component.onCompleted: {
        page.nvidiaDetector.refresh();
        page.nvidiaUpdater.checkForUpdate();
        page.nvidiaUpdater.refreshAvailableVersions();
        page.nvidiaInstaller.refreshProprietaryAgreement();
    }
}
