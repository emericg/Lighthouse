import QtQuick

import ComponentLibrary
import Lighthouse

Item {
    id: mobileMenu
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom

    property int hhh: 56
    property int hhi: (hhh * 0.4)
    property int hhv: visible ? hhh : 0
    property int www: 80

    height: hhh + screenPaddingNavbar + screenPaddingBottom

    visible: (isTablet && (appContent.state === "ScreenDeviceList" ||
                           appContent.state === "DeviceBeacon" ||
                           appContent.state === "DeviceRemote" ||
                           appContent.state === "DeviceLight" ||
                           appContent.state === "ScreenSettings" ||
                           appContent.state === "ScreenAbout" ||
                           appContent.state === "ScreenAboutPermissions")) ||
             (isPhone && screenOrientation === Qt.PortraitOrientation &&
                          (appContent.state === "DeviceLight"))

    ////////////////////////////////////////////////////////////////////////////

    Rectangle { // background
        anchors.fill: parent

        opacity: appWindow.isTablet ? 0.95 : 0.95
        color: appWindow.isTablet ? Theme.colorTabletmenu : Theme.colorBackground

        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 2
            opacity: 0.33
            visible: !appWindow.isPhone
            color: Theme.colorTabletmenuContent
        }
    }

    // prevent clicks below this area
    MouseArea { anchors.fill: parent; acceptedButtons: Qt.AllButtons; }

    ////////////////////////////////////////////////////////////////////////////

    signal deviceDataButtonClicked()
    signal deviceHistoryButtonClicked()
    signal devicePlantButtonClicked()
    signal deviceSettingsButtonClicked()

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

    ////////////////////////////////////////////////////////////////////////////

    Item { // menu area
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        height: mobileMenu.hhh

        ////////////////////////

        Row { // main menu
            anchors.centerIn: parent
            spacing: (!appWindow.wideMode || (isPhone && utilsScreen.screenSize < 5.0)) ? -10 : 20

            visible: (appContent.state === "ScreenDeviceList" ||
                      appContent.state === "ScreenSettings" ||
                      appContent.state === "ScreenAbout" ||
                      appContent.state === "ScreenAboutPermissions")

            MobileMenuItem_horizontal {
                id: menuMainView
                height: mobileMenu.hhh

                text: qsTr("Device list")
                source: "qrc:/IconLibrary/material-symbols/hardware/lightbulb.svg"
                sourceSize: mobileMenu.hhi
                colorContent: Theme.colorTabletmenuContent
                colorHighlight: Theme.colorTabletmenuHighlight

                highlighted: (appContent.state === "ScreenDeviceList")
                onClicked: screenDeviceList.loadScreen()
            }
            MobileMenuItem_horizontal {
                id: menuSettings
                height: mobileMenu.hhh

                text: qsTr("Settings")
                source: "qrc:/IconLibrary/material-symbols/settings.svg"
                sourceSize: mobileMenu.hhi
                colorContent: Theme.colorTabletmenuContent
                colorHighlight: Theme.colorTabletmenuHighlight

                highlighted: (appContent.state === "ScreenSettings")
                onClicked: screenSettings.loadScreen()
            }
            MobileMenuItem_horizontal {
                id: menuAbout
                height: mobileMenu.hhh

                text: qsTr("About")
                source: "qrc:/IconLibrary/material-symbols/info.svg"
                sourceSize: mobileMenu.hhi
                colorContent: Theme.colorTabletmenuContent
                colorHighlight: Theme.colorTabletmenuHighlight

                highlighted: (appContent.state === "ScreenAbout" ||
                              appContent.state === "ScreenAboutPermissions")
                onClicked: screenAbout.loadScreen()
            }
        }

        ////////////////////////
    }

    ////////////////////////////////////////////////////////////////////////////
}
