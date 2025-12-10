import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import QtQuick.Studio.DesignEffects

Item {
    id: root
    width: 823
    height: 180

    property alias system_text: sysName.text
    property alias category_text: categoryName.text
    property alias distance: distance.text
    property string sourceImg: "This is a string"
    signal tapped()

    Rectangle {
        id: rectangleRoot
        width: root.width
        height: root.height
        color: "#2e2e2e"

        Image {
            id: bgImage
            source: root.sourceImg
            asynchronous: true
            fillMode: Image.PreserveAspectCrop
            width: rectangleRoot.width
            height: rectangleRoot.height
            opacity: status === Image.Ready ? 1 : 0
            Behavior on opacity { NumberAnimation { duration: 250 } }
        }

        Text {
            id: sysName
            color: "white"
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignTop
            font.family: "Anton"
            width: rectangleRoot.width
            height: rectangleRoot.height / 2
            padding: rectangleRoot.height / 8
            text: "Hydrae Sector DQ-Y b4"
            font.pixelSize: rectangleRoot.width / 16

            DesignEffect {
                effects: [
                    DesignDropShadow {
                    }
                ]
            }
        }

        Text {
            id: categoryName
            color: "white"
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            width: rectangleRoot.width
            height: rectangleRoot.height
            leftPadding: rectangleRoot.height / 8
            topPadding: rectangleRoot.height / 7
            text: "Trinary collidable or something"
            font.pixelSize: rectangleRoot.width / 29
            font.family: "Lora"

            DesignEffect {
                effects: [
                    DesignDropShadow {
                    }
                ]
            }
        }

        Text {
            id: distance
            color: "white"
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignBottom
            font.wordSpacing: 0
            width: rectangleRoot.width
            height: rectangleRoot.height
            leftPadding: rectangleRoot.height / 8
            topPadding: rectangleRoot.height / 7
            bottomPadding: rectangleRoot.height / 13
            text: "153.8 LY"
            font.pixelSize: rectangleRoot.width / 21
            font.family: "Lora"

            DesignEffect {
                effects: [
                    DesignDropShadow {
                    }
                ]
            }
        }

        RButton {
            id: clickButton
            width: rectangleRoot.width / 4
            height: rectangleRoot.height / 3
            anchors.right: rectangleRoot.right
            anchors.rightMargin: rectangleRoot.width * 0.025
            anchors.bottom: rectangleRoot.bottom
            anchors.bottomMargin: rectangleRoot.height * 0.133
            bcolor: "#ff7700"
            hoverColor: "dark orange"
            pressedColor: "orange"

            onTapped: {
                root.tapped()
            }

            Text {
                id: textButton
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.wordSpacing: 0
                width: clickButton.width
                height: clickButton.height
                text: "View"
                font.pixelSize: clickButton.width / 8
                anchors.centerIn: clickButton
                font.family: "Anton"

                DesignEffect {
                    effects: [
                        DesignDropShadow {
                        }
                    ]
                }
            }
        }
    }
}
