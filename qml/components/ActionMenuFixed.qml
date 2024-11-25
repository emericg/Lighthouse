import QtQuick
import QtQuick.Controls

import ComponentLibrary

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
/*
        Rectangle {
            width: parent.width; height: 1;
            color: Theme.colorSeparator
            visible: (actionGraphMode.visible)
        }
*/
        ActionMenuItem {
            id: actionConnect

            index: 1
            text: qsTr("Connect")
            source: "qrc:/IconLibrary/material-icons/outlined/bluetooth_disabled.svg"
            layoutDirection: actionMenu.layoutDirection
            visible: (deviceManager.bluetooth && selectedDevice && !selectedDevice.busy)

            onClicked: {
                deviceConnectButtonClicked()
                menuSelected(index)
                close()
            }
        }
        ActionMenuItem {
            id: actionDisconnect

            index: 2
            text: qsTr("Disconnect")
            source: "qrc:/IconLibrary/material-icons/outlined/bluetooth_disabled.svg"
            layoutDirection: actionMenu.layoutDirection
            visible: (deviceManager.bluetooth && selectedDevice && selectedDevice.busy)

            onClicked: {
                deviceDisconnectButtonClicked()
                menuSelected(index)
                close()
            }
        }
    }
}
