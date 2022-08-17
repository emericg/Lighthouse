import QtQuick 2.15
import QtQuick.Controls.impl 2.15
import QtQuick.Templates 2.15 as T

import ThemeEngine 1.0

T.Slider {
    id: control
    implicitWidth: 200
    implicitHeight: 32
    padding: 4

    from: 0.0
    value: 0.5
    to: 1.0
    snapMode: T.Slider.NoSnap

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
            GradientStop { position: 0.0; color: "#999" } // almost black
            GradientStop { position: 1.0; color: "#ee0" } // almost yellow
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
