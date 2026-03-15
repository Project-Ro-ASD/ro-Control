import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: settingsPage
    required property var theme
    required property string themeMode
    property bool darkMode: true
    property string githubUrl: ""
    signal themeModeRequested(string mode)
    signal aboutRequested()

    ScrollView {
        anchors.fill: parent
        clip: true

        ColumnLayout {
            width: parent.width
            spacing: 18

            Rectangle {
                Layout.fillWidth: true
                radius: 28
                color: Qt.tint(settingsPage.theme.panel, settingsPage.darkMode ? "#18ff8a3d" : "#121677ff")
                border.width: 1
                border.color: settingsPage.theme.border
                implicitHeight: heroLayout.implicitHeight + 28

                ColumnLayout {
                    id: heroLayout
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 12

                    Label {
                        text: qsTr("Application Settings")
                        font.pixelSize: 30
                        font.bold: true
                        color: settingsPage.theme.text
                    }

                    Label {
                        text: qsTr("Switch between light and dark presentation, inspect app metadata and jump to the repository from a single settings surface.")
                        wrapMode: Text.Wrap
                        Layout.fillWidth: true
                        color: settingsPage.theme.textMuted
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 18

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    radius: 26
                    color: settingsPage.theme.card
                    border.width: 1
                    border.color: settingsPage.theme.border
                    implicitHeight: appearanceColumn.implicitHeight + 26

                    ColumnLayout {
                        id: appearanceColumn
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 12

                        Label {
                            text: qsTr("Appearance")
                            font.pixelSize: 21
                            font.bold: true
                            color: settingsPage.theme.text
                        }

                        Label {
                            text: qsTr("Choose the application theme. The palette keeps the orange-blue gradient language in both modes.")
                            wrapMode: Text.Wrap
                            Layout.fillWidth: true
                            color: settingsPage.theme.textMuted
                        }

                        RowLayout {
                            spacing: 10

                            Button {
                                text: qsTr("Light Theme")
                                highlighted: settingsPage.themeMode === "light"
                                flat: settingsPage.themeMode !== "light"
                                onClicked: settingsPage.themeModeRequested("light")
                            }

                            Button {
                                text: qsTr("Dark Theme")
                                highlighted: settingsPage.themeMode === "dark"
                                flat: settingsPage.themeMode !== "dark"
                                onClicked: settingsPage.themeModeRequested("dark")
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            radius: 18
                            color: settingsPage.theme.cardMuted
                            implicitHeight: 56

                            Label {
                                anchors.fill: parent
                                anchors.margins: 14
                                verticalAlignment: Text.AlignVCenter
                                text: qsTr("Active mode: %1").arg(settingsPage.themeMode === "dark" ? qsTr("Dark") : qsTr("Light"))
                                color: settingsPage.theme.text
                                font.bold: true
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    radius: 26
                    color: settingsPage.theme.card
                    border.width: 1
                    border.color: settingsPage.theme.border
                    implicitHeight: aboutColumn.implicitHeight + 26

                    ColumnLayout {
                        id: aboutColumn
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 12

                        Label {
                            text: qsTr("About & Links")
                            font.pixelSize: 21
                            font.bold: true
                            color: settingsPage.theme.text
                        }

                        Label {
                            text: qsTr("Open the about panel for release notes and version details, or jump directly to the GitHub repository.")
                            wrapMode: Text.Wrap
                            Layout.fillWidth: true
                            color: settingsPage.theme.textMuted
                        }

                        Label {
                            text: qsTr("Application: %1").arg(Qt.application.displayName)
                            color: settingsPage.theme.textMuted
                        }

                        Label {
                            text: qsTr("Version: %1").arg(Qt.application.version)
                            color: settingsPage.theme.textMuted
                        }

                        RowLayout {
                            spacing: 10

                            Button {
                                text: qsTr("Open About")
                                onClicked: settingsPage.aboutRequested()
                            }

                            Button {
                                text: qsTr("GitHub Repository")
                                onClicked: Qt.openUrlExternally(settingsPage.githubUrl)
                            }
                        }
                    }
                }
            }
        }
    }
}
