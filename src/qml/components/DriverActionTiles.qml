import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "." as Components

Rectangle {
    id: root
    Layout.fillWidth: true
    radius: 14
    color: cardColor
    border.width: 1
    border.color: borderColor
    implicitHeight: actionLayout.implicitHeight + 24

    required property var nvidiaDetector
    required property var nvidiaInstaller
    required property var nvidiaUpdater

    property var theme: ({})
    property bool darkMode: false
    property real uiScale: 1.0

    property bool canManageDriverStack: false
    property bool nvidiaHardwareAvailable: false
    property bool waylandDriverFlowSupported: false
    property bool canRunDriverMutation: false
    property bool driverInstalledLocally: false
    property bool closedSourceDriverDetected: false
    property bool openSourceDriverDetected: false
    property string driverMutationBlockedReason: ""
    property string requestedDriverAction: ""
    property bool operationRunning: false
    property string pendingDriverStateText: ""

    signal refreshClicked()
    signal closedSourceClicked()
    signal openSourceClicked()
    signal deepCleanClicked()
    signal rebuildModulesClicked()
    signal updateClicked()
    signal restartClicked()

    readonly property color textColor: theme && theme.text ? theme.text : (darkMode ? "#F8FAFC" : "#0F172A")
    readonly property color softTextColor: theme && theme.textSoft ? theme.textSoft : (darkMode ? "#94A3B8" : "#64748B")
    readonly property color borderColor: theme && theme.border ? theme.border : (darkMode ? "#4D436B" : "#CBD5E1")
    readonly property color cardColor: theme && theme.cardStrong ? theme.cardStrong : (darkMode ? "#342D4A" : "#F1F5F9")
    readonly property color warningBg: theme && theme.warningBg ? theme.warningBg : (darkMode ? "#3A2E12" : "#FFFBEB")
    readonly property color warningColor: theme && theme.warning ? theme.warning : (darkMode ? "#FBBF24" : "#D97706")

    ColumnLayout {
        id: actionLayout
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Label {
                Layout.fillWidth: true
                text: qsTr("Driver Stack")
                color: root.textColor
                font.pixelSize: Math.round(18 * root.uiScale)
                font.weight: Font.DemiBold
            }

            Components.RefreshToolButton {
                id: refreshButton
                enabled: root.canManageDriverStack && !root.nvidiaUpdater.busy && !root.nvidiaInstaller.busy
                busy: root.nvidiaUpdater.busy
                theme: root.theme
                darkMode: root.darkMode
                uiScale: root.uiScale
                tooltip: qsTr("Rescan and check updates")
                onClicked: root.refreshClicked()
            }
        }

        Label {
            Layout.fillWidth: true
            text: root.nvidiaHardwareAvailable
                  ? qsTr("Manage proprietary NVIDIA modules and NVIDIA Open Kernel Modules. Switching modules requires Deep Clean first.")
                  : qsTr("NVIDIA driver controls are disabled because no NVIDIA GPU is detected. CPU, memory, and non-NVIDIA hardware monitoring remain available.")
            color: root.softTextColor
            wrapMode: Text.Wrap
        }

        Rectangle {
            visible: root.nvidiaHardwareAvailable && !root.waylandDriverFlowSupported
            Layout.fillWidth: true
            radius: 8
            color: root.warningBg
            border.width: 1
            border.color: root.warningColor
            implicitHeight: sessionWarning.implicitHeight + Math.round(16 * root.uiScale)

            Label {
                id: sessionWarning
                anchors.fill: parent
                anchors.margins: Math.round(8 * root.uiScale)
                text: qsTr("Managed installation and updates are available only in a Wayland session. Switch sessions, then refresh this page.")
                color: root.textColor
                wrapMode: Text.Wrap
                font.pixelSize: Math.round(11 * root.uiScale)
            }
        }

        GridLayout {
            Layout.fillWidth: true
            columns: width > 920 ? (root.pendingDriverStateText.length > 0 ? 5 : 4) : (width > 560 ? 2 : 1)
            columnSpacing: 10
            rowSpacing: 10

            Components.DriverActionTile {
                title: qsTr("NVIDIA Proprietary Module")
                subtitle: qsTr("NVIDIA Official Release • Proprietary")
                accentColor: "#10B981"
                activeBadge: root.closedSourceDriverDetected
                badgeText: qsTr("INSTALLED")
                busy: root.requestedDriverAction === "closed-install" && root.operationRunning
                enabled: root.canRunDriverMutation && !root.openSourceDriverDetected && !root.nvidiaInstaller.busy && !root.operationRunning
                disabledReason: root.driverMutationBlockedReason
                tooltipText: root.driverMutationBlockedReason.length > 0
                             ? root.driverMutationBlockedReason
                             : (root.openSourceDriverDetected
                                ? qsTr("Deep Clean is required before switching from NVIDIA Open Kernel Modules to the proprietary module.")
                                : qsTr("Install the proprietary NVIDIA kernel module (akmod-nvidia)."))
                theme: root.theme
                darkMode: root.darkMode
                uiScale: root.uiScale
                onClicked: root.closedSourceClicked()
            }

            Components.DriverActionTile {
                title: qsTr("NVIDIA Open Kernel Modules")
                subtitle: qsTr("akmod-nvidia-open")
                accentColor: "#0EA5E9"
                activeBadge: root.openSourceDriverDetected
                badgeText: qsTr("INSTALLED")
                busy: root.requestedDriverAction === "open-install" && root.operationRunning
                enabled: root.canRunDriverMutation && !root.closedSourceDriverDetected && !root.nvidiaInstaller.busy && !root.operationRunning
                disabledReason: root.driverMutationBlockedReason
                tooltipText: root.driverMutationBlockedReason.length > 0
                             ? root.driverMutationBlockedReason
                             : (root.closedSourceDriverDetected
                                ? qsTr("Deep Clean is required before switching from the proprietary module to NVIDIA Open Kernel Modules.")
                                : qsTr("Install NVIDIA Open Kernel Modules (akmod-nvidia-open)."))
                theme: root.theme
                darkMode: root.darkMode
                uiScale: root.uiScale
                onClicked: root.openSourceClicked()
            }

            Components.DriverActionTile {
                title: qsTr("Deep Clean")
                subtitle: qsTr("Remove NVIDIA packages and clear DNF cache")
                accentColor: "#F59E0B"
                busy: root.requestedDriverAction === "deep-clean" && root.operationRunning
                enabled: root.driverInstalledLocally && !root.nvidiaInstaller.busy && !root.operationRunning
                disabledReason: !root.driverInstalledLocally ? qsTr("An installed NVIDIA driver is required for cleanup.") : ""
                tooltipText: qsTr("Remove NVIDIA packages and clear cached repository metadata.")
                theme: root.theme
                darkMode: root.darkMode
                uiScale: root.uiScale
                onClicked: root.deepCleanClicked()
            }

            Components.DriverActionTile {
                title: qsTr("Rebuild Modules")
                subtitle: qsTr("Akmods & initramfs regeneration")
                accentColor: "#8B5CF6"
                busy: root.requestedDriverAction === "rebuild-modules" && root.operationRunning
                enabled: root.driverInstalledLocally && root.waylandDriverFlowSupported && !root.nvidiaInstaller.busy && !root.operationRunning
                disabledReason: !root.driverInstalledLocally
                                ? qsTr("An installed NVIDIA driver is required to rebuild modules.")
                                : (!root.waylandDriverFlowSupported ? qsTr("Managed NVIDIA maintenance requires a Wayland session.") : "")
                tooltipText: qsTr("Force-rebuilds akmod kernel modules and regenerates initramfs after kernel updates.")
                theme: root.theme
                darkMode: root.darkMode
                uiScale: root.uiScale
                onClicked: root.rebuildModulesClicked()
            }

            Components.DriverActionTile {
                visible: root.nvidiaUpdater.updateAvailable
                title: qsTr("Update NVIDIA Driver")
                subtitle: qsTr("Apply the latest compatible package version")
                accentColor: "#2563EB"
                busy: root.requestedDriverAction === "closed-update" && root.operationRunning
                enabled: visible && root.canRunDriverMutation && !root.nvidiaUpdater.busy && !root.operationRunning
                disabledReason: root.driverMutationBlockedReason
                tooltipText: root.driverMutationBlockedReason.length > 0
                             ? root.driverMutationBlockedReason
                             : qsTr("Install the available NVIDIA driver update.")
                theme: root.theme
                darkMode: root.darkMode
                uiScale: root.uiScale
                onClicked: root.updateClicked()
            }

            Components.DriverActionTile {
                visible: root.pendingDriverStateText.length > 0
                title: qsTr("Restart System")
                subtitle: qsTr("Reboot to activate new driver")
                accentColor: "#EF4444"
                enabled: visible && !root.operationRunning
                tooltipText: qsTr("System restart required to load newly installed kernel driver.")
                theme: root.theme
                darkMode: root.darkMode
                uiScale: root.uiScale
                onClicked: root.restartClicked()
            }
        }
    }
}
