import QtQuick
import QtQuick.Effects
import QtQuick.Controls
import Qt.labs.platform

import ComponentLibrary

Popup {
    id: popupPgpKey

    x: singleColumn ? 0 : (appWindow.width / 2) - (width / 2)
    y: singleColumn ? (appWindow.height - appHeader.height - height)
                    : ((appWindow.height / 2) - (height / 2))

    width: singleColumn ? parent.width : 640
    height: columnContent.height + padding*2 + screenPaddingNavbar + screenPaddingBottom
    padding: Theme.componentMarginXL
    margins: 0

    dim: true
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    parent: Overlay.overlay

    signal confirmed()

    property var currentDevice: null

    property url selectedFile
    property string errorString

    onAboutToShow: {
        selectedFile = ""
        errorString = ""
    }

    ////////////////////////////////////////////////////////////////////////////

    enter: Transition { NumberAnimation { property: "opacity"; from: 0.5; to: 1.0; duration: 133; } }
    //exit: Transition { NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: 200; } }

    Overlay.modal: Rectangle {
        color: "#000"
        opacity: Theme.isLight ? 0.24 : 0.48
    }

    background: Rectangle {
        color: Theme.colorBackground
        border.color: Theme.colorSeparator
        border.width: singleColumn ? 0 : Theme.componentBorderWidth
        radius: singleColumn ? 0 : Theme.componentRadius

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            height: Theme.componentBorderWidth
            visible: singleColumn
            color: Theme.colorSeparator
        }

        layer.enabled: !singleColumn
        layer.effect: MultiEffect { // shadow
            autoPaddingEnabled: true
            blurMax: 48
            shadowEnabled: true
            shadowColor: Theme.isLight ? "#aa000000" : "#cc000000"
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    FileDialog {
        id: fileDialogPgpKey
        title: qsTr("Please choose a key dump")

        fileMode: FileDialog.OpenFile
        nameFilters: [qsTr("Key dump") + " (*.json)", qsTr("All files") + " (*)"]

        onAccepted: {
            popupPgpKey.selectedFile = file
            popupPgpKey.errorString = ""
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    contentItem: Item {
        Column {
            id: columnContent
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: Theme.componentMarginXL

            ////////

            Text {
                anchors.left: parent.left
                anchors.right: parent.right

                text: qsTr("Set the device key of this Pokemon GO Plus")
                textFormat: Text.PlainText
                font.pixelSize: Theme.fontSizeContentVeryBig
                color: Theme.colorText
                wrapMode: Text.WordWrap
            }

            ////////

            Column {
                anchors.left: parent.left
                anchors.right: parent.right
                spacing: 8

                Text {
                    anchors.left: parent.left
                    anchors.right: parent.right

                    text: qsTr("A device key MUST be set before you can use this device.") + "<br>" +
                          qsTr("Every Pokemon GO Plus has its own key, burned into its memory at the factory: it cannot be guessed or computed, it has to be extracted from the device itself.")
                    textFormat: Text.StyledText
                    font.pixelSize: Theme.fontSizeContent
                    color: Theme.colorSubText
                    wrapMode: Text.WordWrap
                }

                Text {
                    anchors.left: parent.left
                    anchors.right: parent.right

                    text: qsTr("You can extract it using the <a href=\"https://github.com/Jesus805/Suota-Go-Plus\">SUOTA Go+ tool</a>, which will produce a small '.json' file. Select that file below, it will be copied next to the application settings.")
                    textFormat: Text.StyledText
                    font.pixelSize: Theme.fontSizeContent
                    color: Theme.colorSubText
                    wrapMode: Text.WordWrap
                    linkColor: Theme.colorText
                    onLinkActivated: (link) => { Qt.openUrlExternally(link) }
                }

                Text {
                    anchors.left: parent.left
                    anchors.right: parent.right

                    text: qsTr("Sorry for the inconvenience...")
                    textFormat: Text.StyledText
                    font.pixelSize: Theme.fontSizeContent
                    color: Theme.colorSubText
                    wrapMode: Text.WordWrap
                }
            }

            ////////

            Column {
                anchors.left: parent.left
                anchors.right: parent.right
                spacing: 8

                ButtonSolid {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 40

                    color: (popupPgpKey.selectedFile.toString() !== "" || (popupPgpKey.currentDevice && popupPgpKey.currentDevice.hasDeviceKey)) ?
                               Theme.colorSuccess : Theme.colorSubText

                    source: "qrc:/IconLibrary/material-symbols/folder_open.svg"
                    text: {
                        if (popupPgpKey.selectedFile.toString() !== "") {
                            return decodeURIComponent(popupPgpKey.selectedFile.toString().split('/').pop())
                        }
                        if (popupPgpKey.currentDevice && popupPgpKey.currentDevice.hasDeviceKey) {
                            return qsTr("Replace the current key dump")
                        }
                        return qsTr("Select a key dump")
                    }
                    onClicked: fileDialogPgpKey.open()
                }

                Text {
                    anchors.left: parent.left
                    anchors.right: parent.right

                    visible: (popupPgpKey.errorString !== "")
                    text: popupPgpKey.errorString
                    textFormat: Text.PlainText
                    font.pixelSize: Theme.fontSizeContent
                    color: Theme.colorWarning
                    wrapMode: Text.WordWrap
                }
            }

            ////////

            Flow {
                id: flowContent
                anchors.left: parent.left
                anchors.right: parent.right
                spacing: Theme.componentMargin

                property int btnCount: 2
                property int btnSize: singleColumn ? width : ((width-(spacing*(btnCount-1))) / btnCount)

                ButtonFlat {
                    width: parent.btnSize
                    color: Theme.colorGrey

                    text: qsTr("Cancel")
                    onClicked: popupPgpKey.close()
                }
                ButtonFlat {
                    width: parent.btnSize
                    color: Theme.colorPrimary

                    text: qsTr("Set key")
                    onClicked: {
                        if (popupPgpKey.selectedFile.toString() === "") {
                            popupPgpKey.errorString = qsTr("Please select a key dump first.")
                            return
                        }
                        if (!popupPgpKey.currentDevice) return

                        if (!popupPgpKey.currentDevice.setDeviceKeyFile(popupPgpKey.selectedFile)) {
                            popupPgpKey.errorString = qsTr("This file is not a valid key dump for this device.")
                            return
                        }

                        popupPgpKey.confirmed()
                        popupPgpKey.close()
                    }
                }
            }

            ////////
        }
    }

    ////////////////////////////////////////////////////////////////////////////
}
