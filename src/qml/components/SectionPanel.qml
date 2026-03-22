import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: panel

    required property var theme
    property string title: ""
    property string subtitle: ""
    default property alias content: bodyColumn.data

    radius: 22
    color: theme.card
    border.width: 1
    border.color: theme.border
    implicitHeight: innerColumn.implicitHeight + 36

    ColumnLayout {
        id: innerColumn
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 18
        spacing: 14

        ColumnLayout {
            spacing: 4
            Layout.fillWidth: true

            Label {
                text: panel.title
                font.pixelSize: 18
                font.bold: true
                color: panel.theme.text
                visible: text.length > 0
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }

            Label {
                text: panel.subtitle
                wrapMode: Text.Wrap
                color: panel.theme.textSoft
                Layout.fillWidth: true
                visible: text.length > 0
            }
        }

        ColumnLayout {
            id: bodyColumn
            spacing: 10
            Layout.fillWidth: true
        }
    }
}
