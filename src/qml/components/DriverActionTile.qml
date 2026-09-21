import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

AbstractButton {
    id: tile
    property string title: ""
    property string subtitle: ""
    property color accentColor: "#10B981"
    property bool activeBadge: false
    property string badgeText: ""
    property string tooltipText: ""
    property string disabledReason: ""
    property bool busy: false

    property var theme: ({})
    property bool darkMode: false
    property real uiScale: 1.0

    readonly property color textColor: theme && theme.text ? theme.text : (darkMode ? "#F8FAFC" : "#0F172A")
    readonly property color softTextColor: theme && theme.textSoft ? theme.textSoft : (darkMode ? "#94A3B8" : "#64748B")
    readonly property color borderColor: theme && theme.border ? theme.border : (darkMode ? "#4D436B" : "#CBD5E1")
    readonly property color cardColor: theme && theme.cardStrong ? theme.cardStrong : (darkMode ? "#342D4A" : "#F1F5F9")
    readonly property color bgColor: theme && theme.card ? theme.card : (darkMode ? "#29233B" : "#FFFFFF")

    Layout.fillWidth: true
    implicitHeight: Math.round(60 * uiScale)
    hoverEnabled: true

    scale: !enabled ? 1.0 : (down ? 0.98 : (hovered ? 1.012 : 1.0))
    opacity: enabled ? 1.0 : 0.45

    Behavior on scale {
        NumberAnimation { duration: 120; easing.type: Easing.OutQuad }
    }
    Behavior on opacity {
        NumberAnimation { duration: 150 }
    }

    ToolTip {
        id: tileTip
        visible: tile.tooltipText.length > 0 && tile.hovered
        text: tile.tooltipText
        delay: 300
        timeout: 6000
        topPadding: Math.round(8 * tile.uiScale)
        bottomPadding: Math.round(8 * tile.uiScale)
        leftPadding: Math.round(14 * tile.uiScale)
        rightPadding: Math.round(12 * tile.uiScale)

        contentItem: Label {
            text: tileTip.text
            color: tile.textColor
            font.pixelSize: Math.round(11 * tile.uiScale)
            font.weight: Font.Medium
            wrapMode: Text.Wrap
        }

        background: Rectangle {
            radius: 8
            color: tile.darkMode ? "#241E34" : "#FFFFFF"
            border.width: 1
            border.color: tile.darkMode ? "#4D436B" : "#CBD5E1"

            Rectangle {
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.margins: 3
                width: 3
                radius: 1.5
                color: tile.accentColor
            }
        }
    }

    background: Rectangle {
        radius: 10
        color: !tile.enabled ? tile.cardColor
               : tile.down ? Qt.darker(tile.bgColor, 1.05)
               : tile.hovered ? (tile.darkMode ? Qt.tint(tile.bgColor, Qt.rgba(tile.accentColor.r, tile.accentColor.g, tile.accentColor.b, 0.14))
                                               : Qt.tint(tile.bgColor, Qt.rgba(tile.accentColor.r, tile.accentColor.g, tile.accentColor.b, 0.08)))
               : tile.bgColor
        border.width: tile.hovered && tile.enabled ? 1.5 : 1
        border.color: tile.hovered && tile.enabled ? tile.accentColor : tile.borderColor

        Rectangle {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.margins: 8
            width: 4
            radius: 2
            color: tile.accentColor
            visible: tile.enabled
            opacity: tile.hovered ? 1.0 : 0.7
        }
    }

    contentItem: ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: 18
        anchors.rightMargin: 14
        anchors.topMargin: 8
        anchors.bottomMargin: 8
        spacing: 2

        RowLayout {
            Layout.fillWidth: true
            spacing: 6

            Label {
                Layout.fillWidth: true
                text: tile.title
                color: tile.textColor
                font.pixelSize: Math.round(13 * tile.uiScale)
                font.weight: Font.DemiBold
                elide: Text.ElideRight
            }

            BusyIndicator {
                visible: tile.busy
                running: tile.busy
                Layout.preferredWidth: Math.round(18 * tile.uiScale)
                Layout.preferredHeight: Math.round(18 * tile.uiScale)
            }

            Rectangle {
                visible: tile.activeBadge && !tile.busy
                Layout.preferredHeight: Math.round(18 * tile.uiScale)
                Layout.preferredWidth: badgeLabel.implicitWidth + 10
                radius: 4
                color: tile.darkMode ? "#064E3B" : "#ECFDF5"
                border.width: 1
                border.color: "#10B981"

                Label {
                    id: badgeLabel
                    anchors.centerIn: parent
                    text: tile.badgeText.length > 0 ? tile.badgeText : qsTr("ACTIVE")
                    color: "#10B981"
                    font.pixelSize: Math.round(9 * tile.uiScale)
                    font.weight: Font.Bold
                }
            }
        }

        Label {
            Layout.fillWidth: true
            text: !tile.enabled && tile.disabledReason.length > 0
                  ? tile.disabledReason : tile.subtitle
            color: tile.softTextColor
            font.pixelSize: Math.round(11 * tile.uiScale)
            elide: Text.ElideRight
        }
    }
}
