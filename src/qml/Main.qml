import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    visible: true
    width: 1000
    height: 650
    minimumWidth: 800
    minimumHeight: 500
    title: "ro-Control"
    color: "#11111b"

    Row {
        anchors.fill: parent

        SidebarMenu {
            id: sidebar
            height: parent.height
        }

        StackLayout {
            id: stack
            width: parent.width - sidebar.width
            height: parent.height
            currentIndex: sidebar.currentIndex

            DriverPage {}
            MonitorPage {}
            SettingsPage {}
        }
    }
}
