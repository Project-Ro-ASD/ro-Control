import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: panel

    required property var theme
    property string title: ""
    property string subtitle: ""
    default property alias content: bodyColumn.data

    radius: 24
    color: theme.card
    border.width: 1
    border.color: theme.border
    implicitHeight: innerColumn.implicitHeight + 40

    ColumnLayout {
        id: innerColumn
        x: 20
        y: 20
        width: parent.width - 40
        spacing: 16

        ColumnLayout {
            spacing: 6
            Layout.fillWidth: true

            Label {
                text: panel.title
                font.pixelSize: 16
                font.weight: Font.Medium
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
            spacing: 12
            Layout.fillWidth: true
        }
    }
}
