import QtQuick
import QtQuick.Controls
import QtQuick.Window

import ThemeEngine 1.0
import DeviceUtils 1.0

ApplicationWindow {
    id: appWindow
    flags: Qt.Window
    color: Theme.colorBackground

    property bool isDesktop: true
    property bool isMobile: false
    property bool isPhone: false
    property bool isTablet: false
    property bool isHdpi: (utilsScreen.screenDpi > 128 || utilsScreen.screenPar > 1.0)

    property var selectedDevice: null

    // Desktop stuff ///////////////////////////////////////////////////////////

    minimumWidth: isHdpi ? 400 : 480
    minimumHeight: isHdpi ? 480 : 560

    width: {
        if (settingsManager.initialSize.width > 0)
            return settingsManager.initialSize.width
        else
            return isHdpi ? 800 : 1280
    }
    height: {
        if (settingsManager.initialSize.height > 0)
            return settingsManager.initialSize.height
        else
            return isHdpi ? 560 : 720
    }
    x: settingsManager.initialPosition.width
    y: settingsManager.initialPosition.height
    visibility: settingsManager.initialVisibility
    visible: true

    WindowGeometrySaver {
        windowInstance: appWindow
        Component.onCompleted: {
            // Make sure we handle window visibility correctly
            if (startMinimized) {
                visible = false
                if (settingsManager.systray) {
                    visibility = Window.Hidden
                } else {
                    visibility = Window.Minimized
                }
            }
        }
    }

    // Mobile stuff ////////////////////////////////////////////////////////////

    property int screenOrientation: Screen.primaryOrientation
    property int screenOrientationFull: Screen.orientation

    property int screenPaddingStatusbar: 0
    property int screenPaddingNotch: 0
    property int screenPaddingLeft: 0
    property int screenPaddingRight: 0
    property int screenPaddingBottom: 0

    Item { // compatibility
        id: mobileMenu
        signal deviceDataButtonClicked()
        signal deviceHistoryButtonClicked()
        signal devicePlantButtonClicked()
        signal deviceSettingsButtonClicked()
    }

    // Events handling /////////////////////////////////////////////////////////

    Component.onCompleted: {
        //
    }

    Connections {
        target: appHeader
        function onBackButtonClicked() {
            if (appContent.state === "Tutorial") {
                appContent.state = screenTutorial.entryPoint
            } else if (appContent.state !== "DeviceList") {
                appContent.state = "DeviceList"
            }
        }

        function onDeviceConnectButtonClicked() {
            if (selectedDevice) {
                selectedDevice.actionConnect()
            }
        }
        function onDeviceDisconnectButtonClicked() {
            if (selectedDevice) {
                selectedDevice.actionDisconnect()
            }
        }

        function onDeviceRefreshRealtimeButtonClicked() {
            if (selectedDevice) {
                selectedDevice.refreshStartRealtime()
            }
        }
        function onDeviceRefreshButtonClicked() {
            if (selectedDevice) {
                deviceManager.updateDevice(selectedDevice.deviceAddress)
            }
        }
        function onRefreshButtonClicked() {
            if (!deviceManager.scanning) {
                if (deviceManager.updating) {
                    deviceManager.refreshDevices_stop()
                } else {
                    deviceManager.refreshDevices_start()
                }
            }
        }
        function onSyncButtonClicked() {
            if (!deviceManager.scanning) {
                if (deviceManager.syncing) {
                    deviceManager.syncDevices_stop()
                } else {
                    deviceManager.syncDevices_start()
                }
            }
        }
        function onScanButtonClicked() {
            if (deviceManager.scanning) {
                deviceManager.scanDevices_stop()
            } else {
                deviceManager.scanDevices_start()
            }
        }

        function onDeviceLedButtonClicked() {
            if (selectedDevice) {
                selectedDevice.actionLedBlink()
            }
        }
        function onDeviceRebootButtonClicked() {
            if (selectedDevice) {
                selectedDevice.actionReboot()
            }
        }

        function onDevicesButtonClicked() { appContent.state = "DeviceList" }
        function onSettingsButtonClicked() { screenSettings.loadScreen() }
        function onAboutButtonClicked() { screenAbout.loadScreen() }
    }

    Connections {
        target: systrayManager
        function onSensorsClicked() { appContent.state = "DeviceList" }
        function onSettingsClicked() { screenSettings.loadScreen() }
    }

    Connections {
        target: menubarManager
        function onSensorsClicked() { appContent.state = "DeviceList" }
        function onSettingsClicked() { screenSettings.loadScreen() }
        function onAboutClicked() { screenAbout.loadScreen() }
        function onTutorialClicked() { screenTutorial.loadScreenFrom(appContent.state) }
    }

    Connections {
        target: Qt.application
        function onStateChanged() {
            switch (Qt.application.state) {

                case Qt.ApplicationInactive:
                    //console.log("Qt.ApplicationInactive")
                    break

                case Qt.ApplicationActive:
                    //console.log("Qt.ApplicationActive")

                    deviceManager.listenDevices_start()
                    mprisControls.select_player()

                    // Check if we need an 'automatic' theme change
                    Theme.loadTheme(settingsManager.appTheme)

                    // Check Bluetooth anyway (on macOS)
                    //if (Qt.platform.os === "osx") deviceManager.checkBluetooth()

                    break
            }
        }
    }

    onVisibilityChanged: (visibility) => {
        //console.log("onVisibilityChanged(" + visibility + ")")

        if (visibility === Window.Hidden) {
            if (settingsManager.systray && Qt.platform.os === "osx") {
                utilsDock.toggleDockIconVisibility(false)
            }
        }
        if (visibility === Window.AutomaticVisibility ||
            visibility === Window.Minimized || visibility === Window.Maximized ||
            visibility === Window.Windowed || visibility === Window.FullScreen) {
             if (settingsManager.systray && Qt.platform.os === "osx") {
                 utilsDock.toggleDockIconVisibility(true)
             }
         }

         if (visibility === Window.Hidden) {
             deviceManager.disconnectDevices()
         }
    }

    onClosing: (close) => {
        //console.log("onClosing(" + close + ")")

        deviceManager.disconnectDevices()

        if (settingsManager.systray || Qt.platform.os === "osx") {
            close.accepted = false
            appWindow.hide()
        }
    }

    // User generated events handling //////////////////////////////////////////

    function backAction() {
        if (appContent.state === "Tutorial" && screenTutorial.entryPoint === "DeviceList") {
            return // do nothing
        }

        if (appContent.state === "DeviceList") {
            if (screenDeviceList.selectionList.length !== 0) {
                screenDeviceList.exitSelectionMode()
            }
        } else if (appContent.state === "DeviceBeacon") {
            screenDeviceBeacon.backAction()
        } else if (appContent.state === "DeviceRemote") {
            screenDeviceRemote.backAction()
        } else if (appContent.state === "DeviceLight") {
            screenDeviceLight.backAction()
        } else if (appContent.state === "Tutorial") {
            appContent.state = screenTutorial.entryPoint
        } else { // default
            if (appContent.previousStates.length) {
                appContent.previousStates.pop()
                appContent.state = appContent.previousStates[appContent.previousStates.length-1]
            } else {
                appContent.state = "DeviceList"
            }
        }
    }
    function forwardAction() {
        if (appContent.state === "DeviceList") {
            appContent.previousStates.pop()

            if (appContent.previousStates[appContent.previousStates.length-1] === "DeviceBeacon")
                appContent.state = "DeviceBeacon"
            else if (appContent.previousStates[appContent.previousStates.length-1] === "DeviceRemote")
                appContent.state = "DeviceRemote"
            else if (appContent.previousStates[appContent.previousStates.length-1] === "DeviceLight")
                appContent.state = "DeviceLight"
        }
    }

    MouseArea {
        anchors.fill: parent
        z: 99
        acceptedButtons: Qt.BackButton | Qt.ForwardButton
        onClicked: (mouse) => {
            if (mouse.button === Qt.BackButton) {
                backAction()
            } else if (mouse.button === Qt.ForwardButton) {
                forwardAction()
            }
        }
    }

    Shortcut {
        sequences: [StandardKey.Back, StandardKey.Backspace]
        onActivated: backAction()
    }
    Shortcut {
        sequences: [StandardKey.Forward]
        onActivated: forwardAction()
    }
    Shortcut {
        sequences: [StandardKey.Refresh]
        onActivated: deviceManager.refreshDevices_check()
    }
    Shortcut {
        sequence: "Ctrl+F5"
        onActivated: deviceManager.refreshDevices_start()
    }
    Shortcut {
        sequence: StandardKey.Preferences
        onActivated: screenSettings.loadScreen()
    }
    Shortcut {
        sequences: [StandardKey.Close]
        onActivated: appWindow.close()
    }
    Shortcut {
        sequence: StandardKey.Quit
        onActivated: utilsApp.appExit()
    }

    // UI sizes ////////////////////////////////////////////////////////////////

    property bool headerUnicolor: (Theme.colorHeader === Theme.colorBackground)

    property bool singleColumn: {
        if (isMobile) {
            if (screenOrientation === Qt.PortraitOrientation ||
                (isTablet && width < 480)) { // can be a 2/3 split screen on tablet
                return true
            } else {
                return false
            }
        } else {
            return (appWindow.width < appWindow.height)
        }
    }

    property bool wideMode: (isDesktop && width >= 560) || (isTablet && width >= 480)
    property bool wideWideMode: (width >= 640)

    // QML /////////////////////////////////////////////////////////////////////

    DesktopHeader {
        id: appHeader

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
    }
/*
    property alias appContent: appContentLoader.item

    Loader {
        id: appContentLoader

        anchors.top: appHeader.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        function load() {
            appContentLoader.active = true
        }
        function unload() {
            appContentLoader.active = false
        }

        active: false

        sourceComponent: Item {
            anchors.fill: parent
*/
    Item {
        id: appContent
        anchors.top: appHeader.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        Tutorial {
            anchors.fill: parent
            id: screenTutorial
        }

        DeviceList {
            anchors.fill: parent
            id: screenDeviceList
        }
        DeviceBeacon {
            anchors.fill: parent
            id: screenDeviceBeacon
        }
        DevicePGP {
            anchors.fill: parent
            id: screenDevicePGP
        }
        DeviceRemote {
            anchors.fill: parent
            id: screenDeviceRemote
        }
        DeviceLight {
            anchors.fill: parent
            id: screenDeviceLight
        }

        Settings {
            anchors.fill: parent
            id: screenSettings
        }
        About {
            anchors.fill: parent
            id: screenAbout
        }

        // Start on the tutorial?
        Component.onCompleted: {
            //
        }

        // Initial state
        state: "DeviceList"

        property var previousStates: []

        onStateChanged: {
            screenDeviceList.exitSelectionMode()
            appHeader.setActiveMenu()

            if (previousStates[previousStates.length-1] !== state) previousStates.push(state)
            if (previousStates.length > 4) previousStates.splice(0, 1)
            //console.log("states > " + appContent.previousStates)
        }

        states: [
            State {
                name: "Tutorial"
                PropertyChanges { target: screenTutorial; visible: true; enabled: true; focus: true; }
                PropertyChanges { target: screenDeviceList; visible: false; enabled: false; }
                PropertyChanges { target: screenDeviceBeacon; visible: false; enabled: false; }
                PropertyChanges { target: screenDevicePGP; visible: false; enabled: false }
                PropertyChanges { target: screenDeviceRemote; visible: false; enabled: false; }
                PropertyChanges { target: screenDeviceLight; visible: false; enabled: false; }
                PropertyChanges { target: screenSettings; visible: false; enabled: false; }
                PropertyChanges { target: screenAbout; visible: false; enabled: false; }
            },
            State {
                name: "DeviceList"
                PropertyChanges { target: screenTutorial; visible: false; enabled: false; }
                PropertyChanges { target: screenDeviceList; visible: true; enabled: true; focus: true; }
                PropertyChanges { target: screenDeviceBeacon; visible: false; enabled: false; }
                PropertyChanges { target: screenDevicePGP; visible: false; enabled: false }
                PropertyChanges { target: screenDeviceRemote; visible: false; enabled: false; }
                PropertyChanges { target: screenDeviceLight; visible: false; enabled: false; }
                PropertyChanges { target: screenSettings; visible: false; enabled: false; }
                PropertyChanges { target: screenAbout; visible: false; enabled: false; }
            },
            State {
                name: "DeviceBeacon"
                PropertyChanges { target: screenTutorial; visible: false; enabled: false; }
                PropertyChanges { target: screenDeviceList; visible: false; enabled: false; }
                PropertyChanges { target: screenDeviceBeacon; visible: true; enabled: true; }
                PropertyChanges { target: screenDevicePGP; visible: false; enabled: false }
                PropertyChanges { target: screenDeviceRemote; visible: false; enabled: false }
                PropertyChanges { target: screenDeviceLight; visible: false; enabled: false; }
                PropertyChanges { target: screenSettings; visible: false; enabled: false; }
                PropertyChanges { target: screenAbout; visible: false; enabled: false; }
            },
            State {
                name: "DevicePGP"
                PropertyChanges { target: screenTutorial; visible: false; enabled: false; }
                PropertyChanges { target: screenDeviceList; visible: false; enabled: false; }
                PropertyChanges { target: screenDeviceBeacon; visible: false; enabled: false; }
                PropertyChanges { target: screenDevicePGP; visible: true; enabled: true; }
                PropertyChanges { target: screenDeviceRemote; visible: false; enabled: false; }
                PropertyChanges { target: screenDeviceLight; visible: false; enabled: false; }
                PropertyChanges { target: screenSettings; visible: false; enabled: false; }
                PropertyChanges { target: screenAbout; visible: false; enabled: false; }
            },
            State {
                name: "DeviceRemote"
                PropertyChanges { target: screenTutorial; visible: false; enabled: false; }
                PropertyChanges { target: screenDeviceList; visible: false; enabled: false; }
                PropertyChanges { target: screenDeviceBeacon; visible: false; enabled: false; }
                PropertyChanges { target: screenDevicePGP; visible: false; enabled: false }
                PropertyChanges { target: screenDeviceRemote; visible: true; enabled: true; }
                PropertyChanges { target: screenDeviceLight; visible: false; enabled: false; }
                PropertyChanges { target: screenSettings; visible: false; enabled: false; }
                PropertyChanges { target: screenAbout; visible: false; enabled: false; }
            },
            State {
                name: "DeviceLight"
                PropertyChanges { target: screenTutorial; visible: false; enabled: false; }
                PropertyChanges { target: screenDeviceList; visible: false; enabled: false; }
                PropertyChanges { target: screenDeviceBeacon; visible: false; enabled: false; }
                PropertyChanges { target: screenDevicePGP; visible: false; enabled: false }
                PropertyChanges { target: screenDeviceRemote; visible: false; enabled: false; }
                PropertyChanges { target: screenDeviceLight; visible: true; enabled: true; }
                PropertyChanges { target: screenSettings; visible: false; enabled: false; }
                PropertyChanges { target: screenAbout; visible: false; enabled: false; }
            },
            State {
                name: "Settings"
                PropertyChanges { target: screenTutorial; visible: false; enabled: false; }
                PropertyChanges { target: screenDeviceList; visible: false; enabled: false; }
                PropertyChanges { target: screenDeviceBeacon; visible: false; enabled: false; }
                PropertyChanges { target: screenDevicePGP; visible: false; enabled: false }
                PropertyChanges { target: screenDeviceRemote; visible: false; enabled: false; }
                PropertyChanges { target: screenDeviceLight; visible: false; enabled: false; }
                PropertyChanges { target: screenSettings; visible: true; enabled: true; focus: true; }
                PropertyChanges { target: screenAbout; visible: false; enabled: false; }
            },
            State {
                name: "About"
                PropertyChanges { target: screenTutorial; visible: false; enabled: false; }
                PropertyChanges { target: screenDeviceList; visible: false; enabled: false; }
                PropertyChanges { target: screenDeviceBeacon; visible: false; enabled: false; }
                PropertyChanges { target: screenDevicePGP; visible: false; enabled: false }
                PropertyChanges { target: screenDeviceRemote; visible: false; enabled: false; }
                PropertyChanges { target: screenDeviceLight; visible: false; enabled: false; }
                PropertyChanges { target: screenSettings; visible: false; enabled: false; }
                PropertyChanges { target: screenAbout; visible: true; enabled: true; focus: true; }
            }
        ]
    }
}
