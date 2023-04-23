import QtQuick
import QtQuick.Controls

import ThemeEngine 1.0
import DeviceUtils 1.0
import "qrc:/js/UtilsDeviceSensors.js" as UtilsDeviceSensors

Loader {
    id: deviceBeacon

    property var currentDevice: null

    ////////

    function loadDevice(clickedDevice) {
        if (typeof clickedDevice === "undefined" || !clickedDevice) return
        if (!clickedDevice.isBeacon) return

        appContent.state = "DeviceBeacon"

        // set device
        if (clickedDevice === currentDevice) return
        currentDevice = clickedDevice

        // load screen
        deviceBeacon.active = true
        deviceBeacon.item.loadDevice()
    }

    ////////

    function backAction() {
        if (deviceBeacon.status === Loader.Ready)
            deviceBeacon.item.backAction()
    }

    active: false
    asynchronous: false

    sourceComponent: Item {
        id: itemDeviceBeacon
        implicitWidth: 480
        implicitHeight: 720

        focus: parent.focus

        // 1: single column (single column view or portrait tablet)
        // 2: wide mode (wide view)
        property int uiMode: (singleColumn || (isTablet && screenOrientation === Qt.PortraitOrientation)) ? 1 : 2

        property string cccc: headerUnicolor ? Theme.colorHeaderContent : "white"

        ////////

        Connections {
            target: currentDevice
            function onStatusUpdated() { updateHeader() }
            function onDataUpdated() { updateData() }
        }

        Connections {
            target: appHeader
            // desktop only
            function onDeviceDataButtonClicked() {
                appHeader.setActiveDeviceData()
            }
            function onDeviceSettingsButtonClicked() {
                appHeader.setActiveDeviceSettings()
            }
            // mobile only
            function onRightMenuClicked() {
                //
            }
        }

        Keys.onPressed: (event) => {
            if (event.key === Qt.Key_Backspace) {
                event.accepted = true
                backAction()
            }
        }

        ////////

        function loadDevice() {
            //console.log("DeviceBeacon // loadDevice() >> " + currentDevice)

            updateHeader()
            updateData()
        }

        function updateHeader() {
            if (typeof currentDevice === "undefined" || !currentDevice) return
            if (!currentDevice.isBeacon) return
            //console.log("DeviceBeacon // updateHeader() >> " + currentDevice)

            // Status
            updateStatusText()
        }

        function updateStatusText() {
            if (typeof currentDevice === "undefined" || !currentDevice) return
            if (!currentDevice.isBeacon) return
            //console.log("DeviceBeacon // updateStatusText() >> " + currentDevice)
        }

        function updateData() {
            if (typeof currentDevice === "undefined" || !currentDevice) return
            if (!currentDevice.isBeacon) return
        }

        ////////

        function backAction() {
            appContent.state = "DeviceList"
        }

        function isHistoryMode() {
            //return deviceScreenChart.isIndicator()
        }
        function resetHistoryMode() {
            //deviceScreenChart.resetIndicator()
        }

        ////////////////////////////////////////////////////////////////////////
    }
}
