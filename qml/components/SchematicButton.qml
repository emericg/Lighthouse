import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects

import ThemeEngine 1.0

Rectangle {
    id: buttonSchematics
    anchors.centerIn: parent

    width: parent.width * 0.6
    height: parent.width * 0.6
    radius: parent.width * 0.1

    color: Theme.colorForeground
    border.width: 2
    border.color: (itemDeviceRemote.btnClicked === 1) ? Theme.colorPrimary: Theme.colorSeparator

    Connections {
        target: currentDevice
        function onButton1Pressed() { clickAnimation.start() }
        function onButton1DoublePressed() { clickAnimation.start() }
    }

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
