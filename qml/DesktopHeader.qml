import QtQuick

import ThemeEngine 1.0

Rectangle {
    id: rectangleHeaderBar
    width: parent.width
    height: headerHeight
    z: 10
    color: Theme.colorHeader

    property int headerHeight: isHdpi ? 60 : 64

    ////////////////////////////////////////////////////////////////////////////

    signal backButtonClicked()
    signal rightMenuClicked() // compatibility

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

    signal deviceDataButtonClicked()
    signal deviceHistoryButtonClicked()
    signal devicePlantButtonClicked()
    signal deviceSettingsButtonClicked()

    signal refreshButtonClicked()
    signal syncButtonClicked()
    signal scanButtonClicked()
    signal devicesButtonClicked()
    signal settingsButtonClicked()
    signal aboutButtonClicked()

    function setActiveDeviceData() {
        menuDeviceData.selected = true
        menuDeviceHistory.selected = false
        menuDevicePlant.selected = false
        menuDeviceSettings.selected = false
    }
    function setActiveDeviceHistory() {
        menuDeviceData.selected = false
        menuDeviceHistory.selected = true
        menuDevicePlant.selected = false
        menuDeviceSettings.selected = false
    }
    function setActiveDevicePlant() {
        menuDeviceData.selected = false
        menuDeviceHistory.selected = false
        menuDevicePlant.selected = true
        menuDeviceSettings.selected = false
    }
    function setActiveDeviceSettings() {
        menuDeviceData.selected = false
        menuDeviceHistory.selected = false
        menuDevicePlant.selected = false
        menuDeviceSettings.selected = true
    }

    function setActiveMenu() {
        if (appContent.state === "Tutorial") {
            title.text = qsTr("Welcome")
            menus.visible = false

            buttonBack.source = "qrc:/assets/menus/menu_close.svg"
        } else {
            title.text = "Lighthouse"
            menus.visible = true

            if (appContent.state === "DeviceList") {
                buttonBack.source = "qrc:/assets/logos/logo.svg"
            } else {
                buttonBack.source = "qrc:/assets/menus/menu_back.svg"
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    DragHandler {
        // make that surface draggable
        // also, prevent clicks below this area
        onActiveChanged: if (active) appWindow.startSystemMove()
        target: null
    }

    MouseArea {
        width: 40
        height: 40
        anchors.left: parent.left
        anchors.leftMargin: 12
        anchors.verticalCenter: parent.verticalCenter

        hoverEnabled: (buttonBack.source !== "qrc:/assets/logos/logo.svg")
        onEntered: { buttonBackBg.opacity = 0.5; }
        onExited: { buttonBackBg.opacity = 0; buttonBack.width = 24; }

        onPressed: buttonBack.width = 20
        onReleased: buttonBack.width = 24
        onClicked: backButtonClicked()

        enabled: (buttonBack.source !== "qrc:/assets/logos/logo.svg" || wideMode)
        visible: enabled

        Rectangle {
            id: buttonBackBg
            anchors.fill: parent
            radius: height
            z: -1
            color: Theme.colorHeaderHighlight
            opacity: 0
            Behavior on opacity { OpacityAnimator { duration: 333 } }
        }

        IconSvg {
            id: buttonBack
            width: 24
            height: width
            anchors.centerIn: parent

            source: "qrc:/assets/menus/menu_logo_large.svg"
            color: Theme.colorHeaderContent
        }
    }

    Text {
        id: title
        anchors.left: parent.left
        anchors.leftMargin: 64
        anchors.right: menus.left
        anchors.rightMargin: 8
        anchors.verticalCenter: parent.verticalCenter

        visible: wideMode

        text: "Lighthouse"
        font.bold: true
        font.pixelSize: Theme.fontSizeHeader
        color: Theme.colorHeaderContent
        elide: Text.ElideRight
    }

    ////////////////////////////////////////////////////////////////////////////

    Row {
        id: menus
        anchors.top: parent.top
        anchors.topMargin: 0
        anchors.right: parent.right
        anchors.rightMargin: 0
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0

        spacing: isHdpi ? 4 : 12
        visible: true

        // MAIN MENU ACTIONS //////////

        ButtonExperimental {
            anchors.verticalCenter: parent.verticalCenter

            visible: networkServer.clientConnected
            text: qsTr("CLIENT CONNECTED")
            color: Theme.colorHeaderContent
        }

        Item { width: 12; height: 12; } // spacer

        // DEVICE ACTIONS //////////

        ButtonCompactable {
            id: buttonSort
            height: compact ? 36 : 34
            anchors.verticalCenter: parent.verticalCenter

            visible: (appContent.state === "DeviceList")
            enabled: visible

            source: "qrc:/assets/icons_material/baseline-filter_list-24px.svg"
            iconColor: Theme.colorHeaderContent
            textColor: Theme.colorHeaderContent
            backgroundColor: Theme.colorHeaderHighlight

            function setText() {
                var txt = qsTr("Order by:") + " "
                if (settingsManager.orderBy === "model") {
                    txt += qsTr("sensor model")
                } else if (settingsManager.orderBy === "location") {
                    txt += qsTr("location")
                }
                buttonSort.tooltipText = txt
            }

            Component.onCompleted: buttonSort.setText()
            Connections {
                target: settingsManager
                function onOrderByChanged() { buttonSort.setText() }
                function onAppLanguageChanged() { buttonSort.setText() }
            }

            property var sortmode: {
                if (settingsManager.orderBy === "model") {
                    return 1
                } else { // if (settingsManager.orderBy === "location") {
                    return 0
                }
            }

            onClicked: {
                sortmode++
                if (sortmode > 3) sortmode = 0

                if (sortmode === 0) {
                    settingsManager.orderBy = "location"
                    deviceManager.orderby_location()
                } else if (sortmode === 1) {
                    settingsManager.orderBy = "model"
                    deviceManager.orderby_model()
                }
            }
        }

        Rectangle { // separator
            anchors.verticalCenter: parent.verticalCenter
            height: 40
            width: Theme.componentBorderWidth
            color: Theme.colorHeaderHighlight
            visible: (deviceManager.bluetooth && appContent.state === "DeviceList")
        }

        // DEVICE MENU //////////

        Row {
            id: menuDevice
            anchors.verticalCenter: parent.verticalCenter
            spacing: 0

            visible: (appContent.state === "DeviceBeacon" ||
                      appContent.state === "DeviceRemote" ||
                      appContent.state === "DeviceLight")

            ButtonCompactable {
                id: buttonDisConnect
                height: compact ? 36 : 34
                anchors.verticalCenter: parent.verticalCenter

                backgroundColor: Theme.colorHeaderHighlight
                textColor: Theme.colorHeaderContent
                iconColor: Theme.colorHeaderContent

                visible: (deviceManager.bluetooth && selectedDevice &&
                          (appContent.state === "DeviceLight"))

                source: (selectedDevice && selectedDevice.connected) ?
                            "qrc:/assets/icons_material/baseline-bluetooth_disabled-24px.svg" :
                            "qrc:/assets/icons_material/duotone-bluetooth_connected-24px.svg"
                tooltipText: (selectedDevice && selectedDevice.connected) ?
                                 qsTr("Disconnect") : qsTr("Connect")

                onClicked: selectedDevice.connected ? deviceDisconnectButtonClicked() : deviceConnectButtonClicked()
            }
        }

        // MAIN MENU //////////

        Row {
            id: menuMain

            visible: (appContent.state === "DeviceList" ||
                      appContent.state === "Settings" ||
                      appContent.state === "About")
            spacing: 0

            DesktopHeaderItem {
                id: menuDevices
                width: headerHeight
                height: headerHeight

                source: "qrc:/assets/icons_material/duotone-devices-24px.svg"
                colorContent: Theme.colorHeaderContent
                colorHighlight: Theme.colorHeaderHighlight

                selected: (appContent.state === "DeviceList")
                onClicked: devicesButtonClicked()
            }
            DesktopHeaderItem {
                id: menuSettings
                width: headerHeight
                height: headerHeight

                source: "qrc:/assets/icons_material/baseline-settings-20px.svg"
                colorContent: Theme.colorHeaderContent
                colorHighlight: Theme.colorHeaderHighlight

                selected: (appContent.state === "Settings")
                onClicked: settingsButtonClicked()
            }
            DesktopHeaderItem {
                id: menuAbout
                width: headerHeight
                height: headerHeight

                source: "qrc:/assets/menus/menu_infos.svg"
                colorContent: Theme.colorHeaderContent
                colorHighlight: Theme.colorHeaderHighlight

                selected: (appContent.state === "About")
                onClicked: aboutButtonClicked()
            }
        }
    }

    ////////////

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        visible: (!headerUnicolor &&
                  appContent.state !== "DeviceThermometer" &&
                  appContent.state !== "DeviceEnvironmental" &&
                  appContent.state !== "Tutorial")

        height: 2
        opacity: 0.33
        color: Theme.colorHeaderHighlight
    }
}
