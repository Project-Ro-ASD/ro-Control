import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: driverPage
    property bool busy: false

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 16

        // Başlık
        Label {
            text: "Sürücü Yönetimi"
            font.pixelSize: 26
            font.bold: true
            color: "#cdd6f4"
        }

        // ── GPU Bilgisi Kartı ──
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: gpuInfoCol.implicitHeight + 40
            radius: 12
            color: "#1e1e2e"

            ColumnLayout {
                id: gpuInfoCol
                anchors.fill: parent
                anchors.margins: 20
                spacing: 8

                Label {
                    text: "GPU Bilgisi"
                    font.pixelSize: 16
                    font.bold: true
                    color: "#cdd6f4"
                }

                RowLayout {
                    spacing: 8
                    Rectangle {
                        width: 10
                        height: 10
                        radius: 5
                        color: nvidiaDetector.gpuFound ? "#a6e3a1" : "#f38ba8"
                    }
                    Label {
                        text: nvidiaDetector.gpuFound ? nvidiaDetector.gpuName : "NVIDIA GPU bulunamadı"
                        font.pixelSize: 14
                        color: "#cdd6f4"
                    }
                }

                Label {
                    text: {
                        if (!nvidiaDetector.gpuFound)
                            return "Sistemde NVIDIA ekran kartı tespit edilemedi.";
                        if (nvidiaDetector.driverLoaded)
                            return "Aktif sürücü: Kapalı kaynak (NVIDIA proprietary) — v" + nvidiaDetector.driverVersion;
                        if (nvidiaDetector.nouveauActive)
                            return "Aktif sürücü: Açık kaynak (Nouveau)";
                        return "Sürücü yüklü değil";
                    }
                    font.pixelSize: 13
                    color: "#bac2de"
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }

                Label {
                    visible: nvidiaUpdater.updateAvailable
                    text: "Güncelleme mevcut: v" + nvidiaUpdater.latestVersion
                    font.pixelSize: 13
                    color: "#f9e2af"
                }
            }
        }

        // ── Secure Boot Uyarısı ──
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: sbRow.implicitHeight + 24
            radius: 12
            color: "#302020"
            border.color: "#f38ba8"
            border.width: 1
            visible: nvidiaDetector.secureBootEnabled

            RowLayout {
                id: sbRow
                anchors.fill: parent
                anchors.margins: 12
                spacing: 10

                Label {
                    text: "⚠"
                    font.pixelSize: 20
                    color: "#f38ba8"
                }

                Label {
                    text: "Secure Boot açık! NVIDIA kapalı kaynak sürücüsü (akmod-nvidia) imzasız kernel modülü " + "kullandığı için Secure Boot açıkken yüklenemeyebilir. Kurulum öncesi BIOS/UEFI " + "ayarlarından Secure Boot'u kapatmanız önerilir."
                    font.pixelSize: 12
                    color: "#f38ba8"
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
            }
        }

        // ── Sürücü İşlemleri Kartı ──
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: actionsCol.implicitHeight + 40
            radius: 12
            color: "#1e1e2e"
            visible: nvidiaDetector.gpuFound

            ColumnLayout {
                id: actionsCol
                anchors.fill: parent
                anchors.margins: 20
                spacing: 12

                Label {
                    text: "Sürücü İşlemleri"
                    font.pixelSize: 16
                    font.bold: true
                    color: "#cdd6f4"
                }

                // Kapalı kaynak sürücü kur
                Button {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 42
                    text: "Kapalı Kaynak Sürücü Kur (akmod-nvidia)"
                    visible: !nvidiaDetector.driverLoaded
                    enabled: !driverPage.busy

                    contentItem: Label {
                        text: parent.text
                        color: "#1e1e2e"
                        font.pixelSize: 13
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        radius: 8
                        color: parent.enabled ? (parent.hovered ? "#94e2d5" : "#a6e3a1") : "#585b70"
                    }

                    onClicked: {
                        driverPage.busy = true;
                        statusLog.text = "";
                        nvidiaInstaller.install();
                    }
                }

                // Açık kaynak sürücüye geç
                Button {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 42
                    text: "Açık Kaynak Sürücüye Geç (Nouveau)"
                    visible: nvidiaDetector.driverLoaded
                    enabled: !driverPage.busy

                    contentItem: Label {
                        text: parent.text
                        color: "#cdd6f4"
                        font.pixelSize: 13
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        radius: 8
                        color: parent.enabled ? (parent.hovered ? "#45475a" : "#313244") : "#585b70"
                    }

                    onClicked: {
                        driverPage.busy = true;
                        statusLog.text = "";
                        nvidiaInstaller.remove();
                    }
                }

                // Güncelleme kontrol + uygula
                RowLayout {
                    visible: nvidiaDetector.driverLoaded
                    spacing: 8

                    Button {
                        Layout.preferredHeight: 38
                        text: "Güncelleme Kontrol Et"
                        enabled: !driverPage.busy

                        contentItem: Label {
                            text: parent.text
                            color: "#cdd6f4"
                            font.pixelSize: 13
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            radius: 8
                            color: parent.enabled ? (parent.hovered ? "#45475a" : "#313244") : "#585b70"
                        }

                        onClicked: nvidiaUpdater.checkForUpdate()
                    }

                    Button {
                        Layout.preferredHeight: 38
                        text: "Güncelle → v" + nvidiaUpdater.latestVersion
                        visible: nvidiaUpdater.updateAvailable
                        enabled: !driverPage.busy

                        contentItem: Label {
                            text: parent.text
                            color: "#1e1e2e"
                            font.pixelSize: 13
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            radius: 8
                            color: parent.enabled ? (parent.hovered ? "#74c7ec" : "#89b4fa") : "#585b70"
                        }

                        onClicked: {
                            driverPage.busy = true;
                            statusLog.text = "";
                            nvidiaUpdater.applyUpdate();
                        }
                    }
                }

                // Kalıntı temizleme
                Button {
                    Layout.preferredHeight: 36
                    text: "Eski Sürücü Kalıntılarını Temizle"
                    enabled: !driverPage.busy

                    contentItem: Label {
                        text: parent.text
                        color: "#f38ba8"
                        font.pixelSize: 12
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        radius: 8
                        color: parent.enabled ? (parent.hovered ? "#45475a" : "transparent") : "transparent"
                        border.color: parent.enabled ? "#f38ba8" : "#585b70"
                        border.width: 1
                    }

                    onClicked: {
                        driverPage.busy = true;
                        statusLog.text = "";
                        nvidiaInstaller.deepClean();
                        driverPage.busy = false;
                        nvidiaDetector.refresh();
                    }
                }
            }
        }

        // ── İşlem Durumu Kartı ──
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 12
            color: "#1e1e2e"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 8

                Label {
                    text: "İşlem Durumu"
                    font.pixelSize: 16
                    font.bold: true
                    color: "#cdd6f4"
                }

                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    TextArea {
                        id: statusLog
                        readOnly: true
                        color: "#a6adc8"
                        font.pixelSize: 12
                        font.family: "monospace"
                        text: "Hazır."
                        wrapMode: Text.WordWrap
                        background: null
                    }
                }
            }
        }

        // Yenile butonu
        Button {
            Layout.alignment: Qt.AlignRight
            Layout.preferredHeight: 36
            text: "  Yenile  "
            enabled: !driverPage.busy

            contentItem: Label {
                text: parent.text
                color: "#cdd6f4"
                font.pixelSize: 13
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            background: Rectangle {
                radius: 8
                color: parent.hovered ? "#45475a" : "#313244"
            }

            onClicked: nvidiaDetector.refresh()
        }
    }

    // Backend sinyallerini dinle
    Connections {
        target: nvidiaInstaller
        function onProgressMessage(message) {
            statusLog.append(message);
        }
        function onInstallFinished(success, message) {
            statusLog.append(message);
            driverPage.busy = false;
            nvidiaDetector.refresh();
        }
        function onRemoveFinished(success, message) {
            statusLog.append(message);
            driverPage.busy = false;
            nvidiaDetector.refresh();
        }
    }

    Connections {
        target: nvidiaUpdater
        function onProgressMessage(message) {
            statusLog.append(message);
        }
        function onUpdateFinished(success, message) {
            statusLog.append(message);
            driverPage.busy = false;
            nvidiaDetector.refresh();
        }
    }

    // Sayfa yüklendiğinde GPU bilgisini tara
    Component.onCompleted: nvidiaDetector.refresh()
}
