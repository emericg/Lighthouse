import QtQuick
import QtQuick.Effects
import QtQuick.Controls

import ThemeEngine

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
    signal mediaPrevious()
    signal mediaPlayPause()
    signal mediaNext()

    // content
    Row {
        anchors.centerIn: parent
        spacing: 0

        ButtonRowElement { // button previous
            width: btnSize
            height: btnSize

            colorBackground: control.colorBackground
            colorHighlight: control.colorHighlight
            colorIcon: control.colorIcon

            source: "qrc:/assets/icons/material-symbols/skip_previous.svg"

            onClicked: mediaPrevious()
        }

        Rectangle { // separator
            width: control.border.width
            height: control.btnSize
            color: control.colorBorder
            visible: control.btnSeparator
        }

        ButtonRowElement { // button play/pause
            width: btnSize
            height: btnSize

            checkable: true
            checked: false

            colorBackground: control.colorBackground
            colorHighlight: control.colorHighlight
            colorIcon: control.colorIcon

            source: "qrc:/assets/icons/material-symbols/play_pause.svg"
            source_checked: "qrc:/assets/icons/material-symbols/play_pause.svg"

            onClicked: mediaPlayPause()
        }

        Rectangle { // separator
            width: control.border.width
            height: control.btnSize
            color: control.colorBorder
            visible: control.btnSeparator
        }

        ButtonRowElement { // button next
            width: btnSize
            height: btnSize

            colorBackground: control.colorBackground
            colorHighlight: control.colorHighlight
            colorIcon: control.colorIcon

            source: "qrc:/assets/icons/material-symbols/skip_next.svg"

            onClicked: mediaNext()
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
