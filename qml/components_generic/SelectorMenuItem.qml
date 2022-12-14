import QtQuick 2.15

import ThemeEngine 1.0

Item {
    id: control
    implicitWidth: 16 + contentText.width + (source.toString().length ? sourceSize : 0) + 16
    implicitHeight: 32

    height: parent.height

    // actions
    signal clicked()
    signal pressed()
    signal pressAndHold()

    // states
    property bool selected: false

    // settings
    property int index
    property string text
    property url source
    property int sourceSize: 32

    // colors
    property string colorContent: Theme.colorComponentText
    property string colorContentHighlight: "white"
    property string colorBackgroundHighlight: Theme.colorPrimary

    ////////////////////////////////////////////////////////////////////////////

    MouseArea {
        id: mouseArea
        anchors.fill: control
        hoverEnabled: (isDesktop && control.enabled)

        onClicked: control.clicked()
        onPressed: control.pressed()
        onPressAndHold: control.pressAndHold()
    }

    ////////////////////////////////////////////////////////////////////////////

    Rectangle {
        id: background
        anchors.fill: control
        radius: height

        color: control.colorBackgroundHighlight
        opacity: {
            if (control.selected) return 1
            else if (mouseArea.containsMouse) return 0.2
            return 0
        }
        Behavior on opacity { OpacityAnimator { duration: 133 } }
    }

    ////////////////////////////////////////////////////////////////////////////

    IconSvg {
        id: contentImage
        width: control.sourceSize
        height: control.sourceSize
        anchors.centerIn: control

        source: control.source
        color: control.selected ? control.colorContentHighlight : control.colorContent
        opacity: control.selected ? 1 : 0.66
    }

    Text {
        id: contentText
        anchors.centerIn: control

        text: control.text
        textFormat: Text.PlainText
        font.pixelSize: Theme.fontSizeComponent
        verticalAlignment: Text.AlignVCenter

        color: control.selected ? control.colorContentHighlight : control.colorContent
        opacity: control.selected ? 1 : 0.66
    }

    ////////////////////////////////////////////////////////////////////////////
}
