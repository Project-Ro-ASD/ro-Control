import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: settingsPage

    required property var theme
    property bool darkMode: false
    property bool compactMode: false
    property bool showAdvancedInfo: true
    readonly property string themeMode: uiPreferences.themeMode

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
                    title: qsTr("Appearance & Behavior")
                    subtitle: qsTr("Control theme, density and operator-focused interface behavior.")

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            Label {
                                text: qsTr("Theme mode")
                                font.bold: true
                                color: settingsPage.theme.text
                            }

                            Label {
                                text: qsTr("Choose whether the application follows the OS theme or uses an explicit light or dark shell.")
                                wrapMode: Text.Wrap
                                color: settingsPage.theme.textSoft
                                Layout.fillWidth: true
                            }
                        }

                        ComboBox {
                            id: themePicker
                            Layout.preferredWidth: 220
                            model: uiPreferences.availableThemeModes
                            textRole: "label"

                            Component.onCompleted: settingsPage.syncThemePicker()

                            onActivated: {
                                const selected = model[currentIndex];
                                if (selected && selected.code) {
                                    uiPreferences.setThemeMode(selected.code);
                                }
                            }
                        }
                    }

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
                            textRole: "nativeLabel"

                            Component.onCompleted: settingsPage.syncLanguagePicker()

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
                            checked: uiPreferences.compactMode
                            onToggled: uiPreferences.setCompactMode(checked)
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
                            checked: uiPreferences.showAdvancedInfo
                            onToggled: uiPreferences.setShowAdvancedInfo(checked)
                        }
                    }

                    Flow {
                        Layout.fillWidth: true
                        spacing: 8

                        InfoBadge {
                            text: qsTr("Language: ") + languageManager.currentLanguageLabel
                            backgroundColor: settingsPage.theme.cardStrong
                            foregroundColor: settingsPage.theme.text
                        }

                        InfoBadge {
                            text: settingsPage.themeMode === "system"
                                  ? qsTr("Theme: Follow System")
                                  : settingsPage.darkMode ? qsTr("Theme: Dark")
                                                          : qsTr("Theme: Light")
                            backgroundColor: settingsPage.theme.infoBg
                            foregroundColor: settingsPage.theme.text
                        }

                        InfoBadge {
                            text: uiPreferences.compactMode ? qsTr("Compact Active") : qsTr("Comfort Active")
                            backgroundColor: settingsPage.theme.cardStrong
                            foregroundColor: settingsPage.theme.text
                        }

                        InfoBadge {
                            text: uiPreferences.showAdvancedInfo ? qsTr("Advanced Visible") : qsTr("Advanced Hidden")
                            backgroundColor: settingsPage.theme.cardStrong
                            foregroundColor: settingsPage.theme.text
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        Label {
                            Layout.fillWidth: true
                            text: qsTr("Restore the recommended interface defaults if the shell starts to feel cluttered.")
                            wrapMode: Text.Wrap
                            color: settingsPage.theme.textSoft
                        }

                        ActionButton {
                            theme: settingsPage.theme
                            text: qsTr("Reset Interface Defaults")
                            onClicked: uiPreferences.resetToDefaults()
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
                            value: settingsPage.themeMode === "system"
                                   ? qsTr("Follow System")
                                   : settingsPage.darkMode ? qsTr("Dark")
                                                           : qsTr("Light")
                        }

                        DetailRow {
                            Layout.fillWidth: true
                            theme: settingsPage.theme
                            label: qsTr("Effective language")
                            value: languageManager.displayNameForLanguage(languageManager.effectiveLanguage)
                        }

                        DetailRow {
                            Layout.fillWidth: true
                            theme: settingsPage.theme
                            label: qsTr("Layout density")
                            value: uiPreferences.compactMode ? qsTr("Compact") : qsTr("Comfort")
                        }

                        DetailRow {
                            Layout.fillWidth: true
                            theme: settingsPage.theme
                            label: qsTr("Advanced diagnostics")
                            value: uiPreferences.showAdvancedInfo ? qsTr("Visible") : qsTr("Hidden")
                        }
                    }
                }
            }
        }
    }

    function syncLanguagePicker() {
        for (let i = 0; i < languagePicker.model.length; ++i) {
            if (languagePicker.model[i].code === languageManager.currentLanguage) {
                languagePicker.currentIndex = i;
                break;
            }
        }
    }

    function syncThemePicker() {
        for (let i = 0; i < themePicker.model.length; ++i) {
            if (themePicker.model[i].code === uiPreferences.themeMode) {
                themePicker.currentIndex = i;
                break;
            }
        }
    }

    Connections {
        target: languageManager

        function onCurrentLanguageChanged() {
            settingsPage.syncLanguagePicker()
        }
    }

    Connections {
        target: uiPreferences

        function onThemeModeChanged() {
            settingsPage.syncThemePicker()
        }
    }
}
