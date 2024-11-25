import QtQuick
import QtQuick.Controls

import ComponentLibrary

Rectangle {
    id: dimmerSchematics
    anchors.centerIn: parent

    width: parent.width * 0.8
    height: parent.width * 0.8
    radius: 16

    color: Theme.colorForeground
    border.width: 2
    border.color: Theme.colorSeparator

    property int mmm: (width / 8)
    property int bbb: (width * 0.32)

    ////////////////////////////////////////////////////////////////////////////

    Connections {
        target: currentDevice
        function onButtonPressed() { dledAnimation.start() }
        function onButton1Pressed() { dimmerClickAnimation.start() }
        function onButton1LongPressed() { dimmerClickAnimation.start() }
        function onButton2Pressed() { dimmerLeftAnimation.start() }
        function onButton2LongPressed() { dimmerLeftAnimation.start() }
        function onButton3Pressed() { dimmerRightAnimation.start() }
        function onButton3LongPressed() { dimmerRightAnimation.start() }
    }

    ////////////////////////////////////////////////////////////////////////////

    Rectangle {
        id: led2
        anchors.top: parent.top
        anchors.topMargin: 16
        anchors.horizontalCenter: parent.horizontalCenter

        width: 6
        height: 6
        radius: 6

        color: Theme.colorSeparator

        ColorAnimation on color {
            id: dledAnimation
            loops: 1
            running: false

            duration: 333
            from: Theme.colorPrimary
            to: Theme.colorSeparator
        }
    }

    ////////

    Rectangle {
        anchors.centerIn: parent
        width: parent.width*0.6
        height: parent.width*0.6
        radius: parent.width*0.6

        color: Theme.colorBackground
        border.width: 2
        border.color: (itemDeviceRemote.btnClicked === 1) ? Theme.colorPrimary: Theme.colorSeparator

        ColorAnimation on color {
            id: dimmerClickAnimation
            loops: 1
            running: false

            duration: 333
            from: Theme.colorPrimary
            to: Theme.colorBackground
        }

        ////

        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (itemDeviceRemote.btnClicked === 1)
                    itemDeviceRemote.btnClicked = 0
                else
                    itemDeviceRemote.btnClicked = 1
            }
        }

        ////

        ProgressArc {
            anchors.top: parent.top
            anchors.topMargin: 24
            anchors.left: parent.left
            anchors.leftMargin: 24
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 24
            width: height

            arcCap: "round" // Qt.RoundCap
            arcOffset: -90
            arcSpan: 120
            arcWidth: 12

            value: 8
            valueMin: 0
            valueMax: 100

            arcColor: (itemDeviceRemote.btnClicked === 2) ? Theme.colorPrimary: Theme.colorSeparator
            backgroundColor: Theme.colorForeground

            ColorAnimation on arcColor {
                id: dimmerLeftAnimation
                loops: 1
                running: false

                duration: 333
                from: Theme.colorPrimary
                to: Theme.colorForeground
            }

            MouseArea {
                anchors.top: parent.top
                anchors.left: parent.left
                width: parent.width*0.3
                anchors.bottom: parent.bottom

                onClicked: {
                    if (itemDeviceRemote.btnClicked === 2)
                        itemDeviceRemote.btnClicked = 0
                    else
                        itemDeviceRemote.btnClicked = 2
                }
            }
        }

        ////

        ProgressArc {
            anchors.top: parent.top
            anchors.topMargin: 24
            anchors.left: parent.right
            anchors.leftMargin: -24
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 24
            width: height

            arcCap: "round" // Qt.RoundCap
            arcOffset: -90
            arcSpan: 120
            arcWidth: 12
            transform: Scale{ xScale: -1 } // invert the orientation

            value: 8
            valueMin: 0
            valueMax: 100

            arcColor: (itemDeviceRemote.btnClicked === 3) ? Theme.colorPrimary: Theme.colorSeparator
            backgroundColor: Theme.colorForeground

            MouseArea {
                anchors.top: parent.top
                anchors.left: parent.left
                width: parent.width*0.3
                anchors.bottom: parent.bottom

                onClicked: {
                    if (itemDeviceRemote.btnClicked === 3)
                        itemDeviceRemote.btnClicked = 0
                    else
                        itemDeviceRemote.btnClicked = 3
                }
            }

            ColorAnimation on arcColor {
                id: dimmerRightAnimation
                loops: 1
                running: false

                duration: 333
                from: Theme.colorPrimary
                to: Theme.colorForeground
            }
        }

        ////
    }

    ////////
}
