import QtQuick
import QtQuick.Controls.impl
import QtQuick.Templates as T

import ComponentLibrary
import Lighthouse

T.Slider {
    id: control
    implicitWidth: 200
    implicitHeight: 32
    padding: 4

    from: 0.0
    value: 0.5
    to: 1.0
    snapMode: T.Slider.NoSnap

    property string mode: "color" // "luminosity", "warmth", "color"

    // settings
    property int hhh: 32
    property int radius: 16

    // colors
    property string colorBg: Theme.colorForeground
    property string colorFg: "white" // Theme.colorBackground

    ////////////////////////////////////////////////////////////////////////////

    background: Rectangle {
        x: control.leftPadding - (control.padding / 2)
        y: control.topPadding + (control.availableHeight / 2) - (height / 2)
        width: control.availableWidth + control.padding

        height: control.hhh
        radius: control.hhh
        opacity: 1

        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0/6; color: Qt.hsva(0 / 6, 1, 1, 1) }
            GradientStop { position: 1/6; color: Qt.hsva(1 / 6, 1, 1, 1) }
            GradientStop { position: 2/6; color: Qt.hsva(2 / 6, 1, 1, 1) }
            GradientStop { position: 3/6; color: Qt.hsva(3 / 6, 1, 1, 1) }
            GradientStop { position: 4/6; color: Qt.hsva(4 / 6, 1, 1, 1) }
            GradientStop { position: 5/6; color: Qt.hsva(5 / 6, 1, 1, 1) }
            GradientStop { position: 6/6; color: Qt.hsva(6 / 6, 1, 1, 1) }
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    handle: Rectangle {
        x: control.leftPadding + control.visualPosition * (control.availableWidth - width)
        y: control.topPadding + (control.availableHeight / 2) - (height / 2)
        implicitWidth: control.hhh - control.padding
        implicitHeight: control.hhh - control.padding
        width: control.hhh - control.padding
        radius: width

        color: control.colorFg
        border.color: control.colorFg
        opacity: control.pressed ? 1 : 1
    }

    ////////////////////////////////////////////////////////////////////////////
}
