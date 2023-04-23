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
    /*
        Connections {
            target: settingsManager
            function onAppLanguageChanged() {
                updateData()
            }
        }
    */
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
            //return deviceScreenChart.isIndicator()
        }
        function resetHistoryMode() {
            //deviceScreenChart.resetIndicator()
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

                    ButtonWireframe {
                        width: 160
                        fullColor: true
                        text: qsTr("connect")

                        onClicked: currentDevice.actionConnect()
                    }

                    RemoteButtonWidget { // id: btn1
                        currentButton: currentDevice.btn1
                    }
                }
            }

            Item {
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: parent.width/3

                SchematicPokeball {
                    id: pokeball
                    anchors.centerIn: parent
                }
/*
                IconSvg {
                    id: pokeball
                    anchors.centerIn: parent

                    width: parent.width * 0.66
                    height: parent.width * 0.66

                    source: "qrc:/assets/gfx/pokeball.svg"
                }
*/
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
