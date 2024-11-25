import QtQuick

import ComponentLibrary
import Lighthouse

Rectangle {
    id: appHeader

    anchors.top: parent.top
    anchors.left: parent.left
    anchors.right: parent.right

    height: headerHeight + Math.max(screenPaddingStatusbar, screenPaddingTop)
    color: Theme.colorHeader
    clip: true
    z: 10

    property int headerHeight: 52

    property int headerPosition: 56

    property string headerTitle: utilsApp.appName()

    ////////////////////////////////////////////////////////////////////////////

    property string leftMenuMode: "drawer" // drawer / back / close
    signal leftMenuClicked()

    property string rightMenuMode: "off" // on / off
    signal rightMenuClicked()

    ////////////////////////////////////////////////////////////////////////////

    function rightMenuIsOpen() { return actionMenu.visible; }
    function rightMenuClose() { actionMenu.close(); }

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
    signal deviceDataButtonClicked() // compatibility
    signal deviceHistoryButtonClicked() // compatibility
    signal devicePlantButtonClicked() // compatibility
    signal deviceSettingsButtonClicked() // compatibility

    function setActiveDeviceData() { } // compatibility
    function setActiveDeviceHistory() { } // compatibility
    function setActiveDevicePlant() { } // compatibility
    function setActiveDeviceSettings() { } // compatibility

    ////////////////////////////////////////////////////////////////////////////

    // prevent clicks below this area
    MouseArea { anchors.fill: parent; acceptedButtons: Qt.AllButtons; }

    ActionMenuFixed {
        id: actionMenu

        x: appHeader.width - actionMenu.width - 12
        y: screenPaddingStatusbar + 16

        onMenuSelected: (index) => {
            //console.log("ActionMenu clicked #" + index)
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    Rectangle { // OS statusbar area
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        height: Math.max(screenPaddingStatusbar, screenPaddingTop)
        color: Theme.colorStatusbar
    }

    Item {
        anchors.fill: parent
        anchors.topMargin: Math.max(screenPaddingStatusbar, screenPaddingTop)
        anchors.leftMargin: screenPaddingLeft
        anchors.rightMargin: screenPaddingRight

        Row { // left area
            id: leftArea
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: 4
            anchors.bottom: parent.bottom

            spacing: 4

            ////

            MouseArea { // left button
                width: headerHeight
                height: headerHeight

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
                    width: (headerHeight / 2)
                    height: (headerHeight / 2)

                    source: {
                        if (leftMenuMode === "drawer") return "qrc:/IconLibrary/material-symbols/menu.svg"
                        else if (leftMenuMode === "close") return "qrc:/IconLibrary/material-symbols/close.svg"
                        return "qrc:/IconLibrary/material-symbols/arrow_back.svg"
                    }
                    color: Theme.colorHeaderContent
                }
            }

            ////
        }

        ////////////

        Text { // header title
            height: parent.height
            anchors.left: parent.left
            anchors.leftMargin: headerPosition
            anchors.right: rightArea.left
            anchors.rightMargin: 8
            anchors.verticalCenter: parent.verticalCenter

            text: headerTitle
            color: Theme.colorHeaderContent
            font.bold: true
            font.pixelSize: Theme.fontSizeHeader
            font.capitalization: Font.Capitalize
            verticalAlignment: Text.AlignVCenter
        }

        ////////////

        Row { // right area
            id: rightArea
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.rightMargin: 4
            anchors.bottom: parent.bottom

            spacing: 4

            ////

            Item { // network status
                width: headerHeight
                height: headerHeight
                visible: (appContent.state === "ScreenDeviceList")

                IconSvg {
                    width: 24; height: 24;
                    anchors.centerIn: parent
                    source: networkClient.connected ?
                                "qrc:/IconLibrary/material-symbols/signal_wifi_0_bar.svg" :
                                "qrc:/IconLibrary/material-symbols/signal_wifi_0_bar.svg" // _off.svg
                    color: Theme.colorHeaderContent

                    Rectangle {
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        width: 8; height: 8; radius: 8;
                        color: Theme.colorGreen
                        visible: (networkClient.connected)
                    }
                }
            }

            ////

            Item { // bluetooth status
                width: headerHeight
                height: headerHeight
                visible: (appContent.state === "ScreenDeviceList")

                IconSvg {
                    width: 24; height: 24;
                    anchors.centerIn: parent
                    source: deviceManager.bluetooth ?
                                "qrc:/IconLibrary/material-symbols/sensors/bluetooth.svg" :
                                "qrc:/IconLibrary/material-symbols/sensors/bluetooth_disabled.svg"
                    color: Theme.colorHeaderContent

                    Rectangle {
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        width: 8; height: 8; radius: 8;
                        color: Theme.colorGreen
                        visible: (deviceManager.scanning || deviceManager.listening || deviceManager.syncing)

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
                width: headerHeight
                height: headerHeight

                visible: (deviceManager.bluetooth &&
                          (appContent.state === "DeviceLight"))

                onClicked: {
                    rightMenuClicked()
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
                    width: (headerHeight / 2)
                    height: (headerHeight / 2)
                    anchors.centerIn: parent

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
