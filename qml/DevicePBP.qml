import QtQuick
import QtQuick.Controls

import ComponentLibrary
import DeviceUtils
import Lighthouse

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

                SelectorMenuColorful {
                    id: selectorPbpView
                    anchors.verticalCenter: parent.verticalCenter
                    height: 40

                    model: ListModel {
                        ListElement { idx: 1; txt: qsTr("2D"); src: ""; sz: 16; }
                        ListElement { idx: 2; txt: qsTr("3D"); src: ""; sz: 16; }
                    }

                    currentSelection: 1
                    onMenuSelected: (index) => {
                        currentSelection = index
                    }
                }

                SelectorMenuColorful {
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
/*
        Column { // DEBUG PBP AXIS
            anchors.top: parent.top
            anchors.topMargin: 24 + 72
            anchors.left: parent.left
            anchors.leftMargin: 24

            Row {
                DataBarSolid {
                    legend: "x"
                    value: currentDevice.gyroX
                    valueMin: -180
                    valueMax: 180
                    floatprecision: 1
                }
                DataBarSolid {
                    legend: "y"
                    value: currentDevice.gyroY
                    floatprecision: 1
                    valueMin: -180
                    valueMax: 180
                }
                DataBarSolid {
                    legend: "z"
                    value: currentDevice.gyroZ
                    valueMin: -180
                    valueMax: 180
                    floatprecision: 1
                }
            }
            Row {
                DataBarSolid {
                    legend: "x"
                    value: currentDevice.acclX
                    valueMin: -180
                    valueMax: 180
                    floatprecision: 1
                }
                DataBarSolid {
                    legend: "y"
                    value: currentDevice.acclY
                    floatprecision: 1
                    valueMin: -180
                    valueMax: 180
                }
                DataBarSolid {
                    legend: "z"
                    value: currentDevice.acclZ
                    valueMin: -180
                    valueMax: 180
                    floatprecision: 1
                }
            }
            Row {
                SliderThemed {
                    id: fakeX
                    value: 0
                    from: -180
                    to: 180
                }
                SliderThemed {
                    id: fakeY
                    value: 0
                    from: -180
                    to: 180
                }
                SliderThemed {
                    id: fakeZ
                    value: 0
                    from: -180
                    to: 180
                }
            }
        }
*/
        ////////////////////////////////////////////////////////////////////////

        Row {
            anchors.fill: parent
            anchors.topMargin: 80

            ////

            Row {
                width: parent.width*0.5

                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter

                SchematicPokeball { // pokeball 2d
                    id: pokeball_2d
                    width: parent.width * 0.5
                    anchors.verticalCenter: parent.verticalCenter

                    visible: (selectorPbpView.currentSelection === 1)
                }

                Pokeball3dView { // pokeball 3d
                    id: pokeball_3d
                    width: parent.width * 0.66
                    anchors.verticalCenter: parent.verticalCenter

                    visible: (selectorPbpView.currentSelection === 2)

                    // DEBUG PBP AXIS
                    //xx : fakeX.value
                    //yy : fakeY.value
                    //zz : fakeZ.value

                    xx : UtilsNumber.mapNumber(currentDevice.gyroX, -18, 18, 90 , -90)
                    //yy : UtilsNumber.mapNumber(currentDevice.gyroZ, -20, 20, 180 , -180) // DISABLED
                    zz : UtilsNumber.mapNumber(currentDevice.gyroY, -18, 18, 180, -180)

                    joyx: currentDevice.axis_x * -6
                    joyy: currentDevice.axis_y * -6
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
                    Item {
                        width: 80; height: 80;
                    }
                    ItemKey {
                        key: "right click"
                    }

                    Item {
                        width: 80; height: 80;
                    }
                    ItemKey {
                        key: "↑"
                    }
                    Item {
                        width: 80; height: 80;
                    }

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
