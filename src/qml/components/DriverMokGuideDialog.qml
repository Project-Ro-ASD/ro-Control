import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "." as Components

Popup {
    id: mokGuidePopup
    modal: true
    focus: true

    property var theme: ({})
    property bool darkMode: false
    property real uiScale: 1.0

    readonly property color textColor: theme && theme.text ? theme.text : (darkMode ? "#F8FAFC" : "#0F172A")
    readonly property color softTextColor: theme && theme.textSoft ? theme.textSoft : (darkMode ? "#94A3B8" : "#64748B")
    readonly property color borderColor: theme && theme.border ? theme.border : (darkMode ? "#4D436B" : "#CBD5E1")
    readonly property color cardColor: theme && theme.cardStrong ? theme.cardStrong : (darkMode ? "#342D4A" : "#F1F5F9")
    readonly property color bgColor: theme && theme.card ? theme.card : (darkMode ? "#29233B" : "#FFFFFF")

    width: Math.min((parent ? parent.width : 600) - 40, Math.round(560 * uiScale))
    height: Math.min((parent ? parent.height : 800) - 40, mokGuideContent.implicitHeight + topPadding + bottomPadding)
    x: Math.round(((parent ? parent.width : 600) - width) / 2)
    y: Math.round(((parent ? parent.height : 800) - height) / 2)
    padding: Math.round(20 * uiScale)
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    enter: Transition {
        ParallelAnimation {
            NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: 180; easing.type: Easing.OutCubic }
            NumberAnimation { property: "scale"; from: 0.95; to: 1.0; duration: 180; easing.type: Easing.OutBack }
        }
    }
    exit: Transition {
        ParallelAnimation {
            NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: 140; easing.type: Easing.InCubic }
            NumberAnimation { property: "scale"; from: 1.0; to: 0.96; duration: 140 }
        }
    }

    background: Rectangle {
        radius: Math.round(16 * mokGuidePopup.uiScale)
        color: mokGuidePopup.bgColor
        border.width: 1
        border.color: mokGuidePopup.borderColor
    }

    contentItem: ScrollView {
        id: mokGuideScroll
        clip: true
        contentWidth: availableWidth

        ColumnLayout {
            id: mokGuideContent
            width: mokGuideScroll.availableWidth
            spacing: Math.round(14 * mokGuidePopup.uiScale)

            // Header
            RowLayout {
                Layout.fillWidth: true
                spacing: Math.round(12 * mokGuidePopup.uiScale)

                Rectangle {
                    implicitWidth: Math.round(38 * mokGuidePopup.uiScale)
                    implicitHeight: Math.round(38 * mokGuidePopup.uiScale)
                    radius: Math.round(10 * mokGuidePopup.uiScale)
                    color: mokGuidePopup.darkMode ? "#2E2442" : "#F3E8FF"
                    border.width: 1
                    border.color: "#7C3AED"

                    Text {
                        anchors.fill: parent
                        text: "🔐"
                        font.pixelSize: Math.round(18 * mokGuidePopup.uiScale)
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Secure Boot MOK Enrollment")
                        color: mokGuidePopup.textColor
                        font.pixelSize: Math.round(16 * mokGuidePopup.uiScale)
                        font.weight: Font.Bold
                    }

                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Required before restarting after an NVIDIA driver installation")
                        color: mokGuidePopup.softTextColor
                        font.pixelSize: Math.round(11 * mokGuidePopup.uiScale)
                    }
                }

                Button {
                    id: closeMokBtn
                    implicitWidth: Math.round(30 * mokGuidePopup.uiScale)
                    implicitHeight: Math.round(30 * mokGuidePopup.uiScale)
                    background: Rectangle {
                        radius: 15
                        color: closeMokBtn.hovered ? (mokGuidePopup.darkMode ? "#3B3156" : "#E2E8F0") : "transparent"
                    }
                    contentItem: Text {
                        text: "✕"
                        color: mokGuidePopup.softTextColor
                        font.pixelSize: Math.round(13 * mokGuidePopup.uiScale)
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: mokGuidePopup.close()
                }
            }

            // 3-Step Clean Action Flow
            ColumnLayout {
                Layout.fillWidth: true
                spacing: Math.round(8 * mokGuidePopup.uiScale)

                Repeater {
                    model: [
                        {
                            step: "1",
                            title: qsTr("Reboot & Intercept"),
                            desc: qsTr("Before installation, generate and import the akmods key using your Fedora Secure Boot procedure. Then restart and enter Shim UEFI Key Management when prompted.")
                        },
                        {
                            step: "2",
                            title: qsTr("Select 'Enroll MOK'"),
                            desc: qsTr("Choose 'Enroll MOK' from the menu, select 'Continue', and confirm with 'Yes'.")
                        },
                        {
                            step: "3",
                            title: qsTr("Confirm & Reboot"),
                            desc: qsTr("Enter your enrollment password if prompted, then select 'Reboot'. Your modules are now permanently trusted.")
                        }
                    ]

                    delegate: Rectangle {
                        required property var modelData
                        Layout.fillWidth: true
                        radius: 10
                        color: mokGuidePopup.cardColor
                        border.width: 1
                        border.color: mokGuidePopup.borderColor
                        implicitHeight: stepRow.implicitHeight + Math.round(18 * mokGuidePopup.uiScale)

                        RowLayout {
                            id: stepRow
                            anchors.fill: parent
                            anchors.margins: Math.round(10 * mokGuidePopup.uiScale)
                            spacing: Math.round(12 * mokGuidePopup.uiScale)

                            Rectangle {
                                width: Math.round(24 * mokGuidePopup.uiScale)
                                height: Math.round(24 * mokGuidePopup.uiScale)
                                radius: Math.round(12 * mokGuidePopup.uiScale)
                                color: mokGuidePopup.darkMode ? "#2E2442" : "#F3E8FF"
                                border.width: 1
                                border.color: "#7C3AED"
                                Layout.alignment: Qt.AlignVCenter

                                Label {
                                    anchors.centerIn: parent
                                    text: modelData.step
                                    color: mokGuidePopup.darkMode ? "#C084FC" : "#7C3AED"
                                    font.pixelSize: Math.round(11 * mokGuidePopup.uiScale)
                                    font.weight: Font.Bold
                                }
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2

                                Label {
                                    text: modelData.title
                                    color: mokGuidePopup.textColor
                                    font.pixelSize: Math.round(12 * mokGuidePopup.uiScale)
                                    font.weight: Font.DemiBold
                                }

                                Label {
                                    Layout.fillWidth: true
                                    text: modelData.desc
                                    color: mokGuidePopup.softTextColor
                                    font.pixelSize: Math.round(11 * mokGuidePopup.uiScale)
                                    wrapMode: Text.Wrap
                                }
                            }
                        }
                    }
                }
            }

            // Footer
            RowLayout {
                Layout.fillWidth: true
                spacing: Math.round(8 * mokGuidePopup.uiScale)

                Item { Layout.fillWidth: true }

                Components.ModernDialogButton {
                    Layout.preferredWidth: Math.round(120 * mokGuidePopup.uiScale)
                    text: qsTr("Got It")
                    tone: "primary"
                    theme: mokGuidePopup.theme
                    darkMode: mokGuidePopup.darkMode
                    uiScale: mokGuidePopup.uiScale
                    onClicked: mokGuidePopup.close()
                }
            }
        }
    }
}
