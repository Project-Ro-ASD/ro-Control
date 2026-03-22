import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: settingsPage

    required property var theme
    property bool darkMode: false
    property bool compactMode: false
    property bool showAdvancedInfo: true

    ScrollView {
        id: pageScroll
        anchors.fill: parent
        clip: true
        contentWidth: availableWidth

        ColumnLayout {
            width: pageScroll.availableWidth
            spacing: settingsPage.compactMode ? 12 : 16

            GridLayout {
                Layout.fillWidth: true
                columns: width > 980 ? 2 : 1
                columnSpacing: 16
                rowSpacing: 16

                SectionPanel {
                    Layout.fillWidth: true
                    theme: settingsPage.theme
                    title: qsTr("Interface")
                    subtitle: qsTr("Tune the shell density and how much operational detail the app exposes.")

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            Label {
                                text: qsTr("Language")
                                font.bold: true
                                color: settingsPage.theme.text
                            }

                            Label {
                                text: qsTr("Changes the application language immediately and keeps the selection for the next launch.")
                                wrapMode: Text.Wrap
                                color: settingsPage.theme.textSoft
                                Layout.fillWidth: true
                            }
                        }

                        ComboBox {
                            id: languagePicker
                            Layout.preferredWidth: 220
                            model: languageManager.availableLanguages
                            textRole: "label"

                            Component.onCompleted: {
                                for (let i = 0; i < model.length; ++i) {
                                    if (model[i].code === languageManager.currentLanguage) {
                                        currentIndex = i;
                                        break;
                                    }
                                }
                            }

                            onActivated: {
                                const selected = model[currentIndex];
                                if (selected && selected.code) {
                                    languageManager.setCurrentLanguage(selected.code);
                                }
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            Label {
                                text: qsTr("Compact layout")
                                font.bold: true
                                color: settingsPage.theme.text
                            }

                            Label {
                                text: qsTr("Reduces spacing to fit more information on screen.")
                                wrapMode: Text.Wrap
                                color: settingsPage.theme.textSoft
                                Layout.fillWidth: true
                            }
                        }

                        Switch {
                            checked: settingsPage.compactMode
                            onToggled: settingsPage.compactMode = checked
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            Label {
                                text: qsTr("Show advanced diagnostics")
                                font.bold: true
                                color: settingsPage.theme.text
                            }

                            Label {
                                text: qsTr("Shows verification reports and expanded monitor metrics.")
                                wrapMode: Text.Wrap
                                color: settingsPage.theme.textSoft
                                Layout.fillWidth: true
                            }
                        }

                        Switch {
                            checked: settingsPage.showAdvancedInfo
                            onToggled: settingsPage.showAdvancedInfo = checked
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        InfoBadge {
                            text: qsTr("Language: ") + languageManager.currentLanguageLabel
                            backgroundColor: settingsPage.theme.cardStrong
                            foregroundColor: settingsPage.theme.text
                        }

                        InfoBadge {
                            text: settingsPage.darkMode ? qsTr("System Dark Theme") : qsTr("System Light Theme")
                            backgroundColor: settingsPage.theme.infoBg
                            foregroundColor: settingsPage.theme.text
                        }

                        InfoBadge {
                            text: settingsPage.compactMode ? qsTr("Compact Active") : qsTr("Comfort Active")
                            backgroundColor: settingsPage.theme.cardStrong
                            foregroundColor: settingsPage.theme.text
                        }
                    }
                }

                SectionPanel {
                    Layout.fillWidth: true
                    theme: settingsPage.theme
                    title: qsTr("Diagnostics")
                    subtitle: qsTr("Useful runtime context before filing issues or performing support work.")

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        DetailRow {
                            Layout.fillWidth: true
                            theme: settingsPage.theme
                            label: qsTr("Application")
                            value: Qt.application.name + " " + Qt.application.version
                        }

                        DetailRow {
                            Layout.fillWidth: true
                            theme: settingsPage.theme
                            label: qsTr("GPU")
                            value: nvidiaDetector.gpuFound ? nvidiaDetector.gpuName : qsTr("Not detected")
                        }

                        DetailRow {
                            Layout.fillWidth: true
                            theme: settingsPage.theme
                            label: qsTr("Driver")
                            value: nvidiaDetector.activeDriver
                        }

                        DetailRow {
                            Layout.fillWidth: true
                            theme: settingsPage.theme
                            label: qsTr("Session")
                            value: nvidiaDetector.sessionType.length > 0 ? nvidiaDetector.sessionType : qsTr("Unknown")
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        wrapMode: Text.Wrap
                        color: settingsPage.theme.textSoft
                        text: qsTr("Use the Driver page to refresh detection before copying any diagnostic context.")
                    }
                }

                SectionPanel {
                    Layout.fillWidth: true
                    theme: settingsPage.theme
                    title: qsTr("Workflow Guidance")
                    subtitle: qsTr("Recommended order of operations when changing drivers.")

                    Label {
                        Layout.fillWidth: true
                        wrapMode: Text.Wrap
                        color: settingsPage.theme.text
                        text: qsTr("1. Verify GPU detection and session type.\n2. Install or switch the driver stack.\n3. Check repository updates.\n4. Restart after successful package operations.")
                    }

                    StatusBanner {
                        Layout.fillWidth: true
                        theme: settingsPage.theme
                        tone: nvidiaDetector.secureBootEnabled ? "warning" : "info"
                        text: nvidiaDetector.secureBootEnabled
                              ? qsTr("Secure Boot is enabled. Kernel module signing may still be required after package installation.")
                              : qsTr("No Secure Boot blocker is currently reported by the detector.")
                    }
                }

                SectionPanel {
                    Layout.fillWidth: true
                    theme: settingsPage.theme
                    title: qsTr("About")
                    subtitle: qsTr("Project identity and current shell mode.")

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        DetailRow {
                            Layout.fillWidth: true
                            theme: settingsPage.theme
                            label: qsTr("Application")
                            value: Qt.application.name + " (" + Qt.application.version + ")"
                        }

                        DetailRow {
                            Layout.fillWidth: true
                            theme: settingsPage.theme
                            label: qsTr("Theme")
                            value: settingsPage.darkMode ? qsTr("System Dark") : qsTr("System Light")
                        }

                        DetailRow {
                            Layout.fillWidth: true
                            theme: settingsPage.theme
                            label: qsTr("Layout density")
                            value: settingsPage.compactMode ? qsTr("Compact") : qsTr("Comfort")
                        }

                        DetailRow {
                            Layout.fillWidth: true
                            theme: settingsPage.theme
                            label: qsTr("Advanced diagnostics")
                            value: settingsPage.showAdvancedInfo ? qsTr("Visible") : qsTr("Hidden")
                        }
                    }
                }
            }
        }
    }

    Connections {
        target: languageManager

        function onCurrentLanguageChanged() {
            for (let i = 0; i < languagePicker.model.length; ++i) {
                if (languagePicker.model[i].code === languageManager.currentLanguage) {
                    languagePicker.currentIndex = i;
                    break;
                }
            }
        }
    }
}
