import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: settingsPage
    property bool darkMode: false

    ScrollView {
        anchors.fill: parent
        anchors.margins: 20

        ColumnLayout {
            width: parent.width
            spacing: 12

            Label {
                text: "Ayarlar"
                font.pixelSize: 24
                font.bold: true
            }

            Rectangle {
                Layout.fillWidth: true
                border.width: 1
                border.color: settingsPage.darkMode ? "#4f5f82" : "#c6cfdf"
                color: "transparent"
                radius: 8
                implicitHeight: aboutCol.implicitHeight + 20

                ColumnLayout {
                    id: aboutCol
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 8

                    Label {
                        text: "Hakkinda"
                        font.pixelSize: 20
                        font.bold: true
                    }

                    Label {
                        text: "Uygulama: " + Qt.application.name + " (" + Qt.application.version + ")"
                    }

                    Label {
                        text: "Tema: " + (settingsPage.darkMode ? "Sistem Koyu" : "Sistem Acik")
                    }

                    Label {
                        text: "Iyilestirmeler:"
                        font.bold: true
                    }

                    Label {
                        text: "- Secure Boot kontrolu\n- Surucu versiyonu teyit raporu\n- CommandRunner stdout duzeltmesi\n- RPM Fusion URL duzeltmesi"
                        wrapMode: Text.Wrap
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }
}
