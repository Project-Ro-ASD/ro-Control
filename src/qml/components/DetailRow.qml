import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: row

    required property var theme
    property string label: ""
    property string value: ""

    radius: 12
    color: theme.cardStrong
    border.width: 1
    border.color: theme.border
    implicitHeight: contentRow.implicitHeight + 20

    RowLayout {
        id: contentRow
        anchors.fill: parent
        anchors.margins: 10
        spacing: 12

        Label {
            Layout.preferredWidth: Math.min(180, row.width * 0.42)
            text: row.label
            color: row.theme.textMuted
            font.pixelSize: 13
            font.weight: Font.DemiBold
            elide: Text.ElideRight
        }

        Label {
            Layout.fillWidth: true
            text: row.value
            color: row.theme.text
            font.pixelSize: 14
            wrapMode: Text.Wrap
        }
    }
}
