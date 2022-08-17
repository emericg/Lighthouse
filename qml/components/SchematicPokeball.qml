import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects

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
        function onButton1Pressed() { clickAnimation.start() }
    }

    ////////

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        height: 16

        color: Theme.colorSeparator

        Rectangle {
            anchors.centerIn: parent

            width: parent.width * 0.33
            height: parent.width * 0.33
            radius: parent.width

            color: Theme.colorForeground
            border.width: 16
            border.color: (itemDeviceRemote.btnClicked === 1) ? Theme.colorPrimary: Theme.colorSeparator

            ColorAnimation on color {
                id: clickAnimation
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
    }
}
