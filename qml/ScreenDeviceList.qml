import QtQuick
import QtQuick.Controls

import ComponentLibrary
import Lighthouse

Item {
    id: screenDeviceList
    anchors.fill: parent

    ////////////////////////////////////////////////////////////////////////////

    function loadScreen() {
        // check BLE status
        checkBluetoothStatus()

        // change screen
        appContent.state = "ScreenDeviceList"
    }

    function backAction() {
        if (isSelected()) exitSelectionMode()
    }

    ////////////////////////////////////////////////////////////////////////////

    property bool deviceAvailable: deviceManager.hasDevices
    onDeviceAvailableChanged: exitSelectionMode()

    Connections {
        target: deviceManager
        function onBluetoothChanged() { checkBluetoothStatus() }
    }

    function checkBluetoothStatus() {
        if (deviceManager.hasDevices) {
            // The device list is shown
            loaderItemStatus.source = ""

            if (!deviceManager.bluetoothPermissions) {
                actionbarBluetoothStatus.setPermissionWarning()
            } else if (!deviceManager.bluetoothAdapter) {
                actionbarBluetoothStatus.setAdapterWarning()
            } else if (!deviceManager.bluetoothEnabled) {
                actionbarBluetoothStatus.setBluetoothWarning()
            } else {
                actionbarBluetoothStatus.hide()
            }
        } else {
            // The sensor list is not populated
            actionbarBluetoothStatus.hide()

            if (!deviceManager.bluetoothPermissions) {
                loaderItemStatus.source = "ItemNoPermissions.qml"
            } else if (!deviceManager.bluetoothAdapter || !deviceManager.bluetoothEnabled) {
                loaderItemStatus.source = "ItemNoBluetooth.qml"
            } else {
                loaderItemStatus.source = ""
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    property bool selectionMode: false
    property var selectionList: []
    property int selectionCount: 0

    function selectDevice(index) {
        // make sure it's not already selected
        if (deviceManager.getDeviceByProxyIndex(index).selected) return

        // then add
        selectionMode = true
        selectionList.push(index)
        selectionCount++

        deviceManager.getDeviceByProxyIndex(index).selected = true
    }
    function deselectDevice(index) {
        var i = selectionList.indexOf(index)
        if (i > -1) { selectionList.splice(i, 1); selectionCount--; }
        if (selectionList.length <= 0 || selectionCount <= 0) { exitSelectionMode() }

        deviceManager.getDeviceByProxyIndex(index).selected = false
    }
    function exitSelectionMode() {
        selectionMode = false
        selectionList = []
        selectionCount = 0

        for (var i = 0; i < devicesView.count; i++) {
            deviceManager.getDeviceByProxyIndex(i).selected = false
        }
    }

    function updateSelectedDevice() {
        for (var i = 0; i < devicesView.count; i++) {
            if (deviceManager.getDeviceByProxyIndex(i).selected) {
                deviceManager.updateDevice(deviceManager.getDeviceByProxyIndex(i).deviceAddress)
            }
        }
        exitSelectionMode()
    }
    function syncSelectedDevice() {
        for (var i = 0; i < devicesView.count; i++) {
            if (deviceManager.getDeviceByProxyIndex(i).selected) {
                deviceManager.syncDevice(deviceManager.getDeviceByProxyIndex(i).deviceAddress)
            }
        }
        exitSelectionMode()
    }
    function removeSelectedDevice() {
        var devicesAddr = []
        for (var i = 0; i < devicesView.count; i++) {
            if (deviceManager.getDeviceByProxyIndex(i).selected) {
                devicesAddr.push(deviceManager.getDeviceByProxyIndex(i).deviceAddress)
            }
        }
        for (var count = 0; count < devicesAddr.length; count++) {
            deviceManager.removeDevice(devicesAddr[count])
        }
        exitSelectionMode()
    }

    ////////////////////////////////////////////////////////////////////////////

    PopupDeleteDevice {
        id: confirmDeleteDevice
        onConfirmed: screenDeviceList.removeSelectedDevice()
    }

    ////////////////////////////////////////////////////////////////////////////

    Column {
        id: rowbar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        z: 2

        ////////

        ActionbarBluetooth {
            id: actionbarBluetoothStatus
            anchors.left: parent.left
            anchors.right: parent.right
        }

        ////////

        ActionbarSelection {
            id: actionbarSelection
            anchors.left: parent.left
            anchors.right: parent.right
        }

        ////////
    }

    ////////////////////////////////////////////////////////////////////////////

    GridView {
        id: devicesView

        anchors.top: rowbar.bottom
        anchors.topMargin: singleColumn ? 0 : 16
        anchors.left: screenDeviceList.left
        anchors.leftMargin: singleColumn ? 6 : 12
        anchors.right: screenDeviceList.right
        anchors.rightMargin: singleColumn ? 6 : 12
        anchors.bottom: screenDeviceList.bottom
        anchors.bottomMargin: singleColumn ? 0 : 16

        ScrollBar.vertical: ScrollBar {
            visible: false
            anchors.right: parent.right
            anchors.rightMargin: -6
            policy: ScrollBar.AsNeeded
        }

        property int listMargin: (cellColumnsTarget === 1) ? -(Theme.componentMargin / 2) : (Theme.componentMargin / 2)

        ////////////////

        header: Column {
            anchors.left: parent.left
            anchors.right: parent.right

            topPadding: isDesktop ? 4 : 0
            bottomPadding: isDesktop ? 16 : 0
            spacing: isDesktop ? 20 : 8

            visible: (isDesktop) || (isMobile && networkClient.connected) || (settingsManager.fakeIt)

            ListTitle { ////////////////////////////////////////////////////////
                anchors.leftMargin: devicesView.listMargin
                anchors.rightMargin: devicesView.listMargin
                visible: isDesktop
                text: qsTr("Local control(s)")
            }

            Grid {
                anchors.left: parent.left
                anchors.leftMargin: 8
                anchors.right: parent.right
                anchors.rightMargin: 8

                visible: isDesktop
                rows: singleColumn ? 3 : 1
                columns: singleColumn ? 1 : 3
                spacing: 12

                Rectangle { // MPRIS
                    width: singleColumn ? parent.width : 520
                    height: 128
                    radius: 4

                    visible: mprisControls && mprisControls.available

                    color: Theme.colorDeviceWidget
                    border.width: 2
                    border.color: singleColumn ? "transparent" : Theme.colorSeparator

                    Column {
                        anchors.left: parent.left
                        anchors.leftMargin: 16
                        anchors.right: thumbnail.left
                        anchors.rightMargin: 16
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 6

                        Text {
                            anchors.left: parent.left
                            anchors.right: parent.right

                            text: mprisControls.playerName + " / " + mprisControls.playbackStatus
                            font.pixelSize: Theme.fontSizeContentSmall
                            color: Theme.colorSubText
                            elide: Text.ElideRight
                        }

                        Text {
                            anchors.left: parent.left
                            anchors.right: parent.right

                            visible: (text.length > 1)

                            text: mprisControls.metaTitle + " " + mprisControls.metaAlbum
                            font.pixelSize: Theme.fontSizeContentBig
                            color: Theme.colorText
                            elide: Text.ElideRight
                        }

                        MediaButtonRow {
                            btnSize: 36
                            //visible: mprisControls.canControl
                            onMediaPrevious: mprisControls.media_prev()
                            onMediaPlayPause: mprisControls.media_playpause()
                            onMediaNext: mprisControls.media_next()
                        }

                        SliderThemed {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            visible: (mprisControls.position > 0 && mprisControls.metaDuration > 0)

                            from: 0
                            to: mprisControls.metaDuration
                            value: mprisControls.position
                        }
                    }

                    IconSvg {
                        anchors.top: parent.top
                        anchors.topMargin: 8
                        anchors.right: parent.right
                        anchors.rightMargin: 8
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 8
                        width: height

                        source: {
                            if (mprisControls.playbackStatus === "paused")
                                return "qrc:/IconLibrary/material-symbols/media/slideshow.svg"
                            else
                                return "qrc:/IconLibrary/material-symbols/media/slideshow.svg"
                        }
                        color: Theme.colorSeparator
                        opacity: 0.5
                    }

                    Image {
                        id: thumbnail
                        anchors.top: parent.top
                        anchors.topMargin: 8
                        anchors.right: parent.right
                        anchors.rightMargin: 8
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 8
                        width: height

                        source: mprisControls.metaThumbnail
                        sourceSize: Qt.size(width, height)
                        fillMode: Image.PreserveAspectCrop
                    }
                }

                Rectangle { // KEYBOARD
                    width: singleColumn ? parent.width : 420
                    height: 128
                    radius: 4

                    color: Theme.colorDeviceWidget
                    border.width: 2
                    border.color: singleColumn ? "transparent" : Theme.colorSeparator

                    Row {
                        anchors.centerIn: parent
                        spacing: 6

                        MediaButtonRow {
                            anchors.verticalCenter: parent.verticalCenter
                            btnSize: 48
                            onMediaPrevious: localControls.keyboard_media_prev()
                            onMediaPlayPause: localControls.keyboard_media_playpause()
                            onMediaNext: localControls.keyboard_media_next()
                        }

                        IconSvg {
                            anchors.verticalCenter: parent.verticalCenter

                            width: 64
                            height: 64

                            color: Theme.colorSeparator
                            opacity: 0.5
                            source: "qrc:/assets/gfx/icons/keyboard-variant.svg"
                        }

                        VolumeButtonRow {
                            anchors.verticalCenter: parent.verticalCenter
                            btnSize: 48
                            onVolumeMute: localControls.keyboard_volume_mute()
                            onVolumeDown: localControls.keyboard_volume_down()
                            onVolumeUp: localControls.keyboard_volume_up()
                        }
                    }
                }

                Rectangle { // VIRTUAL INPUTS
                    width: singleColumn ? parent.width : 420
                    height: 128
                    radius: 4

                    visible: isDesktop
                    color: Theme.colorDeviceWidget
                    border.width: 2
                    border.color: singleColumn ? "transparent" : Theme.colorSeparator

                    Column {
                        anchors.left: parent.left
                        anchors.leftMargin: 16
                        anchors.verticalCenter: parent.verticalCenter

                        Text {
                            text: "Virtual Controls"
                            textFormat: Text.PlainText
                            color: Theme.colorText
                            font.pixelSize: 22
                            verticalAlignment: Text.AlignVCenter
                        }
                        Text {
                            text: "click to open"
                            textFormat: Text.PlainText
                            color: Theme.colorSubText
                            font.pixelSize: 20
                            verticalAlignment: Text.AlignVCenter
                        }
                    }

                    IconSvg {
                        width: 80
                        height: 80
                        anchors.right: parent.right
                        anchors.rightMargin: 16
                        anchors.verticalCenter: parent.verticalCenter

                        opacity: 0.66
                        color: Theme.colorSeparator
                        source: "qrc:/IconLibrary/material-icons/duotone/devices.svg"
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: screenVirtualInputs.loadScreen()
                    }
                }
            }

            //ListTitle { //////////////////////////////////////////////////////
            //    anchors.leftMargin: devicesView.listMargin
            //    anchors.rightMargin: devicesView.listMargin
            //    visible: (isMobile && networkClient.connected)
            //    text: qsTr("Network control(s)")
            //}

            Rectangle { // NETWORK
                width: singleColumn ? parent.width : 480
                height: singleColumn ? 112 : 128
                radius: 4

                visible: (isMobile && networkClient.connected) || (settingsManager.fakeIt)

                color: Theme.colorDeviceWidget
                border.width: 2
                border.color: singleColumn ? "transparent" : Theme.colorSeparator

                Rectangle {
                    anchors.fill: parent
                    anchors.leftMargin: -8
                    anchors.rightMargin: -8
                    color: Theme.colorForeground
                }

                Row {
                    anchors.centerIn: parent
                    spacing: 16

                    MediaButtonRow {
                        btnSize: 52

                        onMediaPrevious: networkControls.media_prev()
                        onMediaPlayPause: networkControls.media_playpause()
                        onMediaNext: networkControls.media_next()
                    }
                    VolumeButtonRow {
                        btnSize: 52

                        onVolumeMute: networkControls.volume_mute()
                        onVolumeDown: networkControls.volume_down()
                        onVolumeUp: networkControls.volume_up()
                    }
                }
            }

            Rectangle { // VIRTUAL INPUTS
                width: singleColumn ? parent.width : 480
                height: singleColumn ? 96 : 96
                radius: 4

                visible: (isMobile && networkClient.connected) || (settingsManager.fakeIt)

                color: Theme.colorForeground // Theme.colorDeviceWidget
                border.width: 2
                border.color: singleColumn ? "transparent" : Theme.colorSeparator

                Column {
                    anchors.left: parent.left
                    anchors.leftMargin: 12
                    anchors.verticalCenter: parent.verticalCenter

                    Text {
                        text: "Virtual Controls"
                        textFormat: Text.PlainText
                        color: Theme.colorText
                        font.pixelSize: 20
                        verticalAlignment: Text.AlignVCenter
                    }
                    Text {
                        text: "click to open"
                        textFormat: Text.PlainText
                        color: Theme.colorSubText
                        font.pixelSize: 18
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                IconSvg {
                    width: 48
                    height: 48
                    anchors.right: parent.right
                    anchors.rightMargin: 48
                    anchors.verticalCenter: parent.verticalCenter

                    color: Theme.colorHighContrast
                    source: "qrc:/IconLibrary/material-icons/duotone/devices.svg"
                }
                IconSvg {
                    width: 32
                    height: 32
                    anchors.right: parent.right
                    anchors.rightMargin: singleColumn ? 4 : 12
                    anchors.verticalCenter: parent.verticalCenter

                    color: Theme.colorHighContrast
                    source: "qrc:/IconLibrary/material-symbols/chevron_right.svg"
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: screenVirtualInputs.loadScreen()
                }
            }

            ListTitle { ////////////////////////////////////////////////////////
                anchors.leftMargin: devicesView.listMargin
                anchors.rightMargin: devicesView.listMargin
                visible: isDesktop
                text: qsTr("Bluetooth Low Energy device(s)")
            }
        }

        ////////////////

        property bool bigWidget: (!isHdpi || (isTablet && width >= 480))

        property int cellWidthTarget: {
            if (singleColumn) return devicesView.width
            if (isTablet) return (bigWidget ? 350 : 280)
            return (bigWidget ? 440 : 320)
        }
        property int cellColumnsTarget: Math.trunc(devicesView.width / cellWidthTarget)

        cellWidth: (devicesView.width / cellColumnsTarget)
        cellHeight: (bigWidget ? 144 : 100)

        model: deviceManager.devicesList
        delegate: DeviceWidget {
            width: devicesView.cellWidth
            height: devicesView.cellHeight
            bigAssMode: devicesView.bigWidget
            singleColumn: (appWindow.singleColumn || devicesView.cellColumnsTarget === 1)
        }

        ////////////////

        footer: Item {
            //
        }

        ////////////////
    }

    ////////////////////////////////////////////////////////////////////////////

    Loader {
        id: loaderItemStatus
        anchors.fill: parent
        asynchronous: true
    }

    ////////////////////////////////////////////////////////////////////////////
}
