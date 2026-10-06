import QtQuick
import QtQuick.Controls

import ComponentLibrary
import AppUtils

DrawerThemed {
    contentItem: Item {

        ////////////////////////////////////////////////////////////////////////

        Column {
            id: headerColumn
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            z: 5

            ////////

            Rectangle { // statusbar area
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.rightMargin: -1

                height: Math.max(screenPaddingTop, screenPaddingStatusbar)
                color: Theme.colorStatusbar // to be able to read statusbar content
            }

            ////////

            Rectangle { // logo area
                anchors.left: parent.left
                anchors.right: parent.right

                height: 80
                color: Theme.colorBackground

                IconSvg {
                    id: imageHeader
                    anchors.left: parent.left
                    anchors.leftMargin: 12
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.verticalCenterOffset: 4
                    width: 48
                    height: 48

                    source: "qrc:/assets/icons/desk-lamp-logo.svg"
                    //sourceSize: Qt.size(width, height)
                    color: Theme.colorIcon
                }
                Text {
                    id: textHeader
                    anchors.left: imageHeader.right
                    anchors.leftMargin: 12
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.verticalCenterOffset: 6

                    text: "Lighthouse"
                    textFormat: Text.PlainText
                    color: Theme.colorText
                    font.bold: true
                    font.pixelSize: Theme.fontSizeTitle
                }
            }

            ////////
        }

        ////////////////////////////////////////////////////////////////////////

        Flickable {
            anchors.top: headerColumn.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            contentWidth: -1
            contentHeight: contentColumn.height

            Column {
                id: contentColumn
                anchors.left: parent.left
                anchors.right: parent.right

                topPadding: 0
                bottomPadding: 0

                ////////

                ListSeparatorPadded { }

                ////////

                DrawerItem {
                    highlighted: (appContent.state === "ScreenDeviceList")
                    text: qsTr("Sensors")
                    source: "qrc:/assets/logos/logo_drawer.svg"

                    onClicked: {
                        screenDeviceList.loadScreen()
                        appDrawer.close()
                    }
                }

                DrawerItem {
                    highlighted: (appContent.state === "ScreenSettings" || appContent.state === "ScreenSettingsAdvanced")
                    text: qsTr("Settings")
                    source: "qrc:/IconLibrary/material-icons/duotone/tune.svg"

                    onClicked: {
                        screenSettings.loadScreen()
                        appDrawer.close()
                    }
                }

                DrawerItem {
                    highlighted: (appContent.state === "ScreenAbout" || appContent.state === "ScreenAboutPermissions")
                    text: qsTr("About")
                    source: "qrc:/IconLibrary/material-icons/duotone/info.svg"

                    onClicked: {
                        screenAbout.loadScreen()
                        appDrawer.close()
                    }
                }

                ////////

                ListSeparatorPadded { }

                ////////

                DrawerItem {
                    source: "qrc:/IconLibrary/material-symbols/sort.svg"
                    text: {
                        var txt = qsTr("Order by:") + " "
                        if (SettingsManager.orderBy === "location") {
                            txt += qsTr("location")
                        } else {
                            txt += qsTr("device model")
                        }
                        return txt
                    }

                    property int sortmode: {
                        if (SettingsManager.orderBy === "model") {
                            return 1
                        } else { // if (SettingsManager.orderBy === "location") {
                            return 0
                        }
                    }

                    onClicked: {
                        sortmode++
                        if (sortmode > 1) sortmode = 0

                        if (sortmode === 0) {
                            SettingsManager.orderBy = "location"
                            deviceManager.orderby_location()
                        } else if (sortmode === 1) {
                            SettingsManager.orderBy = "model"
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
