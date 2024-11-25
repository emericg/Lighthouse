import QtQuick
import QtQuick.Controls.impl
import QtQuick.Templates as T

import ComponentLibrary

T.Popup {
    id: actionMenu

    width: appWindow.width
    height: actualHeight

    y: appWindow.height - actualHeight

    padding: 0
    margins: 0

    modal: true
    dim: true
    focus: appWindow.isMobile
    closePolicy: T.Popup.CloseOnEscape | T.Popup.CloseOnPressOutside
    parent: T.Overlay.overlay

    signal menuSelected(var index)

    ////////////////////////////////////////////////////////////////////////////

    property int layoutDirection: Qt.RightToLeft

    property int actualHeight: {
        if (typeof mobileMenu !== "undefined" && mobileMenu.height)
            return contentColumn.height + appWindow.screenPaddingNavbar + appWindow.screenPaddingBottom
        return contentColumn.height
    }

    property bool opening: false
    property bool closing: false

    onAboutToShow: {
        opening = true
        closing = false
    }
    onAboutToHide: {
        opening = false
        closing = true
    }

    ////////////////////////////////////////////////////////////////////////////

    enter: Transition { NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: 233; } }
    exit: Transition { NumberAnimation { property: "opacity"; from: 1.0; to: 0.66; duration: 233; } }

    T.Overlay.modal: Rectangle {
        color: "#000"
        opacity: Theme.isLight ? 0.24 : 0.48
    }

    background: Item { }

    contentItem: Item { }

    ////////////////////////////////////////////////////////////////////////////

    Rectangle {
        id: actualPopup
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        height: opening ? actionMenu.actualHeight : 0
        Behavior on height { NumberAnimation { duration: 233 } }

        color: Theme.colorComponentBackground

        Rectangle { // separator
            anchors.left: parent.left
            anchors.right: parent.right
            height: Theme.componentBorderWidth
            color: Theme.colorSeparator
        }

        Column { // content
            id: contentColumn
            anchors.top: parent.top
            anchors.topMargin: 0
            anchors.left: parent.left
            anchors.leftMargin: screenPaddingLeft
            anchors.right: parent.right
            anchors.rightMargin: screenPaddingRight

            topPadding: Theme.componentMargin
            bottomPadding: 8
            spacing: 4

            ////////

            ActionMenuItem {
                id: actionConnect
                anchors.left: parent.left
                anchors.right: parent.right
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
                anchors.left: parent.left
                anchors.right: parent.right
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

            ////////
        }
    }

    ////////////////////////////////////////////////////////////////////////////
}
