import QtQuick
import QtQuick.Effects
import QtQuick.Controls

import ComponentLibrary

Rectangle {
    id: control

    width: btnSize*3 + border.width*2
    height: btnSize + border.width*2
    radius: Theme.componentRadius

    border.width: 2
    border.color: colorBorder

    // settings
    property int btnSize: 64
    property bool btnSeparator: false

    // colors
    property color colorBackground: Theme.colorBackground
    property color colorHighlight: Theme.colorMaterialBlue
    property color colorBorder: Theme.colorComponentBorder
    property color colorIcon: Theme.colorIcon

    // signals
    signal volumeMute()
    signal volumeDown()
    signal volumeUp()

    // content
    Row {
        anchors.centerIn: parent
        spacing: 0

        ButtonRowElement { // button mute
            width: btnSize
            height: btnSize

            checkable: true
            checked: false

            colorBackground: control.colorBackground
            colorHighlight: control.colorHighlight
            colorIcon: control.colorIcon

            source: "qrc:/IconLibrary/material-icons/duotone/volume_off.svg"
            source_checked: "qrc:/IconLibrary/material-icons/duotone/volume_off.svg"

            onClicked: volumeMute()
        }

        Rectangle { // separator
            width: control.border.width
            height: control.btnSize
            color: control.colorBorder
            visible: control.btnSeparator
        }

        ButtonRowElement { // button volume down
            width: btnSize
            height: btnSize

            colorBackground: control.colorBackground
            colorHighlight: control.colorHighlight
            colorIcon: control.colorIcon

            source: "qrc:/IconLibrary/material-icons/duotone/volume_down.svg"

            onClicked: volumeDown()
        }

        Rectangle { // separator
            width: control.border.width
            height: control.btnSize
            color: control.colorBorder
            visible: control.btnSeparator
        }

        ButtonRowElement { // button volume up
            width: btnSize
            height: btnSize

            colorBackground: control.colorBackground
            colorHighlight: control.colorHighlight
            colorIcon: control.colorIcon

            source: "qrc:/IconLibrary/material-icons/duotone/volume_up_1.svg"

            onClicked: volumeUp()
        }

        layer.enabled: true
        layer.effect: MultiEffect {
            maskEnabled: true
            maskInverted: false
            maskThresholdMin: 0.5
            maskSpreadAtMin: 1.0
            maskSpreadAtMax: 0.0
            maskSource: ShaderEffectSource {
                sourceItem: Rectangle {
                    x: control.x
                    y: control.y
                    width: control.width
                    height: control.height
                    radius: control.radius
                }
            }
        }
    }
}
