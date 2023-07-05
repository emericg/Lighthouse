import QtQuick
import QtQuick.Controls

import ThemeEngine 1.0

Drawer {
    width: (appWindow.screenOrientation === Qt.PortraitOrientation || appWindow.width < 480)
           ? 0.8 * appWindow.width : 0.5 * appWindow.width
    height: appWindow.height

    ////////////////////////////////////////////////////////////////////////////

    background: Rectangle {
        color: Theme.colorBackground

        Rectangle { // left border
            x: parent.width - 1
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
            anchors.rightMargin: 1
            z: 5

            ////////

            Rectangle {
                id: rectangleStatusbar
                anchors.left: parent.left
                anchors.right: parent.right

                height: Math.max(screenPaddingTop, screenPaddingStatusbar)
                color: Theme.colorBackground // "red" // to hide flickable content
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
                anchors.rightMargin: 1

                ////////

                Rectangle {
                    id: rectangleHome
                    height: 48
                    anchors.left: parent.left
                    anchors.right: parent.right
                    color: (appContent.state === "DeviceList") ? Theme.colorForeground : Theme.colorBackground

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            appContent.state = "DeviceList"
                            appDrawer.close()
                        }
                    }

                    IconSvg {
                        width: 24
                        height: 24
                        anchors.left: parent.left
                        anchors.leftMargin: screenPaddingLeft + 16
                        anchors.verticalCenter: parent.verticalCenter

                        source: "qrc:/assets/logos/logo_tray_symbolic.svg"
                        color: Theme.colorText
                    }
                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: screenPaddingLeft + 56
                        anchors.verticalCenter: parent.verticalCenter

                        text: qsTr("Sensors")
                        font.pixelSize: 13
                        font.bold: true
                        color: Theme.colorText
                    }
                }

                ////////

                Rectangle {
                    id: rectangleSettings
                    height: 48
                    anchors.left: parent.left
                    anchors.right: parent.right
                    color: (appContent.state === "Settings") ? Theme.colorForeground : Theme.colorBackground

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            screenSettings.loadScreen()
                            appDrawer.close()
                        }
                    }

                    IconSvg {
                        width: 24
                        height: 24
                        anchors.left: parent.left
                        anchors.leftMargin: screenPaddingLeft + 16
                        anchors.verticalCenter: parent.verticalCenter

                        source: "qrc:/assets/icons_material/outline-settings-24px.svg"
                        color: Theme.colorText
                    }
                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: screenPaddingLeft + 56
                        anchors.verticalCenter: parent.verticalCenter

                        text: qsTr("Settings")
                        font.pixelSize: 13
                        font.bold: true
                        color: Theme.colorText
                    }
                }

                Rectangle {
                    id: rectangleAbout
                    height: 48
                    anchors.right: parent.right
                    anchors.left: parent.left
                    color: (appContent.state === "About" || appContent.state === "Permissions")
                           ? Theme.colorForeground : Theme.colorBackground

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            screenAbout.loadScreen()
                            appDrawer.close()
                        }
                    }

                    IconSvg {
                        width: 24
                        height: 24
                        anchors.left: parent.left
                        anchors.leftMargin: screenPaddingLeft + 16
                        anchors.verticalCenter: parent.verticalCenter

                        source: "qrc:/assets/icons_material/outline-info-24px.svg"
                        color: Theme.colorText
                    }
                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: screenPaddingLeft + 56
                        anchors.verticalCenter: parent.verticalCenter

                        text: qsTr("About")
                        font.pixelSize: 13
                        font.bold: true
                        color: Theme.colorText
                    }
                }

                ////////

                Item { // spacer
                    height: 8
                    anchors.right: parent.right
                    anchors.left: parent.left
                }
                Rectangle {
                    height: 1
                    anchors.right: parent.right
                    anchors.left: parent.left
                    color: Theme.colorSeparator
                }
                Item {
                    height: 8
                    anchors.right: parent.right
                    anchors.left: parent.left
                }

                ////////

                Item {
                    id: rectangleOrderBy
                    height: 48
                    anchors.right: parent.right
                    anchors.left: parent.left

                    MouseArea {
                        anchors.fill: parent

                        property var sortmode: {
                            if (settingsManager.orderBy === "model") {
                                return 1
                            } else { // if (settingsManager.orderBy === "location") {
                                return 0
                            }
                        }

                        onClicked: {
                            sortmode++
                            if (sortmode > 1) sortmode = 0

                            if (sortmode === 0) {
                                settingsManager.orderBy = "location"
                                deviceManager.orderby_location()
                            } else if (sortmode === 1) {
                                settingsManager.orderBy = "model"
                                deviceManager.orderby_model()
                            }
                        }
                    }

                    IconSvg {
                        width: 24
                        height: 24
                        anchors.left: parent.left
                        anchors.leftMargin: screenPaddingLeft + 16
                        anchors.verticalCenter: parent.verticalCenter

                        source: "qrc:/assets/icons_material/baseline-sort-24px.svg"
                        color: Theme.colorText
                    }
                    Text {
                        id: textOrderBy
                        anchors.left: parent.left
                        anchors.leftMargin: screenPaddingLeft + 56
                        anchors.verticalCenter: parent.verticalCenter

                        function setText() {
                            var txt = qsTr("Order by:") + " "
                            if (settingsManager.orderBy === "model") {
                                txt += qsTr("sensor model")
                            } else if (settingsManager.orderBy === "location") {
                                txt += qsTr("location")
                            }
                            textOrderBy.text = txt
                        }

                        Component.onCompleted: textOrderBy.setText()
                        Connections {
                            target: settingsManager
                            function onOrderByChanged() { textOrderBy.setText() }
                            function onAppLanguageChanged() { textOrderBy.setText() }
                        }

                        font.pixelSize: 13
                        font.bold: true
                        color: Theme.colorText
                    }
                }

                ////////

                Item { // spacer
                    height: 8
                    anchors.right: parent.right
                    anchors.left: parent.left
                }
                Rectangle {
                    height: 1
                    anchors.right: parent.right
                    anchors.left: parent.left
                    color: Theme.colorSeparator
                }
                Item {
                    height: 8
                    anchors.right: parent.right
                    anchors.left: parent.left
                }

                ////////
            }
        }

        ////////////////////////////////////////////////////////////////////////
    }
}
