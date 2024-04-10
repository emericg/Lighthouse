import QtQuick
import QtQuick.Controls

import ThemeEngine

Item {
    id: virtualInput_gamepad

    Rectangle {
        id: joystick1

        anchors.centerIn: parent
        width: 320
        height: 320
        radius: 320

        color: Theme.colorForeground
        border.width: 3
        border.color: Theme.colorSeparator

        property bool verticalOnly: false
        property bool horizontalOnly: false

        signal joystick_moved(double x, double y)

        property real angle: 0
        property real distance: 0

        ParallelAnimation {
            id: returnAnimation
            NumberAnimation { target: thumb.anchors; property: "horizontalCenterOffset";
                              to: 0; duration: 200; easing.type: Easing.OutSine }
            NumberAnimation { target: thumb.anchors; property: "verticalCenterOffset";
                              to: 0; duration: 200; easing.type: Easing.OutSine }
        }

        MouseArea {
            id: mousearea_j1
            anchors.fill: parent

            property real mouseX2: joystick1.verticalOnly ? width * 0.5 : mouseX
            property real mouseY2: joystick1.horizontalOnly ? height * 0.5 : mouseY
            property real fingerAngle: Math.atan2(mouseX2, mouseY2)
            property int mcx: mouseX2 - width * 0.5
            property int mcy: mouseY2 - height * 0.5
            property bool fingerInBounds: fingerDistance2 < distanceBound2
            property real fingerDistance2: mcx * mcx + mcy * mcy
            property real distanceBound: width * 0.5 - thumb.width * 0.5
            property real distanceBound2: distanceBound * distanceBound

            property double signal_x: (mouseX2 - joystick1.width/2) / distanceBound
            property double signal_y: -(mouseY2 - joystick1.height/2) / distanceBound

            onPressed: {
                returnAnimation.stop()
            }
            onReleased: {
                returnAnimation.restart()
                joystick1.joystick_moved(0, 0)
            }

            onPositionChanged: {
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
                    joystick1.joystick_moved(
                        joystick1.verticalOnly ? 0 : Math.cos(angle) * Math.sqrt(fingerDistance2) / distanceBound,
                        joystick1.horizontalOnly ? 0 : Math.sin(angle) * Math.sqrt(fingerDistance2) / distanceBound
                    )
                } else {
                    joystick1.joystick_moved(
                        joystick1.verticalOnly ? 0 : Math.cos(angle) * 1,
                        joystick1.horizontalOnly ? 0 : Math.sin(angle) * 1
                    )
                }
            }
        }

        Rectangle {
            id: thumb
            anchors.centerIn: parent

            width: 64
            height: 64
            radius: 64

            color: mousearea_j1.containsPress ? Theme.colorMaterialDeepOrange : Theme.colorSeparator
            //opacity: 0.8
        }
    }
}
