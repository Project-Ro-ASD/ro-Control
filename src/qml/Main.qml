pragma ComponentBehavior: Bound

import QtQuick
import "style" as Style
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import "pages" as Pages

ApplicationWindow {
    id: root
    required property var nvidiaDetector
    required property var nvidiaInstaller
    required property var nvidiaUpdater
    required property var cpuMonitor
    required property var gpuMonitor
    required property var ramMonitor
    required property var fanController
    required property var powerController
    required property var healthGuard
    required property var systemInfo
    required property var languageManager
    required property var uiPreferences
    visible: true
    width: 1280
    height: 800
    minimumWidth: 960
    minimumHeight: 600
    title: qsTr("ro-Control")

    readonly property bool hasUiPreferences: root.uiPreferences !== null
    readonly property bool hasLanguageManager: root.languageManager !== null
    readonly property string themeMode: (hasUiPreferences && root.uiPreferences.themeMode)
                                        ? root.uiPreferences.themeMode
                                        : "light"
    readonly property bool darkMode: themeMode === "dark"
    readonly property bool showAdvancedInfo: (hasUiPreferences && root.uiPreferences.showAdvancedInfo !== undefined)
                                             ? root.uiPreferences.showAdvancedInfo
                                             : true
    readonly property real uiScale: Math.max(1.0, Qt.application.font.pixelSize / 13)
    readonly property var visibleLanguages: root.hasLanguageManager
                                            ? root.languageManager.availableLanguages
                                            : []
    readonly property var visibleThemeModes: root.hasUiPreferences
                                             ? root.uiPreferences.availableThemeModes
                                             : []

    function currentLanguageLabel() {
        return root.hasLanguageManager ? root.languageManager.currentLanguageLabel : qsTr("Language");
    }

    function currentThemeLabel() {
        if (root.themeMode === "dark")
            return qsTr("Dark");
        if (root.themeMode === "light")
            return qsTr("Light");
        return qsTr("Light");
    }

    function sessionLabel() {
        if (!root.nvidiaDetector || root.nvidiaDetector.sessionType.length === 0)
            return qsTr("Unknown");
        const value = root.nvidiaDetector.sessionType;
        return value.charAt(0).toUpperCase() + value.slice(1);
    }

    function deviceTypeLabel() {
        if (root.systemInfo && root.systemInfo.deviceType && root.systemInfo.deviceType.length > 0)
            return root.systemInfo.deviceType;
        return qsTr("Desktop");
    }

    function powerSummaryLabel() {
        const dev = root.deviceTypeLabel();
        if (!root.systemInfo || !root.systemInfo.powerSource || root.systemInfo.powerSource.length === 0)
            return dev;
        const pwr = root.systemInfo.powerSource;
        if (pwr === "AC / Desktop" || pwr === "AC Power")
            return dev + " • ⚡ " + qsTr("AC");
        if (pwr === "Battery")
            return dev + " • 🔋 " + qsTr("Battery");
        return dev + " • " + pwr;
    }

    function openSettingsMenu(sourceButton) {
        if (!sourceButton)
            return;
        settingsPopup.width = Math.max(Math.round(220 * root.uiScale), Math.min(root.width - Math.round(24 * root.uiScale), Math.round(320 * root.uiScale)));
        var mapped = sourceButton.mapToItem(root.contentItem, 0, sourceButton.height);
        settingsPopup.x = Math.max(Math.round(16 * root.uiScale),
                                   Math.min(mapped.x + sourceButton.width - settingsPopup.width,
                                            root.width - settingsPopup.width - Math.round(16 * root.uiScale)));
        settingsPopup.y = Math.max(Math.round(12 * root.uiScale), Math.min(mapped.y + Math.round(8 * root.uiScale), root.height - settingsPopup.height - Math.round(12 * root.uiScale)));
        settingsPopup.open();
    }

    function refreshAfterResume() {
        if (root.cpuMonitor)
            root.cpuMonitor.start();
        if (root.gpuMonitor)
            root.gpuMonitor.start();
        if (root.ramMonitor)
            root.ramMonitor.start();
        if (root.fanController)
            root.fanController.start();
    }

    onActiveChanged: {
        if (active)
            resumeRefreshTimer.restart();
    }

    Timer {
        id: resumeRefreshTimer
        interval: 350
        repeat: false
        onTriggered: root.refreshAfterResume()
    }

    Connections {
        target: root.systemInfo
        function onInfoChanged() {
            if (root.systemInfo && root.fanController) {
                root.fanController.syncPowerSource(root.systemInfo.onBattery);
            }
        }
    }

    Style.Theme {
        id: colors
        darkMode: root.darkMode
        followSystem: root.hasUiPreferences && root.uiPreferences.selectedThemeMode === "system"
    }

    palette.window: colors.window
    palette.base: colors.card
    palette.alternateBase: colors.cardStrong
    palette.button: colors.cardStrong
    palette.windowText: colors.text
    palette.text: colors.text
    palette.buttonText: colors.text
    palette.highlight: colors.accentA
    palette.highlightedText: colors.onAccent
    Binding {
        target: Style.Motion
        property: "factor"
        value: root.hasUiPreferences && root.uiPreferences.animationDurationFactor !== undefined
               ? root.uiPreferences.animationDurationFactor : 1
    }
    color: colors.window
    font.pixelSize: Math.round(13 * uiScale)
    Material.theme: darkMode ? Material.Dark : Material.Light
    Material.accent: colors.accentA
    Material.primary: colors.accentB
    Material.background: colors.window
    Material.foreground: colors.text

    Rectangle {
        anchors.fill: parent
        color: colors.window
    }

    ColumnLayout {
        objectName: "applicationSurface"
        anchors.fill: parent
        anchors.margins: Math.round(Style.RoTokens.spaceLg * root.uiScale)
        spacing: Math.round(Style.RoTokens.spaceMd * root.uiScale)

        Rectangle {
            Layout.fillWidth: true
            radius: Math.round(Style.RoTokens.radiusMd * root.uiScale)
            color: colors.shellAlt
            border.width: 1
            border.color: colors.border
            implicitHeight: Math.round(58 * root.uiScale)

            RowLayout {
                id: topBar
                anchors.fill: parent
                anchors.leftMargin: Math.round(Style.RoTokens.spaceMd * root.uiScale)
                anchors.rightMargin: Math.round(Style.RoTokens.spaceMd * root.uiScale)
                anchors.topMargin: Math.round(Style.RoTokens.spaceSm * root.uiScale)
                anchors.bottomMargin: Math.round(Style.RoTokens.spaceSm * root.uiScale)
                spacing: Math.round(Style.RoTokens.spaceMd * root.uiScale)

                TabBar {
                    id: tabBar
                    objectName: "navigationTabs"
                    Layout.alignment: Qt.AlignVCenter
                    spacing: Math.round(Style.RoTokens.spaceSm * root.uiScale)

                    background: Item {}

                    contentItem: ListView {
                        model: tabBar.contentModel
                        currentIndex: tabBar.currentIndex
                        spacing: tabBar.spacing
                        orientation: ListView.Horizontal
                        boundsBehavior: Flickable.StopAtBounds
                        highlightMoveDuration: 0
                        highlightResizeDuration: 0
                        highlight: null
                    }

                    TabButton {
                        id: driverTab
                        text: qsTr("Driver")
                        hoverEnabled: true
                        implicitHeight: Math.round(38 * root.uiScale)
                        implicitWidth: Math.max(Math.round(96 * root.uiScale), driverTabText.implicitWidth + Math.round(24 * root.uiScale))
                        contentItem: Text {
                            id: driverTabText
                            text: driverTab.text
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            color: tabBar.currentIndex === 0 ? colors.onAccent : colors.textMuted
                            font.pixelSize: Math.round(13 * root.uiScale)
                            font.weight: tabBar.currentIndex === 0 ? Font.DemiBold : Font.Medium
                        }
                        background: Rectangle {
                            radius: Math.round(Style.RoTokens.radiusSm * root.uiScale)
                            color: tabBar.currentIndex === 0 ? colors.accentA : (driverTab.hovered ? colors.cardStrong : "transparent")
                            border.width: driverTab.visualFocus ? 2 : 1
                            border.color: driverTab.visualFocus ? colors.accentA : tabBar.currentIndex === 0 ? colors.accentA : (driverTab.hovered ? colors.border : "transparent")
                        }
                    }

                    TabButton {
                        id: monitorTab
                        text: qsTr("Monitor")
                        hoverEnabled: true
                        implicitHeight: Math.round(38 * root.uiScale)
                        implicitWidth: Math.max(Math.round(96 * root.uiScale), monitorTabText.implicitWidth + Math.round(24 * root.uiScale))
                        contentItem: Text {
                            id: monitorTabText
                            text: monitorTab.text
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            color: tabBar.currentIndex === 1 ? colors.onAccent : colors.textMuted
                            font.pixelSize: Math.round(13 * root.uiScale)
                            font.weight: tabBar.currentIndex === 1 ? Font.DemiBold : Font.Medium
                        }
                        background: Rectangle {
                            radius: Math.round(Style.RoTokens.radiusSm * root.uiScale)
                            color: tabBar.currentIndex === 1 ? colors.accentA : (monitorTab.hovered ? colors.cardStrong : "transparent")
                            border.width: monitorTab.visualFocus ? 2 : 1
                            border.color: monitorTab.visualFocus ? colors.accentA : tabBar.currentIndex === 1 ? colors.accentA : (monitorTab.hovered ? colors.border : "transparent")
                        }
                    }

                    TabButton {
                        id: fanTab
                        text: qsTr("Cooling & Fans")
                        hoverEnabled: true
                        implicitHeight: Math.round(38 * root.uiScale)
                        implicitWidth: Math.max(Math.round(120 * root.uiScale), fanTabText.implicitWidth + Math.round(24 * root.uiScale))
                        contentItem: Text {
                            id: fanTabText
                            text: fanTab.text
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            color: tabBar.currentIndex === 2 ? colors.onAccent : colors.textMuted
                            font.pixelSize: Math.round(13 * root.uiScale)
                            font.weight: tabBar.currentIndex === 2 ? Font.DemiBold : Font.Medium
                        }
                        background: Rectangle {
                            radius: Math.round(Style.RoTokens.radiusSm * root.uiScale)
                            color: tabBar.currentIndex === 2 ? colors.accentA : (fanTab.hovered ? colors.cardStrong : "transparent")
                            border.width: fanTab.visualFocus ? 2 : 1
                            border.color: fanTab.visualFocus ? colors.accentA : tabBar.currentIndex === 2 ? colors.accentA : (fanTab.hovered ? colors.border : "transparent")
                        }
                    }

                    TabButton {
                        id: systemTab
                        text: qsTr("System")
                        hoverEnabled: true
                        implicitHeight: Math.round(38 * root.uiScale)
                        implicitWidth: Math.max(Math.round(96 * root.uiScale), systemTabText.implicitWidth + Math.round(24 * root.uiScale))
                        contentItem: Text {
                            id: systemTabText
                            text: systemTab.text
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            color: tabBar.currentIndex === 3 ? colors.onAccent : colors.textMuted
                            font.pixelSize: Math.round(13 * root.uiScale)
                            font.weight: tabBar.currentIndex === 3 ? Font.DemiBold : Font.Medium
                        }
                        background: Rectangle {
                            radius: Math.round(Style.RoTokens.radiusSm * root.uiScale)
                            color: tabBar.currentIndex === 3 ? colors.accentA : (systemTab.hovered ? colors.cardStrong : "transparent")
                            border.width: systemTab.visualFocus ? 2 : 1
                            border.color: systemTab.visualFocus ? colors.accentA : tabBar.currentIndex === 3 ? colors.accentA : (systemTab.hovered ? colors.border : "transparent")
                        }
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                RowLayout {
                    spacing: Math.round(Style.RoTokens.spaceMd * root.uiScale)
                    Layout.alignment: Qt.AlignVCenter

                    Rectangle {
                        Layout.preferredHeight: Math.round(38 * root.uiScale)
                        Layout.preferredWidth: Math.max(Math.round(136 * root.uiScale), Math.max(sessionText.implicitWidth, deviceBadgeText.implicitWidth) + Math.round(28 * root.uiScale))
                        radius: Math.round(Style.RoTokens.radiusSm * root.uiScale)
                        color: colors.card
                        border.width: 1
                        border.color: colors.border

                        ColumnLayout {
                            anchors.centerIn: parent
                            spacing: 0

                            Label {
                                id: sessionText
                                Layout.alignment: Qt.AlignHCenter
                                text: {
                                    if (!root.nvidiaDetector || root.nvidiaDetector.sessionType.length === 0)
                                        return qsTr("Wayland");
                                    var s = root.nvidiaDetector.sessionType;
                                    return s.charAt(0).toUpperCase() + s.slice(1);
                                }
                                color: colors.text
                                font.pixelSize: Math.round(12 * root.uiScale)
                                font.weight: Font.DemiBold
                            }

                            Label {
                                id: deviceBadgeText
                                Layout.alignment: Qt.AlignHCenter
                                text: {
                                    var dev = (root.systemInfo && root.systemInfo.deviceType && root.systemInfo.deviceType.length > 0)
                                              ? root.systemInfo.deviceType : qsTr("Desktop");
                                    var pwr = (root.systemInfo && root.systemInfo.powerSource && root.systemInfo.powerSource.length > 0)
                                              ? root.systemInfo.powerSource : "";
                                    if (pwr === "AC / Desktop" || pwr === "AC Power" || pwr === "AC")
                                        return dev + " • ⚡ " + qsTr("AC");
                                    if (pwr === "Battery")
                                        return dev + " • 🔋 " + qsTr("Battery");
                                    if (pwr.length > 0)
                                        return dev + " • " + pwr;
                                    return dev;
                                }
                                color: colors.text
                                elide: Text.ElideRight
                                font.pixelSize: Math.round(12 * root.uiScale)
                                font.weight: Font.DemiBold
                            }
                        }
                    }

                    ToolButton {
                        id: settingsButton
                        implicitWidth: Math.round(38 * root.uiScale)
                        implicitHeight: Math.round(38 * root.uiScale)
                        icon.source: "qrc:/qt/qml/rocontrol/assets/icon-settings.svg"
                        icon.width: Math.round(20 * root.uiScale)
                        icon.height: Math.round(20 * root.uiScale)
                        icon.color: (settingsButton.down || settingsPopup.visible) ? colors.onAccent : colors.text
                        display: AbstractButton.IconOnly
                        onClicked: {
                            if (settingsPopup.visible)
                                settingsPopup.close();
                            else
                                root.openSettingsMenu(settingsButton);
                        }
                        ToolTip {
                            id: settingsTip
                            visible: settingsButton.hovered && !settingsPopup.visible
                            text: qsTr("Settings")
                            delay: 300
                            timeout: 5000
                            topPadding: Math.round(Style.RoTokens.spaceSm * root.uiScale)
                            bottomPadding: Math.round(Style.RoTokens.spaceSm * root.uiScale)
                            leftPadding: Math.round(Style.RoTokens.spaceMd * root.uiScale)
                            rightPadding: Math.round(Style.RoTokens.spaceMd * root.uiScale)

                            contentItem: Label {
                                text: settingsTip.text
                                color: colors.text
                                font.pixelSize: Math.round(11 * root.uiScale)
                                font.weight: Font.Medium
                            }

                            background: Rectangle {
                                radius: Style.RoTokens.radiusSm
                                color: colors.card
                                border.width: 1
                                border.color: root.darkMode ? colors.border : colors.textMuted

                                Rectangle {
                                    anchors.left: parent.left
                                    anchors.top: parent.top
                                    anchors.bottom: parent.bottom
                                    anchors.margins: 3
                                    width: 3
                                    radius: 1.5
                                    color: colors.accentA
                                }
                            }
                        }

                        background: Rectangle {
                            radius: width / 2
                            color: settingsButton.down || settingsPopup.visible
                                   ? colors.accentA
                                   : colors.card
                            border.width: 1
                            border.color: colors.border
                        }
                    }
                }

            }
        }

        Popup {
            id: settingsPopup
            modal: false
            focus: true
            padding: Math.round(Style.RoTokens.spaceMd * root.uiScale)
            closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
            height: Math.min(root.height - Math.round(24 * root.uiScale), implicitHeight)

            background: Rectangle {
                radius: Math.round(Style.RoTokens.radiusMd * root.uiScale)
                color: colors.shellAlt
                border.width: 1
                border.color: colors.border
            }

            contentItem: ColumnLayout {
                spacing: Math.round(Style.RoTokens.spaceMd * root.uiScale)

                RowLayout {
                    Layout.fillWidth: true
                    spacing: Math.round(Style.RoTokens.spaceSm * root.uiScale)

                    Label {
                        text: qsTr("Settings")
                        color: colors.text
                        font.pixelSize: Math.round(14 * root.uiScale)
                        font.weight: Font.DemiBold
                        Layout.fillWidth: true
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: Math.round(Style.RoTokens.spaceSm * root.uiScale)

                    Label {
                        text: qsTr("Language")
                        color: colors.textSoft
                        font.pixelSize: Math.round(11 * root.uiScale)
                        font.weight: Font.DemiBold
                        Layout.fillWidth: true
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        columns: 2
                        columnSpacing: Math.round(6 * root.uiScale)
                        rowSpacing: Math.round(6 * root.uiScale)

                        Repeater {
                            model: root.visibleLanguages

                            delegate: Button {
                                id: langBtn
                                required property var modelData
                                Layout.fillWidth: true
                                implicitHeight: Math.round(34 * root.uiScale)
                                text: modelData.nativeLabel

                                background: Rectangle {
                                    radius: Math.round(Style.RoTokens.radiusSm * root.uiScale)
                                    color: (root.hasLanguageManager && !root.languageManager.followsSystem && langBtn.modelData.code === root.languageManager.currentLanguage)
                                           ? colors.accentA
                                           : (langBtn.hovered ? colors.cardStrong : colors.card)
                                    border.width: 1
                                    border.color: (root.hasLanguageManager && !root.languageManager.followsSystem && langBtn.modelData.code === root.languageManager.currentLanguage)
                                                  ? colors.accentA
                                                  : colors.border
                                }

                                contentItem: Text {
                                    text: langBtn.text
                                    color: (root.hasLanguageManager && !root.languageManager.followsSystem && langBtn.modelData.code === root.languageManager.currentLanguage)
                                           ? colors.onAccent
                                           : colors.text
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                    font.pixelSize: Math.round(12 * root.uiScale)
                                    font.weight: (root.hasLanguageManager && !root.languageManager.followsSystem && langBtn.modelData.code === root.languageManager.currentLanguage)
                                                 ? Font.DemiBold : Font.Medium
                                }

                                onClicked: {
                                    if (root.hasLanguageManager)
                                        root.languageManager.setCurrentLanguage(modelData.code);
                                }
                            }
                        }
                    }

                    Switch {
                        Layout.fillWidth: true
                        text: qsTr("Follow system language")
                        checked: root.hasLanguageManager && root.languageManager.followsSystem
                        enabled: root.hasLanguageManager
                        onToggled: if (root.hasLanguageManager) root.languageManager.setFollowsSystem(checked)
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: colors.border
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: Math.round(Style.RoTokens.spaceSm * root.uiScale)

                    Label {
                        text: qsTr("Theme")
                        color: colors.textSoft
                        font.pixelSize: Math.round(11 * root.uiScale)
                        font.weight: Font.DemiBold
                        Layout.fillWidth: true
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Math.round(Style.RoTokens.spaceSm * root.uiScale)

                        Repeater {
                            model: root.visibleThemeModes

                            delegate: Button {
                                id: themeBtn
                                required property var modelData
                                Layout.fillWidth: true
                                implicitHeight: Math.round(34 * root.uiScale)
                                text: modelData.label

                                background: Rectangle {
                                    radius: Math.round(Style.RoTokens.radiusSm * root.uiScale)
                                    color: (root.hasUiPreferences && themeBtn.modelData.code === root.uiPreferences.selectedThemeMode)
                                           ? colors.accentA
                                           : (themeBtn.hovered ? colors.cardStrong : colors.card)
                                    border.width: 1
                                    border.color: (root.hasUiPreferences && themeBtn.modelData.code === root.uiPreferences.selectedThemeMode)
                                                  ? colors.accentA
                                                  : colors.border
                                }

                                contentItem: Text {
                                    text: themeBtn.text
                                    color: (root.hasUiPreferences && themeBtn.modelData.code === root.uiPreferences.selectedThemeMode)
                                           ? colors.onAccent
                                           : colors.text
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                    font.pixelSize: Math.round(12 * root.uiScale)
                                    font.weight: (root.hasUiPreferences && themeBtn.modelData.code === root.uiPreferences.selectedThemeMode)
                                                 ? Font.DemiBold : Font.Medium
                                }

                                onClicked: {
                                    if (root.hasUiPreferences)
                                        root.uiPreferences.setThemeMode(modelData.code);
                                }
                            }
                        }
                    }

                    Switch {
                        Layout.fillWidth: true
                        text: qsTr("Show advanced information")
                        checked: root.hasUiPreferences && root.uiPreferences.showAdvancedInfo
                        enabled: root.hasUiPreferences
                        onToggled: if (root.hasUiPreferences) root.uiPreferences.setShowAdvancedInfo(checked)
                    }

                    Button {
                        Layout.fillWidth: true
                        text: qsTr("Reset appearance settings")
                        enabled: root.hasUiPreferences
                        onClicked: if (root.hasUiPreferences) root.uiPreferences.resetToDefaults()
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: Math.round(Style.RoTokens.radiusLg * root.uiScale)
            color: colors.card
            border.width: 1
            border.color: colors.border

            StackLayout {
                anchors.fill: parent
                anchors.margins: Math.round(Style.RoTokens.spaceMd * root.uiScale)
                currentIndex: tabBar.currentIndex

                Pages.DriverPage {
                    theme: colors
                    darkMode: root.darkMode
                    showAdvancedInfo: root.showAdvancedInfo
                    uiScale: root.uiScale
                    nvidiaDetector: root.nvidiaDetector
                    nvidiaInstaller: root.nvidiaInstaller
                    nvidiaUpdater: root.nvidiaUpdater
                    systemInfo: root.systemInfo
                }

                Pages.MonitorPage {
                    theme: colors
                    darkMode: root.darkMode
                    showAdvancedInfo: root.showAdvancedInfo
                    uiScale: root.uiScale
                    systemInfo: root.systemInfo
                    cpuMonitor: root.cpuMonitor
                    gpuMonitor: root.gpuMonitor
                    ramMonitor: root.ramMonitor
                    fanController: root.fanController
                    powerController: root.powerController
                    healthGuard: root.healthGuard
                    nvidiaDetector: root.nvidiaDetector
                }

                Pages.FanPage {
                    theme: colors
                    darkMode: root.darkMode
                    showAdvancedInfo: root.showAdvancedInfo
                    uiScale: root.uiScale
                    systemInfo: root.systemInfo
                    cpuMonitor: root.cpuMonitor
                    gpuMonitor: root.gpuMonitor
                    fanController: root.fanController
                }

                Pages.SystemPage {
                    theme: colors
                    darkMode: root.darkMode
                    showAdvancedInfo: root.showAdvancedInfo
                    uiScale: root.uiScale
                    systemInfo: root.systemInfo
                    cpuMonitor: root.cpuMonitor
                    gpuMonitor: root.gpuMonitor
                    ramMonitor: root.ramMonitor
                    nvidiaDetector: root.nvidiaDetector
                    powerController: root.powerController
                }

            }
        }
    }
}
