import QtQuick
import QtQuick.Controls

import ThemeEngine
import "qrc:/js/UtilsNumber.js" as UtilsNumber

Loader {
    id: screenSettings

    ////////////////////////////////////////////////////////////////////////////

    function loadScreen() {
        // load screen
        screenSettings.active = true

        // change screen
        appContent.state = "Settings"
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

            ////////////////

            SectionTitle {
                anchors.left: parent.left
                text: qsTr("Application")
                source: "qrc:/assets/icons_material/baseline-settings-20px.svg"
            }

            ////////////////

            Item {
                id: element_appTheme
                height: 48
                anchors.left: parent.left
                anchors.leftMargin: screenPaddingLeft
                anchors.right: parent.right
                anchors.rightMargin: screenPaddingRight

                IconSvg {
                    id: image_appTheme
                    width: 24
                    height: 24
                    anchors.left: parent.left
                    anchors.leftMargin: 16
                    anchors.verticalCenter: parent.verticalCenter

                    color: Theme.colorIcon
                    source: "qrc:/assets/icons_material/duotone-style-24px.svg"
                }

                Text {
                    id: text_appTheme
                    height: 40
                    anchors.left: image_appTheme.right
                    anchors.leftMargin: 24
                    anchors.right: appTheme_selector.left
                    anchors.rightMargin: 16
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
                    anchors.rightMargin: 16
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

            Item {
                id: element_appThemeAuto
                height: 48
                anchors.left: parent.left
                anchors.leftMargin: screenPaddingLeft
                anchors.right: parent.right
                anchors.rightMargin: screenPaddingRight

                IconSvg {
                    id: image_appThemeAuto
                    width: 24
                    height: 24
                    anchors.left: parent.left
                    anchors.leftMargin: 16
                    anchors.verticalCenter: parent.verticalCenter

                    color: Theme.colorIcon
                    source: "qrc:/assets/icons_material/duotone-brightness_4-24px.svg"
                }

                Text {
                    id: text_appThemeAuto
                    height: 40
                    anchors.left: image_appThemeAuto.right
                    anchors.leftMargin: 24
                    anchors.right: switch_appThemeAuto.left
                    anchors.rightMargin: 16
                    anchors.verticalCenter: parent.verticalCenter

                    text: qsTr("Automatic dark mode")
                    textFormat: Text.PlainText
                    font.pixelSize: Theme.fontSizeContent
                    color: Theme.colorText
                    wrapMode: Text.WordWrap
                    verticalAlignment: Text.AlignVCenter
                }

                SwitchThemedDesktop {
                    id: switch_appThemeAuto
                    anchors.right: parent.right
                    anchors.rightMargin: 0
                    anchors.verticalCenter: parent.verticalCenter
                    z: 1

                    checked: settingsManager.appThemeAuto
                    onClicked: {
                        settingsManager.appThemeAuto = checked
                        Theme.loadTheme(settingsManager.appTheme)
                    }
                }
            }
            Text {
                id: legend_appThemeAuto
                anchors.left: parent.left
                anchors.leftMargin: screenPaddingLeft + 64
                anchors.right: parent.right
                anchors.rightMargin: 12

                topPadding: -12
                bottomPadding: isDesktop ? 12 : 12
                visible: element_appThemeAuto.visible

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

            Item {
                id: element_minimized
                height: 48
                anchors.left: parent.left
                anchors.leftMargin: screenPaddingLeft
                anchors.right: parent.right
                anchors.rightMargin: screenPaddingRight

                visible: isDesktop

                IconSvg {
                    id: image_minimized
                    width: 24
                    height: 24
                    anchors.left: parent.left
                    anchors.leftMargin: 16
                    anchors.verticalCenter: parent.verticalCenter

                    color: Theme.colorIcon
                    source: "qrc:/assets/icons_material/duotone-minimize-24px.svg"
                }

                Text {
                    id: text_minimized
                    height: 40
                    anchors.left: image_minimized.right
                    anchors.leftMargin: 24
                    anchors.right: switch_minimized.left
                    anchors.rightMargin: 16
                    anchors.verticalCenter: parent.verticalCenter

                    text: qsTr("Start application minimized")
                    textFormat: Text.PlainText
                    font.pixelSize: Theme.fontSizeContent
                    color: Theme.colorText
                    wrapMode: Text.WordWrap
                    verticalAlignment: Text.AlignVCenter
                }

                SwitchThemedDesktop {
                    id: switch_minimized
                    anchors.right: parent.right
                    anchors.rightMargin: 0
                    anchors.verticalCenter: parent.verticalCenter
                    z: 1

                    checked: settingsManager.minimized
                    onClicked: settingsManager.minimized = checked
                }
            }

            ////////////////

            Item {
                id: element_service
                height: 48
                anchors.left: parent.left
                anchors.leftMargin: screenPaddingLeft
                anchors.right: parent.right
                anchors.rightMargin: screenPaddingRight

                visible: isDesktop

                IconSvg {
                    id: image_service
                    width: 24
                    height: 24
                    anchors.left: parent.left
                    anchors.leftMargin: screenPaddingLeft + 16
                    anchors.verticalCenter: parent.verticalCenter

                    color: Theme.colorIcon
                    source: "qrc:/assets/icons_material/baseline-autorenew-24px.svg"
                }

                Text {
                    id: text_service
                    height: 40
                    anchors.left: image_service.right
                    anchors.leftMargin: 24
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

                SwitchThemedDesktop {
                    id: switch_service
                    anchors.right: parent.right
                    anchors.rightMargin: 0
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
            Text {
                id: legend_service_desktop
                anchors.left: parent.left
                anchors.leftMargin: screenPaddingLeft + 64
                anchors.right: parent.right
                anchors.rightMargin: 12
                topPadding: -12
                bottomPadding: element_notifications.visible ? 0 : 12

                visible: (element_service.visible && isDesktop)

                text: settingsManager.systray ?
                          qsTr("Lighthouse will remain active in the background after the window is closed, and show in the notification area.") :
                          qsTr("Lighthouse is only active while the window is open.")
                textFormat: Text.PlainText
                wrapMode: Text.WordWrap
                color: Theme.colorSubText
                font.pixelSize: Theme.fontSizeContentSmall
            }

            ////////

            Item {
                id: element_notifications
                height: 48
                anchors.left: parent.left
                anchors.leftMargin: screenPaddingLeft
                anchors.right: parent.right
                anchors.rightMargin: screenPaddingRight

                visible: isDesktop
                enabled: false // settingsManager.systray
                opacity: settingsManager.systray ? 1 : 0.4

                IconSvg {
                    id: image_notifications
                    width: 24
                    height: 24
                    anchors.left: parent.left
                    anchors.leftMargin: 16
                    anchors.verticalCenter: parent.verticalCenter

                    color: Theme.colorIcon
                    source: "qrc:/assets/icons_material/baseline-notifications_none-24px.svg"
                }

                Text {
                    id: text_notifications
                    height: 40
                    anchors.left: image_notifications.right
                    anchors.leftMargin: 24
                    anchors.right: switch_notifications.left
                    anchors.rightMargin: 16
                    anchors.verticalCenter: parent.verticalCenter

                    text: qsTr("Enable notifications")
                    textFormat: Text.PlainText
                    wrapMode: Text.WordWrap
                    font.pixelSize: Theme.fontSizeContent
                    color: Theme.colorText
                    verticalAlignment: Text.AlignVCenter
                }

                SwitchThemedDesktop {
                    id: switch_notifications
                    anchors.right: parent.right
                    anchors.rightMargin: screenPaddingRight
                    anchors.verticalCenter: parent.verticalCenter
                    z: 1

                    checked: settingsManager.notifications
                    onClicked: settingsManager.notifications = checked
                }
            }
            Text {
                id: legend_notifications
                anchors.left: parent.left
                anchors.leftMargin: screenPaddingLeft + 64
                anchors.right: parent.right
                anchors.rightMargin: 12
                topPadding: -12
                bottomPadding: 12

                visible: element_notifications.visible
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

            SectionTitle {
                anchors.left: parent.left
                text: qsTr("Bluetooth")
                source: "qrc:/assets/icons_material/baseline-bluetooth-24px.svg"

                // Android only
                visible: (Qt.platform.os === "android")
            }

            ////////

            Item {
                id: element_bluetoothControl
                height: 48
                anchors.left: parent.left
                anchors.leftMargin: screenPaddingLeft
                anchors.right: parent.right
                anchors.rightMargin: screenPaddingRight

                // Android only
                visible: (Qt.platform.os === "android")

                IconSvg {
                    id: image_bluetoothControl
                    width: 24
                    height: 24
                    anchors.left: parent.left
                    anchors.leftMargin: 16
                    anchors.verticalCenter: parent.verticalCenter

                    color: Theme.colorIcon
                    source: "qrc:/assets/icons_material/baseline-bluetooth_disabled-24px.svg"
                }

                Text {
                    id: text_bluetoothControl
                    height: 40
                    anchors.left: image_bluetoothControl.right
                    anchors.leftMargin: 24
                    anchors.right: switch_bluetoothControl.left
                    anchors.rightMargin: 16
                    anchors.verticalCenter: parent.verticalCenter

                    text: qsTr("Bluetooth control")
                    textFormat: Text.PlainText
                    wrapMode: Text.WordWrap
                    font.pixelSize: Theme.fontSizeContent
                    color: Theme.colorText
                    verticalAlignment: Text.AlignVCenter
                }

                SwitchThemedDesktop {
                    id: switch_bluetoothControl
                    anchors.right: parent.right
                    anchors.rightMargin: 0
                    anchors.verticalCenter: parent.verticalCenter
                    z: 1

                    checked: settingsManager.bluetoothControl
                    onClicked: settingsManager.bluetoothControl = checked
                }
            }
            Text {
                id: legend_bluetoothControl
                anchors.left: parent.left
                anchors.leftMargin: screenPaddingLeft + 64
                anchors.right: parent.right
                anchors.rightMargin: 12

                topPadding: -12
                bottomPadding: 12
                visible: element_bluetoothControl.visible

                text: settingsManager.bluetoothControl ?
                          qsTr("Lighthouse will only operate if your device's Bluetooth is already enabled.") :
                          qsTr("Lighthouse will enable your device's Bluetooth in order to operate.")
                textFormat: Text.PlainText
                wrapMode: Text.WordWrap
                color: Theme.colorSubText
                font.pixelSize: Theme.fontSizeContentSmall
            }

            ////////////////

            SectionTitle {
                anchors.left: parent.left
                visible: isDesktop
                text: qsTr("Network server")
                source: "qrc:/assets/icons_material/duotone-list-24px.svg"
            }

            ////////////////

            Item {
                id: element_server
                height: 48
                anchors.left: parent.left
                anchors.leftMargin: screenPaddingLeft
                anchors.right: parent.right
                anchors.rightMargin: screenPaddingRight

                visible: isDesktop

                IconSvg {
                    id: image_server
                    width: 24
                    height: 24
                    anchors.left: parent.left
                    anchors.leftMargin: screenPaddingLeft + 16
                    anchors.verticalCenter: parent.verticalCenter

                    color: Theme.colorIcon
                    source: "qrc:/assets/icons_material/duotone-devices-24px.svg"
                }

                Text {
                    id: text_server
                    height: 40
                    anchors.left: image_server.right
                    anchors.leftMargin: 24
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

                SwitchThemedDesktop {
                    id: switch_server
                    anchors.right: parent.right
                    anchors.rightMargin: 0
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
            Text {
                id: legend_server_desktop
                anchors.left: parent.left
                anchors.leftMargin: screenPaddingLeft + 64
                anchors.right: parent.right
                anchors.rightMargin: 12
                topPadding: -12
                bottomPadding: element_notifications.visible ? 0 : 12

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
                anchors.leftMargin: screenPaddingLeft + 64
                anchors.right: parent.right
                anchors.rightMargin: 12

                visible: networkServer && networkServer.running

                text: qsTr("Running on %1 @ %2").arg(networkServer.serverAddress).arg(networkServer.serverPort)
                textFormat: Text.PlainText
                wrapMode: Text.WordWrap
                color: Theme.colorSubText
                font.pixelSize: Theme.fontSizeContentSmall
            }

            ////////////////

            SectionTitle {
                anchors.left: parent.left
                visible: isMobile
                text: qsTr("Desktop remote control")
                source: "qrc:/assets/icons_material/duotone-devices-24px.svg"
            }

            ////////////////

            Item {
                id: element_remoteServer_ip
                height: 48
                anchors.left: parent.left
                anchors.leftMargin: screenPaddingLeft
                anchors.right: parent.right
                anchors.rightMargin: screenPaddingRight

                visible: isMobile

                IconSvg {
                    id: image_remoteServer_host
                    width: 24
                    height: 24
                    anchors.left: parent.left
                    anchors.leftMargin: 16
                    anchors.verticalCenter: parent.verticalCenter

                    color: Theme.colorIcon
                    source: "qrc:/assets/icons_material/baseline-storage-24px.svg"
                }

                TextFieldThemed {
                    id: textfield_remoteServer_host
                    anchors.left: image_remoteServer_host.right
                    anchors.leftMargin: 16
                    anchors.right: parent.right
                    anchors.rightMargin: 16
                    anchors.verticalCenter: parent.verticalCenter
                    height: 36

                    placeholderText: qsTr("Host")
                    text: settingsManager.netctrlHost
                    onEditingFinished: settingsManager.netctrlHost = text
                    selectByMouse: true
/*
                    IconSvg {
                        width: 20; height: 20;
                        anchors.right: parent.right
                        anchors.rightMargin: 12
                        anchors.verticalCenter: parent.verticalCenter

                        color: Theme.colorSubText
                        source: "qrc:/assets/icons_material/baseline-storage-24px.svg"
                    }
*/
                }
            }

            ////////

            Item {
                id: element_remoteServer_port
                height: 48
                anchors.left: parent.left
                anchors.leftMargin: screenPaddingLeft
                anchors.right: parent.right
                anchors.rightMargin: screenPaddingRight

                visible: isMobile

                IconSvg {
                    id: image_remoteServer_port
                    width: 24
                    height: 24
                    anchors.left: parent.left
                    anchors.leftMargin: 16
                    anchors.verticalCenter: parent.verticalCenter

                    color: Theme.colorIcon
                    source: "qrc:/assets/icons_material/baseline-pin-24px.svg"
                }

                TextFieldThemed {
                    id: textfield_remoteServer_port
                    anchors.left: image_remoteServer_port.right
                    anchors.leftMargin: 16
                    anchors.right: parent.right
                    anchors.rightMargin: 16
                    anchors.verticalCenter: parent.verticalCenter
                    height: 36

                    placeholderText: qsTr("Port")
                    text: settingsManager.netctrlPort
                    onEditingFinished: settingsManager.netctrlPort = parseInt(text, 10)
                    validator: IntValidator { bottom: 1; top: 65535; }
                    selectByMouse: true
/*
                    IconSvg {
                        width: 20; height: 20;
                        anchors.right: parent.right
                        anchors.rightMargin: 12
                        anchors.verticalCenter: parent.verticalCenter

                        color: Theme.colorSubText
                        source: "qrc:/assets/icons_material/baseline-pin-24px.svg"
                    }
*/
                }
            }

            ////////
        }
    }
}
