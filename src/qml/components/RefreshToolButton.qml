import QtQuick
import QtQuick.Controls

ToolButton {
    id: control

    required property var theme
    property real uiScale: 1.0
    property bool busy: false
    property bool darkMode: false
    property string tooltip: qsTr("Refresh")

    implicitWidth: Math.round((darkMode ? 40 : 42) * uiScale)
    implicitHeight: Math.round((darkMode ? 40 : 42) * uiScale)
    display: AbstractButton.IconOnly
    opacity: enabled ? 1.0 : 0.55
    ToolTip {
        id: refreshTip
        visible: control.tooltip.length > 0 && control.hovered
        text: control.tooltip
        delay: 300
        timeout: 5000
        topPadding: Math.round(6 * control.uiScale)
        bottomPadding: Math.round(6 * control.uiScale)
        leftPadding: Math.round(12 * control.uiScale)
        rightPadding: Math.round(12 * control.uiScale)

        contentItem: Label {
            text: refreshTip.text
            color: control.theme && control.theme.text ? control.theme.text : (control.darkMode ? "#F3F4F6" : "#1F2937")
            font.pixelSize: Math.round(11 * control.uiScale)
            font.weight: Font.Medium
        }

        background: Rectangle {
            radius: 8
            color: control.darkMode ? "#241E34" : "#FFFFFF"
            border.width: 1
            border.color: control.darkMode ? "#4D436B" : "#CBD5E1"

            Rectangle {
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.margins: 3
                width: 3
                radius: 1.5
                color: control.theme && control.theme.accentA ? control.theme.accentA : "#6366F1"
            }
        }
    }

    contentItem: Item {
        implicitWidth: Math.round(22 * control.uiScale)
        implicitHeight: Math.round(22 * control.uiScale)

        Image {
            id: refreshIcon
            anchors.centerIn: parent
            width: Math.round(20 * control.uiScale)
            height: Math.round(20 * control.uiScale)
            source: control.darkMode ? "qrc:/qt/qml/rocontrol/assets/icon-refresh-light.svg"
                                     : "qrc:/qt/qml/rocontrol/assets/icon-refresh.svg"
            fillMode: Image.PreserveAspectFit
            smooth: true
            antialiasing: true
            opacity: control.enabled ? 1.0 : 0.65
            scale: control.down ? 0.92 : control.hovered && control.enabled ? 1.06 : 1.0

            Behavior on scale {
                NumberAnimation { duration: 120; easing.type: Easing.OutCubic }
            }

            RotationAnimation {
                id: busySpin
                target: refreshIcon
                property: "rotation"
                running: control.busy
                from: 0
                to: 360
                duration: 900
                loops: Animation.Infinite
            }

            NumberAnimation {
                id: clickSpin
                target: refreshIcon
                property: "rotation"
                from: refreshIcon.rotation
                to: refreshIcon.rotation + 180
                duration: 260
                easing.type: Easing.OutCubic
            }
        }
    }

    background: Item {
        Rectangle {
            anchors.fill: parent
            visible: !control.darkMode
            radius: width / 2
            color: "transparent"
            border.width: control.enabled ? Math.max(1, Math.round(2 * control.uiScale)) : 1
            border.color: control.enabled
                          ? (control.darkMode ? control.theme.accentB : control.theme.accentA)
                          : control.theme.border
            opacity: control.enabled ? 0.95 : 0.45
        }

        Rectangle {
            anchors.fill: parent
            anchors.margins: control.darkMode ? 0 : Math.max(3, Math.round(3 * control.uiScale))
            radius: width / 2
            color: !control.enabled ? "transparent"
                  : control.down ? Qt.tint(control.theme.infoBg, control.darkMode ? "#30ffffff" : "#18000000")
                  : control.hovered ? Qt.tint(control.theme.infoBg, control.darkMode ? "#40ffffff" : "#12000000")
                  : control.theme.infoBg
            border.width: 1
            border.color: control.hovered && control.enabled ? control.theme.accentA
                                                             : control.theme.accentB
        }
    }

    onClicked: {
        if (!busy)
            clickSpin.restart();
    }
}
