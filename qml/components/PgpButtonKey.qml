import QtQuick
import QtQuick.Controls

import ComponentLibrary

Rectangle {
    id: pgpButtonKey
    width: pw
    height: ph/2
    radius: 12

    color: Theme.colorBackground
    border.width: 2
    border.color: Theme.colorSeparator

    property var currentDevice: null

    property int pw: parent.www
    property int ph: parent.hhh/3
    property int layoutDirection: Qt.LeftToRight

    property bool isValid: (!!currentDevice && currentDevice.hasDeviceKey === true)

    ////////////////////////////////////////////////////////////////////////////

    PopupPgpKey {
        id: popupPgpKey
        currentDevice: pgpButtonKey.currentDevice
    }

    ////////////////////////////////////////////////////////////////////////////

    Row {
        anchors.fill: parent
        spacing: 12
        layoutDirection: pgpButtonKey.layoutDirection

        Rectangle {
            width: parent.height
            height: parent.height
            radius: 12

            color: Theme.colorSeparator

            IconSvg {
                anchors.centerIn: parent
                color: Theme.colorIcon
                source: "qrc:/IconLibrary/material-symbols/lock.svg"
            }
        }

        IconSvg {
            anchors.verticalCenter: parent.verticalCenter
            width: 24
            height: 24

            color: pgpButtonKey.isValid ? Theme.colorSuccess : Theme.colorWarning
            source: pgpButtonKey.isValid ?
                        "qrc:/IconLibrary/material-symbols/check.svg" :
                        "qrc:/IconLibrary/material-symbols/warning.svg"
        }

        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: pgpButtonKey.isValid ?
                      qsTr("Device key loaded") :
                      qsTr("Missing device key!")
            color: Theme.colorText
        }
    }

    ////////

    MouseArea {
        anchors.fill: parent
        onClicked: popupPgpKey.open()
    }

    ////////////////////////////////////////////////////////////////////////////
}
