import QtQuick
import QtQuick.Controls

import ComponentLibrary
import DeviceUtils
import Lighthouse

Loader {
    id: devicePGP

    property var currentDevice: null

    ////////

    function loadDevice(clickedDevice) {
        if (typeof clickedDevice === "undefined" || !clickedDevice) return
        if (!clickedDevice.isPGP) return

        appContent.state = "DevicePGP"

        // set device
        if (clickedDevice === currentDevice) return
        currentDevice = clickedDevice

        // load screen
        devicePGP.active = true
        devicePGP.item.loadDevice()
    }

    ////////

    function backAction() {
        if (devicePGP.status === Loader.Ready)
            devicePGP.item.backAction()
    }

    active: false
    asynchronous: false

    sourceComponent: Item { // itemDevicePGP // Pokemon Go Plus
        id: itemDeviceRemote

        focus: parent.focus

        // 1: single column (single column view or portrait tablet)
        // 2: wide mode (wide view)
        property int uiMode: (singleColumn || (isTablet && screenOrientation === Qt.PortraitOrientation)) ? 1 : 2

        property string cccc: headerUnicolor ? Theme.colorHeaderContent : "white"

        property int btnClicked: 0

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
            console.log("DevicePGP // loadDevice() >> " + currentDevice)

            btnClicked = 0

            updateHeader()
            updateData()
        }

        function updateHeader() {
            if (typeof currentDevice === "undefined" || !currentDevice) return
            if (!currentDevice.isPGP) return
            //console.log("DevicePGP // updateHeader() >> " + currentDevice)

            // Status
            updateStatusText()
        }

        function updateStatusText() {
            if (typeof currentDevice === "undefined" || !currentDevice) return
            if (!currentDevice.isPGP) return
            //console.log("DevicePGP // updateStatusText() >> " + currentDevice)
        }

        function updateData() {
            if (typeof currentDevice === "undefined" || !currentDevice) return
            if (!currentDevice.isPGP) return
        }

        ////////

        function backAction() {
            screenDeviceList.loadScreen()
        }

        function isHistoryMode() {
            return false
        }
        function resetHistoryMode() {
            //
        }

        ////////////////////////////////////////////////////////////////////////

        PopupActions {
            id: popupActionsChooser
        }

        ////////////////////////////////////////////////////////////////////////

        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right

            height: 72
            color: Theme.colorForeground

            Row { // left
                anchors.left: parent.left
                anchors.leftMargin: 16
                anchors.verticalCenter: parent.verticalCenter
                spacing: 16

                ButtonSolid {
                    anchors.verticalCenter: parent.verticalCenter
                    width: 160

                    color: {
                        if (currentDevice.connected) return Theme.colorSuccess
                        if (currentDevice.available) return Theme.colorSuccess
                        return Theme.colorWarning
                    }
                    text: {
                        if (currentDevice.status === DeviceUtils.DEVICE_OFFLINE) return qsTr("Offline")
                        if (currentDevice.status === DeviceUtils.DEVICE_QUEUED) return qsTr("Queued")
                        if (currentDevice.status === DeviceUtils.DEVICE_DISCONNECTING) return qsTr("Disconnecting...")
                        if (currentDevice.status === DeviceUtils.DEVICE_CONNECTING) return qsTr("Connecting...")
                        if (currentDevice.status === DeviceUtils.DEVICE_CONNECTED) return qsTr("Connected")
                    }
                }

                ButtonWireframe {
                    anchors.verticalCenter: parent.verticalCenter

                    width: 128
                    font.bold: currentDevice.connected

                    text: {
                        if (currentDevice.status === DeviceUtils.DEVICE_OFFLINE) return qsTr("Connect")
                        return qsTr("Disconnect")
                    }
                    onClicked: {
                        if (currentDevice.status === DeviceUtils.DEVICE_OFFLINE) currentDevice.actionConnect()
                        currentDevice.actionDisconnect()
                    }
                }

                CheckBoxThemed {
                    id: cbAutoConnect
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("auto-connect device")

                    checked: currentDevice.autoConnect
                    onClicked: currentDevice.autoConnect = checked
                }
            }

            Row { // right
                anchors.right: parent.right
                anchors.rightMargin: 16
                anchors.verticalCenter: parent.verticalCenter
                spacing: 16
            }
        }

        ////////////////////////////////////////////////////////////////////////

        Row {
            anchors.fill: parent
            anchors.topMargin: 80

            Item {
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: parent.width*0.05
            }

            Item {
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: parent.width*0.4

                Column {
                    anchors.left: parent.left
                    anchors.leftMargin: 32
                    anchors.right: parent.right
                    anchors.rightMargin: 32
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 24

                    property int www: (parent.width * 1)
                    property int hhh: (parent.height * 0.4)

                    RemoteButtonWidget { // btn1
                        currentButton: currentDevice.btn1
                    }
                }
            }

            Item {
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: parent.width*0.05
            }

            Item {
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: parent.width*0.4

                SchematicPokemonGoPlus {
                    id: pokeball
                    width: parent.width * 0.6
                }
            }
        }

        ////////////////////////////////////////////////////////////////////////
    }
}
