import QtQuick
import QtQuick.Controls

import ThemeEngine

Drawer {
    width: (appWindow.screenOrientation === Qt.PortraitOrientation || appWindow.width < 480)
            ? 0.8 * appWindow.width : 0.5 * appWindow.width
    height: appWindow.height

    ////////////////////////////////////////////////////////////////////////////

    background: Rectangle {
        color: Theme.colorBackground

        Rectangle { // left border
            x: parent.width
            width: 1
            height: parent.height
            color: Theme.colorSeparator
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    contentItem: Item {

        Column {
            id: rectangleHeader
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            z: 5

            ////////

            Rectangle {
                id: rectangleStatusbar
                anchors.left: parent.left
                anchors.right: parent.right

                height: Math.max(screenPaddingTop, screenPaddingStatusbar)
                color: Theme.colorBackground // to hide flickable content
            }

            ////////

            Rectangle {
                id: rectangleLogo
                anchors.left: parent.left
                anchors.right: parent.right

                height: 80
                color: Theme.colorBackground

                IconSvg {
                    id: imageHeader
                    width: 40
                    height: 40
                    anchors.left: parent.left
                    anchors.leftMargin: 12
                    anchors.verticalCenter: parent.verticalCenter

                    source: "qrc:/assets/icons_material/desk-lamp.svg"
                    //sourceSize: Qt.size(width, height)
                    color: Theme.colorIcon
                }
                Text {
                    id: textHeader
                    anchors.left: imageHeader.right
                    anchors.leftMargin: 10
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.verticalCenterOffset: 2

                    text: "Lighthouse"
                    color: Theme.colorText
                    font.bold: true
                    font.pixelSize: 22
                }
            }
        }

        ////////////////////////////////////////////////////////////////////////////

        Flickable {
            anchors.top: rectangleHeader.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            contentWidth: -1
            contentHeight: column.height

            Column {
                id: column
                anchors.left: parent.left
                anchors.right: parent.right

                ////////

                DrawerItem {
                    highlighted: (appContent.state === "DeviceList")
                    text: qsTr("Sensors")
                    iconSource: "qrc:/assets/logos/logo_tray_symbolic.svg"

                    onClicked: {
                        //screenDeviceList.loadScreen()
                        appContent.state = "DeviceList"
                        appDrawer.close()
                    }
                }

                DrawerItem {
                    highlighted: (appContent.state === "Settings")
                    text: qsTr("Settings")
                    iconSource: "qrc:/assets/icons_material/outline-settings-24px.svg"

                    onClicked: {
                        screenSettings.loadScreen()
                        appDrawer.close()
                    }
                }

                DrawerItem {
                    highlighted: (appContent.state === "About" || appContent.state === "AboutPermissions")
                    text: qsTr("About")
                    iconSource: "qrc:/assets/icons_material/outline-info-24px.svg"

                    onClicked: {
                        screenAbout.loadScreen()
                        appDrawer.close()
                    }
                }

                ////////

                ListSeparatorPadded { }

                ////////

                DrawerItem {
                    iconSource: "qrc:/assets/icons_material/baseline-sort-24px.svg"
                    text: {
                        var txt = qsTr("Order by:") + " "
                        if (settingsManager.orderBy === "model") {
                            txt += qsTr("sensor model")
                        } else if (settingsManager.orderBy === "location") {
                            txt += qsTr("location")
                        }
                        return txt
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

                ////////

                ListSeparatorPadded { }

                ////////
            }
        }

        ////////////////////////////////////////////////////////////////////////
    }
}
