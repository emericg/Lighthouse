import QtQuick

import ThemeEngine
import DeviceUtils
import "qrc:/js/UtilsDeviceSensors.js" as UtilsDeviceSensors

Rectangle {
    id: itemLoadData
    anchors.centerIn: parent

    width: singleColumn ? (parent.width*0.33) : (parent.height*0.26)
    height: width
    radius: width
    color: Theme.colorForeground
    opacity: 0.8

    visible: (currentDevice.status !== DeviceUtils.DEVICE_CONNECTED)

    IconSvg {
        anchors.centerIn: parent
        width: parent.width*0.8
        height: width

        source: {
            if (currentDevice.status === DeviceUtils.DEVICE_CONNECTED)
                return ""
            else if (currentDevice.status === DeviceUtils.DEVICE_CONNECTING)
                return "qrc:/assets/icons/material-icons/duotone/bluetooth_searching.svg"
            else
                return "qrc:/assets/icons/material-icons/outlined/bluetooth_disabled.svg"
        }

        fillMode: Image.PreserveAspectFit
        color: Theme.colorSubText
        //smooth: true
    }

    MouseArea {
        anchors.fill: parent
        onClicked: currentDevice.actionConnect()
    }
}
