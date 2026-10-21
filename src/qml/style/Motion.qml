pragma Singleton
import QtQuick

QtObject {
    property real factor: 1
    readonly property int fastMs: Math.round(RoTokens.motionFastMs * factor)
}
