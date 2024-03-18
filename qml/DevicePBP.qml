import QtQuick
import QtQuick.Controls

import ThemeEngine
import DeviceUtils
import "qrc:/js/UtilsDeviceSensors.js" as UtilsDeviceSensors

Loader {
    id: devicePBP

    property var currentDevice: null

    ////////

    function loadDevice(clickedDevice) {
        if (typeof clickedDevice === "undefined" || !clickedDevice) return
        if (!clickedDevice.isPBP) return

        appContent.state = "DevicePBP"

        // set device
        if (clickedDevice === currentDevice) return
        currentDevice = clickedDevice

        // load screen
        devicePBP.active = true
        devicePBP.item.loadDevice()
    }

    ////////

    function backAction() {
        if (devicePBP.status === Loader.Ready)
            devicePBP.item.backAction()
    }

    active: false
    asynchronous: false

    sourceComponent: Item { // itemdevicePBP // Pokeball Plus
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
            console.log("devicePBP // loadDevice() >> " + currentDevice)

            btnClicked = 0

            updateHeader()
            updateData()
        }

        function updateHeader() {
            if (typeof currentDevice === "undefined" || !currentDevice) return
            if (!currentDevice.isPBP) return
            //console.log("devicePBP // updateHeader() >> " + currentDevice)

            // Status
            updateStatusText()
        }

        function updateStatusText() {
            if (typeof currentDevice === "undefined" || !currentDevice) return
            if (!currentDevice.isPBP) return
            //console.log("devicePBP // updateStatusText() >> " + currentDevice)
        }

        function updateData() {
            if (typeof currentDevice === "undefined" || !currentDevice) return
            if (!currentDevice.isPBP) return
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

                SelectorMenu {
                    anchors.verticalCenter: parent.verticalCenter
                    height: 40

                    model: ListModel {
                        ListElement { idx: 1; txt: qsTr("gamepad"); src: ""; sz: 16; }
                        ListElement { idx: 2; txt: qsTr("keyboard"); src: ""; sz: 16; }
                        ListElement { idx: 3; txt: qsTr("mouse"); src: ""; sz: 16; }
                        ListElement { idx: 4; txt: qsTr("button"); src: ""; sz: 16; }
                    }

                    currentSelection: {
                        if (currentDevice.deviceMode === "gamepad") return 1
                        if (currentDevice.deviceMode === "keyboard") return 2
                        if (currentDevice.deviceMode === "mouse") return 3
                        if (currentDevice.deviceMode === "button") return 4
                        return 1
                    }
                    onMenuSelected: (index) => {
                        currentSelection = index
                        if (index === 1) currentDevice.deviceMode = "gamepad"
                        if (index === 2) currentDevice.deviceMode = "keyboard"
                        if (index === 3) currentDevice.deviceMode = "mouse"
                        if (index === 4) currentDevice.deviceMode = "button"
                    }
                }
            }
        }

        ////////////////////////////////////////////////////////////////////////

        Row {
            anchors.fill: parent
            anchors.topMargin: 80

            ////

            Item {
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: parent.width*0.5

                SchematicPokeball { // pokeball 2d
                    id: pokeball_2d
                    width: parent.width * 0.5
                }
            }

            ////

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

                    visible: (currentDevice.deviceMode === "button")

                    property int www: (parent.width * 1)
                    property int hhh: (parent.height * 0.4)

                    RemoteButtonWidget { // btn1
                        currentButton: currentDevice.btn1
                    }

                    RemoteButtonWidget { // btn2
                        currentButton: currentDevice.btn2
                    }
                }

                ////

                Grid {
                    anchors.centerIn: parent
                    spacing: 8
                    columns: 3
                    rows: 3

                    visible: (currentDevice.deviceMode === "keyboard")

                    ItemKey {
                        key: "left click"
                    }
                    Item { width: 80; height: 80; }
                    ItemKey {
                        key: "right click"
                    }

                    Item { width: 80; height: 80; }
                    ItemKey {
                        key: "↑"
                    }
                    Item { width: 80; height: 80; }

                    ItemKey {
                        key: "←"
                    }
                    ItemKey {
                        key: "↓"
                    }
                    ItemKey {
                        key: "→"
                    }
                }
            }

            ////
        }

        ////////////////////////////////////////////////////////////////////////
    }
}
