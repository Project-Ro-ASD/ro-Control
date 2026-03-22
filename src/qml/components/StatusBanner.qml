import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: banner

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
    readonly property color textTone: tone === "success" ? "#16351d"
                                   : tone === "warning" ? "#4b3202"
                                   : tone === "error" ? "#5b1820"
                                   : "#12304f"

    radius: 18
    color: bannerColor
    border.width: 1
    border.color: borderTone
    visible: text.length > 0

    implicitHeight: bannerLayout.implicitHeight + 24

    RowLayout {
        id: bannerLayout
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.margins: 12
        spacing: 12

        Rectangle {
            width: 10
            height: 10
            radius: 5
            color: banner.borderTone
        }

        Label {
            Layout.fillWidth: true
            text: banner.text
            wrapMode: Text.Wrap
            color: banner.textTone
        }
    }
}
