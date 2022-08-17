import QtQuick 2.15
import QtQuick.Controls 2.15

import ThemeEngine 1.0

Popup {
    id: actionMenu
    width: 200

    padding: 0
    margins: 0

    parent: Overlay.overlay
    modal: true
    dim: false
    focus: isMobile
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    enter: Transition { NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: 133; } }
    exit: Transition { NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: 133; } }

    property int layoutDirection: Qt.RightToLeft

    signal menuSelected(var index)

    ////////////////////////////////////////////////////////////////////////////

    background: Rectangle {
        color: Theme.colorBackground
        radius: Theme.componentRadius
        border.color: Theme.colorSeparator
        border.width: Theme.componentBorderWidth
    }

    ////////////////////////////////////////////////////////////////////////////

    Column {
        anchors.left: parent.left
        anchors.right: parent.right

        topPadding: 8
        bottomPadding: 8
        spacing: 4

        ////////

        ActionMenuItem {
            id: actionUpdate

            index: 0
            text: qsTr("Update data")
            source: "qrc:/assets/icons_material/baseline-refresh-24px.svg"
            layoutDirection: actionMenu.layoutDirection
            visible: (deviceManager.bluetooth && (selectedDevice && selectedDevice.hasBluetoothConnection))

            onClicked: {
                deviceRefreshButtonClicked()
                menuSelected(index)
                close()
            }
        }

        ActionMenuItem {
            id: actionRealtime

            index: 1
            text: qsTr("Real time data")
            source: "qrc:/assets/icons_material/duotone-update-24px.svg"
            layoutDirection: actionMenu.layoutDirection
            visible: (deviceManager.bluetooth && (selectedDevice && selectedDevice.hasRealTime))

            onClicked: {
                deviceRefreshRealtimeButtonClicked()
                menuSelected(index)
                close()
            }
        }

        ////////

        Rectangle {
            width: parent.width; height: 1;
            color: Theme.colorSeparator
            visible: (actionLed.visible || actionGraphMode.visible)
        }

        ActionMenuItem {
            id: actionLed

            index: 8
            text: qsTr("Blink LED")
            source: "qrc:/assets/icons_material/lightbulb_FILL0_wght400_GRAD0_opsz48.svg"
            layoutDirection: actionMenu.layoutDirection
            visible: (deviceManager.bluetooth && (selectedDevice && selectedDevice.hasLED))

            onClicked: {
                deviceLedButtonClicked()
                menuSelected(index)
                close()
            }
        }

        ActionMenuItem {
            id: actionGraphMode

            index: 16
            text: qsTr("Switch graph")
            source: (settingsManager.graphThermometer === "minmax") ? "qrc:/assets/icons_material/duotone-insert_chart-24px.svg" : "qrc:/assets/icons_material/baseline-timeline-24px.svg"
            layoutDirection: actionMenu.layoutDirection
            visible: (appContent.state === "DeviceThermometer")

            onClicked: {
                if (settingsManager.graphThermometer === "minmax") settingsManager.graphThermometer = "lines"
                else settingsManager.graphThermometer = "minmax"
                menuSelected(index)
                close()
            }
        }

        ActionMenuItem {
            id: actionShowSettings

            index: 17
            text: qsTr("Sensor infos")
            source: "qrc:/assets/icons_material/duotone-memory-24px.svg"
            layoutDirection: actionMenu.layoutDirection
            visible: (appContent.state === "DeviceThermometer" || appContent.state === "DeviceEnvironmental")

            onClicked: {
                deviceSettingsButtonClicked()
                menuSelected(index)
                close()
            }
        }

        ActionMenuItem {
            id: actionReboot

            index: 32
            text: qsTr("Reboot sensor")
            source: "qrc:/assets/icons_material/baseline-refresh-24px.svg"
            layoutDirection: actionMenu.layoutDirection
            visible: (deviceManager.bluetooth && (selectedDevice && selectedDevice.hasReboot))

            onClicked: {
                deviceRebootButtonClicked()
                menuSelected(index)
                close()
            }
        }
    }
}
