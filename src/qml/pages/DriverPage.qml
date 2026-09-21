import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components" as Components

Item {
    id: page
    required property var nvidiaDetector
    required property var nvidiaInstaller
    required property var nvidiaUpdater
    property var systemInfo: null

    property var theme: ({})
    property bool darkMode: false
    property bool showAdvancedInfo: true
    property real uiScale: 1.0

    property string bannerText: qsTr("Ready")
    property string bannerTone: "info"
    property string operationSource: ""
    property string operationPhase: ""
    property string operationDetail: ""
    property bool operationActive: false
    property bool suppressPassiveStatus: true
    property bool activityFollowTail: operationLog ? operationLog.activityFollowTail : true
    property bool activityExpanded: operationLog ? operationLog.activityExpanded : false
    property string lastOperationText: ""
    property string lastOperationTone: "info"
    property string requestedDriverAction: ""
    property string pendingDriverStateText: ""
    property string pendingDriverStateTone: "info"
    property bool postOperationRefreshPending: false

    readonly property bool backendBusy: page.nvidiaInstaller.busy || page.nvidiaUpdater.busy
    readonly property bool operationRunning: page.operationActive || page.backendBusy
    readonly property bool remoteDriverCatalogAvailable: page.nvidiaUpdater.latestVersion.length > 0 || page.nvidiaUpdater.availableVersions.length > 0
    readonly property bool canInstallLatestRemoteDriver: page.nvidiaDetector.gpuFound && remoteDriverCatalogAvailable
    readonly property bool confirmedDriverInstalledLocally: page.nvidiaDetector.driverVersion.length > 0 || page.nvidiaDetector.driverPackageInstalled || page.nvidiaUpdater.currentVersion.length > 0
    readonly property string installedDriverSource: page.nvidiaDetector.installedDriverSource || "none"
    readonly property bool driverInstalledLocally: page.confirmedDriverInstalledLocally || page.pendingDriverStateText.length > 0 || page.installedDriverSource !== "none"
    readonly property bool virtualMachine: page.systemInfo && page.systemInfo.virtualMachine
    readonly property string virtualizationType: page.virtualMachine ? page.systemInfo.virtualizationType : ""
    readonly property bool canManageDriverStack: page.nvidiaDetector.gpuFound || page.driverInstalledLocally
    readonly property bool nvidiaHardwareAvailable: page.nvidiaDetector.gpuFound
    readonly property bool waylandDriverFlowSupported: page.nvidiaDetector.sessionType.toLowerCase() === "wayland"
    readonly property bool canRunDriverMutation: page.nvidiaHardwareAvailable && page.waylandDriverFlowSupported
    readonly property bool closedSourceDriverDetected: page.installedDriverSource === "closed-source" || page.installedDriverSource === "mixed"
    readonly property bool openSourceDriverDetected: page.installedDriverSource === "open-source" || page.installedDriverSource === "mixed"
    readonly property string installedVersionLabel: page.nvidiaDetector.driverVersion.length > 0 ? page.nvidiaDetector.driverVersion : page.nvidiaUpdater.currentVersion
    readonly property bool catalogAvailable: page.nvidiaUpdater.latestVersion.length > 0 || page.nvidiaUpdater.availableVersions.length > 0
    readonly property color driverVersionStatusColor: page.pendingDriverStateText.length > 0
                                                      ? (page.pendingDriverStateTone === "warning"
                                                         ? (theme && theme.warning ? theme.warning : page.softTextColor)
                                                         : (theme && theme.success ? theme.success : page.softTextColor))
                                                      : page.nvidiaUpdater.updateAvailable
                                                      ? (theme && theme.warning ? theme.warning : page.softTextColor)
                                                      : page.softTextColor

    readonly property color bgColor: theme && theme.card ? theme.card : (page.darkMode ? "#29233B" : "#FFFFFF")
    readonly property color cardColor: theme && theme.cardStrong ? theme.cardStrong : (page.darkMode ? "#342D4A" : "#F1F5F9")
    readonly property color borderColor: theme && theme.border ? theme.border : (page.darkMode ? "#4D436B" : "#CBD5E1")
    readonly property color textColor: theme && theme.text ? theme.text : (page.darkMode ? "#F8FAFC" : "#0F172A")
    readonly property color softTextColor: theme && theme.textSoft ? theme.textSoft : (page.darkMode ? "#94A3B8" : "#64748B")
    readonly property color infoBg: theme && theme.infoBg ? theme.infoBg : (page.darkMode ? "#1E2548" : "#EFF6FF")
    readonly property color successBg: theme && theme.successBg ? theme.successBg : (page.darkMode ? "#143828" : "#ECFDF5")
    readonly property color warningBg: theme && theme.warningBg ? theme.warningBg : (page.darkMode ? "#3A2E12" : "#FFFBEB")
    readonly property color dangerBg: theme && theme.dangerBg ? theme.dangerBg : (page.darkMode ? "#3D171E" : "#FEF2F2")
    readonly property color dangerColor: theme && theme.danger ? theme.danger : (page.darkMode ? "#F87171" : "#EF4444")
    readonly property color accentColor: theme && theme.accentA ? theme.accentA : (page.darkMode ? "#818CF8" : "#4F46E5")

    function classifyOperationPhase(message) {
        const lowered = (message || "").toLowerCase();
        if (lowered.indexOf("update") >= 0 || lowered.indexOf("version") >= 0)
            return qsTr("Update");
        if (lowered.indexOf("install") >= 0 || lowered.indexOf("remove") >= 0)
            return qsTr("Package");
        if (lowered.indexOf("kernel") >= 0 || lowered.indexOf("akmods") >= 0)
            return qsTr("Kernel");
        return qsTr("General");
    }

    function setOperationState(source, message, tone, running) {
        operationSource = source || "";
        operationDetail = message || "";
        operationPhase = classifyOperationPhase(operationDetail);
        bannerText = operationDetail.length > 0 ? operationDetail : qsTr("Ready");
        bannerTone = tone || "info";
        operationActive = !!running;
    }

    function finishOperation(source, success, message) {
        setOperationState(source, message, success ? "success" : "error", false);
    }

    function recordOperationResult(source, success, message) {
        lastOperationTone = success ? "success" : "error";
        lastOperationText = success
                            ? qsTr("%1 completed: %2").arg(source).arg(message)
                            : qsTr("%1 failed: %2").arg(source).arg(message);
    }

    function requestCancelDriverOperation() {
        if (page.nvidiaInstaller.busy)
            page.nvidiaInstaller.cancelOperation();
        if (page.nvidiaUpdater.busy)
            page.nvidiaUpdater.cancelOperation();
        page.setOperationState(qsTr("System"), qsTr("Cancel requested. Waiting for the active command to stop safely..."), "warning", true);
        page.appendLog(qsTr("System"), qsTr("Cancel requested. Waiting for the active command to stop safely..."));
    }

    function requestSystemRestart() {
        if (!page.systemInfo || !page.systemInfo.requestRestart()) {
            page.appendLog(qsTr("System"), qsTr("Restart request failed. Please restart the computer manually."));
            page.setOperationState(qsTr("System"), qsTr("Restart request failed. Please restart the computer manually."), "error", false);
            return;
        }

        page.appendLog(qsTr("System"), qsTr("Restart requested."));
        page.setOperationState(qsTr("System"), qsTr("Restart requested."), "success", false);
    }

    function markDriverActionStarted(action) {
        requestedDriverAction = action || "";
        pendingDriverStateText = "";
        pendingDriverStateTone = "info";
        postOperationRefreshPending = false;
    }

    function markDriverActionFinished(success) {
        if (!success)
            return;

        if (requestedDriverAction === "closed-install" || requestedDriverAction === "closed-update") {
            const version = page.nvidiaUpdater.latestVersion.length > 0 ? page.nvidiaUpdater.latestVersion : page.installedVersionLabel;
            pendingDriverStateText = version.length > 0
                                     ? qsTr("NVIDIA proprietary kernel module prepared: %1. Restart required.").arg(version)
                                     : qsTr("NVIDIA proprietary kernel module prepared. Restart required.");
            pendingDriverStateTone = "success";
        } else if (requestedDriverAction === "open-install") {
            pendingDriverStateText = qsTr("NVIDIA Open Kernel Modules prepared. Restart required.");
            pendingDriverStateTone = "success";
        } else if (requestedDriverAction === "deep-clean") {
            pendingDriverStateText = qsTr("NVIDIA driver cleanup completed. Restart recommended.");
            pendingDriverStateTone = "warning";
        }
    }

    function refreshAfterDriverAction() {
        postOperationRefreshPending = true;
        page.appendLog(qsTr("System"), qsTr("Refreshing driver status shown on this page..."));
        page.nvidiaDetector.refresh();
        page.suppressPassiveStatus = true;
        page.nvidiaUpdater.checkForUpdate();
        page.nvidiaInstaller.refreshProprietaryAgreement();
    }

    function closedSourceDriverAlreadyCurrent() {
        return page.driverInstalledLocally && page.catalogAvailable && !page.nvidiaUpdater.updateAvailable;
    }

    function openRestartDialog() {
        restartDialogLoader.active = true;
        restartDialogLoader.item.open();
    }

    function openMokGuide() {
        mokGuideDialogLoader.active = true;
        mokGuideDialogLoader.item.open();
    }

    function openCurrentDriverDialog() {
        currentDriverDialogLoader.active = true;
        currentDriverDialogLoader.item.open();
    }

    function openSourceSwitchDialog(target) {
        sourceSwitchDialogLoader.active = true;
        sourceSwitchDialogLoader.item.requestedTarget = target;
        sourceSwitchDialogLoader.item.open();
    }

    function openLicenseDialog() {
        licenseDialogLoader.active = true;
        licenseDialogLoader.item.open();
    }

    function openDriverActionInfo(action) {
        if (action === "update") {
            driverActionModalLoader.active = true;
            const modal = driverActionModalLoader.item;
            modal.actionKey = "update";
            modal.actionTitle = qsTr("Update NVIDIA Driver");
            modal.actionSubtitle = qsTr("Apply the latest version from configured repositories");
            modal.actionAccentColor = "#2563EB";
            modal.actionDescription = qsTr("Updates the installed NVIDIA driver package set, rebuilds its kernel module, and regenerates initramfs.");
            modal.actionPoints = [
                qsTr("Applies the latest compatible driver package version."),
                qsTr("Rebuilds the active NVIDIA kernel module with akmods."),
                qsTr("A restart is required before the updated kernel module is active.")
            ];
            modal.actionWarning = page.nvidiaDetector.secureBootEnabled
                    ? qsTr("Secure Boot is enabled. Confirm that a MOK key is enrolled before restarting, or the NVIDIA module may not load.")
                    : qsTr("The display session must be Wayland for this managed update flow.");
            modal.actionConfirmText = qsTr("Update Driver");
            modal.actionConfirmTone = "primary";
            modal.open();
            return;
        }
        if (action === "closed") {
            if (page.openSourceDriverDetected) {
                openSourceSwitchDialog("closed");
                return;
            }
            if (page.closedSourceDriverAlreadyCurrent()) {
                openCurrentDriverDialog();
                return;
            }
            driverActionModalLoader.active = true;
            const modal = driverActionModalLoader.item;
            modal.actionKey = "closed";
            modal.actionTitle = qsTr("NVIDIA Proprietary Kernel Module (akmod-nvidia)");
            modal.actionSubtitle = qsTr("Official Package • akmod-nvidia & CUDA libraries");
            modal.actionAccentColor = "#10B981";
            modal.actionDescription = qsTr("Installs NVIDIA's official proprietary binary driver stack. This stack delivers full hardware feature support including DLSS, CUDA acceleration, NVENC hardware encoding, OptiX, and Ray Tracing.");
            modal.actionPoints = [
                qsTr("Downloads and installs akmod-nvidia, xorg-x11-drv-nvidia, and core libraries."),
                qsTr("Compiles the proprietary kernel module against your active Linux kernel (%1).").arg(page.systemInfo ? page.systemInfo.kernelVersion : "active"),
                qsTr("Configures kernel parameters (nvidia-drm.modeset=1) and updates initramfs.")
            ];
            modal.actionWarning = page.nvidiaDetector.secureBootEnabled
                    ? qsTr("Secure Boot is enabled. Enroll the akmods MOK key before restarting, or the NVIDIA module may not load.")
                    : qsTr("A system reboot is required after installation to activate the kernel driver.");
            modal.actionConfirmText = qsTr("Install Proprietary Module");
            modal.actionConfirmTone = "primary";
            modal.open();
        } else if (action === "open") {
            if (page.closedSourceDriverDetected) {
                openSourceSwitchDialog("open");
                return;
            }
            driverActionModalLoader.active = true;
            const modal = driverActionModalLoader.item;
            modal.actionKey = "open";
            modal.actionTitle = qsTr("NVIDIA Open Kernel Modules (akmod-nvidia-open)");
            modal.actionSubtitle = qsTr("NVIDIA driver with open kernel modules");
            modal.actionAccentColor = "#0EA5E9";
            modal.actionDescription = qsTr("Installs NVIDIA Open Kernel Modules. This is not a full community graphics stack: NVIDIA userspace components remain part of the installation.");
            modal.actionPoints = [
                qsTr("Hardware Requirement: Turing (RTX 2000 / GTX 1600) or newer GPU architecture."),
                qsTr("Compiles akmod-nvidia-open module directly with standard Linux kernel interfaces."),
                qsTr("Updates bootloader image (dracut initramfs) with NVIDIA Open Kernel Modules.")
            ];
            modal.actionWarning = page.nvidiaDetector.secureBootEnabled
                    ? qsTr("Secure Boot is enabled. Enroll the akmods MOK key before restarting, or the NVIDIA module may not load.")
                    : qsTr("Older architectures (Pascal/Maxwell/GTX 1000 and earlier) are not supported by the open kernel module.");
            modal.actionConfirmText = qsTr("Install Open Kernel Modules");
            modal.actionConfirmTone = "primary";
            modal.open();
        } else if (action === "clean") {
            driverActionModalLoader.active = true;
            const modal = driverActionModalLoader.item;
            modal.actionKey = "clean";
            modal.actionTitle = qsTr("Deep Clean & Module Purge");
            modal.actionSubtitle = qsTr("Remove NVIDIA packages and clear cached metadata");
            modal.actionAccentColor = "#F59E0B";
            modal.actionDescription = qsTr("Removes installed NVIDIA driver packages and clears DNF's cached metadata so a later installation starts from a clean package state.");
            modal.actionPoints = [
                qsTr("Removes akmod-nvidia, akmod-nvidia-open, NVIDIA Xorg packages, and nvidia-settings."),
                qsTr("Runs 'dnf clean all' to remove cached repository metadata."),
                qsTr("Prepares system for a clean, conflict-free driver installation or stack switch.")
            ];
            modal.actionWarning = qsTr("Does not delete personal files or desktop settings. Restart is recommended after cleanup.");
            modal.actionConfirmText = qsTr("Run Deep Clean");
            modal.actionConfirmTone = "warning";
            modal.open();
        } else if (action === "rebuild") {
            driverActionModalLoader.active = true;
            const modal = driverActionModalLoader.item;
            modal.actionKey = "rebuild";
            modal.actionTitle = qsTr("Rebuild Kernel Modules & Initramfs");
            modal.actionSubtitle = qsTr("Akmods Force Recompilation & Dracut Image Regeneration");
            modal.actionAccentColor = "#8B5CF6";
            modal.actionDescription = qsTr("Forces a complete recompilation of NVIDIA kernel modules against the currently running Linux kernel and updates the early boot ramdisk (initramfs).");
            modal.actionPoints = [
                qsTr("Executes 'akmods --force' to recompile the driver for kernel: %1.").arg(page.systemInfo ? page.systemInfo.kernelVersion : "Linux"),
                qsTr("Executes 'dracut -f' to package the compiled modules into the bootloader image."),
                qsTr("Repairs NVIDIA module build failures that can follow Linux kernel updates.")
            ];
            modal.actionWarning = qsTr("This operation may take 30 to 90 seconds depending on system CPU speed.");
            modal.actionConfirmText = qsTr("Rebuild Modules");
            modal.actionConfirmTone = "primary";
            modal.open();
        }
    }

    function executeDriverAction(action) {
        if (action === "closed") {
            page.continueClosedSourceInstall();
        } else if (action === "open") {
            page.markDriverActionStarted("open-install");
            page.setOperationState(qsTr("Installer"), qsTr("Installing NVIDIA Open Kernel Modules..."), "info", true);
            page.nvidiaInstaller.installOpenSource();
        } else if (action === "clean") {
            page.markDriverActionStarted("deep-clean");
            page.setOperationState(qsTr("Installer"), qsTr("Cleaning NVIDIA artifacts..."), "info", true);
            page.nvidiaInstaller.deepClean();
        } else if (action === "rebuild") {
            page.markDriverActionStarted("rebuild-modules");
            page.setOperationState(qsTr("Installer"), qsTr("Rebuilding kernel modules & initramfs..."), "info", true);
            page.nvidiaInstaller.rebuildKernelModules();
        } else if (action === "update") {
            page.markDriverActionStarted("closed-update");
            page.setOperationState(qsTr("Updater"), qsTr("Updating NVIDIA driver..."), "info", true);
            page.nvidiaUpdater.applyUpdate();
        }
    }

    function beginClosedSourceInstall() {
        page.openDriverActionInfo("closed");
    }

    function beginOpenSourceInstall() {
        page.openDriverActionInfo("open");
    }

    function continueClosedSourceInstall() {
        if (page.nvidiaInstaller.proprietaryAgreementRequired) {
            openLicenseDialog();
        } else {
            page.markDriverActionStarted("closed-install");
            page.setOperationState(qsTr("Installer"), qsTr("Installing closed-source NVIDIA driver..."), "info", true);
            page.nvidiaInstaller.installProprietary(false);
        }
    }

    function driverVersionMainLabel() {
        if (page.pendingDriverStateText.length > 0)
            return page.pendingDriverStateText;
        if (page.installedVersionLabel.length > 0)
            return page.installedVersionLabel;
        if (page.nvidiaUpdater.latestVersion.length > 0)
            return qsTr("Latest available: %1").arg(page.nvidiaUpdater.latestVersion);
        if (page.catalogAvailable)
            return qsTr("Driver catalog loaded");
        return qsTr("Driver scan pending");
    }

    function gpuMainLabel() {
        if (page.nvidiaDetector.gpuFound)
            return page.nvidiaDetector.gpuName;
        if (page.nvidiaDetector.displayAdapterName.length > 0)
            return page.nvidiaDetector.displayAdapterName;
        return qsTr("No NVIDIA GPU");
    }

    function driverVersionStatusLabel() {
        if (!page.canManageDriverStack && page.virtualMachine)
            return qsTr("VM detected. NVIDIA passthrough required.");
        if (!page.canManageDriverStack)
            return qsTr("No NVIDIA GPU or driver.");
        if (page.postOperationRefreshPending)
            return qsTr("Refreshing status...");
        if (page.pendingDriverStateText.length > 0)
            return qsTr("Restart may be required.");
        if (page.installedVersionLabel.length > 0 && page.nvidiaUpdater.latestVersion.length > 0) {
            if (page.nvidiaUpdater.updateAvailable)
                return qsTr("New version available: %1").arg(page.nvidiaUpdater.latestVersion);
            return qsTr("Up to date.");
        }
        if (page.installedVersionLabel.length > 0)
            return qsTr("Installed.");
        if (page.nvidiaUpdater.latestVersion.length > 0)
            return qsTr("Latest: %1").arg(page.nvidiaUpdater.latestVersion);
        if (page.catalogAvailable)
            return qsTr("Catalog loaded.");
        return qsTr("Checking updates...");
    }

    function closedLicenseText() {
        const agreement = page.nvidiaInstaller.proprietaryAgreementText || "";
        if (agreement.length > 0)
            return agreement;
        return qsTr("Closed-source NVIDIA driver installation requires reviewing and accepting the NVIDIA license terms before ro-Control can start the closed-source install workflow.");
    }

    function appendLog(source, message) {
        operationLog.appendLog(source, message);
    }

    function resumeActivityFollow() {
        operationLog.resumeActivityFollow();
    }

    function refreshDriverState(showProgress) {
        if (showProgress !== false)
            page.setOperationState(qsTr("Updater"), qsTr("Checking official NVIDIA driver sources..."), "info", true);
        page.nvidiaDetector.refresh();
        page.nvidiaInstaller.refreshProprietaryAgreement();
        page.suppressPassiveStatus = true;
        page.nvidiaUpdater.checkForUpdate();
    }

    function driverSourceLabel() {
        if (page.installedDriverSource === "closed-source")
            return qsTr("NVIDIA Proprietary Kernel Module");
        if (page.installedDriverSource === "open-source")
            return qsTr("NVIDIA Open Kernel Modules");
        if (page.installedDriverSource === "mixed")
            return qsTr("Mixed driver state");
        return qsTr("Not detected");
    }

    function secureBootStatusDetail() {
        if (!page.nvidiaDetector.secureBootKnown)
            return qsTr("State unreadable.");
        return page.nvidiaDetector.secureBootEnabled
               ? qsTr("Signing may be required.")
               : qsTr("No signing required.");
    }

    function driverMutationBlockedReason() {
        if (!page.nvidiaHardwareAvailable)
            return qsTr("An NVIDIA GPU or NVIDIA passthrough device is required.");
        if (!page.waylandDriverFlowSupported)
            return qsTr("Managed NVIDIA installation and updates require a Wayland session.");
        return "";
    }

    ScrollView {
        id: pageScroll
        anchors.fill: parent
        clip: true
        contentWidth: availableWidth

        ColumnLayout {
            width: pageScroll.availableWidth
            spacing: 10

            Components.DriverOverviewCards {
                nvidiaDetector: page.nvidiaDetector
                nvidiaUpdater: page.nvidiaUpdater
                systemInfo: page.systemInfo
                theme: page.theme
                darkMode: page.darkMode
                uiScale: page.uiScale
                gpuMainLabel: page.gpuMainLabel()
                driverVersionMainLabel: page.driverVersionMainLabel()
                driverVersionStatusLabel: page.driverVersionStatusLabel()
                driverVersionStatusColor: page.driverVersionStatusColor
                secureBootStatusDetail: page.secureBootStatusDetail()
                virtualMachine: page.virtualMachine
                onOpenMokGuide: page.openMokGuide()
            }

            Components.DriverActionTiles {
                nvidiaDetector: page.nvidiaDetector
                nvidiaInstaller: page.nvidiaInstaller
                nvidiaUpdater: page.nvidiaUpdater
                theme: page.theme
                darkMode: page.darkMode
                uiScale: page.uiScale
                canManageDriverStack: page.canManageDriverStack
                nvidiaHardwareAvailable: page.nvidiaHardwareAvailable
                waylandDriverFlowSupported: page.waylandDriverFlowSupported
                canRunDriverMutation: page.canRunDriverMutation
                driverInstalledLocally: page.driverInstalledLocally
                closedSourceDriverDetected: page.closedSourceDriverDetected
                openSourceDriverDetected: page.openSourceDriverDetected
                driverMutationBlockedReason: page.driverMutationBlockedReason()
                requestedDriverAction: page.requestedDriverAction
                operationRunning: page.operationRunning
                pendingDriverStateText: page.pendingDriverStateText
                onRefreshClicked: page.refreshDriverState(true)
                onClosedSourceClicked: page.beginClosedSourceInstall()
                onOpenSourceClicked: page.beginOpenSourceInstall()
                onDeepCleanClicked: page.openDriverActionInfo("clean")
                onRebuildModulesClicked: page.openDriverActionInfo("rebuild")
                onUpdateClicked: page.openDriverActionInfo("update")
                onRestartClicked: page.openRestartDialog()
            }

            Components.DriverOperationLog {
                id: operationLog
                theme: page.theme
                darkMode: page.darkMode
                uiScale: page.uiScale
                operationRunning: page.operationRunning
                lastOperationText: page.lastOperationText
                lastOperationTone: page.lastOperationTone
                onCancelRequested: page.requestCancelDriverOperation()
            }
        }
    }

    Connections {
        target: page.nvidiaInstaller

        function onProgressMessage(message) {
            page.setOperationState(qsTr("Installer"), message, "info", true);
            page.appendLog(qsTr("Installer"), message);
        }

        function onInstallFinished(success, message) {
            page.finishOperation(qsTr("Installer"), success, message);
            page.recordOperationResult(qsTr("Installer"), success, message);
            page.markDriverActionFinished(success);
            page.appendLog(qsTr("Installer"), message);
            page.refreshAfterDriverAction();
        }

        function onRemoveFinished(success, message) {
            page.finishOperation(qsTr("Installer"), success, message);
            page.recordOperationResult(qsTr("Installer"), success, message);
            page.markDriverActionFinished(success);
            page.appendLog(qsTr("Installer"), message);
            page.refreshAfterDriverAction();
        }
    }

    Connections {
        target: page.nvidiaUpdater

        function onProgressMessage(message) {
            page.setOperationState(qsTr("Updater"), message, "info", true);
            page.appendLog(qsTr("Updater"), message);
        }

        function onCheckFinished(success, message) {
            if (success && page.suppressPassiveStatus && !page.nvidiaUpdater.updateAvailable)
                page.setOperationState(qsTr("Updater"), qsTr("Ready"), "info", false);
            else
                page.finishOperation(qsTr("Updater"), success, message);
            page.appendLog(qsTr("Updater"), message);
            if (success && !page.nvidiaUpdater.updateAvailable && page.installedVersionLabel.length > 0)
                page.appendLog(qsTr("Updater"), qsTr("Driver is already up to date."));
            if (page.postOperationRefreshPending) {
                page.postOperationRefreshPending = false;
                page.appendLog(qsTr("System"), success ? qsTr("Driver page status refreshed.") : qsTr("Driver page status refresh failed."));
            }
            page.suppressPassiveStatus = false;
        }

        function onUpdateFinished(success, message) {
            page.finishOperation(qsTr("Updater"), success, message);
            page.recordOperationResult(qsTr("Updater"), success, message);
            if (page.requestedDriverAction.length === 0)
                page.requestedDriverAction = "closed-update";
            page.markDriverActionFinished(success);
            page.appendLog(qsTr("Updater"), message);
            page.refreshAfterDriverAction();
        }
    }

    Component.onCompleted: {
        page.nvidiaDetector.refresh();
        page.nvidiaInstaller.refreshProprietaryAgreement();
        if (!page.catalogAvailable && page.nvidiaDetector.gpuFound) {
            page.nvidiaUpdater.checkForUpdate();
        }
    }

    Loader {
        id: restartDialogLoader
        active: false
        sourceComponent: Components.DriverRestartDialog {
            parent: page
            theme: page.theme
            darkMode: page.darkMode
            uiScale: page.uiScale
            secureBootEnabled: page.nvidiaDetector.secureBootEnabled
            onRestartRequested: page.requestSystemRestart()
            onOpenMokGuideRequested: page.openMokGuide()
        }
    }

    Loader {
        id: currentDriverDialogLoader
        active: false
        sourceComponent: Components.DriverCurrentWarningDialog {
            parent: page
            theme: page.theme
            darkMode: page.darkMode
            uiScale: page.uiScale
            onReinstallRequested: page.continueClosedSourceInstall()
        }
    }

    Loader {
        id: sourceSwitchDialogLoader
        active: false
        sourceComponent: Components.DriverSourceSwitchDialog {
            parent: page
            theme: page.theme
            darkMode: page.darkMode
            uiScale: page.uiScale
            canManageDriverStack: page.canManageDriverStack
            driverInstalledLocally: page.driverInstalledLocally
            operationRunning: page.operationRunning
            onDeepCleanRequested: {
                page.markDriverActionStarted("deep-clean");
                page.setOperationState(qsTr("Installer"), qsTr("Cleaning NVIDIA artifacts..."), "info", true);
                page.nvidiaInstaller.deepClean();
            }
        }
    }

    Loader {
        id: licenseDialogLoader
        active: false
        sourceComponent: Components.DriverProprietaryLicenseDialog {
            parent: page
            theme: page.theme
            darkMode: page.darkMode
            uiScale: page.uiScale
            licenseText: page.closedLicenseText()
            onAccepted: {
                page.markDriverActionStarted("closed-install");
                page.setOperationState(qsTr("Installer"), qsTr("Installing closed-source NVIDIA driver..."), "info", true);
                page.nvidiaInstaller.installProprietary(true);
            }
        }
    }

    Loader {
        id: driverActionModalLoader
        active: false
        sourceComponent: Components.DriverActionModalDialog {
            parent: page
            theme: page.theme
            darkMode: page.darkMode
            uiScale: page.uiScale
            secureBootEnabled: page.nvidiaDetector.secureBootEnabled
            onActionConfirmed: (key) => page.executeDriverAction(key)
        }
    }

    Loader {
        id: mokGuideDialogLoader
        active: false
        sourceComponent: Components.DriverMokGuideDialog {
            parent: page
            theme: page.theme
            darkMode: page.darkMode
            uiScale: page.uiScale
        }
    }
}
