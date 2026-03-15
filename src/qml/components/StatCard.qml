import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: card
    required property var theme
    property string title: ""
    property string value: ""
    property string subtitle: ""
    property string accentColor: theme.accentB
    property bool emphasized: false
    property bool busy: false

    radius: 24
    color: emphasized ? Qt.tint(theme.cardStrong, "#15000000") : theme.card
    border.width: 1
    border.color: theme.border
    implicitHeight: cardLayout.implicitHeight + 30

    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 6
        radius: 24
        color: card.accentColor
        opacity: 0.9
    }

    ColumnLayout {
        id: cardLayout
        anchors.fill: parent
        anchors.margins: 18
        spacing: 10

        Label {
            text: card.title
            color: card.theme.textMuted
            font.pixelSize: 13
            font.bold: true
        }

        Label {
            text: card.value
            color: card.theme.text
            font.pixelSize: 28
            font.bold: true
        }

        Label {
            text: card.subtitle
            color: card.theme.textSoft
            wrapMode: Text.Wrap
            Layout.fillWidth: true
            visible: text.length > 0
        }

        BusyIndicator {
            running: card.busy
            visible: running
            Layout.alignment: Qt.AlignLeft
        }
    }
}
