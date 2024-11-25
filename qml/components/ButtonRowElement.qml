import QtQuick
import QtQuick.Effects
import QtQuick.Controls.impl
import QtQuick.Templates as T

import ComponentLibrary

T.Button {
    id: control

    implicitWidth: implicitBackgroundWidth
    implicitHeight: implicitBackgroundHeight

    clip: true
    flat: true
    checkable: false
    hoverEnabled: isDesktop
    focusPolicy: Qt.NoFocus

    // icon
    property url source
    property url source_checked
    property int sourceSize: height * 0.66
    property int sourceRotation: 0

    // colors
    property color colorBackground: Theme.colorForeground
    property color colorHighlight: Theme.colorPrimary
    property color colorRipple: Qt.rgba(colorHighlight.r, colorHighlight.g, colorHighlight.b, 0.2)
    property color colorIcon: Theme.colorIcon

    ////////////////

    background: Item {
        implicitWidth: Theme.componentHeight
        implicitHeight: Theme.componentHeight

        Rectangle {
            anchors.fill: parent
            color: control.colorBackground
        }

        RippleThemed {
            width: parent.height
            height: parent.height

            anchor: control
            pressed: control.pressed
            active: control.enabled && (control.down || control.hovered || control.visualFocus)
            color: control.colorRipple
        }
    }

    ////////////////

    contentItem: Item {
        IconSvg {
            anchors.centerIn: parent

            width: control.sourceSize
            height: control.sourceSize

            visible: control.source.toString().length
            color: control.colorIcon
            opacity: control.enabled ? 1 : 0.66
            rotation: control.sourceRotation

            source: control.source
        }
    }

    ////////////////
}
