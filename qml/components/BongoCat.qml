import QtQuick

Item {
    id: bongoCat

    implicitWidth: crop.width
    implicitHeight: crop.height

    // Area of the kit's 248x248 canvas to display, in canvas units
    property rect crop: Qt.rect(20, 20, 208, 208)

    property bool typing: false

    property string face: "cute"    // file name in qrc:/assets/bongocat/face/, without extension
    property string hat: ""         // file name in qrc:/assets/bongocat/hat/, empty for none

    property int pawInterval: 250   // delay between two paw slaps while typing, in ms

    // 0: idle, 1: left paw down, 2: right paw down
    property int frame: 0

    // Slap a single paw, for input that is not a typing state
    function tap() {
        frame = (frame === 1) ? 2 : 1
        tapTimer.restart()
    }

    onTypingChanged: if (!typing && !tapTimer.running) frame = 0

    Timer {
        interval: bongoCat.pawInterval
        repeat: true
        running: bongoCat.typing
        triggeredOnStart: true

        onTriggered: bongoCat.frame = (bongoCat.frame === 1) ? 2 : 1
    }
    Timer {
        id: tapTimer
        interval: 120

        onTriggered: if (!bongoCat.typing) bongoCat.frame = 0
    }

    ////////////////

    clip: true

    Item {
        id: canvas

        readonly property real ratio: Math.min(bongoCat.width / bongoCat.crop.width,
                                               bongoCat.height / bongoCat.crop.height)

        x: (bongoCat.width - bongoCat.crop.width * ratio) / 2 - (bongoCat.crop.x * ratio)
        y: (bongoCat.height - bongoCat.crop.height * ratio) / 2 - (bongoCat.crop.y * ratio)
        width: 248 * ratio
        height: 248 * ratio

        component Layer: Image {
            anchors.fill: parent
            sourceSize: Qt.size(width, height)
            visible: source.toString() !== ""
        }

        Layer { source: "qrc:/assets/bongocat/body/base.svg" }
        Layer { source: "qrc:/assets/bongocat/body/left-up.svg"; visible: bongoCat.frame !== 1 }
        Layer { source: "qrc:/assets/bongocat/body/left-down.svg"; visible: bongoCat.frame === 1 }
        Layer { source: bongoCat.face ? "qrc:/assets/bongocat/face/" + bongoCat.face + ".svg" : "" }
        Layer { source: bongoCat.hat ? "qrc:/assets/bongocat/hat/" + bongoCat.hat + ".svg" : "" }
        Layer { source: "qrc:/assets/bongocat/body/right-up.svg"; visible: bongoCat.frame !== 2 }
        Layer { source: "qrc:/assets/bongocat/body/right-down.svg"; visible: bongoCat.frame === 2 }
    }

    ////////////////
}
