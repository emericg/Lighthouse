import QtQuick 2.15
import QtQuick.Controls.impl 2.15
import QtQuick.Templates 2.15 as T

//import QtGraphicalEffects 1.15 // Qt5
import Qt5Compat.GraphicalEffects // Qt6

import ThemeEngine 1.0

T.Button {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    leftPadding: 12
    rightPadding: 12

    font.pixelSize: Theme.fontSizeComponent
    font.bold: false

    focusPolicy: Qt.NoFocus

    // colors
    property string color: Theme.colorPrimary

    ////////////////////////////////////////////////////////////////////////////

    background: Rectangle {
        implicitWidth: 80
        implicitHeight: Theme.componentHeight

        radius: Theme.componentRadius
        opacity: enabled ? (control.down ? 0.8 : 1.0) : 0.4
        color: "transparent"
        border.width: Theme.componentBorderWidth
        border.color: control.color
    }

    ////////////////////////////////////////////////////////////////////////////

    contentItem: Text {
        text: control.text
        textFormat: Text.PlainText

        font: control.font
        elide: Text.ElideMiddle
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        opacity: enabled ? (control.down ? 0.8 : 1.0) : 0.66
        color: control.color
    }
}
