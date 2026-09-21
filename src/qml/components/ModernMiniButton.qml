import QtQuick
import QtQuick.Controls

Button {
    id: miniBtn
    property string tone: "neutral"
    property real uiScale: 1.0
    property var theme: ({})
    property bool darkMode: false

    readonly property color textColor: theme && theme.text ? theme.text : (darkMode ? "#F8FAFC" : "#0F172A")
    readonly property color softTextColor: theme && theme.textSoft ? theme.textSoft : (darkMode ? "#94A3B8" : "#64748B")
    readonly property color borderColor: theme && theme.border ? theme.border : (darkMode ? "#4D436B" : "#CBD5E1")
    readonly property color cardColor: theme && theme.cardStrong ? theme.cardStrong : (darkMode ? "#342D4A" : "#F1F5F9")
    readonly property color bgColor: theme && theme.card ? theme.card : (darkMode ? "#29233B" : "#FFFFFF")
    readonly property color accentColor: theme && theme.accentA ? theme.accentA : (darkMode ? "#818CF8" : "#4F46E5")

    implicitHeight: Math.round(32 * uiScale)
    leftPadding: Math.round(14 * uiScale)
    rightPadding: Math.round(14 * uiScale)

    scale: !enabled ? 1.0 : (down ? 0.96 : (hovered ? 1.02 : 1.0))
    Behavior on scale { NumberAnimation { duration: 100 } }

    contentItem: Label {
        text: miniBtn.text
        font.pixelSize: Math.round(11 * miniBtn.uiScale)
        font.weight: Font.DemiBold
        color: !miniBtn.enabled ? miniBtn.softTextColor
               : tone === "danger" ? (miniBtn.hovered ? "#FFFFFF" : "#EF4444")
               : tone === "primary" ? "#FFFFFF"
               : tone === "success" ? "#FFFFFF"
               : miniBtn.textColor
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle {
        radius: 6
        color: !miniBtn.enabled ? Qt.rgba(0,0,0,0)
               : tone === "primary" ? (miniBtn.down ? Qt.darker(miniBtn.accentColor, 1.1) : (miniBtn.hovered ? Qt.lighter(miniBtn.accentColor, 1.1) : miniBtn.accentColor))
               : tone === "danger" ? (miniBtn.down ? "#DC2626" : (miniBtn.hovered ? "#EF4444" : (miniBtn.darkMode ? "#3D171E" : "#FEE2E2")))
               : tone === "success" ? (miniBtn.down ? "#16A34A" : (miniBtn.hovered ? "#22C55E" : (miniBtn.darkMode ? "#143828" : "#DCFCE7")))
               : (miniBtn.down ? Qt.darker(miniBtn.bgColor, 1.1) : (miniBtn.hovered ? miniBtn.cardColor : miniBtn.bgColor))
        border.width: 1
        border.color: !miniBtn.enabled ? miniBtn.borderColor
                     : tone === "danger" ? "#EF4444"
                     : tone === "primary" ? miniBtn.accentColor
                     : tone === "success" ? "#22C55E"
                     : miniBtn.borderColor
        opacity: miniBtn.enabled ? 1.0 : 0.5
    }
}
