import QtQuick

import ComponentLibrary
import DeviceUtils
import Lighthouse

Rectangle {
    id: appHeader

    anchors.top: parent.top
    anchors.left: parent.left
    anchors.right: parent.right

    height: headerHeight
    color: Theme.colorHeader
    clip: false
    z: 10

    property int headerHeight: isHdpi ? 60 : 64

    property int headerPosition: 64

    property string headerTitle: "Lighthouse"

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
        menuDeviceData.highlighted = true
        menuDeviceHistory.highlighted = false
        menuDevicePlant.highlighted = false
        menuDeviceSettings.highlighted = false
    }
    function setActiveDeviceHistory() {
        menuDeviceData.highlighted = false
        menuDeviceHistory.highlighted = true
        menuDevicePlant.highlighted = false
        menuDeviceSettings.highlighted = false
    }
    function setActiveDevicePlant() {
        menuDeviceData.highlighted = false
        menuDeviceHistory.highlighted = false
        menuDevicePlant.highlighted = true
        menuDeviceSettings.highlighted = false
    }
    function setActiveDeviceSettings() {
        menuDeviceData.highlighted = false
        menuDeviceHistory.highlighted = false
        menuDevicePlant.highlighted = false
        menuDeviceSettings.highlighted = true
    }

    function setActiveMenu() {
        if (appContent.state === "ScreenTutorial") {
            title.text = qsTr("Welcome")
            menus.visible = false

            buttonBack.source = "qrc:/assets/gfx/icons/menu_close.svg"
        } else {
            title.text = "Lighthouse"
            menus.visible = true

            if (appContent.state === "ScreenDeviceList") {
                buttonBack.source = "qrc:/assets/gfx/logos/logo.svg"
            } else {
                buttonBack.source = "qrc:/assets/gfx/icons/menu_back.svg"
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
        width: 48
        height: 48
        anchors.left: parent.left
        anchors.leftMargin: 12
        anchors.verticalCenter: parent.verticalCenter

        hoverEnabled: (buttonBack.source !== "qrc:/assets/gfx/logos/logo.svg")
        onEntered: { buttonBackBg.opacity = 0.5; }
        onExited: { buttonBackBg.opacity = 0; buttonBack.width = 32; }

        onPressed: buttonBack.width = 28
        onReleased: buttonBack.width = 32
        onClicked: backButtonClicked()

        enabled: (buttonBack.source !== "qrc:/assets/gfx/logos/logo.svg" || wideMode)
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
            width: 32
            height: width
            anchors.centerIn: parent

            source: "qrc:/assets/gfx/icons/menu_back.svg"
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

        ButtonOutline {
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

            visible: (appContent.state === "ScreenDeviceList")
            enabled: visible

            source: "qrc:/IconLibrary/material-symbols/filter_list.svg"
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
            visible: (deviceManager.bluetooth && appContent.state === "ScreenDeviceList")
        }

        // DEVICE MENU //////////

        Row {
            id: menuDevice
            anchors.verticalCenter: parent.verticalCenter
            spacing: 0

            visible: (deviceManager.bluetooth && selectedDevice)

            ButtonCompactable {
                id: buttonDisConnect
                height: compact ? 36 : 34
                anchors.verticalCenter: parent.verticalCenter

                backgroundColor: Theme.colorHeaderHighlight
                textColor: Theme.colorHeaderContent
                iconColor: Theme.colorHeaderContent

                visible: (appContent.state === "DeviceLight" ||
                          appContent.state === "DevicePGP" ||
                          appContent.state === "DevicePBP")

                source: {
                    if (!selectedDevice) return ""
                    if (selectedDevice.status === DeviceUtils.DEVICE_OFFLINE)
                        return "qrc:/IconLibrary/material-icons/outlined/bluetooth_disabled.svg"
                    if (selectedDevice.status === DeviceUtils.DEVICE_QUEUED ||
                        selectedDevice.status === DeviceUtils.DEVICE_CONNECTING ||
                        selectedDevice.status === DeviceUtils.DEVICE_DISCONNECTING)
                        return "qrc:/IconLibrary/material-icons/duotone/bluetooth_searching.svg"
                    if (selectedDevice.status === DeviceUtils.DEVICE_CONNECTED)
                        return "qrc:/IconLibrary/material-icons/duotone/bluetooth_connected.svg"
                }
                tooltipText: {
                    if (!selectedDevice) return ""
                    if (selectedDevice.status === DeviceUtils.DEVICE_OFFLINE) return qsTr("Connect")
                    if (selectedDevice.status === DeviceUtils.DEVICE_DISCONNECTING) return qsTr("Disconnecting...")
                    if (selectedDevice.status === DeviceUtils.DEVICE_QUEUED ||
                        selectedDevice.status === DeviceUtils.DEVICE_CONNECTING) return qsTr("Connecting...")
                    if (selectedDevice.status === DeviceUtils.DEVICE_CONNECTED) return qsTr("Disconnect")
                }

                onClicked: {
                    if (!selectedDevice) return
                    if (selectedDevice.status === DeviceUtils.DEVICE_OFFLINE) deviceConnectButtonClicked()
                    if (selectedDevice.connected) deviceDisconnectButtonClicked()
                }
            }

            Item { // spacer
                anchors.verticalCenter: parent.verticalCenter
                width: 16
                height: 16
                visible: !menuMain.visible
            }
        }

        // MAIN MENU //////////

        Row {
            id: menuMain

            visible: (appContent.state === "ScreenDeviceList" ||
                      appContent.state === "ScreenSettings" ||
                      appContent.state === "ScreenAbout")
            spacing: 0

            DesktopHeaderItem {
                id: menuDevices
                width: headerHeight
                height: headerHeight

                source: "qrc:/IconLibrary/material-icons/duotone/devices.svg"
                colorContent: Theme.colorHeaderContent
                colorHighlight: Theme.colorHeaderHighlight

                highlighted: (appContent.state === "ScreenDeviceList")
                onClicked: devicesButtonClicked()
            }
            DesktopHeaderItem {
                id: menuSettings
                width: headerHeight
                height: headerHeight

                source: "qrc:/IconLibrary/material-icons/duotone/tune.svg"
                colorContent: Theme.colorHeaderContent
                colorHighlight: Theme.colorHeaderHighlight

                highlighted: (appContent.state === "ScreenSettings")
                onClicked: settingsButtonClicked()
            }
            DesktopHeaderItem {
                id: menuAbout
                width: headerHeight
                height: headerHeight

                source: "qrc:/IconLibrary/material-icons/duotone/info.svg"
                colorContent: Theme.colorHeaderContent
                colorHighlight: Theme.colorHeaderHighlight

                highlighted: (appContent.state === "ScreenAbout")
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
                  appContent.state !== "ScreenTutorial")

        height: 2
        opacity: 0.33
        color: Theme.colorHeaderHighlight
    }

    ////////////
}
