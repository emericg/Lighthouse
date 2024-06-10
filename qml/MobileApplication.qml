import QtQuick
import QtQuick.Controls
import QtQuick.Window

import ThemeEngine
import MobileUI

ApplicationWindow {
    id: appWindow
    minimumWidth: 480
    minimumHeight: 960

    flags: Qt.Window | Qt.MaximizeUsingFullscreenGeometryHint
    color: Theme.colorBackground
    visible: true

    property bool isHdpi: (utilsScreen.screenDpi >= 128 || utilsScreen.screenPar >= 2.0)
    property bool isDesktop: (Qt.platform.os !== "android" && Qt.platform.os !== "ios")
    property bool isMobile: (Qt.platform.os === "android" || Qt.platform.os === "ios")
    property bool isPhone: ((Qt.platform.os === "android" || Qt.platform.os === "ios") && (utilsScreen.screenSize < 7.0))
    property bool isTablet: ((Qt.platform.os === "android" || Qt.platform.os === "ios") && (utilsScreen.screenSize >= 7.0))

    // Mobile stuff ////////////////////////////////////////////////////////////

    // 1 = Qt.PortraitOrientation, 2 = Qt.LandscapeOrientation
    // 4 = Qt.InvertedPortraitOrientation, 8 = Qt.InvertedLandscapeOrientation
    property int screenOrientation: Screen.primaryOrientation
    property int screenOrientationFull: Screen.orientation

    property int screenPaddingStatusbar: 0
    property int screenPaddingNavbar: 0

    property int screenPaddingTop: 0
    property int screenPaddingLeft: 0
    property int screenPaddingRight: 0
    property int screenPaddingBottom: 0

    onScreenOrientationChanged: handleSafeAreas()
    onVisibilityChanged: handleSafeAreas()

    function handleSafeAreas() {
        // safe areas are only taken into account when using maximized geometry / full screen mode
        if (appWindow.visibility === Window.FullScreen ||
            appWindow.flags & Qt.MaximizeUsingFullscreenGeometryHint) {

            screenPaddingStatusbar = mobileUI.statusbarHeight
            screenPaddingNavbar = mobileUI.navbarHeight

            screenPaddingTop = mobileUI.safeAreaTop
            screenPaddingLeft = mobileUI.safeAreaLeft
            screenPaddingRight = mobileUI.safeAreaRight
            screenPaddingBottom = mobileUI.safeAreaBottom

            // hacks
            if (Qt.platform.os === "android") {
                if (appWindow.visibility === Window.FullScreen) {
                    screenPaddingStatusbar = 0
                    screenPaddingNavbar = 0
                }
            }
            // hacks
            if (Qt.platform.os === "ios") {
                if (appWindow.visibility === Window.FullScreen) {
                    screenPaddingStatusbar = 0
                }
            }
        } else {
            screenPaddingStatusbar = 0
            screenPaddingNavbar = 0
            screenPaddingTop = 0
            screenPaddingLeft = 0
            screenPaddingRight = 0
            screenPaddingBottom = 0
        }
/*
        console.log("> handleSafeAreas()")
        console.log("- window mode:         " + appWindow.visibility)
        console.log("- window flags:        " + appWindow.flags)
        console.log("- screen dpi:          " + Screen.devicePixelRatio)
        console.log("- screen width:        " + Screen.width)
        console.log("- screen width avail:  " + Screen.desktopAvailableWidth)
        console.log("- screen height:       " + Screen.height)
        console.log("- screen height avail: " + Screen.desktopAvailableHeight)
        console.log("- screen orientation (full): " + Screen.orientation)
        console.log("- screen orientation (primary): " + Screen.primaryOrientation)
        console.log("- screenSizeStatusbar: " + screenPaddingStatusbar)
        console.log("- screenSizeNavbar:    " + screenPaddingNavbar)
        console.log("- screenPaddingTop:    " + screenPaddingTop)
        console.log("- screenPaddingLeft:   " + screenPaddingLeft)
        console.log("- screenPaddingRight:  " + screenPaddingRight)
        console.log("- screenPaddingBottom: " + screenPaddingBottom)
*/
    }

    MobileUI {
        id: mobileUI

        statusbarTheme: Theme.themeStatusbar
        navbarColor: {
            if (appContent.state === "ScreenTutorial") return Theme.colorHeader
            return Theme.colorBackground
        }
    }

    MobileHeader {
        id: appHeader
        width: appWindow.width
    }

    MobileDrawer {
        id: appDrawer
        interactive: (appContent.state !== "ScreenTutorial")
    }

    // Events handling /////////////////////////////////////////////////////////

    property var selectedDevice: null

    Connections {
        target: appHeader
        function onLeftMenuClicked() {
            if (appContent.state === "ScreenDeviceList") {
                appDrawer.open()
            } else {
                if (appContent.state === "ScreenTutorial")
                    appContent.state = screenTutorial.entryPoint
                 else if (appContent.state === "PlantBrowser")
                    appContent.state = screenPlantBrowser.entryPoint
                else if (appContent.state === "ScreenAboutPermissions")
                    appContent.state = screenAboutPermissions.entryPoint
                else
                    screenDeviceList.loadScreen()
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
                    Theme.loadTheme(settingsManager.appTheme)

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
                    if (exitTimer.running)
                        Qt.quit()
                    else
                        exitTimer.start()
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
            anchors.bottomMargin: mobileMenu.hhv
        }
        ScreenAbout {
            id: screenAbout
            anchors.bottomMargin: mobileMenu.hhv
        }
        MobilePermissions {
            id: screenAboutPermissions
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

    ////////////////

    Rectangle { // navbar area
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: screenPaddingNavbar + screenPaddingBottom

        //visible: (mobileMenu.visible || appContent.state === "ScreenTutorial")
        opacity: 0.95
        color: {
            if (appContent.state === "ScreenTutorial") return Theme.colorHeader
            return Theme.colorBackground
        }
    }

    ////////////////

    MobileMenu {
        id: mobileMenu
    }

    ////////////////

    Timer {
        id: exitTimer
        interval: 2222
        running: false
        repeat: false
    }
    Rectangle {
        id: exitWarning

        anchors.left: parent.left
        anchors.leftMargin: Theme.componentMargin
        anchors.right: parent.right
        anchors.rightMargin: Theme.componentMargin
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Theme.componentMargin + screenPaddingNavbar + screenPaddingBottom

        height: Theme.componentHeightL
        radius: Theme.componentRadius

        color: Theme.colorComponentBackground
        border.color: Theme.colorSeparator
        border.width: Theme.componentBorderWidth

        opacity: exitTimer.running ? 1 : 0
        Behavior on opacity { OpacityAnimator { duration: 333 } }
        visible: opacity

        Text {
            anchors.centerIn: parent

            text: qsTr("Press one more time to exit...")
            textFormat: Text.PlainText
            font.pixelSize: Theme.fontSizeContent
            color: Theme.colorText
        }
    }

    ////////////////////////////////////////////////////////////////////////////
}
