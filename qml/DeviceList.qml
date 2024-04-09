import QtQuick
import QtQuick.Controls

import ThemeEngine

Item {
    id: screenDeviceList
    anchors.fill: parent

    ////////////////////////////////////////////////////////////////////////////

    function loadScreen() {
        // check BLE status
        checkBluetoothStatus()

        // change screen
        appContent.state = "DeviceList"
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
        if (!utilsApp.checkMobileBleLocationPermission()) {
            utilsApp.getMobileBleLocationPermission()
        }

        if (deviceManager.hasDevices) {
            // The device list is shown
            itemStatus.source = ""

            if (!deviceManager.bluetooth) {
                rectangleBluetoothStatus.setBluetoothWarning()
            } else if (!deviceManager.bluetoothPermissions) {
                rectangleBluetoothStatus.setPermissionWarning()
            } else {
                rectangleBluetoothStatus.hide()
            }
        } else {
            // The device list is not populated
            rectangleBluetoothStatus.hide()

            if (!deviceManager.bluetooth) {
                itemStatus.source = "ItemNoBluetooth.qml"
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

        ////////////////

        Rectangle {
            id: rectangleBluetoothStatus
            anchors.left: parent.left
            anchors.right: parent.right

            height: 0
            Behavior on height { NumberAnimation { duration: 133 } }

            clip: true
            visible: (height > 0)
            color: Theme.colorActionbar

            // prevent clicks below this area
            MouseArea { anchors.fill: parent; acceptedButtons: Qt.AllButtons; }

            Text {
                id: textBluetoothStatus
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16

                color: Theme.colorActionbarContent
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignLeft
                font.bold: isDesktop ? true : false
                font.pixelSize: Theme.componentFontSize
            }

            ButtonWireframe {
                id: buttonBluetoothStatus
                height: 32
                anchors.right: parent.right
                anchors.rightMargin: 16
                anchors.verticalCenter: parent.verticalCenter

                fullColor: true
                primaryColor: Theme.colorActionbarHighlight

                text: {
                    if (Qt.platform.os === "android") {
                        if (!deviceManager.bluetoothEnabled) return qsTr("Enable")
                        else if (!deviceManager.bluetoothPermissions) return qsTr("About")
                    }
                    return qsTr("Retry")
                }
                onClicked: {
                    if (Qt.platform.os === "android" && !deviceManager.bluetoothPermissions) {
                        //utilsApp.getMobileBleLocationPermission()
                        //deviceManager.checkBluetoothPermissions()

                        // someone clicked 'never ask again'?
                        screenAboutPermissions.loadScreenFrom("DeviceList")
                    } else {
                        deviceManager.enableBluetooth(settingsManager.bluetoothControl)
                    }
                }
            }

            function hide() {
                rectangleBluetoothStatus.height = 0
            }
            function setBluetoothWarning() {
                textBluetoothStatus.text = qsTr("Bluetooth is disabled...")
                rectangleBluetoothStatus.height = 48
            }
            function setPermissionWarning() {
                textBluetoothStatus.text = qsTr("Bluetooth permission is missing...")
                rectangleBluetoothStatus.height = 48
            }
        }

        ////////////////

        Rectangle {
            id: rectangleActions
            anchors.left: parent.left
            anchors.right: parent.right

            height: (screenDeviceList.selectionCount) ? 48 : 0
            Behavior on height { NumberAnimation { duration: 133 } }

            clip: true
            visible: (height > 0)
            color: Theme.colorActionbar

            // prevent clicks below this area
            MouseArea { anchors.fill: parent; acceptedButtons: Qt.AllButtons; }

            Row {
                anchors.left: parent.left
                anchors.leftMargin: 12
                anchors.verticalCenter: parent.verticalCenter
                spacing: 8

                RoundButtonIcon {
                    id: buttonClear
                    width: 36
                    height: 36
                    anchors.verticalCenter: parent.verticalCenter

                    source: "qrc:/assets/icons/material-symbols/backspace-fill.svg"
                    rotation: 180
                    iconColor: Theme.colorActionbarContent
                    backgroundColor: Theme.colorActionbarHighlight
                    onClicked: screenDeviceList.exitSelectionMode()
                }

                Text {
                    id: textActions
                    anchors.verticalCenter: parent.verticalCenter

                    text: qsTr("%n device(s) selected", "", screenDeviceList.selectionCount)
                    color: Theme.colorActionbarContent
                    font.bold: true
                    font.pixelSize: Theme.componentFontSize
                }
            }

            Row {
                anchors.right: parent.right
                anchors.rightMargin: 12
                anchors.verticalCenter: parent.verticalCenter
                spacing: 8

                ButtonCompactable {
                    id: buttonDelete
                    height: compact ? 36 : 34
                    anchors.verticalCenter: parent.verticalCenter

                    compact: !wideMode
                    iconColor: Theme.colorActionbarContent
                    backgroundColor: Theme.colorActionbarHighlight
                    onClicked: confirmDeleteDevice.open()

                    text: qsTr("Delete")
                    source: "qrc:/assets/icons/material-symbols/delete.svg"
                }
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    Loader {
        id: itemStatus
        anchors.fill: parent
        asynchronous: true
    }

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

        ////////////////

        header: Column {
            anchors.left: parent.left
            anchors.right: parent.right

            spacing: 8

            Text {
                height: 40
                visible: isDesktop

                text: qsTr("Local control(s)")
                font.pixelSize: Theme.fontSizeContentBig
                verticalAlignment: Text.AlignVCenter
                color: Theme.colorText
            }

            Row {
                anchors.left: parent.left
                anchors.leftMargin: 8
                anchors.right: parent.right

                visible: isDesktop
                spacing: 12

                Rectangle { // KEYBOARD
                    width: 420
                    height: 128
                    radius: 4

                    color: Theme.colorDeviceWidget
                    border.width: 2
                    border.color: singleColumn ? "transparent" : Theme.colorSeparator

                    Row {
                        anchors.centerIn: parent
                        spacing: 6

                        RoundButtonIcon {
                            anchors.verticalCenter: parent.verticalCenter

                            backgroundVisible: true
                            highlightMode: "color"
                            source: "qrc:/assets/icons/material-symbols/skip_previous.svg"
                            onClicked: localControls.keyboard_media_prev()
                        }
                        RoundButtonIcon {
                            anchors.verticalCenter: parent.verticalCenter

                            backgroundVisible: true
                            highlightMode: "color"
                            source: "qrc:/assets/icons/material-symbols/play_pause.svg"
                            onClicked: localControls.keyboard_media_playpause()
                        }
                        RoundButtonIcon {
                            anchors.verticalCenter: parent.verticalCenter

                            backgroundVisible: true
                            highlightMode: "color"
                            source: "qrc:/assets/icons/material-symbols/skip_next.svg"
                            onClicked: localControls.keyboard_media_next()
                        }

                        IconSvg {
                            anchors.verticalCenter: parent.verticalCenter

                            width: 64
                            height: 64

                            color: Theme.colorSeparator
                            opacity: 0.5
                            source: "qrc:/assets/gfx/icons/keyboard-variant.svg"
                        }

                        RoundButtonIcon {
                            anchors.verticalCenter: parent.verticalCenter

                            backgroundVisible: true
                            highlightMode: "color"
                            source: "qrc:/assets/icons/material-icons/duotone/volume_off.svg"
                            onClicked: localControls.keyboard_volume_mute()
                        }
                        RoundButtonIcon {
                            anchors.verticalCenter: parent.verticalCenter

                            backgroundVisible: true
                            highlightMode: "color"
                            source: "qrc:/assets/icons/material-icons/duotone/volume_down.svg"
                            onClicked: localControls.keyboard_volume_down()
                        }
                        RoundButtonIcon {
                            anchors.verticalCenter: parent.verticalCenter

                            backgroundVisible: true
                            highlightMode: "color"
                            source: "qrc:/assets/icons/material-icons/duotone/volume_up_1.svg"
                            onClicked: localControls.keyboard_volume_up()
                        }
                    }
                }

                Rectangle { // MPRIS
                    width: 540
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

                        Row {
                            spacing: 4

                            RoundButtonIcon {
                                //visible: mprisControls.canGoPrevious
                                backgroundVisible: true
                                highlightMode: "color"
                                source: "qrc:/assets/icons/material-symbols/skip_previous.svg"
                                onClicked: mprisControls.media_prev()
                            }
                            RoundButtonIcon {
                                //visible: mprisControls.canControl
                                backgroundVisible: true
                                highlightMode: "color"
                                source: "qrc:/assets/icons/material-symbols/play_pause.svg"
                                onClicked: mprisControls.media_playpause()
                            }
                            RoundButtonIcon {
                                //visible: mprisControls.canGoNext
                                backgroundVisible: true
                                highlightMode: "color"
                                source: "qrc:/assets/icons/material-symbols/skip_next.svg"
                                onClicked: mprisControls.media_next()
                            }
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
                            if (mprisControls.playbackStatus == "paused")
                                return "qrc:/assets/icons/material-symbols/media/slideshow.svg"
                            else
                                return "qrc:/assets/icons/material-symbols/media/slideshow.svg"
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

                Rectangle { // VIRTUAL INPUTS
                    width: 420
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
                        source: "qrc:/assets/icons/material-icons/duotone/devices.svg"
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: screenVirtualInputs.loadScreen()
                    }
                }
            }
/*
            Text {
                height: 40
                visible: (isMobile && networkClient.connected)

                text: qsTr("Network control(s)")
                font.pixelSize: Theme.fontSizeContentBig
                verticalAlignment: Text.AlignVCenter
                color: Theme.colorText
            }
*/
            Rectangle { // NETWORK
                width: singleColumn ? parent.width : 480
                height: singleColumn ? 112 : 128
                radius: 4

                visible: (isMobile && networkClient.connected)

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
                    spacing: 4

                    RoundButtonIcon {
                        width: isMobile ? 48 : Theme.componentHeight
                        height: isMobile ? 48 : Theme.componentHeight
                        borderVisible: true
                        backgroundVisible: true
                        backgroundColor: Theme.colorBackground
                        highlightMode: "color"

                        source: "qrc:/assets/icons/material-symbols/skip_previous.svg"
                        onClicked: networkClient.sendPress(4)
                    }
                    RoundButtonIcon {
                        width: isMobile ? 48 : Theme.componentHeight
                        height: isMobile ? 48 : Theme.componentHeight
                        borderVisible: true
                        backgroundVisible: true
                        backgroundColor: Theme.colorBackground
                        highlightMode: "color"

                        source: "qrc:/assets/icons/material-symbols/play_pause.svg"
                        onClicked: networkClient.sendPress(1)
                    }
                    RoundButtonIcon {
                        width: isMobile ? 48 : Theme.componentHeight
                        height: isMobile ? 48 : Theme.componentHeight
                        borderVisible: true
                        backgroundVisible: true
                        backgroundColor: Theme.colorBackground
                        highlightMode: "color"

                        source: "qrc:/assets/icons/material-symbols/skip_next.svg"
                        onClicked: networkClient.sendPress(3)
                    }

                    Item { width: 24; height: 8; } // spacer

                    RoundButtonIcon {
                        width: isMobile ? 48 : Theme.componentHeight
                        height: isMobile ? 48 : Theme.componentHeight
                        borderVisible: true
                        backgroundVisible: true
                        backgroundColor: Theme.colorBackground
                        highlightMode: "color"

                        source: "qrc:/assets/icons/material-icons/duotone/volume_off.svg"
                        onClicked: networkClient.sendPress(5)
                    }
                    RoundButtonIcon {
                        width: isMobile ? 48 : Theme.componentHeight
                        height: isMobile ? 48 : Theme.componentHeight
                        borderVisible: true
                        backgroundVisible: true
                        backgroundColor: Theme.colorBackground
                        highlightMode: "color"

                        source: "qrc:/assets/icons/material-icons/duotone/volume_down.svg"
                        onClicked: networkClient.sendPress(7)
                    }
                    RoundButtonIcon {
                        width: isMobile ? 48 : Theme.componentHeight
                        height: isMobile ? 48 : Theme.componentHeight
                        borderVisible: true
                        backgroundVisible: true
                        backgroundColor: Theme.colorBackground
                        highlightMode: "color"
                        source: "qrc:/assets/icons/material-icons/duotone/volume_up_1.svg"
                        onClicked: networkClient.sendPress(6)
                    }
                }
            }

            Rectangle { // VIRTUAL INPUTS
                width: singleColumn ? parent.width : 480
                height: singleColumn ? 96 : 96
                radius: 4

                visible: (isMobile && networkClient.connected)

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
                    source: "qrc:/assets/icons/material-icons/duotone/devices.svg"
                }
                IconSvg {
                    width: 32
                    height: 32
                    anchors.right: parent.right
                    anchors.rightMargin: singleColumn ? 4 : 12
                    anchors.verticalCenter: parent.verticalCenter

                    color: Theme.colorHighContrast
                    source: "qrc:/assets/icons/material-symbols/chevron_right.svg"
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: screenVirtualInputs.loadScreen()
                }
            }

            Text {
                height: 40
                visible: isDesktop

                text: "Bluetooth Low Energy device(s)"
                font.pixelSize: Theme.fontSizeContentBig
                verticalAlignment: Text.AlignVCenter
                color: Theme.colorText
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

        ScrollBar.vertical: ScrollBar {
            visible: false
            anchors.right: parent.right
            anchors.rightMargin: -6
            policy: ScrollBar.AsNeeded
        }

        model: deviceManager.devicesList
        delegate: DeviceWidget {
            width: devicesView.cellWidth
            height: devicesView.cellHeight
            bigAssMode: devicesView.bigWidget
            singleColumn: (appWindow.singleColumn || devicesView.cellColumnsTarget === 1)
        }

        ////////////////
/*
        footer: Item {
            //
        }
*/
        ////////////////
    }

    ////////////////////////////////////////////////////////////////////////////
}
