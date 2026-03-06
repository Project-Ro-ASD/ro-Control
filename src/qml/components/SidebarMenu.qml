import QtQuick
import QtQuick.Controls

Rectangle {
    id: sidebar
    width: 220
    color: "#181825"

    property int currentIndex: 0

    ListModel {
        id: menuModel
        ListElement {
            label: "Sürücü Yönetimi"
        }
        ListElement {
            label: "Sistem İzleme"
        }
        ListElement {
            label: "Ayarlar"
        }
    }

    Column {
        anchors.fill: parent
        spacing: 0

        // Başlık
        Item {
            width: parent.width
            height: 70

            Label {
                anchors.centerIn: parent
                text: "ro-Control"
                font.pixelSize: 22
                font.bold: true
                color: "#cdd6f4"
            }
        }

        Rectangle {
            width: parent.width - 32
            height: 1
            anchors.horizontalCenter: parent.horizontalCenter
            color: "#313244"
        }

        Item {
            width: 1
            height: 12
        }

        Repeater {
            model: menuModel

            delegate: Rectangle {
                id: menuItem
                required property int index
                required property string label

                width: sidebar.width - 16
                height: 44
                x: 8
                radius: 8
                color: sidebar.currentIndex === menuItem.index ? "#313244" : mouseArea.containsMouse ? "#1e1e2e" : "transparent"

                Label {
                    anchors.verticalCenter: parent.verticalCenter
                    leftPadding: 16
                    text: menuItem.label
                    font.pixelSize: 14
                    color: sidebar.currentIndex === menuItem.index ? "#89b4fa" : "#a6adc8"
                }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: sidebar.currentIndex = menuItem.index
                }
            }
        }
    }

    // Versiyon — alt köşe
    Label {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 16
        text: "v" + Qt.application.version
        font.pixelSize: 11
        color: "#585b70"
    }
}
