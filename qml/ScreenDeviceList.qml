import QtQuick
import QtQuick.Controls

import ComponentLibrary

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

            //visible: (isDesktop) || (isMobile && networkClient.connected) || (SettingsManager.fakeIt)

            ////////////////

            Loader {
                anchors.left: parent.left
                anchors.right: parent.right

                active: isDesktop
                asynchronous: false
                sourceComponent: DeviceListHeader_desktop { }
            }

            Loader {
                anchors.left: parent.left
                anchors.right: parent.right

                active: isMobile
                asynchronous: false
                sourceComponent: DeviceListHeader_mobile { }
            }

            ////////////////

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
