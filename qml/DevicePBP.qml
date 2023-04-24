import QtQuick
import QtQuick.Controls

import ThemeEngine 1.0
import DeviceUtils 1.0
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
    sourceComponent: Item {
        id: itemDeviceRemote // itemdevicePBP
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
    /*
        Timer {
            interval: 60000; running: true; repeat: true;
            onTriggered: updateStatusText()
        }
    */
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
            appContent.state = "DeviceList"
        }

        function isHistoryMode() {
            return false
        }
        function resetHistoryMode() {
            //
        }

        ////////////////////////////////////////////////////////////////////////

        Row {
            anchors.fill: parent

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

                    RemoteButtonWidget { // id: btn2
                        currentButton: currentDevice.btn2
                    }
                }
            }

            Item {
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: parent.width/3

                ButtonWireframe {
                    anchors.top: parent.top
                    anchors.topMargin: 16
                    anchors.left: parent.left
                    anchors.right: parent.right

                    fullColor: currentDevice.connected
                    text: currentDevice.connected ? qsTr("connected") : qsTr("connect")
                    onClicked: currentDevice.actionConnect()
                }

                SchematicPokeball {
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
    }
}
