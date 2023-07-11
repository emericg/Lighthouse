import QtQuick

import ThemeEngine

Rectangle {
    id: appHeader
    anchors.top: parent.top
    anchors.left: parent.left
    anchors.right: parent.right

    height: screenPaddingStatusbar + headerHeight
    color: Theme.colorHeader
    clip: false
    z: 10

    property int headerHeight: 52

    property int headerPosition: 64

    property string headerTitle: "Lighthouse"

    ////////////////////////////////////////////////////////////////////////////

    property string leftMenuMode: "drawer" // drawer / back / close
    signal leftMenuClicked()

    property string rightMenuMode: "off" // on / off
    signal rightMenuClicked()

    ////////////////////////////////////////////////////////////////////////////

    function rightMenuIsOpen() { return actionMenu.visible; }
    function rightMenuClose() { actionMenu.close(); }

    signal deviceConnectButtonClicked()
    signal deviceDisconnectButtonClicked()

    signal deviceRebootButtonClicked()
    signal deviceCalibrateButtonClicked()
    signal deviceWateringButtonClicked()
    signal deviceLedButtonClicked()
    signal deviceRefreshButtonClicked()
    signal deviceRefreshRealtimeButtonClicked()
    signal deviceRefreshHistoryButtonClicked()
    signal deviceClearButtonClicked()
    signal deviceDataButtonClicked() // compatibility
    signal deviceHistoryButtonClicked() // compatibility
    signal devicePlantButtonClicked() // compatibility
    signal deviceSettingsButtonClicked() // compatibility

    function setActiveDeviceData() { } // compatibility
    function setActiveDeviceHistory() { } // compatibility
    function setActiveDevicePlant() { } // compatibility
    function setActiveDeviceSettings() { } // compatibility

    ////////////////////////////////////////////////////////////////////////////

    // prevent clicks below this area
    MouseArea { anchors.fill: parent; acceptedButtons: Qt.AllButtons; }

    ActionMenuFixed {
        id: actionMenu

        x: parent.width - actionMenu.width - 12
        y: screenPaddingStatusbar + 16

        onMenuSelected: (index) => {
            //console.log("ActionMenu clicked #" + index)
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    Rectangle { // OS statusbar area
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        height: screenPaddingStatusbar
        color: Theme.colorStatusbar
    }

    Item {
        anchors.fill: parent
        anchors.topMargin: screenPaddingStatusbar

        Row { // left area
            id: leftArea
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: 4
            anchors.bottom: parent.bottom

            MouseArea { // left button
                width: headerHeight
                height: headerHeight

                visible: true
                onClicked: leftMenuClicked()

                RippleThemed {
                    anchor: parent
                    width: headerHeight
                    height: headerHeight

                    pressed: parent.pressed
                    //active: enabled && parent.containsPress
                    color: Qt.rgba(Theme.colorForeground.r, Theme.colorForeground.g, Theme.colorForeground.b, 0.33)
                }

                IconSvg {
                    anchors.centerIn: parent
                    width: (headerHeight / 2)
                    height: (headerHeight / 2)

                    source: {
                        if (leftMenuMode === "drawer") return "qrc:/assets/icons_material/baseline-menu-24px.svg"
                        else if (leftMenuMode === "close") return "qrc:/assets/icons_material/baseline-close-24px.svg"
                        return "qrc:/assets/icons_material/baseline-arrow_back-24px.svg"
                    }
                    color: Theme.colorHeaderContent
                }
            }
        }

        ////////////

        Text { // header title
            height: parent.height
            anchors.left: parent.left
            anchors.leftMargin: headerPosition
            anchors.right: rightArea.left
            anchors.rightMargin: 8
            anchors.verticalCenter: parent.verticalCenter

            text: headerTitle
            color: Theme.colorHeaderContent
            font.bold: true
            font.pixelSize: Theme.fontSizeHeader
            font.capitalization: Font.Capitalize
            verticalAlignment: Text.AlignVCenter
        }

        ////////////

        Row { // right area
            id: rightArea
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.rightMargin: 4
            anchors.bottom: parent.bottom

            spacing: 4
            visible: true

            Item { // right indicators
                width: parent.height
                height: width
                anchors.verticalCenter: parent.verticalCenter
                visible: (appContent.state === "DeviceList")

                IconSvg {
                    id: workingIndicator
                    width: 24; height: 24;
                    anchors.centerIn: parent

                    source: "qrc:/assets/icons_material/baseline-autorenew-24px.svg"
                    color: Theme.colorHeaderContent
                    opacity: 0
                    Behavior on opacity { OpacityAnimator { duration: 333 } }

                    SequentialAnimation on opacity { // (fade)
                        loops: Animation.Infinite
                        running: (deviceManager.scanning || deviceManager.listening || deviceManager.syncing)
                        onStopped: workingIndicator.opacity = 0
                        PropertyAnimation { to: 1; duration: 750; }
                        PropertyAnimation { to: 0.33; duration: 750; }
                    }
                }
            }

            MouseArea { // right button
                width: headerHeight
                height: headerHeight

                visible: (deviceManager.bluetooth &&
                          (appContent.state === "DeviceLight"))

                onClicked: {
                    rightMenuClicked()
                    actionMenu.open()
                }

                RippleThemed {
                    width: headerHeight
                    height: headerHeight

                    pressed: parent.pressed
                    //active: enabled && parent.containsPress
                    color: Qt.rgba(Theme.colorForeground.r, Theme.colorForeground.g, Theme.colorForeground.b, 0.33)
                }

                IconSvg {
                    width: (headerHeight / 2)
                    height: (headerHeight / 2)
                    anchors.centerIn: parent

                    source: "qrc:/assets/icons_material/baseline-more_vert-24px.svg"
                    color: Theme.colorHeaderContent
                }
            }
        }

        ////////////
    }

    ////////////////////////////////////////////////////////////////////////////
}
