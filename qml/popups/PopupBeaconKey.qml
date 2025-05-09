import QtQuick
import QtQuick.Effects
import QtQuick.Controls

import ComponentLibrary

Popup {
    id: popupBeaconKey

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

    onAboutToShow: {
        textInputBeaconKey.text = selectedDevice.beaconkey
        textInputBeaconKey.focus = false
    }
    onAboutToHide: {
        textInputBeaconKey.focus = false
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

                text: qsTr("Set the beacon key of this remote")
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

                    text: qsTr("A beacon key MUST be set before you can use this remote. The format expected is a 24 hexadecimal characters string.")
                    textFormat: Text.StyledText
                    font.pixelSize: Theme.fontSizeContent
                    color: Theme.colorSubText
                    wrapMode: Text.WordWrap
                }

                Text {
                    anchors.left: parent.left
                    anchors.right: parent.right

                    text: qsTr("You can extract the key using this <a href=\"https://github.com/custom-components/ble_monitor/blob/master/custom_components/ble_monitor/ble_parser/get_beacon_key.py\">python script</a>.")
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

                    text: qsTr("Sorry for the inconvenience.")
                    textFormat: Text.StyledText
                    font.pixelSize: Theme.fontSizeContent
                    color: Theme.colorSubText
                    wrapMode: Text.WordWrap
                }

                TextFieldThemed {
                    id: textInputBeaconKey
                    anchors.left: parent.left
                    anchors.right: parent.right

                    font.pixelSize: 18
                    font.bold: false
                    color: Theme.colorText

                    overwriteMode: true
                    maximumLength: 24
                    inputMask: "HHHHHHHHHHHHHHHHHHHHHHHH"
                    //validator: RegularExpressionValidator { regularExpression: /[0-9A-F]+/ }
                    inputMethodHints: Qt.ImhNoPredictiveText

                    placeholderText: "AABBCCDDEEFFGGHHIIJJKKLL"
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
                    onClicked: {
                        textInputBeaconKey.focus = false
                        popupBeaconKey.close()
                    }
                }
                ButtonFlat {
                    width: parent.btnSize
                    color: Theme.colorPrimary

                    text: qsTr("Set key")
                    onClicked: {
                        if (selectedDevice /*&& textInputBeaconKey.text.length === 24*/) {
                            selectedDevice.setBeaconKey(textInputBeaconKey.text)
                        }
                        textInputBeaconKey.focus = false
                        popupBeaconKey.confirmed()
                        popupBeaconKey.close()
                    }
                }
            }

            ////////
        }
    }

    ////////////////////////////////////////////////////////////////////////////
}
