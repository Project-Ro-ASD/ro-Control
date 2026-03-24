import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: page

    required property var theme
    property bool darkMode: false
    property bool compactMode: false
    property bool showAdvancedInfo: true

    property string operationSource: ""
    property string operationDetail: ""
    property bool useOpenModules: false
    property bool deepCleanInstall: false
    property int selectedVersionIndex: -1
    property bool pendingInstallAfterClean: false
    readonly property bool wideLayout: width >= 1220

    readonly property bool backendBusy: nvidiaInstaller.busy || nvidiaUpdater.busy
    readonly property string currentDriverLabel: nvidiaDetector.driverVersion.length > 0
                                                  ? "nvidia-" + nvidiaDetector.driverVersion + " (" + page.driverFlavorLabel() + ")"
                                                  : qsTr("Not installed")

    function driverFlavorLabel() {
        if (nvidiaDetector.driverLoaded)
            return nvidiaDetector.nouveauActive ? qsTr("fallback") : (nvidiaDetector.activeDriver.indexOf("Open") >= 0 ? qsTr("open") : qsTr("proprietary"));
        if (nvidiaDetector.nouveauActive)
            return qsTr("fallback");
        return qsTr("inactive");
    }

    function cleanVersionLabel(rawVersion) {
        let normalized = (rawVersion || "").trim();
        if (normalized.length === 0)
            return "";

        const epochIndex = normalized.indexOf(":");
        if (epochIndex >= 0)
            normalized = normalized.substring(epochIndex + 1);

        const releaseMatch = normalized.match(/^([0-9]+(?:\.[0-9]+)+)/);
        if (releaseMatch && releaseMatch.length > 1)
            return releaseMatch[1];

        const hyphenIndex = normalized.indexOf("-");
        if (hyphenIndex > 0)
            return normalized.substring(0, hyphenIndex);

        return normalized;
    }

    function compareVersionLabels(leftVersion, rightVersion) {
        const leftParts = cleanVersionLabel(leftVersion).split(".");
        const rightParts = cleanVersionLabel(rightVersion).split(".");
        const maxLength = Math.max(leftParts.length, rightParts.length);

        for (let i = 0; i < maxLength; ++i) {
            const leftValue = i < leftParts.length ? parseInt(leftParts[i], 10) : 0;
            const rightValue = i < rightParts.length ? parseInt(rightParts[i], 10) : 0;

            if (leftValue > rightValue)
                return -1;
            if (leftValue < rightValue)
                return 1;
        }

        return 0;
    }

    function versionTag(option, index) {
        if (option.isInstalled)
            return qsTr("Installed");
        if (option.isLatest || index === 0)
            return qsTr("Latest");
        return qsTr("Available");
    }

    function buildAvailableVersionOptions(rawVersions) {
        const options = [];
        const seenLabels = {};
        const installedVersionLabel = cleanVersionLabel(nvidiaUpdater.currentVersion.length > 0
                                                        ? nvidiaUpdater.currentVersion
                                                        : nvidiaDetector.driverVersion);
        const latestVersionLabel = cleanVersionLabel(nvidiaUpdater.latestVersion);

        for (let i = 0; i < rawVersions.length; ++i) {
            const rawVersion = rawVersions[i];
            const displayVersion = cleanVersionLabel(rawVersion);
            if (displayVersion.length === 0 || seenLabels[displayVersion])
                continue;

            seenLabels[displayVersion] = true;
            options.push({
                rawVersion: rawVersion,
                displayVersion: displayVersion,
                isInstalled: installedVersionLabel.length > 0 && displayVersion === installedVersionLabel,
                isLatest: latestVersionLabel.length > 0 && displayVersion === latestVersionLabel
            });
        }

        options.sort(function(left, right) {
            return compareVersionLabels(left.displayVersion, right.displayVersion);
        });

        for (let j = 0; j < options.length; ++j)
            options[j].tag = versionTag(options[j], j);

        return options;
    }

    readonly property var availableVersionOptions: buildAvailableVersionOptions(nvidiaUpdater.availableVersions)

    function ensureSelection() {
        if (availableVersionOptions.length === 0) {
            selectedVersionIndex = -1;
            return;
        }

        if (selectedVersionIndex >= 0 && selectedVersionIndex < availableVersionOptions.length)
            return;

        for (let i = 0; i < availableVersionOptions.length; ++i) {
            if (availableVersionOptions[i].isInstalled) {
                selectedVersionIndex = i;
                return;
            }
        }

        selectedVersionIndex = 0;
    }

    function installSelectedVersion() {
        if (deepCleanInstall) {
            pendingInstallAfterClean = true;
            operationSource = qsTr("Installer");
            operationDetail = qsTr("Cleaning legacy driver leftovers...");
            nvidiaInstaller.deepClean();
            return;
        }

        if (useOpenModules) {
            operationSource = qsTr("Installer");
            operationDetail = qsTr("Switching to NVIDIA open kernel modules...");
            nvidiaInstaller.installOpenSource();
            return;
        }

        if (selectedVersionIndex >= 0 && selectedVersionIndex < availableVersionOptions.length) {
            operationSource = qsTr("Updater");
            operationDetail = qsTr("Applying selected NVIDIA driver version...");
            nvidiaUpdater.applyVersion(availableVersionOptions[selectedVersionIndex].rawVersion);
            return;
        }

        operationSource = qsTr("Installer");
        operationDetail = qsTr("Installing the proprietary NVIDIA driver...");
        nvidiaInstaller.installProprietary(true);
    }

    component ExpertHeaderRow: Rectangle {
        id: expertHeaderRow
        required property string title
        required property string value
        required property string markerText

        radius: 20
        color: page.theme.cardStrong
        implicitHeight: 68

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 16
            anchors.rightMargin: 18
            spacing: 14

            Rectangle {
                width: 40
                height: 40
                radius: 14
                color: page.theme.accentA

                Label {
                    anchors.centerIn: parent
                    text: expertHeaderRow.markerText
                    color: "#ffffff"
                    font.pixelSize: 15
                    font.weight: Font.DemiBold
                }
            }

            Label {
                color: page.theme.textSoft
                text: expertHeaderRow.title
                font.pixelSize: 14
                font.weight: Font.DemiBold
            }

            Item {
                Layout.fillWidth: true
            }

            Label {
                text: expertHeaderRow.value
                color: page.theme.text
                font.pixelSize: 15
                font.weight: Font.DemiBold
            }
        }
    }

    component VersionRow: Rectangle {
        id: versionRow
        required property int itemIndex
        required property var optionData

        readonly property bool selected: page.selectedVersionIndex === itemIndex

        radius: 20
        color: selected ? page.theme.infoBg : page.theme.cardStrong
        border.width: selected ? 2 : 0
        border.color: selected ? page.theme.accentA : "transparent"
        implicitHeight: 70

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 18
            anchors.rightMargin: 18
            spacing: 14

            Rectangle {
                width: 30
                height: 30
                radius: 15
                color: "transparent"
                border.width: 3
                border.color: versionRow.selected ? page.theme.accentA : page.theme.textSoft

                Rectangle {
                    anchors.centerIn: parent
                    width: 14
                    height: 14
                    radius: 7
                    visible: versionRow.selected
                    color: page.theme.accentA
                }
            }

            Label {
                text: optionData.displayVersion
                color: page.theme.text
                font.pixelSize: 18
                font.weight: Font.DemiBold
            }

            Rectangle {
                radius: 14
                color: optionData.isInstalled ? page.theme.successBg : page.theme.card
                implicitHeight: 30
                implicitWidth: tagLabel.implicitWidth + 20

                Label {
                    id: tagLabel
                    anchors.centerIn: parent
                    text: optionData.tag
                    color: optionData.isInstalled ? page.theme.success : page.theme.textSoft
                    font.pixelSize: 13
                    font.weight: Font.DemiBold
                }
            }

            Item {
                Layout.fillWidth: true
            }

            Rectangle {
                width: 38
                height: 38
                radius: 14
                color: page.theme.successBg

                Label {
                    anchors.centerIn: parent
                    text: "\u2713"
                    color: page.theme.success
                    font.pixelSize: 20
                    font.weight: Font.DemiBold
                }
            }
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: page.selectedVersionIndex = itemIndex
        }
    }

    ScrollView {
        id: pageScroll
        anchors.fill: parent
        clip: true
        contentWidth: availableWidth

        ColumnLayout {
            width: pageScroll.availableWidth
            spacing: 14

            Label {
                text: qsTr("Expert Driver Management")
                color: page.theme.text
                font.pixelSize: 30
                font.weight: Font.DemiBold
            }

            GridLayout {
                Layout.fillWidth: true
                columns: page.wideLayout ? 2 : 1
                columnSpacing: 16
                rowSpacing: 16

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    SectionPanel {
                        Layout.fillWidth: true
                        theme: page.theme
                        title: ""
                        subtitle: ""

                        ExpertHeaderRow {
                            Layout.fillWidth: true
                            title: qsTr("Current Driver")
                            value: page.currentDriverLabel
                            markerText: "D"
                        }

                        ExpertHeaderRow {
                            Layout.fillWidth: true
                            title: qsTr("Kernel Version")
                            value: systemInfo.kernelVersion.length > 0 ? systemInfo.kernelVersion : qsTr("Unavailable")
                            markerText: "K"
                        }
                    }

                    Label {
                        text: qsTr("Available Versions")
                        color: page.theme.text
                        font.pixelSize: 20
                        font.weight: Font.DemiBold
                    }

                    SectionPanel {
                        Layout.fillWidth: true
                        theme: page.theme
                        title: ""
                        subtitle: ""

                        Repeater {
                            model: page.availableVersionOptions.length

                            delegate: VersionRow {
                                Layout.fillWidth: true
                                itemIndex: index
                                optionData: page.availableVersionOptions[index]
                            }
                        }

                        Label {
                            Layout.fillWidth: true
                            visible: page.availableVersionOptions.length === 0
                            text: qsTr("No remote driver versions have been loaded yet. Use refresh to query the repository.")
                            wrapMode: Text.Wrap
                            color: page.theme.textSoft
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    Label {
                        text: qsTr("Configuration")
                        color: page.theme.text
                        font.pixelSize: 20
                        font.weight: Font.DemiBold
                    }

                    SectionPanel {
                        Layout.fillWidth: true
                        theme: page.theme
                        title: qsTr("Kernel Module Type")
                        subtitle: ""

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12

                            Rectangle {
                                Layout.fillWidth: true
                                implicitHeight: 64
                                radius: 18
                                color: !page.useOpenModules ? page.theme.infoBg : page.theme.card
                                border.width: !page.useOpenModules ? 2 : 1
                                border.color: !page.useOpenModules ? page.theme.accentA : page.theme.border

                                RowLayout {
                                    anchors.centerIn: parent
                                    spacing: 12

                                    Rectangle {
                                        width: 24
                                        height: 24
                                        radius: 12
                                        color: !page.useOpenModules ? page.theme.accentA : "transparent"
                                        border.width: 3
                                        border.color: !page.useOpenModules ? page.theme.accentA : page.theme.textSoft
                                    }

                                    Label {
                                        text: qsTr("Proprietary")
                                        color: page.theme.text
                                        font.pixelSize: 16
                                        font.weight: Font.DemiBold
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: page.useOpenModules = false
                                }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                implicitHeight: 64
                                radius: 18
                                color: page.useOpenModules ? page.theme.infoBg : page.theme.card
                                border.width: page.useOpenModules ? 2 : 1
                                border.color: page.useOpenModules ? page.theme.accentA : page.theme.border

                                RowLayout {
                                    anchors.centerIn: parent
                                    spacing: 12

                                    Rectangle {
                                        width: 24
                                        height: 24
                                        radius: 12
                                        color: page.useOpenModules ? page.theme.accentA : "transparent"
                                        border.width: 3
                                        border.color: page.useOpenModules ? page.theme.accentA : page.theme.textSoft
                                    }

                                    Label {
                                        text: qsTr("Open")
                                        color: page.theme.text
                                        font.pixelSize: 16
                                        font.weight: Font.DemiBold
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: page.useOpenModules = true
                                }
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        radius: 22
                        color: Qt.tint(page.theme.warningBg, "#22ffffff")
                        border.width: 1
                        border.color: Qt.tint(page.theme.warning, "#55ffffff")
                        implicitHeight: 82

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 18
                            anchors.rightMargin: 18
                            spacing: 14

                            Rectangle {
                                width: 28
                                height: 28
                                radius: 14
                                color: "transparent"
                                border.width: 3
                                border.color: page.deepCleanInstall ? page.theme.text : page.theme.textSoft

                                Rectangle {
                                    anchors.centerIn: parent
                                    width: 14
                                    height: 14
                                    radius: 7
                                    visible: page.deepCleanInstall
                                    color: page.theme.warning
                                }
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2

                                Label {
                                    text: qsTr("Deep Clean Installation")
                                    color: page.theme.text
                                    font.pixelSize: 15
                                    font.weight: Font.DemiBold
                                }

                                Label {
                                    text: qsTr("Remove all previous driver configurations and cache")
                                    color: page.theme.textSoft
                                    font.pixelSize: 13
                                }
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: page.deepCleanInstall = !page.deepCleanInstall
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 14

                        Rectangle {
                            Layout.fillWidth: true
                            implicitHeight: 64
                            radius: 20
                            gradient: Gradient {
                                orientation: Gradient.Horizontal
                                GradientStop { position: 0.0; color: "#4b87f4" }
                                GradientStop { position: 1.0; color: "#8d57f7" }
                            }
                            opacity: page.backendBusy ? 0.6 : 1.0

                            Label {
                                anchors.centerIn: parent
                                text: qsTr("Install Selected Version")
                                color: "#ffffff"
                                font.pixelSize: 16
                                font.weight: Font.DemiBold
                            }

                            MouseArea {
                                anchors.fill: parent
                                enabled: !page.backendBusy
                                cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                                onClicked: page.installSelectedVersion()
                            }
                        }

                        Rectangle {
                            Layout.preferredWidth: 190
                            implicitHeight: 64
                            radius: 20
                            gradient: Gradient {
                                orientation: Gradient.Horizontal
                                GradientStop { position: 0.0; color: "#ff644f" }
                                GradientStop { position: 1.0; color: "#ff4a4a" }
                            }
                            opacity: nvidiaInstaller.busy ? 0.6 : 1.0

                            Label {
                                anchors.centerIn: parent
                                text: qsTr("Remove All")
                                color: "#ffffff"
                                font.pixelSize: 16
                                font.weight: Font.DemiBold
                            }

                            MouseArea {
                                anchors.fill: parent
                                enabled: !nvidiaInstaller.busy
                                cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                                onClicked: {
                                    page.operationSource = qsTr("Installer");
                                    page.operationDetail = qsTr("Removing the NVIDIA driver...");
                                    nvidiaInstaller.remove();
                                }
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        ActionButton {
                            theme: page.theme
                            text: qsTr("Refresh Versions")
                            enabled: !page.backendBusy
                            onClicked: {
                                systemInfo.refresh();
                                nvidiaDetector.refresh();
                                nvidiaUpdater.checkForUpdate();
                                nvidiaUpdater.refreshAvailableVersions();
                            }
                        }

                        Item {
                            Layout.fillWidth: true
                        }

                        InfoBadge {
                            text: page.operationDetail.length > 0
                                  ? page.operationSource + ": " + page.operationDetail
                                  : qsTr("Ready")
                            backgroundColor: page.backendBusy ? page.theme.infoBg : page.theme.cardStrong
                            foregroundColor: page.theme.text
                        }
                    }
                }
            }
        }
    }

    Connections {
        target: nvidiaUpdater

        function onAvailableVersionsChanged() {
            page.ensureSelection();
        }

        function onUpdateFinished(success, message) {
            page.operationSource = success ? qsTr("Updater") : qsTr("Error");
            page.operationDetail = message;
            nvidiaDetector.refresh();
            systemInfo.refresh();
        }
    }

    Connections {
        target: nvidiaInstaller

        function onInstallFinished(success, message) {
            page.operationSource = success ? qsTr("Installer") : qsTr("Error");
            page.operationDetail = message;
            nvidiaDetector.refresh();
            nvidiaUpdater.checkForUpdate();
            nvidiaUpdater.refreshAvailableVersions();
            systemInfo.refresh();
        }

        function onRemoveFinished(success, message) {
            if (success && page.pendingInstallAfterClean) {
                page.pendingInstallAfterClean = false;
                page.deepCleanInstall = false;
                page.installSelectedVersion();
                return;
            }

            page.pendingInstallAfterClean = false;
            page.deepCleanInstall = false;
            page.operationSource = success ? qsTr("Installer") : qsTr("Error");
            page.operationDetail = message;
            nvidiaDetector.refresh();
            nvidiaUpdater.checkForUpdate();
            nvidiaUpdater.refreshAvailableVersions();
            systemInfo.refresh();
        }
    }

    Component.onCompleted: {
        systemInfo.refresh();
        nvidiaDetector.refresh();
        nvidiaUpdater.checkForUpdate();
        nvidiaUpdater.refreshAvailableVersions();
        ensureSelection();
    }
}
