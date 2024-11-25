import QtQuick
import QtQuick.Controls

import ComponentLibrary
import Lighthouse

Item {
    id: virtualInput_gamepad

    function backAction() {
        screenDeviceList.loadScreen()
    }

    ////////

    property real x1: 0
    property real y1: 0
    property real x2: 0
    property real y2: 0

    property bool ba: false
    property bool bb: false
    property bool bx: false
    property bool by: false

    Timer {
        interval: 12
        running: (appContent.state === "VirtualInputs" && virtualInput_gamepad.visible)
        repeat: true
        onTriggered: {
            networkControls.sendGamepad(x1, y1, x2, y2, ba, bb, bx, by)
        }
    }

    ////////

    Column {
        anchors.centerIn: parent
        spacing: 24

        AnalogPad {
            id: leftPad
            rotation: 90
            onJoystick_moved: (x, y) => {
                //console.log("x1:" + x + " y1:" + y)
                x1 = x
                y1 = -y
            }
        }

        AnalogPad {
            id: rightPad
            rotation: 90
            onJoystick_moved: (x, y) => {
                //console.log("x2:" + x + " y2:" + y)
                x2 = x
                y2 = -y
            }
        }
    }

    ////////
}
