import QtQuick
import QtQuick.Controls
import QtQuick.Studio.DesignEffects

Window {
    id: root
    width: textbox.implicitWidth
    height: textbox.implicitHeight
    visible: true

    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool

    property string text: "Skaude AA-A h294 is in Spansh and sadly scanned"

    readonly property var screenGeo: (root.screen && root.screen.availableGeometry)
                                     ? root.screen.availableGeometry
                                     : { x: 0, y: 0, width: Screen.width, height: Screen.height }

    readonly property int shownY: (screenGeo.y + screenGeo.height) - height
    readonly property int hiddenY: (screenGeo.y + screenGeo.height) + 10
    readonly property int shownX: screenGeo.x

    Connections {
        target: SpanshPlotter

        function onShowWindow(message) {
            text = message
            showOverlay()
            timer.restart()
        }
    }

    x: shownX
    y: hiddenY

    Timer {
        id: timer
        repeat: false
        interval: 5000
        onTriggered: hideOverlay()
    }

    Behavior on y {
        NumberAnimation {
            duration: 300
            easing.type: Easing.OutCubic
        }
    }

    function showOverlay() {
        y = shownY
        raise()
    }

    function hideOverlay() {
        y = hiddenY
    }

    color: "transparent"

    Rectangle {
        id: overlay
        color: "#a6262626"
        anchors.fill: parent

        Text {
            id: textbox
            color: "#ffa55a"
            text: root.text
            font.pixelSize: 30
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.family: "Lora"
            fontSizeMode: Text.Fit

            DesignEffect {
                effects: [
                    DesignDropShadow {
                        color: "#000000"
                        spread: 0
                        offsetY: 0
                    }
                ]
            }
        }
    }
}
