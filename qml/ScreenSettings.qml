import QtQuick
import QtQuick.Controls

import ComponentLibrary
import Lighthouse

Loader {
    id: screenSettings
    anchors.fill: parent

    ////////////////////////////////////////////////////////////////////////////

    function loadScreen() {
        // load screen
        screenSettings.active = true

        // change screen
        appContent.state = "ScreenSettings"
    }

    function backAction() {
        if (screenSettings.status === Loader.Ready)
            screenSettings.item.backAction()
    }

    ////////////////////////////////////////////////////////////////////////////

    active: false
    asynchronous: false

    sourceComponent: Flickable {
        anchors.fill: parent

        contentWidth: -1
        contentHeight: contentColumn.height

        boundsBehavior: isDesktop ? Flickable.OvershootBounds : Flickable.DragAndOvershootBounds
        ScrollBar.vertical: ScrollBar { visible: false }

        function backAction() {
            screenDeviceList.loadScreen()
        }

        Column {
            id: contentColumn
            anchors.left: parent.left
            anchors.right: parent.right

            topPadding: 16
            bottomPadding: 16
            spacing: 8

            property int padIcon: singleColumn ? Theme.componentMarginL : Theme.componentMarginL
            property int padText: appHeader.headerPosition

            ////////////////

            ListTitle {
                text: qsTr("Application")
                source: "qrc:/IconLibrary/material-symbols/settings.svg"
            }

            ////////////////

            Item { // element_appTheme
                anchors.left: parent.left
                anchors.right: parent.right
                height: Theme.componentHeightXL

                IconSvg {
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padIcon
                    anchors.verticalCenter: parent.verticalCenter

                    width: 24
                    height: 24
                    color: Theme.colorIcon
                    source: "qrc:/IconLibrary/material-icons/duotone/style.svg"
                }

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padText
                    anchors.right: appTheme_selector.left
                    anchors.rightMargin: Theme.componentMargin
                    anchors.verticalCenter: parent.verticalCenter

                    text: qsTr("Theme")
                    textFormat: Text.PlainText
                    font.pixelSize: Theme.fontSizeContent
                    color: Theme.colorText
                    wrapMode: Text.WordWrap
                    verticalAlignment: Text.AlignVCenter
                }

                Row {
                    id: appTheme_selector
                    anchors.right: parent.right
                    anchors.rightMargin: Theme.componentMargin
                    anchors.verticalCenter: parent.verticalCenter

                    z: 1
                    spacing: 10

                    Rectangle {
                        id: rectangleSnow
                        width: wideWideMode ? 80 : 32
                        height: 32
                        anchors.verticalCenter: parent.verticalCenter

                        radius: 2
                        color: "white"
                        border.color: (settingsManager.appTheme === "THEME_SNOW") ? Theme.colorSubText : "#ddd"
                        border.width: 2

                        Text {
                            anchors.centerIn: parent
                            visible: wideWideMode
                            text: qsTr("snow")
                            textFormat: Text.PlainText
                            color: (settingsManager.appTheme === "THEME_SNOW") ? Theme.colorSubText : "#ccc"
                            font.bold: true
                            font.pixelSize: Theme.fontSizeContentSmall
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: settingsManager.appTheme = "THEME_SNOW"
                        }
                    }
                    Rectangle {
                        id: rectangleDay
                        width: wideWideMode ? 80 : 32
                        height: 32
                        anchors.verticalCenter: parent.verticalCenter

                        radius: 2
                        color: "#FFE400" // day theme colorSecondary
                        border.color: Theme.colorPrimary
                        border.width: (settingsManager.appTheme === "THEME_DAY") ? 2 : 0

                        Text {
                            anchors.centerIn: parent
                            visible: wideWideMode
                            text: qsTr("day")
                            textFormat: Text.PlainText
                            color: "white"
                            font.bold: true
                            font.pixelSize: Theme.fontSizeContentSmall
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: settingsManager.appTheme = "THEME_DAY"
                        }
                    }
                    Rectangle {
                        id: rectangleNight
                        width: wideWideMode ? 80 : 32
                        height: 32
                        anchors.verticalCenter: parent.verticalCenter

                        radius: 2
                        color: "#555151"
                        border.color: Theme.colorPrimary
                        border.width: (settingsManager.appTheme === "THEME_NIGHT") ? 2 : 0

                        Text {
                            anchors.centerIn: parent
                            visible: wideWideMode
                            text: qsTr("night")
                            textFormat: Text.PlainText
                            color: (settingsManager.appTheme === "THEME_NIGHT") ? Theme.colorPrimary : "#ececec"
                            font.bold: true
                            font.pixelSize: Theme.fontSizeContentSmall
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: settingsManager.appTheme = "THEME_NIGHT"
                        }
                    }
                }
            }

            ////////

            Item { // element_appThemeAuto
                anchors.left: parent.left
                anchors.right: parent.right
                height: Theme.componentHeightXL

                IconSvg {
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padIcon
                    anchors.verticalCenter: parent.verticalCenter

                    width: 24
                    height: 24
                    color: Theme.colorIcon
                    source: "qrc:/IconLibrary/material-icons/duotone/brightness_4.svg"
                }

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padText
                    anchors.right: switch_appThemeAuto.left
                    anchors.rightMargin: Theme.componentMargin
                    anchors.verticalCenter: parent.verticalCenter

                    text: qsTr("Automatic dark mode")
                    textFormat: Text.PlainText
                    font.pixelSize: Theme.fontSizeContent
                    color: Theme.colorText
                    wrapMode: Text.WordWrap
                    verticalAlignment: Text.AlignVCenter
                }

                SwitchThemed {
                    id: switch_appThemeAuto
                    anchors.right: parent.right
                    anchors.rightMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    z: 1

                    checked: settingsManager.appThemeAuto
                    onClicked: {
                        settingsManager.appThemeAuto = checked
                        Theme.loadTheme(settingsManager.appTheme)
                    }
                }
            }
            Text { // legend_appThemeAuto
                anchors.left: parent.left
                anchors.leftMargin: contentColumn.padText
                anchors.right: parent.right
                anchors.rightMargin: Theme.componentMargin

                topPadding: -12
                bottomPadding: 12

                text: settingsManager.appThemeAuto ?
                          qsTr("Dark mode will switch on automatically between 9 PM and 9 AM.") :
                          qsTr("Dark mode schedule is disabled.")
                textFormat: Text.PlainText
                wrapMode: Text.WordWrap
                color: Theme.colorSubText
                font.pixelSize: Theme.fontSizeContentSmall
            }

            ////////

            Rectangle { // separator
                anchors.left: parent.left
                anchors.right: parent.right
                height: 1
                visible: isDesktop
                color: Theme.colorSeparator
            }

            ////////

            Item { // element_minimized
                anchors.left: parent.left
                anchors.right: parent.right
                height: Theme.componentHeightXL

                visible: isDesktop

                IconSvg {
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padIcon
                    anchors.verticalCenter: parent.verticalCenter

                    width: 24
                    height: 24
                    color: Theme.colorIcon
                    source: "qrc:/IconLibrary/material-icons/duotone/minimize.svg"
                }

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padText
                    anchors.right: switch_minimized.left
                    anchors.rightMargin: Theme.componentMargin
                    anchors.verticalCenter: parent.verticalCenter

                    text: qsTr("Start application minimized")
                    textFormat: Text.PlainText
                    font.pixelSize: Theme.fontSizeContent
                    color: Theme.colorText
                    wrapMode: Text.WordWrap
                    verticalAlignment: Text.AlignVCenter
                }

                SwitchThemed {
                    id: switch_minimized
                    anchors.right: parent.right
                    anchors.rightMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    z: 1

                    checked: settingsManager.minimized
                    onClicked: settingsManager.minimized = checked
                }
            }

            ////////////////

            Item { // element_service
                anchors.left: parent.left
                anchors.right: parent.right
                height: Theme.componentHeightXL

                visible: isDesktop

                IconSvg {
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padIcon
                    anchors.verticalCenter: parent.verticalCenter

                    width: 24
                    height: 24
                    color: Theme.colorIcon
                    source: "qrc:/IconLibrary/material-symbols/autorenew.svg"
                }

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padText
                    anchors.right: switch_service.left
                    anchors.rightMargin: 0
                    anchors.verticalCenter: parent.verticalCenter

                    text: qsTr("Enable background service")
                    textFormat: Text.PlainText
                    wrapMode: Text.WordWrap
                    font.pixelSize: Theme.fontSizeContent
                    color: Theme.colorText
                    verticalAlignment: Text.AlignVCenter
                }

                SwitchThemed {
                    id: switch_service
                    anchors.right: parent.right
                    anchors.rightMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    z: 1

                    checked: settingsManager.systray
                    onClicked: {
                        if (isMobile) {
                            //
                        } else {
                            settingsManager.systray = checked
                        }
                    }
                }
            }
            Text { // legend_service_desktop
                anchors.left: parent.left
                anchors.leftMargin: contentColumn.padText
                anchors.right: parent.right
                anchors.rightMargin: Theme.componentMargin

                topPadding: -12
                bottomPadding: 0
                visible: isDesktop

                text: settingsManager.systray ?
                          qsTr("Lighthouse will remain active in the background after the window is closed, and show in the notification area.") :
                          qsTr("Lighthouse is only active while the window is open.")
                textFormat: Text.PlainText
                wrapMode: Text.WordWrap
                color: Theme.colorSubText
                font.pixelSize: Theme.fontSizeContentSmall
            }

            ////////

            Item { // element_notifications
                anchors.left: parent.left
                anchors.right: parent.right
                height: Theme.componentHeightXL

                visible: isDesktop
                enabled: false // settingsManager.systray
                opacity: settingsManager.systray ? 1 : 0.4

                IconSvg {
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padIcon
                    anchors.verticalCenter: parent.verticalCenter

                    width: 24
                    height: 24
                    color: Theme.colorIcon
                    source: "qrc:/IconLibrary/material-symbols/notifications.svg"
                }

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padText
                    anchors.right: switch_notifications.left
                    anchors.rightMargin: Theme.componentMargin
                    anchors.verticalCenter: parent.verticalCenter

                    text: qsTr("Enable notifications")
                    textFormat: Text.PlainText
                    wrapMode: Text.WordWrap
                    font.pixelSize: Theme.fontSizeContent
                    color: Theme.colorText
                    verticalAlignment: Text.AlignVCenter
                }

                SwitchThemed {
                    id: switch_notifications
                    anchors.right: parent.right
                    anchors.rightMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    z: 1

                    checked: settingsManager.notifications
                    onClicked: settingsManager.notifications = checked
                }
            }
            Text { // legend_notifications
                anchors.left: parent.left
                anchors.leftMargin: contentColumn.padText
                anchors.right: parent.right
                anchors.rightMargin: Theme.componentMargin

                topPadding: -12
                bottomPadding: 12
                visible: isDesktop
                opacity: settingsManager.systray ? 1 : 0.4

                text: settingsManager.notifications ?
                          qsTr("Notifications are enabled.") :
                          qsTr("Notifications are disabled.")
                textFormat: Text.PlainText
                wrapMode: Text.WordWrap
                color: Theme.colorSubText
                font.pixelSize: Theme.fontSizeContentSmall
            }

            ////////////////

            ListTitle {
                text: qsTr("Bluetooth")
                source: "qrc:/IconLibrary/material-symbols/sensors/bluetooth.svg"

                // Android only
                visible: (Qt.platform.os === "android")
            }

            ////////

            Item { // element_bluetoothControl
                anchors.left: parent.left
                anchors.right: parent.right
                height: Theme.componentHeightXL

                visible: (Qt.platform.os === "android")

                IconSvg {
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padIcon
                    anchors.verticalCenter: parent.verticalCenter

                    width: 24
                    height: 24
                    color: Theme.colorIcon
                    source: "qrc:/IconLibrary/material-icons/outlined/bluetooth_disabled.svg"
                }

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padText
                    anchors.right: switch_bluetoothControl.left
                    anchors.rightMargin: Theme.componentMargin
                    anchors.verticalCenter: parent.verticalCenter

                    text: qsTr("Bluetooth control")
                    textFormat: Text.PlainText
                    wrapMode: Text.WordWrap
                    font.pixelSize: Theme.fontSizeContent
                    color: Theme.colorText
                    verticalAlignment: Text.AlignVCenter
                }

                SwitchThemed {
                    id: switch_bluetoothControl
                    anchors.right: parent.right
                    anchors.rightMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    z: 1

                    checked: settingsManager.bluetoothControl
                    onClicked: settingsManager.bluetoothControl = checked
                }
            }
            Text { // legend_bluetoothControl
                anchors.left: parent.left
                anchors.leftMargin: contentColumn.padText
                anchors.right: parent.right
                anchors.rightMargin: Theme.componentMargin

                topPadding: -12
                bottomPadding: 12
                visible: (Qt.platform.os === "android")

                text: settingsManager.bluetoothControl ?
                          qsTr("Lighthouse will enable your device's Bluetooth in order to operate.") :
                          qsTr("Lighthouse will only operate if your device's Bluetooth is already enabled.")
                textFormat: Text.PlainText
                wrapMode: Text.WordWrap
                color: Theme.colorSubText
                font.pixelSize: Theme.fontSizeContentSmall
            }

            ////////////////

            ListTitle {
                visible: isDesktop
                text: qsTr("Network server")
                source: "qrc:/IconLibrary/material-icons/duotone/list.svg"
            }

            ////////////////

            Item { // element_server
                anchors.left: parent.left
                anchors.right: parent.right
                height: Theme.componentHeightXL

                visible: isDesktop

                IconSvg {
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padIcon
                    anchors.verticalCenter: parent.verticalCenter

                    width: 24
                    height: 24
                    color: Theme.colorIcon
                    source: "qrc:/IconLibrary/material-icons/duotone/devices.svg"
                }

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padText
                    anchors.right: switch_server.left
                    anchors.rightMargin: 0
                    anchors.verticalCenter: parent.verticalCenter

                    text: qsTr("Enable network server")
                    textFormat: Text.PlainText
                    wrapMode: Text.WordWrap
                    font.pixelSize: Theme.fontSizeContent
                    color: Theme.colorText
                    verticalAlignment: Text.AlignVCenter
                }

                SwitchThemed {
                    id: switch_server
                    anchors.right: parent.right
                    anchors.rightMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    z: 1

                    checked: settingsManager.netctrl
                    onClicked: {
                        settingsManager.netctrl = checked

                        if (!networkServer.running && settingsManager.netctrl) networkServer.startServer()
                        if (networkServer.running && !settingsManager.netctrl) networkServer.stopServer()
                    }
                }
            }
            Text { // legend_server_desktop
                anchors.left: parent.left
                anchors.leftMargin: contentColumn.padText
                anchors.right: parent.right
                anchors.rightMargin: Theme.componentMargin

                topPadding: -12
                bottomPadding: 0
                visible: isDesktop

                text: settingsManager.netctrl ?
                          qsTr("Lighthouse will accept network connections, acting as remotes.") :
                          qsTr("Lighthouse won't accept network connections.")
                textFormat: Text.PlainText
                wrapMode: Text.WordWrap
                color: Theme.colorSubText
                font.pixelSize: Theme.fontSizeContentSmall
            }
            Text {
                anchors.left: parent.left
                anchors.leftMargin: contentColumn.padText
                anchors.right: parent.right
                anchors.rightMargin: Theme.componentMargin

                visible: isDesktop && networkServer && networkServer.running

                text: qsTr("Running on %1 @ %2").arg(networkServer.serverAddress).arg(networkServer.serverPort)
                textFormat: Text.PlainText
                wrapMode: Text.WordWrap
                color: Theme.colorSubText
                font.pixelSize: Theme.fontSizeContentSmall
            }

            ////////////////

            Loader { // QrCode generator
                anchors.left: parent.left
                anchors.leftMargin: contentColumn.padText

                active: isDesktop && networkServer && networkServer.running
                asynchronous: true
                source: "components/SettingsQrGenerator.qml"
            }

            ////////////////

            ListTitle {
                anchors.left: parent.left
                visible: isMobile
                text: qsTr("Desktop remote control")
                source: "qrc:/IconLibrary/material-icons/duotone/devices.svg"

                Rectangle {
                    anchors.left: parent.left
                    anchors.leftMargin: 36
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.verticalCenterOffset: 8
                    width: 8
                    height: 8
                    radius: 8

                    visible: (isMobile && networkClient.connected)
                    color: Theme.colorGreen
                }

                RoundButtonSunken {
                    id: buttonReader

                    anchors.right: parent.right
                    anchors.rightMargin: 12
                    anchors.verticalCenter: parent.verticalCenter

                    colorBackground: highlighted ? Theme.colorBackground : Theme.colorForeground
                    source: "qrc:/IconLibrary/material-icons/duotone/qr_code_scanner.svg"

                    onClicked: {
                        if (!utilsApp.checkMobileCameraPermission()) {
                            utilsApp.getMobileCameraPermission()
                        } else {
                            highlighted = !highlighted
                        }
                    }
                }
            }

            ////////

            Loader { // QrCode reader
                anchors.left: parent.left
                anchors.leftMargin: 16
                anchors.right: parent.right
                anchors.rightMargin: 16
                height: active ? (width / 1.333) : 0

                active: isMobile && buttonReader.highlighted
                asynchronous: true
                source: "components/SettingsQrReader.qml"
            }

            ////////

            Item { // element_remoteServer_ip
                anchors.left: parent.left
                anchors.right: parent.right
                height: Theme.componentHeight

                visible: isMobile

                IconSvg {
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padIcon
                    anchors.verticalCenter: parent.verticalCenter

                    width: 24
                    height: 24
                    color: Theme.colorIcon
                    source: "qrc:/assets/icons/material-symbols/storage.svg"
                }

                TextFieldThemed {
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padText
                    anchors.right: parent.right
                    anchors.rightMargin: Theme.componentMargin
                    anchors.verticalCenter: parent.verticalCenter
                    height: 36

                    placeholderText: qsTr("Host")
                    text: settingsManager.netctrlHost
                    selectByMouse: true

                    onEditingFinished: {
                        settingsManager.netctrlHost = text
                        networkClient.connectToServer()
                    }

                    Text {
                        anchors.right: parent.right
                        anchors.rightMargin: Theme.componentMargin
                        anchors.verticalCenter: parent.verticalCenter

                        visible: parent.text
                        text: qsTr("Host")
                        textFormat: Text.PlainText
                        font.pixelSize: Theme.fontSizeContentVerySmall
                        color: Theme.colorSubText
                    }
                }
            }

            ////////

            Item { // element_remoteServer_port
                anchors.left: parent.left
                anchors.right: parent.right
                height: Theme.componentHeight

                visible: isMobile

                IconSvg {
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padIcon
                    anchors.verticalCenter: parent.verticalCenter

                    width: 24
                    height: 24
                    color: Theme.colorIcon
                    source: "qrc:/IconLibrary/material-symbols/pin.svg"
                }

                TextFieldThemed {
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padText
                    anchors.right: parent.right
                    anchors.rightMargin: Theme.componentMargin
                    anchors.verticalCenter: parent.verticalCenter
                    height: 36

                    placeholderText: qsTr("Port")
                    text: settingsManager.netctrlPort
                    validator: IntValidator { bottom: 1; top: 65535; }
                    selectByMouse: true

                    onEditingFinished: {
                        settingsManager.netctrlPort = parseInt(text, 10)
                        networkClient.connectToServer()
                    }

                    Text {
                        anchors.right: parent.right
                        anchors.rightMargin: Theme.componentMargin
                        anchors.verticalCenter: parent.verticalCenter

                        visible: parent.text
                        text: qsTr("Port")
                        textFormat: Text.PlainText
                        font.pixelSize: Theme.fontSizeContentVerySmall
                        color: Theme.colorSubText
                    }
                }
            }

            ////////

            Item { // element_remoteServer_password
                anchors.left: parent.left
                anchors.right: parent.right
                height: Theme.componentHeight

                visible: isMobile

                IconSvg {
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padIcon
                    anchors.verticalCenter: parent.verticalCenter

                    width: 24
                    height: 24
                    color: Theme.colorIcon
                    source: "qrc:/IconLibrary/material-symbols/password.svg"
                }

                TextFieldThemed {
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padText
                    anchors.right: parent.right
                    anchors.rightMargin: Theme.componentMargin
                    anchors.verticalCenter: parent.verticalCenter
                    height: 36

                    placeholderText: qsTr("Password")
                    text: settingsManager.netctrlPassword
                    validator: IntValidator { bottom: 1; top: 65535; }
                    selectByMouse: true

                    onEditingFinished: {
                        settingsManager.netctrlPassword = text
                        networkClient.connectToServer()
                    }

                    Text {
                        anchors.right: parent.right
                        anchors.rightMargin: Theme.componentMargin
                        anchors.verticalCenter: parent.verticalCenter

                        visible: parent.text
                        text: qsTr("Password")
                        textFormat: Text.PlainText
                        font.pixelSize: Theme.fontSizeContentVerySmall
                        color: Theme.colorSubText
                    }
                }
            }

            ////////

            Item { // element_fakeIt
                anchors.left: parent.left
                anchors.right: parent.right
                height: Theme.componentHeight

                visible: isMobile // && !networkClient.connected

                IconSvg {
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padIcon
                    anchors.verticalCenter: parent.verticalCenter

                    width: 24
                    height: 24
                    color: Theme.colorIcon
                    source: "qrc:/IconLibrary/material-symbols/supervisor_account.svg"
                }

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: contentColumn.padText
                    anchors.right: switch_fakeIt.left
                    anchors.rightMargin: Theme.componentMargin
                    anchors.verticalCenter: parent.verticalCenter

                    text: qsTr("Fake it till you make it")
                    textFormat: Text.PlainText
                    font.pixelSize: Theme.fontSizeContent
                    color: Theme.colorText
                    wrapMode: Text.WordWrap
                    verticalAlignment: Text.AlignVCenter
                }

                SwitchThemed {
                    id: switch_fakeIt
                    anchors.right: parent.right
                    anchors.rightMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    z: 1

                    checked: settingsManager.fakeIt
                    onClicked: settingsManager.fakeIt = checked
                }
            }

            ////////
        }
    }
}
