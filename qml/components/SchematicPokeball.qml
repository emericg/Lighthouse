import QtQuick

import ThemeEngine 1.0

Rectangle {
    id: pokeballSchematics
    anchors.centerIn: parent

    width: parent.width * 0.66
    height: parent.width * 0.66
    radius: parent.width

    color: Theme.colorForeground
    border.width: 16
    border.color: Theme.colorSeparator

    Connections {
        target: currentDevice
        function onButton1Pressed() { clickAnimation1.start() }
        function onButton2Pressed() { clickAnimation2.start() }
    }

    ////////

    Rectangle { // top button
        anchors.top: parent.top
        anchors.topMargin: -8
        anchors.horizontalCenter: parent.horizontalCenter

        width: parent.width * 0.33
        height: parent.width * 0.16
        radius: parent.width
        z: -1

        color: (itemDeviceRemote.btnClicked === 1) ? Theme.colorPrimary: Theme.colorForeground
        //border.width: 16
        //border.color: (itemDeviceRemote.btnClicked === 1) ? Theme.colorPrimary: Theme.colorSeparator

        ColorAnimation on color {
            id: clickAnimation1
            loops: 1
            running: false

            duration: 333
            from: Theme.colorPrimary
            to: Theme.colorForeground
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (itemDeviceRemote.btnClicked === 1)
                    itemDeviceRemote.btnClicked = 0
                else
                    itemDeviceRemote.btnClicked = 1
            }
        }
    }

    ////////

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        height: 16

        color: Theme.colorSeparator
    }

    ////////

    Rectangle { // joystick button
        anchors.centerIn: parent
        anchors.horizontalCenterOffset: currentDevice.axis_x * 100
        anchors.verticalCenterOffset: currentDevice.axis_y * 100

        width: parent.width * 0.33
        height: parent.width * 0.33
        radius: parent.width

        color: Theme.colorForeground
        border.width: 16
        border.color: (itemDeviceRemote.btnClicked === 2) ? Theme.colorPrimary: Theme.colorSeparator

        ColorAnimation on color {
            id: clickAnimation2
            loops: 1
            running: false

            duration: 333
            from: Theme.colorPrimary
            to: Theme.colorForeground
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (itemDeviceRemote.btnClicked === 2)
                    itemDeviceRemote.btnClicked = 0
                else
                    itemDeviceRemote.btnClicked = 2
            }
        }
    }

    ////////
}
