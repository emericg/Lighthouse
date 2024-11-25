import QtQuick

import ComponentLibrary

Rectangle {
    width: 92
    height: width
    radius: 6

    border.width: 3
    border.color: Theme.colorSeparator

    property string key: ""

    Text {
        anchors.left: parent.left
        anchors.leftMargin: 8
        anchors.right: parent.right
        anchors.rightMargin: 8
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 8

        text: key
        textFormat: Text.PlainText
        color:Theme.colorText
        font.bold: false
        font.pixelSize: Theme.fontSizeContentVeryBig
        wrapMode: Text.WordWrap
    }
}
