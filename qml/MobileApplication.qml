import QtQuick
import QtQuick.Window
import QtQuick.Controls

import ComponentLibrary
import MobileUI
import AppUtils

Window {
    id: appWindow

    minimumWidth: 480
    minimumHeight: 960

    flags: Qt.Window | Qt.MaximizeUsingFullscreenGeometryHint
    color: Theme.colorBackground
    visible: true

    // Helpers
    property bool isHdpi: (UtilsScreen.screenDpi >= 128 || UtilsScreen.screenPar >= 2.0)
    property bool isDesktop: (Qt.platform.os !== "android" && Qt.platform.os !== "ios")
    property bool isMobile: (Qt.platform.os === "android" || Qt.platform.os === "ios")
    property bool isPhone: ((Qt.platform.os === "android" || Qt.platform.os === "ios") && (UtilsScreen.screenSize < 7.0))
    property bool isTablet: ((Qt.platform.os === "android" || Qt.platform.os === "ios") && (UtilsScreen.screenSize >= 7.0))

    // Setup ThemeEngine
    Binding { target: Theme; property: "appTheme";               value: SettingsManager.appTheme }
    Binding { target: Theme; property: "appThemeAuto";           value: SettingsManager.appThemeAuto }
    Binding { target: Theme; property: "appThemeAutoMethod";     value: SettingsManager.appThemeAutoMethod }
    Binding { target: Theme; property: "appWidth";               value: appWindow.width }
    Binding { target: Theme; property: "appHeight";              value: appWindow.height }
    Binding { target: Theme; property: "screenOrientation";      value: Screen.primaryOrientation }
    Binding { target: Theme; property: "screenDpiLogical";       value: UtilsScreen.screenDpiLogical }
    Binding { target: Theme; property: "screenDpi";              value: UtilsScreen.screenDpi }
    Binding { target: Theme; property: "screenPar";              value: UtilsScreen.screenPar }
    Binding { target: Theme; property: "screenSize";             value: UtilsScreen.screenSize }
    Binding { target: Theme; property: "screenPaddingStatusbar"; value: MobileUI.statusbarHeight }
    Binding { target: Theme; property: "screenPaddingNavbar";    value: MobileUI.navbarHeight }
    Binding { target: Theme; property: "screenPaddingTop";       value: MobileUI.safeAreaTop }
    Binding { target: Theme; property: "screenPaddingLeft";      value: MobileUI.safeAreaLeft }
    Binding { target: Theme; property: "screenPaddingRight";     value: MobileUI.safeAreaRight }
    Binding { target: Theme; property: "screenPaddingBottom";    value: MobileUI.safeAreaBottom }

    // Setup MobileUI
    Binding { target: MobileUI; property: "statusbarContentColor"; value: Theme.colorStatusbar }
    Binding { target: MobileUI; property: "navbarColor"; value: "transparent" }
    Binding { target: MobileUI; property: "navbarContentColor"; value: {
            if (appContent.state === "ScreenTutorial") return Theme.colorHeader
            return Theme.colorBackground
        }
    }

    // Mobile stuff ////////////////////////////////////////////////////////////

    // 1 = Qt.PortraitOrientation, 2 = Qt.LandscapeOrientation
    // 4 = Qt.InvertedPortraitOrientation, 8 = Qt.InvertedLandscapeOrientation
    property int screenOrientation: Screen.primaryOrientation
    property int screenOrientationFull: Screen.orientation

    property int screenPaddingStatusbar: MobileUI.statusbarHeight
    property int screenPaddingNavbar: MobileUI.navbarHeight

    property int screenPaddingTop: MobileUI.safeAreaTop
    property int screenPaddingLeft: MobileUI.safeAreaLeft
    property int screenPaddingRight: MobileUI.safeAreaRight
    property int screenPaddingBottom: MobileUI.safeAreaBottom

    MobileHeader {
        id: appHeader
    }

    MobileDrawer {
        id: appDrawer

        interactive: (appContent.state !== "ScreenTutorial")
    }

    // Events handling /////////////////////////////////////////////////////////

    Connections {
        target: appHeader
        function onLeftMenuClicked() {
            if (appContent.state === "ScreenDeviceList") {
                appDrawer.open()
            } else {
                if (appContent.state === "ScreenTutorial") {
                    appContent.state = screenTutorial.entryPoint
                } else if (appContent.state === "PlantBrowser") {
                    appContent.state = screenPlantBrowser.entryPoint
                } else if (appContent.state === "ScreenAboutPermissions") {
                    appContent.state = screenAboutPermissions.entryPoint
                } else {
                    screenDeviceList.loadScreen()
                }
            }
        }
        function onRightMenuClicked() {
            //
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
    }

    Connections {
        target: Qt.application
        function onStateChanged() {
            switch (Qt.application.state) {
                case Qt.ApplicationSuspended:
                    //console.log("Qt.ApplicationSuspended")
                    deviceManager.listenDevices_stop()
                    networkClient.disconnectFromServer()
                    break
                case Qt.ApplicationHidden:
                    //console.log("Qt.ApplicationHidden")
                    deviceManager.listenDevices_stop()
                    //networkClient.disconnectFromServer()
                    break
                case Qt.ApplicationInactive:
                    //console.log("Qt.ApplicationInactive")
                    deviceManager.listenDevices_stop()
                    //networkClient.disconnectFromServer()
                    break

                case Qt.ApplicationActive:
                    //console.log("Qt.ApplicationActive")
                    deviceManager.listenDevices_start()
                    networkClient.connectToServer()

                    // Check if we need an 'automatic' theme change
                    Theme.loadTheme(SettingsManager.appTheme)

                    break
            }
        }
    }

    // UI sizes ////////////////////////////////////////////////////////////////

    property bool headerUnicolor: (Theme.colorHeader === Theme.colorBackground)

    property bool singleColumn: {
        if (isMobile) {
            if ((isPhone && screenOrientation === Qt.PortraitOrientation) ||
                (isTablet && width < 512)) { // can be a 2/3 split screen on tablet
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

    property var selectedDevice: null

    FocusScope {
        id: appContent

        anchors.top: appHeader.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.bottomMargin: screenPaddingNavbar + screenPaddingBottom

        focus: true
        Keys.onBackPressed: {
            if (appContent.state === "ScreenTutorial" && screenTutorial.entryPoint === "ScreenDeviceList") {
                return // do nothing
            }

            if (appHeader.rightMenuIsOpen()) {
                appHeader.rightMenuClose()
                return
            }

            if (appContent.state === "ScreenDeviceList") {
                if (screenDeviceList.selectionList.length !== 0) {
                    screenDeviceList.exitSelectionMode()
                } else {
                    if (mobileExit.enabled) {
                        if (mobileExit.timerRunning)
                            Qt.quit()
                        else
                            mobileExit.timerStart()
                    } else {
                        MobileUI.backToHomeScreen()
                    }
                }
            } else if (appContent.state === "VirtualInputs") {
                screenVirtualInputs.backAction()
            } else if (appContent.state === "DeviceLight") {
                screenDeviceLight.backAction()
            } else if (appContent.state === "ScreenAboutPermissions") {
                appContent.state = screenAboutPermissions.entryPoint
            } else if (appContent.state === "ScreenTutorial") {
                appContent.state = screenTutorial.entryPoint
            } else {
                screenDeviceList.loadScreen()
            }
        }

        ScreenTutorial {
            id: screenTutorial
            anchors.bottomMargin: mobileMenu.hhv
        }

        ScreenDeviceList {
            id: screenDeviceList
            anchors.bottomMargin: mobileMenu.hhv
        }
        DeviceLight {
            id: screenDeviceLight
            anchors.bottomMargin: mobileMenu.hhv
        }

        VirtualInputs {
            id: screenVirtualInputs
            anchors.bottomMargin: mobileMenu.hhv
        }

        ScreenSettings {
            id: screenSettings
            anchors.leftMargin: screenPaddingLeft
            anchors.rightMargin: screenPaddingRight
            anchors.bottomMargin: mobileMenu.hhv
        }
        ScreenAbout {
            id: screenAbout
            anchors.leftMargin: screenPaddingLeft
            anchors.rightMargin: screenPaddingRight
            anchors.bottomMargin: mobileMenu.hhv
        }
        MobilePermissions {
            id: screenAboutPermissions
            anchors.leftMargin: screenPaddingLeft
            anchors.rightMargin: screenPaddingRight
            anchors.bottomMargin: mobileMenu.hhv
        }

        // Start on the device list or tutorial?
        Component.onCompleted: {
            if (deviceManager.areDevicesAvailable()) {
                screenDeviceList.loadScreen()
            } else {
                screenDeviceList.loadScreen()
                //screenTutorial.loadScreen()
            }
        }

        // Initial state
        state: "ScreenDeviceList"

        onStateChanged: {
            screenDeviceList.exitSelectionMode()

            if (state === "ScreenDeviceList")
                appHeader.leftMenuMode = "drawer"
            else if (state === "ScreenTutorial")
                appHeader.leftMenuMode = "close"
            else
                appHeader.leftMenuMode = "back"
        }

        states: [
            State {
                name: "ScreenTutorial"
                PropertyChanges { target: appHeader; headerTitle: qsTr("Welcome"); }
                PropertyChanges { target: screenTutorial; visible: true; enabled: true; }
                PropertyChanges { target: screenVirtualInputs; visible: false; enabled: false; }
                PropertyChanges { target: screenDeviceList; visible: false; enabled: false; }
                PropertyChanges { target: screenDeviceLight; visible: false; enabled: false; }
                PropertyChanges { target: screenSettings; visible: false; enabled: false; }
                PropertyChanges { target: screenAbout; visible: false; enabled: false; }
                PropertyChanges { target: screenAboutPermissions; visible: false; enabled: false; }
            },
            State {
                name: "VirtualInputs"
                PropertyChanges { target: appHeader; headerTitle: qsTr("Virtual inputs"); }
                PropertyChanges { target: screenTutorial; visible: false; enabled: false; }
                PropertyChanges { target: screenVirtualInputs; visible: true; enabled: true; }
                PropertyChanges { target: screenDeviceList; visible: false; enabled: false; }
                PropertyChanges { target: screenDeviceLight; visible: false; enabled: false; }
                PropertyChanges { target: screenSettings; visible: false; enabled: false; }
                PropertyChanges { target: screenAboutPermissions; visible: false; enabled: false; }
                PropertyChanges { target: screenAbout; visible: false; enabled: false; }
            },
            State {
                name: "ScreenDeviceList"
                PropertyChanges { target: appHeader; headerTitle: "Lighthouse"; }
                PropertyChanges { target: screenTutorial; visible: false; enabled: false; }
                PropertyChanges { target: screenVirtualInputs; visible: false; enabled: false; }
                PropertyChanges { target: screenDeviceList; visible: true; enabled: true; }
                PropertyChanges { target: screenDeviceLight; visible: false; enabled: false; }
                PropertyChanges { target: screenSettings; visible: false; enabled: false; }
                PropertyChanges { target: screenAbout; visible: false; enabled: false; }
                PropertyChanges { target: screenAboutPermissions; visible: false; enabled: false; }
            },
            State {
                name: "DeviceLight"
                PropertyChanges { target: appHeader; headerTitle: selectedDevice.deviceName; }
                PropertyChanges { target: screenTutorial; visible: false; enabled: false; }
                PropertyChanges { target: screenVirtualInputs; visible: false; enabled: false; }
                PropertyChanges { target: screenDeviceList; visible: false; enabled: false; }
                PropertyChanges { target: screenDeviceLight; visible: true; enabled: true; }
                PropertyChanges { target: screenSettings; visible: false; enabled: false; }
                PropertyChanges { target: screenAbout; visible: false; enabled: false; }
                PropertyChanges { target: screenAboutPermissions; visible: false; enabled: false; }
            },

            State {
                name: "ScreenSettings"
                PropertyChanges { target: appHeader; headerTitle: qsTr("Settings"); }
                PropertyChanges { target: screenTutorial; visible: false; enabled: false; }
                PropertyChanges { target: screenVirtualInputs; visible: false; enabled: false; }
                PropertyChanges { target: screenDeviceList; visible: false; enabled: false; }
                PropertyChanges { target: screenDeviceLight; visible: false; enabled: false; }
                PropertyChanges { target: screenSettings; visible: true; enabled: true; }
                PropertyChanges { target: screenAbout; visible: false; enabled: false; }
                PropertyChanges { target: screenAboutPermissions; visible: false; enabled: false; }
            },
            State {
                name: "ScreenAbout"
                PropertyChanges { target: appHeader; headerTitle: qsTr("About"); }
                PropertyChanges { target: screenTutorial; visible: false; enabled: false; }
                PropertyChanges { target: screenVirtualInputs; visible: false; enabled: false; }
                PropertyChanges { target: screenDeviceList; visible: false; enabled: false; }
                PropertyChanges { target: screenDeviceLight; visible: false; enabled: false; }
                PropertyChanges { target: screenSettings; visible: false; enabled: false; }
                PropertyChanges { target: screenAbout; visible: true; enabled: true; }
                PropertyChanges { target: screenAboutPermissions; visible: false; enabled: false; }
            },
            State {
                name: "ScreenAboutPermissions"
                PropertyChanges { target: appHeader; headerTitle: qsTr("About permissions"); }
                PropertyChanges { target: screenTutorial; visible: false; enabled: false; }
                PropertyChanges { target: screenVirtualInputs; visible: false; enabled: false; }
                PropertyChanges { target: screenDeviceList; visible: false; enabled: false; }
                PropertyChanges { target: screenDeviceLight; visible: false; enabled: false; }
                PropertyChanges { target: screenSettings; visible: false; enabled: false; }
                PropertyChanges { target: screenAbout; visible: false; enabled: false; }
                PropertyChanges { target: screenAboutPermissions; visible: true; enabled: true; }
            }
        ]
    }

    ////////////////////////////////////////////////////////////////////////////

    MobileExit {
        id: mobileExit
    }

    MobileMenu {
        id: mobileMenu
    }

    Rectangle { // navbar area
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: screenPaddingNavbar

        visible: (!mobileMenu.visible || appContent.state === "ScreenTutorial")
        opacity: appWindow.isTablet ? 0.5 : 0.95

        color: {
            if (appContent.state === "ScreenTutorial") return Theme.colorHeader
            return Theme.colorBackground
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    Timer {
        id: disconnectTimer
        interval: 33
        running: false
        repeat: true
        onTriggered: {
            if (!deviceManager.areDevicesConnected()) {
                appWindow.close()
            }
        }
    }
    onClosing: (close) => {
        //console.log("onClosing(" + close + ")")

        // If BLE devices are still connected, disconnect them first
        if (deviceManager.areDevicesConnected()) {
            deviceManager.disconnectDevices()
            disconnectTimer.start()
            close.accepted = false
            return
        }
    }

    ////////////////////////////////////////////////////////////////////////////
}
