import QtQuick

import ThemeEngine

Rectangle {
    id: joystick

    width: 300
    height: 300
    radius: 300

    color: Theme.colorForeground
    border.width: 3
    border.color: Theme.colorSeparator

    property bool verticalOnly: false
    property bool horizontalOnly: false

    property real angle: 0
    property real distance: 0

    signal joystick_moved(double x, double y)

    ParallelAnimation {
        id: returnAnimation
        NumberAnimation { target: thumb.anchors; property: "horizontalCenterOffset";
                          to: 0; duration: 200; easing.type: Easing.OutSine }
        NumberAnimation { target: thumb.anchors; property: "verticalCenterOffset";
                          to: 0; duration: 200; easing.type: Easing.OutSine }
    }

    MultiPointTouchArea {
        id: toucharea
        anchors.fill: parent

        touchPoints: [
            TouchPoint { id: touchpoint }
        ]

        property real mouseX2: joystick.verticalOnly ? width * 0.5 : touchpoint.x
        property real mouseY2: joystick.horizontalOnly ? height * 0.5 : touchpoint.y
        property real fingerAngle: Math.atan2(mouseX2, mouseY2)
        property int mcx: mouseX2 - width * 0.5
        property int mcy: mouseY2 - height * 0.5
        property bool fingerInBounds: fingerDistance2 < distanceBound2
        property real fingerDistance2: mcx * mcx + mcy * mcy
        property real distanceBound: width * 0.5 - thumb.width * 0.5
        property real distanceBound2: distanceBound * distanceBound

        property double signal_x: (mouseX2 - joystick.width/2) / distanceBound
        property double signal_y: -(mouseY2 - joystick.height/2) / distanceBound

        onPressed: (points) => {
            returnAnimation.stop()
            moveAround()
        }
        onReleased: (points) => {
            returnAnimation.restart()
            joystick.joystick_moved(0, 0)
        }
        onCanceled: (points) => {
            returnAnimation.restart()
            joystick.joystick_moved(0, 0)
        }
        onTouchUpdated: (points) => {
            //
        }

        onUpdated: (points) => {
            moveAround()
        }

        function moveAround() {
            if (fingerInBounds) {
                thumb.anchors.horizontalCenterOffset = mcx
                thumb.anchors.verticalCenterOffset = mcy
            } else {
                var angle = Math.atan2(mcy, mcx)
                thumb.anchors.horizontalCenterOffset = Math.cos(angle) * distanceBound
                thumb.anchors.verticalCenterOffset = Math.sin(angle) * distanceBound
            }

            // Fire the signal to indicate the joystick has moved
            angle = Math.atan2(signal_y, signal_x)

            if (fingerInBounds) {
                joystick.joystick_moved(
                    joystick.verticalOnly ? 0 : Math.cos(angle) * Math.sqrt(fingerDistance2) / distanceBound,
                    joystick.horizontalOnly ? 0 : Math.sin(angle) * Math.sqrt(fingerDistance2) / distanceBound
                )
            } else {
                joystick.joystick_moved(
                    joystick.verticalOnly ? 0 : Math.cos(angle) * 1,
                    joystick.horizontalOnly ? 0 : Math.sin(angle) * 1
                )
            }
        }
    }

    Rectangle { // joystick thumb cap
        id: thumb
        anchors.centerIn: parent

        width: 64
        height: 64
        radius: 64

        color: touchpoint.pressed ? Theme.colorMaterialDeepOrange : Theme.colorSeparator
    }
}
