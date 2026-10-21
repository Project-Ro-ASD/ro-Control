import QtQuick
import "../style" as Style
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: card
    Style.Theme { id: fallbackTheme; darkMode: false }
    required property var theme
    property string title: ""
    property string value: ""
    property string subtitle: ""
    property string accentColor: theme.accentB
    property bool emphasized: false
    property bool busy: false
    property int minimumBodyHeight: 142
    readonly property int valueLength: value.length
    readonly property int valuePixelSize: valueLength > 22 ? 26 : valueLength > 12 ? 34 : 42

    radius: Style.RoTokens.radiusSm
    color: emphasized ? Qt.tint(theme.cardStrong, Qt.rgba(theme.window.r, theme.window.g, theme.window.b, 0.0824)) : theme.card
    border.width: 1
    border.color: theme.border
    implicitHeight: Math.max(minimumBodyHeight, cardLayout.implicitHeight + 40)


    ColumnLayout {
        id: cardLayout
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: Style.RoTokens.spaceXl
        spacing: Style.RoTokens.spaceMd

        Label {
            text: card.title
            color: card.theme.textMuted
            font.pixelSize: 14
            font.weight: Font.DemiBold
        }

        Label {
            text: card.value
            color: card.theme.text
            font.pixelSize: card.valuePixelSize
            font.weight: Font.DemiBold
            wrapMode: Text.Wrap
            Layout.fillWidth: true
            maximumLineCount: 2
            minimumPixelSize: 22
            fontSizeMode: Text.Fit
        }

        Label {
            text: card.subtitle
            color: card.theme.textSoft
            wrapMode: Text.Wrap
            Layout.fillWidth: true
            visible: text.length > 0
            font.pixelSize: 13
        }

        BusyIndicator {
            running: card.busy
            visible: running
            Layout.alignment: Qt.AlignLeft
        }
    }
}
