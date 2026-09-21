import QtQuick
import QtQuick.Controls

Button {
    id: dlgBtn
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

    implicitHeight: Math.round(38 * uiScale)

    scale: !enabled ? 1.0 : (down ? 0.97 : (hovered ? 1.015 : 1.0))
    Behavior on scale { NumberAnimation { duration: 100 } }

    contentItem: Label {
        text: dlgBtn.text
        font.pixelSize: Math.round(13 * dlgBtn.uiScale)
        font.weight: Font.DemiBold
        color: !dlgBtn.enabled ? dlgBtn.softTextColor
               : tone === "primary" || tone === "danger" || tone === "success" ? "#FFFFFF"
               : dlgBtn.textColor
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle {
        radius: 8
        color: !dlgBtn.enabled ? dlgBtn.cardColor
               : tone === "primary" ? (dlgBtn.down ? Qt.darker(dlgBtn.accentColor, 1.1) : (dlgBtn.hovered ? Qt.lighter(dlgBtn.accentColor, 1.08) : dlgBtn.accentColor))
               : tone === "danger" ? (dlgBtn.down ? "#DC2626" : (dlgBtn.hovered ? "#EF4444" : "#DC2626"))
               : tone === "success" ? (dlgBtn.down ? "#16A34A" : (dlgBtn.hovered ? "#22C55E" : "#16A34A"))
               : (dlgBtn.down ? Qt.darker(dlgBtn.bgColor, 1.08) : (dlgBtn.hovered ? dlgBtn.cardColor : dlgBtn.bgColor))
        border.width: 1
        border.color: !dlgBtn.enabled ? dlgBtn.borderColor
                     : tone === "primary" ? Qt.tint(dlgBtn.accentColor, "#33FFFFFF")
                     : tone === "danger" ? Qt.tint("#EF4444", "#33FFFFFF")
                     : tone === "success" ? Qt.tint("#22C55E", "#33FFFFFF")
                     : dlgBtn.borderColor
    }
}
