import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root
    property var systemInfo: null
    property var theme: ({})
    property bool darkMode: false
    property real uiScale: 1.0
    property string reportText: ""
    property var reportSections: []
    property bool reportCopied: false
    property int reportViewMode: 0
    property string reportFilterText: ""

    signal copySuccess(string message)
    signal copyFailed(string message)
    signal requestRefresh()

    modal: true
    anchors.centerIn: parent
    width: Math.min(parent ? parent.width * 0.94 : 880, Math.round(880 * uiScale))
    height: Math.min(parent ? parent.height * 0.90 : 700, Math.round(700 * uiScale))
    padding: 0
    header: null
    footer: null

    enter: Transition {
        NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: 180; easing.type: Easing.OutQuad }
        NumberAnimation { property: "scale"; from: 0.96; to: 1.0; duration: 180; easing.type: Easing.OutQuad }
    }
    exit: Transition {
        NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: 140; easing.type: Easing.InQuad }
        NumberAnimation { property: "scale"; from: 1.0; to: 0.96; duration: 140; easing.type: Easing.InQuad }
    }

    readonly property color bgColor: theme && theme.card ? theme.card : (darkMode ? "#29233B" : "#FFFFFF")
    readonly property color cardColor: theme && theme.cardStrong ? theme.cardStrong : (darkMode ? "#342D4A" : "#F1F5F9")
    readonly property color borderColor: theme && theme.border ? theme.border : (darkMode ? "#4D436B" : "#CBD5E1")
    readonly property color textColor: theme && theme.text ? theme.text : (darkMode ? "#F8FAFC" : "#0F172A")
    readonly property color softTextColor: theme && theme.textSoft ? theme.textSoft : (darkMode ? "#94A3B8" : "#64748B")
    readonly property color accentColor: theme && theme.accentA ? theme.accentA : (darkMode ? "#818CF8" : "#4F46E5")
    readonly property color successColor: theme && theme.success ? theme.success : (darkMode ? "#4ADE80" : "#059669")
    readonly property color warningColor: theme && theme.warning ? theme.warning : (darkMode ? "#FBBF24" : "#D97706")

    background: Rectangle {
        radius: 16
        color: root.cardColor
        border.width: 1
        border.color: root.borderColor
    }

    // Cached computed filtered sections to prevent quadratic allocations on every layout pass
    readonly property var filteredSections: {
        const query = root.reportFilterText.trim().toLowerCase();
        const raw = root.reportSections || [];
        if (query.length === 0) {
            return raw;
        }
        const filtered = [];
        for (let s = 0; s < raw.length; s++) {
            const sec = raw[s];
            const validItems = [];
            const secItems = sec.items || [];
            for (let i = 0; i < secItems.length; i++) {
                const itm = secItems[i];
                if (!itm.value || itm.value.toString().trim().length === 0)
                    continue;
                if (itm.label.toLowerCase().indexOf(query) !== -1 ||
                    itm.value.toString().toLowerCase().indexOf(query) !== -1 ||
                    sec.title.toLowerCase().indexOf(query) !== -1) {
                    validItems.push(itm);
                }
            }
            if (validItems.length > 0) {
                filtered.push({ title: sec.title, icon: sec.icon, items: validItems });
            }
        }
        return filtered;
    }

    Timer {
        id: copiedResetTimer
        interval: 3000
        repeat: false
        onTriggered: root.reportCopied = false
    }

    function openWithData(text, sections) {
        root.reportText = text;
        root.reportSections = sections;
        root.reportCopied = false;
        root.open();
    }

    contentItem: ColumnLayout {
        spacing: 0

        // Header Bar
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: Math.round(64 * root.uiScale)
            color: root.darkMode ? "#2E2640" : "#F8FAFC"
            radius: 16

            Rectangle {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                height: 16
                color: parent.color
            }
            Rectangle {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                height: 1
                color: root.borderColor
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: Math.round(20 * root.uiScale)
                anchors.rightMargin: Math.round(16 * root.uiScale)
                spacing: Math.round(12 * root.uiScale)

                Rectangle {
                    implicitWidth: Math.round(36 * root.uiScale)
                    implicitHeight: Math.round(36 * root.uiScale)
                    radius: 8
                    color: root.darkMode ? "#312E81" : "#E0E7FF"
                    Label {
                        anchors.centerIn: parent
                        text: "▤"
                        color: root.accentColor
                        font.pixelSize: Math.round(18 * root.uiScale)
                        font.weight: Font.Bold
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2
                    Label {
                        text: qsTr("System Diagnostic Report")
                        color: root.textColor
                        font.pixelSize: Math.round(16 * root.uiScale)
                        font.weight: Font.DemiBold
                    }
                    Label {
                        text: qsTr("System hardware, kernel, driver and security telemetry snapshot")
                        color: root.softTextColor
                        font.pixelSize: Math.round(11 * root.uiScale)
                    }
                }

                ToolButton {
                    id: closeDiagnosticReportButton
                    text: "✕"
                    implicitWidth: Math.round(32 * root.uiScale)
                    implicitHeight: Math.round(32 * root.uiScale)
                    hoverEnabled: true
                    background: Rectangle {
                        radius: 8
                        color: closeDiagnosticReportButton.hovered ? (root.darkMode ? "#43385E" : "#E2E8F0") : "transparent"
                    }
                    contentItem: Text {
                        text: "✕"
                        color: closeDiagnosticReportButton.hovered ? root.textColor : root.softTextColor
                        font.pixelSize: Math.round(14 * root.uiScale)
                        font.weight: Font.Bold
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: root.close()
                }
            }
        }

        // Controls Toolbar
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: controlsRow.implicitHeight + Math.round(20 * root.uiScale)
            color: "transparent"

            RowLayout {
                id: controlsRow
                anchors.fill: parent
                anchors.margins: Math.round(16 * root.uiScale)
                spacing: Math.round(14 * root.uiScale)

                // View Mode Switcher
                Rectangle {
                    implicitHeight: Math.round(34 * root.uiScale)
                    implicitWidth: Math.min(Math.round(260 * root.uiScale), Math.max(Math.round(170 * root.uiScale), controlsRow.width))
                    radius: 8
                    color: root.darkMode ? "#241E34" : "#E2E8F0"
                    border.width: 1
                    border.color: root.borderColor

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 2
                        spacing: 2

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            radius: 6
                            color: root.reportViewMode === 0 ? (root.darkMode ? "#3E355B" : "#FFFFFF") : "transparent"
                            border.width: root.reportViewMode === 0 ? 1 : 0
                            border.color: root.reportViewMode === 0 ? (root.darkMode ? "#5B4E85" : "#CBD5E1") : "transparent"

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                activeFocusOnTab: true
                                onClicked: root.reportViewMode = 0
                                Keys.onReturnPressed: root.reportViewMode = 0
                                Keys.onSpacePressed: root.reportViewMode = 0
                            }
                            Label {
                                anchors.centerIn: parent
                                text: qsTr("Overview Cards")
                                color: root.reportViewMode === 0 ? root.textColor : root.softTextColor
                                font.pixelSize: Math.round(12 * root.uiScale)
                                font.weight: root.reportViewMode === 0 ? Font.DemiBold : Font.Normal
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            radius: 6
                            color: root.reportViewMode === 1 ? (root.darkMode ? "#3E355B" : "#FFFFFF") : "transparent"
                            border.width: root.reportViewMode === 1 ? 1 : 0
                            border.color: root.reportViewMode === 1 ? (root.darkMode ? "#5B4E85" : "#CBD5E1") : "transparent"

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                activeFocusOnTab: true
                                onClicked: root.reportViewMode = 1
                                Keys.onReturnPressed: root.reportViewMode = 1
                                Keys.onSpacePressed: root.reportViewMode = 1
                            }
                            Label {
                                anchors.centerIn: parent
                                text: qsTr("Code / Export")
                                color: root.reportViewMode === 1 ? root.textColor : root.softTextColor
                                font.pixelSize: Math.round(12 * root.uiScale)
                                font.weight: root.reportViewMode === 1 ? Font.DemiBold : Font.Normal
                            }
                        }
                    }
                }

                Item { Layout.fillWidth: true }

                // Search Filter in Overview Cards mode
                Rectangle {
                    visible: root.reportViewMode === 0 && controlsRow.width >= Math.round(430 * root.uiScale)
                    implicitHeight: Math.round(34 * root.uiScale)
                    implicitWidth: Math.min(controlsRow.width * 0.45, Math.round(240 * root.uiScale))
                    radius: 8
                    color: root.bgColor
                    border.width: 1
                    border.color: filterInput.activeFocus ? root.accentColor : root.borderColor

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: Math.round(10 * root.uiScale)
                        anchors.rightMargin: Math.round(8 * root.uiScale)
                        spacing: 6

                        Label {
                            text: "🔍"
                            font.pixelSize: Math.round(12 * root.uiScale)
                            color: root.softTextColor
                        }

                        TextInput {
                            id: filterInput
                            Layout.fillWidth: true
                            text: root.reportFilterText
                            color: root.textColor
                            font.pixelSize: Math.round(12 * root.uiScale)
                            verticalAlignment: TextInput.AlignVCenter
                            onTextChanged: root.reportFilterText = text

                            Text {
                                text: qsTr("Filter properties...")
                                color: root.softTextColor
                                font.pixelSize: Math.round(12 * root.uiScale)
                                visible: !filterInput.text && !filterInput.activeFocus
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }

                        ToolButton {
                            visible: root.reportFilterText.length > 0
                            text: "✕"
                            implicitWidth: Math.round(20 * root.uiScale)
                            implicitHeight: Math.round(20 * root.uiScale)
                            background: null
                            contentItem: Text {
                                text: "✕"
                                color: root.softTextColor
                                font.pixelSize: Math.round(11 * root.uiScale)
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            onClicked: {
                                root.reportFilterText = "";
                                filterInput.text = "";
                            }
                        }
                    }
                }

                // Controls in Code / Export view
                RowLayout {
                    visible: root.reportViewMode === 1
                    Layout.maximumWidth: Math.max(0, controlsRow.width - Math.round(180 * root.uiScale))
                    spacing: Math.round(14 * root.uiScale)

                    // Format Dropdown
                    RowLayout {
                        spacing: Math.round(8 * root.uiScale)
                        Label {
                            text: qsTr("Format:")
                            color: root.softTextColor
                            font.pixelSize: Math.round(12 * root.uiScale)
                            font.weight: Font.DemiBold
                        }

                        Rectangle {
                            id: formatSelectorButton
                            implicitHeight: Math.round(32 * root.uiScale)
                            implicitWidth: formatBtnRow.implicitWidth + Math.round(20 * root.uiScale)
                            radius: 8
                            color: formatMouse.containsMouse ? (root.darkMode ? "#342D4A" : "#E2E8F0") : root.bgColor
                            border.width: 1
                            border.color: (formatMouse.containsMouse || formatPopup.visible) ? root.accentColor : root.borderColor

                            MouseArea {
                                id: formatMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                activeFocusOnTab: true
                                onClicked: formatPopup.open()
                                Keys.onReturnPressed: formatPopup.open()
                                Keys.onSpacePressed: formatPopup.open()
                            }

                            RowLayout {
                                id: formatBtnRow
                                anchors.centerIn: parent
                                spacing: 6

                                Label {
                                    text: {
                                        const fmt = root.systemInfo ? root.systemInfo.diagnosticReportFormat : "markdown";
                                        if (fmt === "json") return "JSON";
                                        if (fmt === "plain") return qsTr("Plain Text");
                                        return "Markdown";
                                    }
                                    color: root.textColor
                                    font.pixelSize: Math.round(12 * root.uiScale)
                                    font.weight: Font.DemiBold
                                }

                                Label {
                                    text: "▾"
                                    color: root.accentColor
                                    font.pixelSize: Math.round(11 * root.uiScale)
                                    font.weight: Font.Bold
                                }
                            }

                            Popup {
                                id: formatPopup
                                y: formatSelectorButton.height + 4
                                width: Math.round(150 * root.uiScale)
                                padding: 4
                                closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

                                background: Rectangle {
                                    radius: 10
                                    color: root.bgColor
                                    border.width: 1
                                    border.color: root.borderColor
                                }

                                contentItem: ColumnLayout {
                                    spacing: 2
                                    Repeater {
                                        model: [
                                            { id: "markdown", label: "Markdown" },
                                            { id: "plain", label: qsTr("Plain Text") },
                                            { id: "json", label: "JSON" }
                                        ]

                                        delegate: AbstractButton {
                                            id: fmtItemBtn
                                            required property var modelData
                                            Layout.fillWidth: true
                                            implicitHeight: Math.round(32 * root.uiScale)
                                            hoverEnabled: true
                                            readonly property bool isSelected: root.systemInfo && root.systemInfo.diagnosticReportFormat === fmtItemBtn.modelData.id

                                            background: Rectangle {
                                                radius: 6
                                                color: fmtItemBtn.hovered
                                                       ? (root.darkMode ? "#43385E" : "#E2E8F0")
                                                       : (fmtItemBtn.isSelected ? (root.darkMode ? "#342D4A" : "#F1F5F9") : "transparent")
                                            }

                                            contentItem: RowLayout {
                                                anchors.fill: parent
                                                anchors.leftMargin: Math.round(10 * root.uiScale)
                                                anchors.rightMargin: Math.round(10 * root.uiScale)
                                                spacing: 6

                                                Label {
                                                    Layout.fillWidth: true
                                                    text: fmtItemBtn.modelData.label
                                                    color: fmtItemBtn.isSelected ? root.accentColor : (fmtItemBtn.hovered ? root.textColor : root.softTextColor)
                                                    font.pixelSize: Math.round(12 * root.uiScale)
                                                    font.weight: fmtItemBtn.isSelected ? Font.Bold : Font.Normal
                                                }

                                                Label {
                                                    visible: fmtItemBtn.isSelected
                                                    text: "✓"
                                                    color: root.accentColor
                                                    font.pixelSize: Math.round(12 * root.uiScale)
                                                    font.weight: Font.Bold
                                                }
                                            }

                                            onClicked: {
                                                if (root.systemInfo) {
                                                    root.systemInfo.setDiagnosticReportFormat(fmtItemBtn.modelData.id);
                                                    root.requestRefresh();
                                                }
                                                formatPopup.close();
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // Destination Dropdown
                    RowLayout {
                        spacing: Math.round(8 * root.uiScale)
                        Label {
                            text: qsTr("Action:")
                            color: root.softTextColor
                            font.pixelSize: Math.round(12 * root.uiScale)
                            font.weight: Font.DemiBold
                        }

                        Rectangle {
                            id: actionSelectorButton
                            implicitHeight: Math.round(32 * root.uiScale)
                            implicitWidth: actionBtnRow.implicitWidth + Math.round(20 * root.uiScale)
                            radius: 8
                            color: actionMouse.containsMouse ? (root.darkMode ? "#342D4A" : "#E2E8F0") : root.bgColor
                            border.width: 1
                            border.color: (actionMouse.containsMouse || actionPopup.visible) ? root.accentColor : root.borderColor

                            MouseArea {
                                id: actionMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                activeFocusOnTab: true
                                onClicked: actionPopup.open()
                                Keys.onReturnPressed: actionPopup.open()
                                Keys.onSpacePressed: actionPopup.open()
                            }

                            RowLayout {
                                id: actionBtnRow
                                anchors.centerIn: parent
                                spacing: 6

                                Label {
                                    text: {
                                        const dest = root.systemInfo ? root.systemInfo.diagnosticReportDestination : "preview";
                                        if (dest === "clipboard") return qsTr("Copy on Open");
                                        return qsTr("Preview");
                                    }
                                    color: root.textColor
                                    font.pixelSize: Math.round(12 * root.uiScale)
                                    font.weight: Font.DemiBold
                                }

                                Label {
                                    text: "▾"
                                    color: root.accentColor
                                    font.pixelSize: Math.round(11 * root.uiScale)
                                    font.weight: Font.Bold
                                }
                            }

                            Popup {
                                id: actionPopup
                                y: actionSelectorButton.height + 4
                                width: Math.round(160 * root.uiScale)
                                padding: 4
                                closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

                                background: Rectangle {
                                    radius: 10
                                    color: root.bgColor
                                    border.width: 1
                                    border.color: root.borderColor
                                }

                                contentItem: ColumnLayout {
                                    spacing: 2
                                    Repeater {
                                        model: [
                                            { id: "preview", label: qsTr("Preview") },
                                            { id: "clipboard", label: qsTr("Copy on Open") }
                                        ]

                                        delegate: AbstractButton {
                                            id: actItemBtn
                                            required property var modelData
                                            Layout.fillWidth: true
                                            implicitHeight: Math.round(32 * root.uiScale)
                                            hoverEnabled: true
                                            readonly property bool isSelected: root.systemInfo && root.systemInfo.diagnosticReportDestination === actItemBtn.modelData.id

                                            background: Rectangle {
                                                radius: 6
                                                color: actItemBtn.hovered
                                                       ? (root.darkMode ? "#43385E" : "#E2E8F0")
                                                       : (actItemBtn.isSelected ? (root.darkMode ? "#342D4A" : "#F1F5F9") : "transparent")
                                            }

                                            contentItem: RowLayout {
                                                anchors.fill: parent
                                                anchors.leftMargin: Math.round(10 * root.uiScale)
                                                anchors.rightMargin: Math.round(10 * root.uiScale)
                                                spacing: 6

                                                Label {
                                                    Layout.fillWidth: true
                                                    text: actItemBtn.modelData.label
                                                    color: actItemBtn.isSelected ? root.accentColor : (actItemBtn.hovered ? root.textColor : root.softTextColor)
                                                    font.pixelSize: Math.round(12 * root.uiScale)
                                                    font.weight: actItemBtn.isSelected ? Font.Bold : Font.Normal
                                                }

                                                Label {
                                                    visible: actItemBtn.isSelected
                                                    text: "✓"
                                                    color: root.accentColor
                                                    font.pixelSize: Math.round(12 * root.uiScale)
                                                    font.weight: Font.Bold
                                                }
                                            }

                                            onClicked: {
                                                if (root.systemInfo) {
                                                    root.systemInfo.setDiagnosticReportDestination(actItemBtn.modelData.id);
                                                }
                                                actionPopup.close();
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // Main Content Area
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: Math.round(16 * root.uiScale)
            Layout.rightMargin: Math.round(16 * root.uiScale)

            // View 0: Interactive System Snapshot Cards
            ScrollView {
                id: cardsScrollView
                visible: root.reportViewMode === 0
                anchors.fill: parent
                clip: true

                ColumnLayout {
                    width: cardsScrollView.availableWidth
                    spacing: Math.round(16 * root.uiScale)

                    Repeater {
                        model: root.filteredSections

                        delegate: Rectangle {
                            id: sectionCard
                            required property var modelData
                            Layout.fillWidth: true
                            radius: 12
                            color: root.bgColor
                            border.width: 1
                            border.color: root.borderColor
                            implicitHeight: secColumn.implicitHeight + Math.round(24 * root.uiScale)

                            ColumnLayout {
                                id: secColumn
                                anchors.fill: parent
                                anchors.margins: Math.round(12 * root.uiScale)
                                spacing: Math.round(10 * root.uiScale)

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: Math.round(8 * root.uiScale)

                                    Label {
                                        text: sectionCard.modelData.icon
                                        font.pixelSize: Math.round(15 * root.uiScale)
                                    }

                                    Label {
                                        text: sectionCard.modelData.title
                                        color: root.textColor
                                        font.pixelSize: Math.round(14 * root.uiScale)
                                        font.weight: Font.DemiBold
                                        Layout.fillWidth: true
                                    }

                                    Rectangle {
                                        implicitHeight: Math.round(20 * root.uiScale)
                                        implicitWidth: secCountLabel.implicitWidth + Math.round(12 * root.uiScale)
                                        radius: 10
                                        color: root.darkMode ? "#342D4A" : "#E2E8F0"
                                        Label {
                                            id: secCountLabel
                                            anchors.centerIn: parent
                                            text: (sectionCard.modelData.items ? sectionCard.modelData.items.length : 0) + " " + qsTr("items")
                                            color: root.softTextColor
                                            font.pixelSize: Math.round(10 * root.uiScale)
                                            font.weight: Font.DemiBold
                                        }
                                    }
                                }

                                GridLayout {
                                    Layout.fillWidth: true
                                    columns: width > 520 ? 2 : 1
                                    columnSpacing: Math.round(8 * root.uiScale)
                                    rowSpacing: Math.round(8 * root.uiScale)

                                    Repeater {
                                        model: sectionCard.modelData.items

                                        delegate: Rectangle {
                                            id: itemTile
                                            required property var modelData
                                            Layout.fillWidth: true
                                            implicitHeight: Math.round(62 * root.uiScale)
                                            radius: 8
                                            color: tileMouse.containsMouse
                                                   ? (root.darkMode ? "#383050" : "#F8FAFC")
                                                   : (root.darkMode ? "#2E2742" : "#F1F5F9")
                                            border.width: 1
                                            border.color: tileMouse.containsMouse ? root.accentColor : root.borderColor

                                            MouseArea {
                                                id: tileMouse
                                                anchors.fill: parent
                                                hoverEnabled: true
                                            }

                                            RowLayout {
                                                anchors.fill: parent
                                                anchors.leftMargin: Math.round(12 * root.uiScale)
                                                anchors.rightMargin: Math.round(12 * root.uiScale)
                                                spacing: Math.round(10 * root.uiScale)

                                                Label {
                                                    text: itemTile.modelData.icon || "•"
                                                    font.pixelSize: Math.round(16 * root.uiScale)
                                                }

                                                ColumnLayout {
                                                    Layout.fillWidth: true
                                                    spacing: 2

                                                    Label {
                                                        Layout.fillWidth: true
                                                        text: itemTile.modelData.label
                                                        color: root.softTextColor
                                                        font.pixelSize: Math.round(11 * root.uiScale)
                                                        font.weight: Font.DemiBold
                                                        elide: Text.ElideRight
                                                    }

                                                    Label {
                                                        Layout.fillWidth: true
                                                        text: itemTile.modelData.value
                                                        color: root.textColor
                                                        font.pixelSize: Math.round(13 * root.uiScale)
                                                        font.weight: Font.DemiBold
                                                        elide: Text.ElideRight
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // Empty Filter State
                    Rectangle {
                        visible: root.filteredSections.length === 0
                        Layout.fillWidth: true
                        implicitHeight: Math.round(160 * root.uiScale)
                        radius: 12
                        color: root.bgColor
                        border.width: 1
                        border.color: root.borderColor

                        ColumnLayout {
                            anchors.centerIn: parent
                            spacing: Math.round(8 * root.uiScale)

                            Label {
                                Layout.alignment: Qt.AlignHCenter
                                text: "🔍"
                                font.pixelSize: Math.round(24 * root.uiScale)
                            }

                            Label {
                                Layout.alignment: Qt.AlignHCenter
                                text: qsTr("No matching properties found")
                                color: root.textColor
                                font.pixelSize: Math.round(14 * root.uiScale)
                                font.weight: Font.DemiBold
                            }

                            Label {
                                Layout.alignment: Qt.AlignHCenter
                                text: qsTr("Try a different search term or clear the filter.")
                                color: root.softTextColor
                                font.pixelSize: Math.round(12 * root.uiScale)
                            }

                            Button {
                                Layout.alignment: Qt.AlignHCenter
                                text: qsTr("Clear Filter")
                                onClicked: {
                                    root.reportFilterText = "";
                                    filterInput.text = "";
                                }
                            }
                        }
                    }
                }
            }

            // View 1: Formatted Code / Export Viewer
            Rectangle {
                visible: root.reportViewMode === 1
                anchors.fill: parent
                radius: 10
                color: root.bgColor
                border.width: 1
                border.color: root.borderColor

                ScrollView {
                    anchors.fill: parent
                    anchors.margins: Math.round(10 * root.uiScale)
                    clip: true

                    TextArea {
                        id: diagnosticReportText
                        text: root.reportText
                        readOnly: true
                        selectByMouse: true
                        textFormat: {
                            const fmt = root.systemInfo ? root.systemInfo.diagnosticReportFormat : "markdown";
                            return fmt === "markdown" ? TextEdit.MarkdownText : TextEdit.PlainText;
                        }
                        wrapMode: {
                            const fmt = root.systemInfo ? root.systemInfo.diagnosticReportFormat : "markdown";
                            return fmt === "json" ? TextEdit.NoWrap : TextEdit.Wrap;
                        }
                        color: root.textColor
                        font.family: {
                            const fmt = root.systemInfo ? root.systemInfo.diagnosticReportFormat : "markdown";
                            return fmt === "json" ? "monospace" : ""
                        }
                        font.pixelSize: Math.round(13 * root.uiScale)
                        selectedTextColor: "#FFFFFF"
                        selectionColor: root.accentColor
                        padding: Math.round(8 * root.uiScale)
                        background: null
                    }
                }
            }
        }

        // Bottom Footer Bar
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: Math.round(64 * root.uiScale)
            color: "transparent"

            RowLayout {
                anchors.fill: parent
                anchors.margins: Math.round(16 * root.uiScale)
                spacing: Math.round(12 * root.uiScale)

                Rectangle {
                    visible: root.reportCopied
                    implicitHeight: Math.round(32 * root.uiScale)
                    implicitWidth: copyFeedbackRow.implicitWidth + Math.round(16 * root.uiScale)
                    radius: 8
                    color: root.darkMode ? "#143828" : "#ECFDF5"
                    border.width: 1
                    border.color: root.successColor

                    RowLayout {
                        id: copyFeedbackRow
                        anchors.centerIn: parent
                        spacing: 6
                        Label {
                            text: "✓"
                            color: root.successColor
                            font.weight: Font.Bold
                            font.pixelSize: Math.round(13 * root.uiScale)
                        }
                        Label {
                            text: qsTr("Copied to clipboard!")
                            color: root.successColor
                            font.pixelSize: Math.round(12 * root.uiScale)
                            font.weight: Font.DemiBold
                        }
                    }
                }

                Item { Layout.fillWidth: true }

                Button {
                    id: closeReportBtn
                    text: qsTr("Close")
                    implicitHeight: Math.round(38 * root.uiScale)
                    implicitWidth: Math.round(90 * root.uiScale)
                    hoverEnabled: true

                    background: Rectangle {
                        radius: 8
                        color: closeReportBtn.hovered
                               ? (root.darkMode ? "#3B3156" : "#E2E8F0")
                               : root.bgColor
                        border.width: 1
                        border.color: closeReportBtn.hovered ? root.accentColor : root.borderColor
                    }

                    contentItem: Label {
                        text: closeReportBtn.text
                        color: closeReportBtn.hovered ? root.textColor : root.softTextColor
                        font.pixelSize: Math.round(13 * root.uiScale)
                        font.weight: Font.DemiBold
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: root.close()
                }

                Button {
                    id: copyReportBtn
                    text: qsTr("Copy Full Report")
                    implicitHeight: Math.round(38 * root.uiScale)
                    implicitWidth: copyLabel.implicitWidth + Math.round(28 * root.uiScale)
                    hoverEnabled: true

                    background: Rectangle {
                        radius: 8
                        color: copyReportBtn.down ? Qt.darker(root.accentColor, 1.15)
                                                  : (copyReportBtn.hovered ? Qt.lighter(root.accentColor, 1.1) : root.accentColor)
                        border.width: 1
                        border.color: root.accentColor
                    }

                    contentItem: Label {
                        id: copyLabel
                        text: copyReportBtn.text
                        color: "#FFFFFF"
                        font.pixelSize: Math.round(13 * root.uiScale)
                        font.weight: Font.Bold
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        root.reportCopied = root.systemInfo && root.systemInfo.copyToClipboard(root.reportText);
                        if (root.reportCopied) {
                            root.copySuccess(qsTr("Diagnostic report copied to clipboard."));
                            copiedResetTimer.restart();
                        } else {
                            root.copyFailed(qsTr("The report could not be copied. Select and copy the text manually."));
                        }
                    }
                }
            }
        }
    }
}
