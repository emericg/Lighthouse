import QtQuick
import QtQuick.Controls

import ThemeEngine

Item {
    id: screenAboutPermissions
    anchors.fill: parent

    property string entryPoint: "About"

    ////////////////////////////////////////////////////////////////////////////

    function loadScreen() {
        // Refresh permissions
        refreshPermissions()

        // Change screen
        appContent.state = "AboutPermissions"
    }

    function loadScreenFrom(screenname) {
        entryPoint = screenname
        loadScreen()
    }

    function refreshPermissions() {
        // Refresh permissions
        button_location_test.validperm = utilsApp.checkMobileBleLocationPermission()
        button_gps_test.validperm = utilsApp.isMobileGpsEnabled()
    }

    Timer {
        id: retryPermissions
        interval: 1000
        repeat: false
        onTriggered: refreshPermissions()
    }

    ////////////////////////////////////////////////////////////////////////////

    Flickable {
        anchors.fill: parent

        contentWidth: -1
        contentHeight: contentColumn.height

        Column {
            id: contentColumn
            anchors.left: parent.left
            anchors.leftMargin: screenPaddingLeft
            anchors.right: parent.right
            anchors.rightMargin: screenPaddingRight

            topPadding: 20
            bottomPadding: 20
            spacing: 8

            ////////

            Item {
                id: element_bluetooth
                height: 24
                anchors.left: parent.left
                anchors.right: parent.right

                RoundButtonIcon {
                    id: button_bluetooth_test
                    width: 32
                    height: 32
                    anchors.left: parent.left
                    anchors.leftMargin: 16
                    anchors.verticalCenter: parent.verticalCenter

                    property bool validperm: true

                    source: (validperm) ? "qrc:/assets/icons_material/baseline-check-24px.svg" : "qrc:/assets/icons_material/baseline-close-24px.svg"
                    iconColor: (validperm) ? "white" : "white"
                    backgroundColor: (validperm) ? Theme.colorSuccess : Theme.colorSubText
                    backgroundVisible: true
                }

                Text {
                    id: text_bluetooth
                    height: 16
                    anchors.left: parent.left
                    anchors.leftMargin: 64
                    anchors.right: parent.right
                    anchors.rightMargin: 16
                    anchors.verticalCenter: parent.verticalCenter

                    text: qsTr("Bluetooth control")
                    textFormat: Text.PlainText
                    wrapMode: Text.WordWrap
                    font.pixelSize: 17
                    color: Theme.colorText
                    verticalAlignment: Text.AlignVCenter
                }
            }
            Text {
                id: legend_bluetooth
                anchors.left: parent.left
                anchors.leftMargin: 64
                anchors.right: parent.right
                anchors.rightMargin: 12

                text: qsTr("Lighthouse can activate your device's Bluetooth in order to operate.")
                textFormat: Text.PlainText
                wrapMode: Text.WordWrap
                color: Theme.colorSubText
                font.pixelSize: Theme.fontSizeContentSmall
            }

            ////////

            ListSeparatorPadded { height: 16+1 }

            ////////

            Item {
                id: element_location
                height: 24
                anchors.left: parent.left
                anchors.right: parent.right

                RoundButtonIcon {
                    id: button_location_test
                    width: 32
                    height: 32
                    anchors.left: parent.left
                    anchors.leftMargin: 16
                    anchors.verticalCenter: parent.verticalCenter

                    property bool validperm: false

                    source: (validperm) ? "qrc:/assets/icons_material/baseline-check-24px.svg" : "qrc:/assets/icons_material/baseline-close-24px.svg"
                    iconColor: (validperm) ? "white" : "white"
                    backgroundColor: (validperm) ? Theme.colorSuccess : Theme.colorSubText
                    backgroundVisible: true

                    onClicked: {
                        utilsApp.vibrate(25)
                        validperm = utilsApp.getMobileBleLocationPermission()
                        retryPermissions.start()
                    }
                }

                Text {
                    id: text_location
                    height: 16
                    anchors.left: parent.left
                    anchors.leftMargin: 64
                    anchors.right: parent.right
                    anchors.rightMargin: 16
                    anchors.verticalCenter: parent.verticalCenter

                    text: qsTr("Location")
                    textFormat: Text.PlainText
                    wrapMode: Text.WordWrap
                    font.pixelSize: 17
                    color: Theme.colorText
                    verticalAlignment: Text.AlignVCenter
                }
            }
            Text {
                id: legend_location
                anchors.left: parent.left
                anchors.leftMargin: 64
                anchors.right: parent.right
                anchors.rightMargin: 12

                text: qsTr("The Android operating system requires applications to ask for device location permission in order to scan for nearby Bluetooth Low Energy sensors.") + "<br>" +
                      qsTr("Lighthouse doesn't use, store nor communicate your location to anyone or anything.")
                textFormat: Text.StyledText
                wrapMode: Text.WordWrap
                color: Theme.colorSubText
                font.pixelSize: Theme.fontSizeContentSmall
            }
            ButtonWireframeIcon {
                height: 36
                anchors.left: parent.left
                anchors.leftMargin: 64

                primaryColor: Theme.colorPrimary
                secondaryColor: Theme.colorBackground

                text: qsTr("Official information")
                source: "qrc:/assets/icons_material/duotone-launch-24px.svg"
                sourceSize: 20

                onClicked: Qt.openUrlExternally("https://developer.android.com/guide/topics/connectivity/bluetooth/permissions#declare-android11-or-lower")
            }

            ////////

            ListSeparatorPadded { height: 16+1 }

            ////////

            Item {
                id: element_gps
                height: 24
                anchors.left: parent.left
                anchors.right: parent.right

                RoundButtonIcon {
                    id: button_gps_test
                    width: 32
                    height: 32
                    anchors.left: parent.left
                    anchors.leftMargin: 16
                    anchors.verticalCenter: parent.verticalCenter

                    property bool validperm: false

                    source: (validperm) ? "qrc:/assets/icons_material/baseline-check-24px.svg" : "qrc:/assets/icons_material/baseline-close-24px.svg"
                    iconColor: (validperm) ? "white" : "white"
                    backgroundColor: (validperm) ? Theme.colorSuccess : Theme.colorSubText
                    backgroundVisible: true

                    onClicked: {
                        utilsApp.vibrate(25)
                        validperm = utilsApp.isMobileGpsEnabled()
                        retryPermissions.start()
                    }
                }

                Text {
                    id: text_gps
                    height: 16
                    anchors.left: parent.left
                    anchors.leftMargin: 64
                    anchors.right: parent.right
                    anchors.rightMargin: 16
                    anchors.verticalCenter: parent.verticalCenter

                    text: qsTr("GPS")
                    textFormat: Text.PlainText
                    wrapMode: Text.WordWrap
                    font.pixelSize: 17
                    color: Theme.colorText
                    verticalAlignment: Text.AlignVCenter
                }
            }
            Text {
                id: legend_gps
                anchors.left: parent.left
                anchors.leftMargin: 64
                anchors.right: parent.right
                anchors.rightMargin: 12

                text: qsTr("Some Android devices also require the GPS to be turned on for Bluetooth operations.")
                textFormat: Text.PlainText
                wrapMode: Text.WordWrap
                color: Theme.colorSubText
                font.pixelSize: Theme.fontSizeContentSmall
            }

            ////////

            ListSeparatorPadded { height: 16+1 }

            ////////

            Item {
                id: element_network
                height: 24
                anchors.left: parent.left
                anchors.right: parent.right

                RoundButtonIcon {
                    id: button_network_test
                    width: 32
                    height: 32
                    anchors.left: parent.left
                    anchors.leftMargin: 16
                    anchors.verticalCenter: parent.verticalCenter
                    z: 1

                    property bool validperm: true

                    source: (validperm) ? "qrc:/assets/icons_material/baseline-check-24px.svg" : "qrc:/assets/icons_material/baseline-close-24px.svg"
                    iconColor: (validperm) ? "white" : "white"
                    backgroundColor: (validperm) ? Theme.colorSuccess : Theme.colorSubText
                    backgroundVisible: true
                }

                Text {
                    id: text_network
                    height: 16
                    anchors.left: parent.left
                    anchors.leftMargin: 64
                    anchors.right: parent.right
                    anchors.rightMargin: 16
                    anchors.verticalCenter: parent.verticalCenter

                    text: qsTr("Network access")
                    textFormat: Text.PlainText
                    wrapMode: Text.WordWrap
                    font.pixelSize: 18
                    color: Theme.colorText
                    verticalAlignment: Text.AlignVCenter
                }
            }
            Text {
                id: legend_network
                anchors.left: parent.left
                anchors.leftMargin: 64
                anchors.right: parent.right
                anchors.rightMargin: 16

                text: qsTr("Network state and internet permissions are used to connect to remote desktop control.")
                textFormat: Text.PlainText
                wrapMode: Text.WordWrap
                color: Theme.colorSubText
                font.pixelSize: Theme.fontSizeContentSmall
            }

            ////////

            ListSeparatorPadded { height: 16+1 }

            ////////

            Item { // element_infos
                anchors.left: parent.left
                anchors.right: parent.right
                height: 32

                IconSvg {
                    anchors.left: parent.left
                    anchors.leftMargin: Theme.componentMargin + 4
                    anchors.verticalCenter: parent.verticalCenter
                    width: 32
                    height: 32

                    opacity: 0.66
                    color: Theme.colorSubText
                    source: "qrc:/assets/icons_material/duotone-info-24px.svg"
                }

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: appHeader.headerPosition
                    anchors.right: parent.right
                    anchors.rightMargin: Theme.componentMargin
                    anchors.verticalCenter: parent.verticalCenter

                    text: qsTr("Click on the checkmarks to request missing permissions.")
                    textFormat: Text.StyledText
                    lineHeight : 0.8
                    wrapMode: Text.WordWrap
                    color: Theme.colorText
                    font.pixelSize: Theme.fontSizeContent
                }
            }

            Text {
                anchors.left: parent.left
                anchors.leftMargin: appHeader.headerPosition
                anchors.right: parent.right
                anchors.rightMargin: Theme.componentMargin

                visible: (Qt.platform.os === "android")

                text: qsTr("If it has no effect, you may have previously refused a permission and clicked on \"don't ask again\".") + "<br>" +
                      qsTr("You can go to the Android \"application info\" panel to change a permission manually.")
                textFormat: Text.StyledText
                wrapMode: Text.WordWrap
                color: Theme.colorSubText
                font.pixelSize: Theme.fontSizeContentSmall
            }

            ButtonWireframeIcon {
                anchors.left: parent.left
                anchors.leftMargin: appHeader.headerPosition
                height: 36

                visible: (Qt.platform.os === "android")
                primaryColor: Theme.colorPrimary
                secondaryColor: Theme.colorBackground

                text: qsTr("Application info")
                source: "qrc:/assets/icons_material/duotone-tune-24px.svg"
                sourceSize: 20

                onClicked: utilsApp.openAndroidAppInfo("io.emeric.lighthouse")
            }

            ////////
        }
    }

    ////////////////////////////////////////////////////////////////////////////
}
