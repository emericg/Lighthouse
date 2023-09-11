import QtQuick
import QtQuick.Shapes

import ThemeEngine

Item {
    id: pogoSchematics
    anchors.centerIn: parent
    anchors.verticalCenterOffset: -width*0.20

    width: parent.width * 0.66
    height: width

    Connections {
        target: currentDevice
        function onButton1Pressed() { clickAnimation.start() }
    }

    ////////

    Shape {
        anchors.fill: parent
        antialiasing: true

        ShapePath {
            strokeColor: Theme.colorSeparator
            strokeWidth: 16
            fillColor: Theme.colorBackground
            scale: Qt.size(pogoSchematics.width/175, pogoSchematics.width/175)

            PathSvg {
                path:"m 88.000978,2.3480984 c -47.701952,0 -86.510369,38.8048916 -86.510369,86.5078436 0,20.509978 7.2996853,40.355718 20.577672,55.979698 l 0.328937,0.40399 c 40.814959,50.76595 46.052395,80.03377 46.089395,80.24977 l 0.122395,0.66316 c 1.693999,7.54099 5.547984,17.56226 19.39197,17.56226 13.842982,0 17.690322,-10.02327 19.384322,-17.56226 l 0.11474,-0.58694 c 0.047,-0.28999 5.28874,-29.56004 46.1047,-80.32599 l 0.26774,-0.32776 c 13.31698,-15.63699 20.63887,-35.51495 20.63887,-56.055928 0,-47.702952 -38.80842,-86.5078436 -86.510372,-86.5078436 z"
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

        Rectangle { // button
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

    ////////
}
