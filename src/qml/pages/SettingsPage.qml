import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: settingsPage
    property bool darkMode: false

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 12

        Label {
            text: qsTr("Settings")
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
                    text: qsTr("About")
                    font.pixelSize: 20
                    font.bold: true
                }

                Label {
                    text: qsTr("Application: ") + Qt.application.name + " (" + Qt.application.version + ")"
                }

                Label {
                    text: qsTr("Theme: ") + (settingsPage.darkMode ? qsTr("System Dark") : qsTr("System Light"))
                }
            }
        }
    }
}
