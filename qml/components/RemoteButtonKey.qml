import QtQuick
import QtQuick.Controls

import ComponentLibrary
import LocalActions
import Lighthouse

Rectangle {
    id: remoteButtonKey
    width: pw
    height: ph/2
    radius: 12

    color: Theme.colorBackground
    border.width: 2
    border.color: Theme.colorSeparator

    property var currentButton: null

    property int pw: parent.www
    property int ph: parent.hhh/3
    property int layoutDirection: Qt.LeftToRight

    property bool isValid: (selectedDevice && selectedDevice.beaconkey && selectedDevice.beaconkey.length === 24)

    ////////////////////////////////////////////////////////////////////////////

    PopupBeaconKey {
        parent: appContent
        id: popupBeaconKey
    }

    ////////////////////////////////////////////////////////////////////////////

    Row {
        anchors.fill: parent
        spacing: 12
        layoutDirection: remoteButtonKey.layoutDirection

        Rectangle {
            width: parent.height
            height: parent.height
            radius: 12

            color: Theme.colorSeparator

            IconSvg {
                anchors.centerIn: parent
                source: "qrc:/IconLibrary/material-symbols/lock.svg"
            }
        }

        IconSvg {
            anchors.verticalCenter: parent.verticalCenter
            width: 24
            height: 24

            color: remoteButtonKey.isValid ? Theme.colorSuccess : Theme.colorWarning
            source: remoteButtonKey.isValid ?
                        "qrc:/IconLibrary/material-symbols/check.svg" :
                        "qrc:/IconLibrary/material-symbols/warning.svg"
        }

        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: remoteButtonKey.isValid ?
                      "0x" + selectedDevice.beaconkey :
                      qsTr("Missing or invalid beacon key!")
            color: Theme.colorText
        }
    }

    ////////

    MouseArea {
        anchors.fill: parent
        onClicked: popupBeaconKey.open()
    }

    ////////////////////////////////////////////////////////////////////////////
}
