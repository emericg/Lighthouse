import QtQuick
import QtQuick.Controls

import ThemeEngine 1.0
import DeviceUtils 1.0
import "qrc:/js/UtilsDeviceSensors.js" as UtilsDeviceSensors

Loader {
    id: deviceRemote

    property var currentDevice: null

    ////////

    function loadDevice(clickedDevice) {
        if (typeof clickedDevice === "undefined" || !clickedDevice) return
        if (!clickedDevice.isRemote) return

        appContent.state = "DeviceRemote"

        // set device
        if (clickedDevice === currentDevice) return
        currentDevice = clickedDevice

        // load screen
        deviceRemote.active = true
        deviceRemote.item.loadDevice()
    }

    ////////

    function backAction() {
        if (deviceRemote.status === Loader.Ready)
            deviceRemote.item.backAction()
    }

    active: false
    asynchronous: false

    sourceComponent: Item {
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
            //console.log("DeviceRemote // loadDevice() >> " + currentDevice.deviceModel)

            // YLAI003              // single button
            // YLKG07YL // YLKG08YL // dimmer
            // YLYK01YL             // remote

            // load device screen
            if (currentDevice.deviceModel === "YLAI003")
            {
                deviceYLAI003.active = true
                deviceYLYK01YL.active = false
                deviceYLKG07YL.active = false
            }
            if (currentDevice.deviceModel === "YLKG07YL" ||
                currentDevice.deviceModel === "YLKG08YL")
            {
                deviceYLAI003.active = false
                deviceYLYK01YL.active = false
                deviceYLKG07YL.active = true
            }
            if (currentDevice.deviceModel === "YLYK01YL")
            {
                deviceYLAI003.active = false
                deviceYLYK01YL.active = true
                deviceYLKG07YL.active = false
            }

            btnClicked = 0

            updateHeader()
            updateData()
        }

        function updateHeader() {
            if (typeof currentDevice === "undefined" || !currentDevice) return
            if (!currentDevice.isRemote) return
            //console.log("DeviceRemote // updateHeader() >> " + currentDevice)

            // Status
            updateStatusText()
        }

        function updateStatusText() {
            if (typeof currentDevice === "undefined" || !currentDevice) return
            if (!currentDevice.isRemote) return
            //console.log("DeviceRemote // updateStatusText() >> " + currentDevice)
        }

        function updateData() {
            if (typeof currentDevice === "undefined" || !currentDevice) return
            if (!currentDevice.isRemote) return
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

        Loader { // dimmer
            id: deviceYLKG07YL
            anchors.fill: parent

            active: false
            asynchronous: true

            sourceComponent: Row {
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
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: parent.height*0.2
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

                    property int www: (width * 0.8)
                    property int hhh: (height * 0.4)

                    SchematicDimmer {
                        id: dimmerSchematics
                        anchors.centerIn: parent

                        width: parent.www
                        height: parent.www
                    }

                    RemoteButtonKey { // beacon key
                        anchors.top: dimmerSchematics.bottom
                        anchors.topMargin: 20
                        anchors.horizontalCenter: parent.horizontalCenter
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
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: parent.height*0.2
                        spacing: 24

                        property int www: (parent.width * 1)
                        property int hhh: (parent.height * 0.4)

                        RemoteButtonWidget { //id: btn3
                            anchors.right: parent.right
                            layoutDirection: Qt.RightToLeft
                            currentButton: currentDevice.btn3
                        }
                    }
                }
            }
        }

        ////////////////////////////////////////////////////////////////////////

        Loader { // button
            id: deviceYLAI003
            anchors.fill: parent

            active: false
            asynchronous: true

            sourceComponent: Row {
                anchors.fill: parent

                Item {
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    width: parent.width/3

                    Rectangle {
                        width: parent.width
                        height: 6
                        color: Theme.colorSeparator
                    }

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

                ////////////////

                Item {
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    width: parent.width/3

                    SchematicButton {
                        id: buttonSchematics
                        anchors.centerIn: parent
                    }
                }

                ////////////////

                Item {
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    width: parent.width/3

                    Rectangle {
                        width: parent.width
                        height: 6
                        color: Theme.colorSeparator
                    }

                    Column {
                        anchors.left: parent.left
                        anchors.leftMargin: 32
                        anchors.right: parent.right
                        anchors.rightMargin: 32
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 24

                        property int www: (parent.width * 1)
                        property int hhh: (parent.height * 0.4)
                    }
                }
            }
        }

        ////////////////////////////////////////////////////////////////////////

        Loader { // remote
            id: deviceYLYK01YL
            anchors.fill: parent

            active: false
            asynchronous: true

            sourceComponent: Row {
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

                        RemoteButtonWidget {
                            id: btn1
                            currentButton: currentDevice.btn1
                        }

                        RemoteButtonWidget {
                            id: btn3
                            currentButton: currentDevice.btn3
                        }

                        RemoteButtonWidget {
                            id: btn5
                            currentButton: currentDevice.btn5
                        }
                    }
                }

                ////////////////

                Item {
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    width: parent.width/3

                    SchematicRemote {
                        id: remoteSchematics
                        anchors.centerIn: parent

                        width: height / 3
                        height: parent.height*0.8
                    }
                }

                ////////////////

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

                        RemoteButtonWidget {
                            id: btn2
                            anchors.right: parent.right
                            layoutDirection: Qt.RightToLeft
                            currentButton: currentDevice.btn2
                        }

                        RemoteButtonWidget {
                            id: btn4
                            anchors.right: parent.right
                            layoutDirection: Qt.RightToLeft
                            currentButton: currentDevice.btn4
                        }

                        RemoteButtonWidget {
                            id: btn6
                            anchors.right: parent.right
                            layoutDirection: Qt.RightToLeft
                            currentButton: currentDevice.btn6
                        }
                    }
                }
            }
        }

        ////////////////////////////////////////////////////////////////////////
    }
}
