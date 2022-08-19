import QtQuick 2.15

import ThemeEngine 1.0
import DeviceUtils 1.0
import "qrc:/js/UtilsNumber.js" as UtilsNumber
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
        function onDataUpdated() { updateSensorData() }
        function onRefreshUpdated() { updateSensorData() }
    }/*
    Connections {
        target: ThemeEngine
        function onCurrentThemeChanged() {
            updateSensorSettings()
            updateSensorStatus()
            updateSensorData()
        }
    }*/

    Component.onCompleted: initBoxData()

    ////////////////////////////////////////////////////////////////////////////

    function initBoxData() {
        // Set icon
        imageDevice.source = UtilsDeviceSensors.getDeviceIcon(boxDevice, false)

        updateSensorSettings()
        updateSensorStatus()
        updateSensorData()
    }

    function updateSensorStatus() {
        // Text
        //textStatus.text = UtilsDeviceSensors.getDeviceStatusText(boxDevice.status)
        //textStatus.color = UtilsDeviceSensors.getDeviceStatusColor(boxDevice.status)

        if (!boxDevice.deviceEnabled) {
            textStatus.color = Theme.colorYellow
            textStatus.text = qsTr("Disabled")
        }

        // Image
        if (boxDevice.status === DeviceUtils.DEVICE_CONNECTING) {
            imageStatus.source = "qrc:/assets/icons_material/duotone-bluetooth_searching-24px.svg"
        } else if (boxDevice.status === DeviceUtils.DEVICE_CONNECTED) {
            imageStatus.source = "qrc:/assets/icons_material/duotone-bluetooth_connected-24px.svg"
        } else if (boxDevice.status >= DeviceUtils.DEVICE_WORKING) {
            imageStatus.source = "qrc:/assets/icons_material/duotone-bluetooth_connected-24px.svg"
        } else {
            updateSensorRSSI()
        }
    }

    function updateSensorRSSI() {

        rssiValue.visible = (boxDevice.rssi < 0)
        rssiValue.text = boxDevice.rssi

        if (boxDevice.deviceEnabled) {

            // Image
            if (boxDevice.status === DeviceUtils.DEVICE_CONNECTING) {
                textStatus.text = UtilsDeviceSensors.getDeviceStatusText(boxDevice.status)
                textStatus.color = UtilsDeviceSensors.getDeviceStatusColor(boxDevice.status)
                imageStatus.source = "qrc:/assets/icons_material/duotone-bluetooth_searching-24px.svg"
            } else if (boxDevice.status === DeviceUtils.DEVICE_CONNECTED) {
                textStatus.text = UtilsDeviceSensors.getDeviceStatusText(boxDevice.status)
                textStatus.color = UtilsDeviceSensors.getDeviceStatusColor(boxDevice.status)
                imageStatus.source = "qrc:/assets/icons_material/duotone-bluetooth_connected-24px.svg"
            } else if (boxDevice.status >= DeviceUtils.DEVICE_WORKING) {
                textStatus.text = UtilsDeviceSensors.getDeviceStatusText(boxDevice.status)
                textStatus.color = UtilsDeviceSensors.getDeviceStatusColor(boxDevice.status)
                imageStatus.source = "qrc:/assets/icons_material/duotone-bluetooth_connected-24px.svg"
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
                    imageStatus.source = "" // "qrc:/assets/icons_material/signal_cellular_off-24px.svg"
                } else if (boxDevice.rssi > -40) {
                    imageStatus.source = "qrc:/assets/icons_material/signal_cellular_full-24px.svg"
                } else if (boxDevice.rssi > -60) {
                    imageStatus.source = "qrc:/assets/icons_material/signal_cellular_mid-24px.svg"
                } else { // if (boxDevice.rssi > -90) {
                    imageStatus.source = "qrc:/assets/icons_material/signal_cellular_low-24px.svg"
                }
            }

        } else {
            textStatus.color = Theme.colorYellow
            textStatus.text = qsTr("Disabled")
        }
    }

    function updateSensorSettings() {
        // Title
        if (boxDevice.isPlantSensor) {
            if (boxDevice.deviceAssociatedName !== "")
                textTitle.text = boxDevice.deviceAssociatedName
            else
                textTitle.text = boxDevice.deviceName
        } else if (boxDevice.isThermometer) {
            if (boxDevice.deviceName === "ThermoBeacon")
                textTitle.text = boxDevice.deviceName
            else
                textTitle.text = qsTr("Thermometer")
        } else {
            textTitle.text = boxDevice.deviceName
        }
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

    function updateSensorIcon() {
        if (boxDevice.isPlantSensor) {
            if (boxDevice.deviceName === "ropot" || boxDevice.deviceName === "Parrot pot")
                imageDevice.source = "qrc:/assets/icons_custom/pot_empty-24px.svg"
            else
                imageDevice.source = "qrc:/assets/icons_material/outline-settings_remote-24px.svg"
        }
    }

    function updateSensorWarnings() {
        // Warnings icons (for sensors with available data)
    }

    function updateSensorData() {
        updateSensorIcon()
        updateSensorWarnings()
    }

    ////////////////////////////////////////////////////////////////////////////

    Rectangle {
        id: bottomSeparator
        height: 1
        anchors.left: parent.left
        anchors.leftMargin: -6
        anchors.right: parent.right
        anchors.rightMargin: -6
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0

        visible: singleColumn
        color: Theme.colorSeparator
    }

    Rectangle {
        id: deviceWidgetRectangleSeparator
        anchors.fill: deviceWidgetRectangle
        anchors.leftMargin: singleColumn ? -12 : 0
        anchors.rightMargin: singleColumn ? -12 : 0
        anchors.topMargin: singleColumn ? -6 : 0
        anchors.bottomMargin: singleColumn ? -6 : 0

        radius: 4
        border.width: 2
        border.color: singleColumn ? "transparent" : Theme.colorSeparator

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
                    //if (boxDevice.rssi > 0) {
                        selectedDevice = boxDevice

                        if (boxDevice.isRemote) {
                            screenDeviceRemote.loadDevice(boxDevice)
                        } else if (boxDevice.isBeacon) {
                            screenDeviceBeacon.loadDevice(boxDevice)
                        } else if (boxDevice.isPGP) {
                            screenDevicePGP.loadDevice(boxDevice)
                        } else if (boxDevice.isLight) {
                            screenDeviceLight.loadDevice(boxDevice)
                        }
                    //}
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

                color: Theme.colorHighContrast
                //visible: (wideAssMode || bigAssMode)
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

                    Connections {
                        target: boxDevice
                        function onButtonPressed() { dotAnimation.start() }
                    }

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

                    textFormat: Text.PlainText
                    color: Theme.colorText
                    font.pixelSize: bigAssMode ? 22 : 20
                    //font.capitalization: Font.Capitalize
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }

                Text {
                    id: textLocation
                    width: rowLeft.width - imageDevice.width - rowLeft.spacing

                    textFormat: Text.PlainText
                    color: Theme.colorSubText
                    font.pixelSize: bigAssMode ? 20 : 18
                    //font.capitalization: Font.Capitalize
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
                source: "qrc:/assets/icons_material/baseline-chevron_right-24px.svg"
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
