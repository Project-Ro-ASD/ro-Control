import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: settingsPage
    property bool darkMode: false

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
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
            }
        }
    }
}
