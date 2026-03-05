import QtQuick
import QtQuick.Controls

// Ana pencere — ilerleyen branch'lerde sidebar + sayfa navigasyonu eklenecek
ApplicationWindow {
    id: root
    visible: true
    width: 900
    height: 600
    title: "ro-Control"

    Text {
        anchors.centerIn: parent
        text: "ro-Control — cmake-setup ✓"
        font.pixelSize: 24
    }
}
