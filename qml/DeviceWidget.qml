import QtQuick

import ThemeEngine
import DeviceUtils
import "qrc:/utils/UtilsNumber.js" as UtilsNumber
import "qrc:/js/UtilsDeviceSensors.js" as UtilsDeviceSensors

Item {
    id: deviceWidget

    implicitWidth: 480
    implicitHeight: 128

    property var boxDevice: pointer

    property bool wideAssMode: (width >= 380) || (isTablet && width >= 480)
    property bool bigAssMode: false
    property bool singleColumn: true

    Connections {
        target: boxDevice
        function onSensorUpdated() { initBoxData() }
        function onSensorsUpdated() { initBoxData() }
        function onCapabilitiesUpdated() { initBoxData() }
        function onStatusUpdated() { updateSensorStatus() }
        function onSettingsUpdated() { updateSensorStatus(); updateSensorSettings(); }
        function onRssiUpdated() { updateSensorRSSI() }
        function onButtonPressed() { dotAnimation.start() }
    }

    Component.onCompleted: initBoxData()

    ////////////////////////////////////////////////////////////////////////////

    function initBoxData() {
        updateSensorSettings()
        updateSensorStatus()
    }

    function updateSensorStatus() {
        //
        if (!boxDevice.deviceEnabled) {
            textStatus.color = Theme.colorYellow
            textStatus.text = qsTr("Disabled")
        }

        // Image
        if (boxDevice.status === DeviceUtils.DEVICE_CONNECTING) {
            imageStatus.source = "qrc:/assets/icons/material-icons/duotone/bluetooth_searching.svg"
        } else if (boxDevice.status === DeviceUtils.DEVICE_CONNECTED) {
            imageStatus.source = "qrc:/assets/icons/material-icons/duotone/bluetooth_connected.svg"
        } else if (boxDevice.status >= DeviceUtils.DEVICE_WORKING) {
            imageStatus.source = "qrc:/assets/icons/material-icons/duotone/bluetooth_connected.svg"
        } else {
            updateSensorRSSI()
        }
    }

    function updateSensorRSSI() {
        //
        rssiValue.visible = (boxDevice.rssi < 0)
        rssiValue.text = boxDevice.rssi

        if (boxDevice.deviceEnabled) {
            // Image
            if (boxDevice.status === DeviceUtils.DEVICE_CONNECTING) {
                textStatus.text = UtilsDeviceSensors.getDeviceStatusText(boxDevice.status)
                textStatus.color = UtilsDeviceSensors.getDeviceStatusColor(boxDevice.status)
                imageStatus.source = "qrc:/assets/icons/material-icons/duotone/bluetooth_searching.svg"
            } else if (boxDevice.status === DeviceUtils.DEVICE_CONNECTED) {
                textStatus.text = UtilsDeviceSensors.getDeviceStatusText(boxDevice.status)
                textStatus.color = UtilsDeviceSensors.getDeviceStatusColor(boxDevice.status)
                imageStatus.source = "qrc:/assets/icons/material-icons/duotone/bluetooth_connected.svg"
            } else if (boxDevice.status >= DeviceUtils.DEVICE_WORKING) {
                textStatus.text = UtilsDeviceSensors.getDeviceStatusText(boxDevice.status)
                textStatus.color = UtilsDeviceSensors.getDeviceStatusColor(boxDevice.status)
                imageStatus.source = "qrc:/assets/icons/material-icons/duotone/bluetooth_connected.svg"
            } else {

                if (boxDevice.rssi < 0) {
                    textStatus.color = Theme.colorGreen
                    textStatus.text = qsTr("Available")
                } else if (boxDevice.rssi > 1) {
                    textStatus.color = Theme.colorYellow
                    textStatus.text = qsTr("Seen")
                }

                rssiValue.visible = (boxDevice.rssi < 0)
                rssiValue.text = boxDevice.rssi

                if (boxDevice.rssi >= 0) {
                    imageStatus.source = "" // "qrc:/assets/icons/material-symbols/signal_cellular_off.svg"
                } else if (boxDevice.rssi > -50) {
                    imageStatus.source = "qrc:/assets/icons/material-symbols/signal_cellular_4_bar.svg"
                } else if (boxDevice.rssi > -65) {
                    imageStatus.source = "qrc:/assets/icons/material-symbols/signal_cellular_3_bar.svg"
                } else if (boxDevice.rssi > -75) {
                    imageStatus.source = "qrc:/assets/icons/material-symbols/signal_cellular_2_bar.svg"
                } else if (boxDevice.rssi > -90) {
                    imageStatus.source = "qrc:/assets/icons/material-symbols/signal_cellular_1_bar.svg"
                } else { // if (boxDevice.rssi > -90) {
                    imageStatus.source = "qrc:/assets/icons/material-symbols/signal_cellular_0_bar.svg"
                }
            }

        } else {
            textStatus.color = Theme.colorYellow
            textStatus.text = qsTr("Disabled")
        }
    }

    function updateSensorSettings() {
        // Location
        textLocation.font.pixelSize = bigAssMode ? 20 : 18
        if (boxDevice.deviceLocationName) {
            textLocation.visible = true
            textLocation.text = boxDevice.deviceLocationName
        } else {
            if (Qt.platform.os === "osx" || Qt.platform.os === "ios") {
                textLocation.visible = false
                textLocation.text = ""
            } else {
                textLocation.visible = true
                textLocation.text = boxDevice.deviceAddress
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    Rectangle { // bottomSeparator
        anchors.left: parent.left
        anchors.leftMargin: -6
        anchors.right: parent.right
        anchors.rightMargin: -6
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0

        height: 1
        visible: singleColumn
        color: Theme.colorSeparator
    }

    Rectangle { // deviceWidgetRectangleSeparator
        anchors.fill: deviceWidgetRectangle
        anchors.leftMargin: singleColumn ? -12 : 0
        anchors.rightMargin: singleColumn ? -12 : 0
        anchors.topMargin: singleColumn ? -6 : 0
        anchors.bottomMargin: singleColumn ? -6 : 0

        radius: 4
        border.width: 2
        border.color: {
            if (singleColumn) return "transparent"
            if (mousearea.containsPress) return Theme.colorSecondary
            return Theme.colorSeparator
        }
        Behavior on border.color { ColorAnimation { duration: 133 } }

        color: boxDevice.selected ? Theme.colorSeparator : Theme.colorDeviceWidget
        Behavior on color { ColorAnimation { duration: 133 } }

        opacity: boxDevice.selected ? 0.5 : (singleColumn ? 0 : 1)
        Behavior on opacity { OpacityAnimator { duration: 133 } }
    }

    ////////////////////////////////////////////////////////////////////////////

    Item {
        id: deviceWidgetRectangle
        anchors.fill: parent
        anchors.margins: 6

        opacity: boxDevice.deviceEnabled ? 1 : 0.66

        MouseArea {
            id: mousearea
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.MiddleButton

            onClicked: (mouse) => {
                if (typeof boxDevice === "undefined" || !boxDevice) return

                if (mouse.button === Qt.LeftButton) {
                    // multi selection
                    if ((mouse.modifiers & Qt.ControlModifier) ||
                        (screenDeviceList.selectionMode)) {
                        if (!boxDevice.selected) {
                            screenDeviceList.selectDevice(index)
                        } else {
                            screenDeviceList.deselectDevice(index)
                        }
                        return
                    }

                    // regular click
                    selectedDevice = boxDevice
                    if (boxDevice.isLight) {
                        screenDeviceLight.loadDevice(boxDevice)
                    }
                    if (isDesktop) {
                        if (boxDevice.isRemote) {
                            screenDeviceRemote.loadDevice(boxDevice)
                        } else if (boxDevice.isBeacon) {
                            screenDeviceBeacon.loadDevice(boxDevice)
                        } else if (boxDevice.isPBP) {
                            screenDevicePBP.loadDevice(boxDevice)
                        } else if (boxDevice.isPGP) {
                            screenDevicePGP.loadDevice(boxDevice)
                        }
                    }
                }

                if (mouse.button === Qt.MiddleButton) {
                   // multi selection
                   if (!boxDevice.selected) {
                       screenDeviceList.selectDevice(index)
                   } else {
                       screenDeviceList.deselectDevice(index)
                   }
                   return
                }
            }

            onPressAndHold: {
                // multi selection
                if (!boxDevice.selected) {
                    utilsApp.vibrate(25)
                    screenDeviceList.selectDevice(index)
                } else {
                    screenDeviceList.deselectDevice(index)
                }
            }
        }

        ////////////////

        Row {
            id: rowLeft
            anchors.top: parent.top
            anchors.topMargin: bigAssMode ? 16 : 8
            anchors.left: parent.left
            anchors.leftMargin: bigAssMode ? (singleColumn ? 4 : 16) : (singleColumn ? 6 : 14)
            anchors.right: (rowRight.width > 0) ? rowRight.left : imageStatus.left
            anchors.rightMargin: singleColumn ? 0 : 8
            anchors.bottom: parent.bottom
            anchors.bottomMargin: bigAssMode ? 16 : 8

            spacing: bigAssMode ? (singleColumn ? 20 : 12) : (singleColumn ? 24 : 10)

            IconSvg {
                id: imageDevice
                width: bigAssMode ? 32 : 28
                height: bigAssMode ? 32 : 28
                anchors.verticalCenter: parent.verticalCenter

                //visible: (wideAssMode || bigAssMode)
                color: Theme.colorHighContrast
                source: UtilsDeviceSensors.getDeviceIcon(boxDevice, false)
                fillMode: Image.PreserveAspectFit
                asynchronous: true

                Rectangle {
                    id: activityDot
                    anchors.top: parent.bottom
                    anchors.topMargin: 4
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: 6
                    height: 6
                    radius: 6
                    color: Theme.colorBackground

                    ColorAnimation on color {
                        id: dotAnimation
                        loops: 1
                        running: false

                        duration: 333
                        from: Theme.colorPrimary
                        to: Theme.colorSeparator
                    }
                }
            }

            Column {
                anchors.verticalCenter: parent.verticalCenter

                Text {
                    id: textTitle
                    width: rowLeft.width - imageDevice.width - rowLeft.spacing

                    text: boxDevice.deviceName
                    textFormat: Text.PlainText
                    color: Theme.colorText
                    font.pixelSize: bigAssMode ? 22 : 20
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }

                Text {
                    id: textLocation
                    width: rowLeft.width - imageDevice.width - rowLeft.spacing

                    textFormat: Text.PlainText
                    color: Theme.colorSubText
                    font.pixelSize: bigAssMode ? 20 : 18
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }

                Row {
                    id: row
                    height: bigAssMode ? 26 : 22
                    anchors.left: parent.left
                    spacing: 8

                    IconSvg {
                        id: imageBattery
                        width: bigAssMode ? 30 : 28
                        height: bigAssMode ? 32 : 30
                        anchors.verticalCenter: parent.verticalCenter

                        visible: (boxDevice.hasBattery && boxDevice.deviceBattery >= 0)
                        source: UtilsDeviceSensors.getDeviceBatteryIcon(boxDevice.deviceBattery)
                        color: Theme.colorIcon
                        rotation: 90
                        fillMode: Image.PreserveAspectCrop
                    }

                    Text {
                        id: textStatus
                        anchors.verticalCenter: parent.verticalCenter

                        textFormat: Text.PlainText
                        color: Theme.colorGreen
                        font.pixelSize: bigAssMode ? 16 : 15

                        SequentialAnimation on opacity {
                            id: opa
                            loops: Animation.Infinite
                            alwaysRunToEnd: true
                            running: (visible &&
                                      boxDevice.status !== DeviceUtils.DEVICE_OFFLINE &&
                                      boxDevice.status !== DeviceUtils.DEVICE_QUEUED &&
                                      boxDevice.status !== DeviceUtils.DEVICE_CONNECTED)

                            PropertyAnimation { to: 0.33; duration: 750; }
                            PropertyAnimation { to: 1; duration: 750; }
                        }
                    }
                }
            }
        }

        ////////////////

        Row {
            id: rowRight
            anchors.top: parent.top
            anchors.topMargin: bigAssMode ? 16 : 8
            anchors.bottom: parent.bottom
            anchors.bottomMargin: bigAssMode ? 16 : 8
            anchors.right: parent.right
            anchors.rightMargin: singleColumn ? (wideAssMode ? 0 : -4) : (bigAssMode ? 14 : 10)

            spacing: 8

            ////

            IconSvg {
                id: imageForward
                width: 32
                height: 32
                anchors.verticalCenter: parent.verticalCenter

                visible: singleColumn && boxDevice.available
                color: boxDevice.hasData ? Theme.colorHighContrast : Theme.colorSubText
                source: "qrc:/assets/icons/material-symbols/chevron_right.svg"
            }
        }

        ////////////////

        IconSvg {
            id: imageStatus
            width: 32
            height: 32
            anchors.right: parent.right
            anchors.rightMargin: singleColumn ? 56 : 36
            anchors.verticalCenter: parent.verticalCenter

            //visible: !boxDevice.isAvailable
            color: Theme.colorIcon
            opacity: 0.8
/*
            SequentialAnimation on opacity {
                id: refreshAnimation
                loops: Animation.Infinite
                running: (visible &&
                          boxDevice.status === DeviceUtils.DEVICE_CONNECTING ||
                          boxDevice.status === DeviceUtils.DEVICE_CONNECTED ||
                          boxDevice.status === DeviceUtils.DEVICE_WORKING)
                alwaysRunToEnd: true
                OpacityAnimator { from: 0.8; to: 0; duration: 750 }
                OpacityAnimator { from: 0; to: 0.8; duration: 750 }
            }
*/
            Text {
                id: rssiValue
                anchors.top: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter

                color: Theme.colorIcon
                opacity: 0.8
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////
}
