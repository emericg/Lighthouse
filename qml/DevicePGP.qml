import QtQuick
import QtQuick.Controls

import ThemeEngine 1.0
import DeviceUtils 1.0
import "qrc:/js/UtilsDeviceSensors.js" as UtilsDeviceSensors

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
    sourceComponent: Item {
        id: itemDeviceRemote // itemDevicePGP
        //anchors.fill: parent

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
            appContent.state = "DeviceList"
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

                ButtonWireframe {
                    anchors.verticalCenter: parent.verticalCenter
                    width: 160

                    fullColor: currentDevice.connected
                    primaryColor: (currentDevice.available || currentDevice.connected) ?
                                      Theme.colorSuccess : Theme.colorWarning
                    text: {
                        if (currentDevice.status === DeviceUtils.DEVICE_OFFLINE) return qsTr("Offline")
                        if (currentDevice.status === DeviceUtils.DEVICE_OFFLINE) return qsTr("Available")
                        if (currentDevice.status === DeviceUtils.DEVICE_QUEUED) return qsTr("Connecting...")
                        if (currentDevice.status === DeviceUtils.DEVICE_CONNECTING) return qsTr("Connecting...")
                        if (currentDevice.status === DeviceUtils.DEVICE_CONNECTED) return qsTr("Connected")
                    }
                }
                ButtonWireframe {
                    anchors.verticalCenter: parent.verticalCenter
                    width: 128

                    fullColor: currentDevice.connected
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
                width: parent.width/3

                Column {
                    anchors.left: parent.left
                    anchors.leftMargin: 32
                    anchors.right: parent.right
                    anchors.rightMargin: 32
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 24

                    property int www: (parent.width * 1)
                    property int hhh: (parent.height * 0.4)

                    RemoteButtonWidget { // id: btn1
                        currentButton: currentDevice.btn1
                    }
                }
            }

            Item {
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: parent.width/3

                SchematicPokemonGoPlus {
                    id: pokeball
                    anchors.centerIn: parent
                }
            }

            Item {
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: parent.width/3

                Column {
                    anchors.left: parent.left
                    anchors.leftMargin: 32
                    anchors.right: parent.right
                    anchors.rightMargin: 32
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 24

                    property int www: (parent.width * 1)
                    property int hhh: (parent.height * 0.4)

                    // nothing...
                }
            }
        }

        ////////////////////////////////////////////////////////////////////////
    }
}
