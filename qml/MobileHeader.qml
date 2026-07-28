import QtQuick
import QtQuick.Layouts

import ComponentLibrary
import AppUtils

Rectangle {
    id: appHeader

    anchors.top: parent.top
    anchors.left: parent.left
    anchors.right: parent.right

    height: (headerHeight + Math.max(screenPaddingStatusbar, screenPaddingTop) - (headerCompact ? 12 : 0))
    color: Theme.colorHeader
    clip: true
    z: 10

    ////////////////////////////////////////////////////////////////////////////

    property int headerHeight: 56 // vertical

    property int headerPosition: 56 // horizontal

    property bool headerCompact: true // appWindow.headerUnicolor

    property string headerTitle: UtilsApp.appName()

    property string headerSubTitle: ""

    ////////////////////////////////////////////////////////////////////////////

    property string leftMenuMode: "drawer" // drawer / back / close
    signal leftMenuClicked()

    property string rightMenuMode: "off" // on / off
    signal rightMenuClicked()

    function rightMenuIsOpen() { return actionMenu.visible; }
    function rightMenuClose() { actionMenu.close(); }

    ////////////////////////////////////////////////////////////////////////////

    signal deviceConnectButtonClicked()
    signal deviceDisconnectButtonClicked()

    signal deviceRebootButtonClicked()
    signal deviceCalibrateButtonClicked()
    signal deviceWateringButtonClicked()
    signal deviceLedButtonClicked()
    signal deviceRefreshButtonClicked()
    signal deviceRefreshRealtimeButtonClicked()
    signal deviceRefreshHistoryButtonClicked()
    signal deviceClearButtonClicked()
    signal deviceDataButtonClicked() // desktop header compatibility
    signal deviceHistoryButtonClicked() // desktop header compatibility
    signal devicePlantButtonClicked() // desktop header compatibility
    signal deviceSettingsButtonClicked() // desktop header compatibility

    function setActiveDeviceData() { } // desktop header compatibility
    function setActiveDeviceHistory() { } // desktop header compatibility
    function setActiveDevicePlant() { } // desktop header compatibility
    function setActiveDeviceSettings() { } // desktop header compatibility

    ////////////////////////////////////////////////////////////////////////////

    // prevent clicks below this area
    MouseArea { anchors.fill: parent; acceptedButtons: Qt.AllButtons; }

    // Action menu
    ActionMenuFixed { id: actionMenu }

    ////////////////////////////////////////////////////////////////////////////

    Rectangle { // OS statusbar area
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        height: Math.max(screenPaddingStatusbar, screenPaddingTop)
        color: Theme.colorStatusbar
        visible: !appHeader.headerCompact
    }

    Item {
        anchors.left: parent.left
        anchors.leftMargin: screenPaddingLeft
        anchors.right: parent.right
        anchors.rightMargin: screenPaddingRight
        anchors.bottom: parent.bottom
        anchors.bottomMargin: appHeader.headerCompact ? -4 : 0

        height: appHeader.headerHeight

        ////////////

        MouseArea { // left button
            width: appHeader.headerHeight
            height: appHeader.headerHeight

            visible: true
            onClicked: leftMenuClicked()

            RippleThemed {
                anchors.fill: parent
                anchor: parent

                pressed: parent.pressed
                //active: enabled && parent.containsPress
                color: Qt.rgba(Theme.colorForeground.r, Theme.colorForeground.g, Theme.colorForeground.b, 0.33)
            }

            IconSvg {
                anchors.centerIn: parent
                width: (appHeader.headerHeight / 2)
                height: (appHeader.headerHeight / 2)

                source: {
                    if (leftMenuMode === "drawer") return "qrc:/IconLibrary/material-symbols/menu.svg"
                    if (leftMenuMode === "close") return "qrc:/IconLibrary/material-symbols/close.svg"
                    return "qrc:/IconLibrary/material-symbols/arrow_back.svg"
                }
                color: Theme.colorHeaderContent
            }
        }

        RowLayout {
            anchors.left: parent.left
            anchors.leftMargin: appHeader.headerPosition
            anchors.right: rightArea.left
            anchors.rightMargin: 8
            anchors.verticalCenter: parent.verticalCenter

            Text { // header title
                Layout.alignment: Qt.AlignVCenter

                text: appHeader.headerTitle
                textFormat: Text.PlainText
                font.bold: true
                font.capitalization: Font.Capitalize
                font.pixelSize: Theme.fontSizeHeader
                color: Theme.colorHeaderContent
                elide: Text.ElideRight
            }
        }

        ////////////

        Row { // right area
            id: rightArea
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            spacing: 4

            ////

            MouseArea { // network status
                width: headerHeight
                height: headerHeight

                visible: (appContent.state === "ScreenDeviceList" ||
                          appContent.state === "VirtualInputs")

                onClicked: {
                    if (!networkClient.connected) {
                        networkClient.connectToServer()
                    }
                }

                RippleThemed {
                    anchors.fill: parent
                    anchor: parent

                    pressed: parent.pressed
                    //active: enabled && parent.containsPress
                    color: Qt.rgba(Theme.colorForeground.r, Theme.colorForeground.g, Theme.colorForeground.b, 0.33)
                }

                IconSvg {
                    width: 26; height: 26;
                    anchors.centerIn: parent
                    source: {
                        if (SettingsManager.netctrlSSID) {
                            if (SettingsManager.netctrlSSID === UtilsWiFi.currentSSID) {
                                return "qrc:/IconLibrary/material-symbols/signal_wifi_4_bar.svg"
                            }
                            return "qrc:/IconLibrary/material-symbols/signal_wifi_off.svg"
                        }
                        return "qrc:/IconLibrary/material-symbols/signal_wifi_0_bar.svg"
                    }
                    color: Theme.colorHeaderContent

                    Rectangle {
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        width: 8; height: 8; radius: 8;
                        visible: !(SettingsManager.netctrlSSID && SettingsManager.netctrlSSID !== UtilsWiFi.currentSSID)
                        color: networkClient.connected ? Theme.colorSuccess : Theme.colorWarning
                    }
                }
            }

            ////

            MouseArea { // bluetooth status
                width: headerHeight
                height: headerHeight

                visible: (appContent.state === "ScreenDeviceList" ||
                          appContent.state === "VirtualInputs")

                onClicked: {
                    if (!deviceManager.bluetoothPermissions) {
                        deviceManager.requestBluetoothPermissions()
                    }
                    if (!deviceManager.bluetoothEnabled) {
                        deviceManager.enableBluetooth(SettingsManager.bluetoothControl)
                    }
                    deviceManager.checkBluetooth()
                }

                RippleThemed {
                    anchors.fill: parent
                    anchor: parent

                    pressed: parent.pressed
                    //active: enabled && parent.containsPress
                    color: Qt.rgba(Theme.colorForeground.r, Theme.colorForeground.g, Theme.colorForeground.b, 0.33)
                }

                IconSvg {
                    width: 26; height: 26;
                    anchors.centerIn: parent
                    source: deviceManager.bluetooth ?
                                "qrc:/IconLibrary/material-symbols/bluetooth.svg" :
                                "qrc:/IconLibrary/material-symbols/bluetooth_disabled.svg"
                    color: Theme.colorHeaderContent

                    Rectangle {
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        width: 8; height: 8; radius: 8;
                        visible: deviceManager.bluetooth
                        color: (deviceManager.scanning || deviceManager.listening || deviceManager.syncing) ? Theme.colorSuccess : Theme.colorWarning

                        //SequentialAnimation on opacity { // (fade)
                        //    loops: Animation.Infinite
                        //    running: (deviceManager.scanning || deviceManager.listening || deviceManager.syncing)
                        //    onStopped: workingIndicator.opacity = 0
                        //    PropertyAnimation { to: 1; duration: 750; }
                        //    PropertyAnimation { to: 0.33; duration: 750; }
                        //}
                    }
                }
            }

            ////

            MouseArea { // right menu button
                width: appHeader.headerHeight
                height: appHeader.headerHeight

                visible: (deviceManager.bluetooth && appContent.state === "DeviceLight")

                onClicked: {
                    appHeader.rightMenuClicked()
                    actionMenu.open()
                }

                RippleThemed {
                    anchors.fill: parent
                    anchor: parent

                    pressed: parent.pressed
                    //active: enabled && parent.containsPress
                    color: Qt.rgba(Theme.colorForeground.r, Theme.colorForeground.g, Theme.colorForeground.b, 0.33)
                }

                IconSvg {
                    anchors.centerIn: parent
                    width: (appHeader.headerHeight / 2)
                    height: (appHeader.headerHeight / 2)

                    source: "qrc:/IconLibrary/material-symbols/more_vert.svg"
                    color: Theme.colorHeaderContent
                }
            }

            ////
        }

        ////////////
    }

    ////////////////////////////////////////////////////////////////////////////
}
