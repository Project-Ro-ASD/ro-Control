import QtQuick
import "../style" as Style
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: banner
    Style.Theme { id: fallbackTheme; darkMode: false }

    required property var theme
    property string tone: "info"
    property string text: ""

    readonly property color bannerColor: tone === "success" ? theme.successBg
                                      : tone === "warning" ? theme.warningBg
                                      : tone === "error" ? theme.dangerBg
                                      : theme.infoBg
    readonly property color borderTone: tone === "success" ? theme.success
                                     : tone === "warning" ? theme.warning
                                     : tone === "error" ? theme.danger
                                     : theme.accentA
    readonly property color textTone: theme.text

    radius: Style.RoTokens.radiusSm
    color: bannerColor
    border.width: 1
    border.color: borderTone
    visible: text.length > 0

    implicitHeight: bannerLayout.implicitHeight + 16

    RowLayout {
        id: bannerLayout
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.margins: Style.RoTokens.spaceMd
        spacing: Style.RoTokens.spaceSm

        Rectangle {
            implicitWidth: 8
            implicitHeight: 8
            radius: Style.RoTokens.radiusXs
            color: banner.borderTone
        }

        Label {
            Layout.fillWidth: true
            text: banner.text
            wrapMode: Text.Wrap
            color: banner.textTone
            font.pixelSize: 12
            maximumLineCount: 2
        }
    }
}
