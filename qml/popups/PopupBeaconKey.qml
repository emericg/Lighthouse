import QtQuick
import QtQuick.Controls

import ThemeEngine 1.0

Popup {
    id: popupBeaconKey
    x: (appWindow.width / 2) - (width / 2)
    y: singleColumn ? (appWindow.height - height) : ((appWindow.height / 2) - (height / 2) - (appHeader.height))

    width: singleColumn ? parent.width : 640
    height: columnContent.height + padding*2
    padding: singleColumn ? 20 : 24

    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    signal confirmed()

    onAboutToShow: {
        textInputBeaconKey.text = selectedDevice.beaconkey
        textInputBeaconKey.focus = false
    }
    onAboutToHide: {
        textInputBeaconKey.focus = false
    }

    ////////////////////////////////////////////////////////////////////////////

    background: Rectangle {
        color: Theme.colorBackground
        border.color: Theme.colorSeparator
        border.width: singleColumn ? 0 : Theme.componentBorderWidth
        radius: singleColumn ? 0 : Theme.componentRadius

        Rectangle {
            width: parent.width
            height: Theme.componentBorderWidth
            visible: singleColumn
            color: Theme.colorSeparator
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    contentItem: Item {
        Column {
            id: columnContent
            width: parent.width
            spacing: 20

            Text {
                width: parent.width

                text: qsTr("Set the beacon key of this remote")
                textFormat: Text.PlainText
                font.pixelSize: Theme.fontSizeContentVeryBig
                color: Theme.colorText
                wrapMode: Text.WordWrap
            }

            Column {
                width: parent.width
                spacing: 8

                Text {
                    width: parent.width

                    text: qsTr("A beacon key MUST be set before you can use this remote. The format expected is a 24 hexadecimal characters string.")
                    textFormat: Text.StyledText
                    font.pixelSize: Theme.fontSizeContent
                    color: Theme.colorSubText
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width

                    text: qsTr("You can extract the key using this <a href=\"https://github.com/custom-components/ble_monitor/blob/master/custom_components/ble_monitor/ble_parser/get_beacon_key.py\">python script</a>.")
                    textFormat: Text.StyledText
                    font.pixelSize: Theme.fontSizeContent
                    color: Theme.colorSubText
                    wrapMode: Text.WordWrap
                    linkColor: Theme.colorText
                    onLinkActivated: (link) => { Qt.openUrlExternally(link) }
                }

                Text {
                    width: parent.width

                    text: qsTr("Sorry for the inconvenience.")
                    textFormat: Text.StyledText
                    font.pixelSize: Theme.fontSizeContent
                    color: Theme.colorSubText
                    wrapMode: Text.WordWrap
                }

                TextFieldThemed {
                    id: textInputBeaconKey
                    width: parent.width

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

            Flow {
                id: flowContent
                width: parent.width
                height: singleColumn ? 80+16 : 40

                property var btnSize: singleColumn ? width : ((width-spacing) / 2)
                spacing: 16

                ButtonWireframe {
                    width: parent.btnSize

                    text: qsTr("Cancel")
                    primaryColor: Theme.colorSubText
                    secondaryColor: Theme.colorForeground

                    onClicked: {
                        textInputBeaconKey.focus = false
                        popupBeaconKey.close()
                    }
                }
                ButtonWireframe {
                    width: parent.btnSize

                    text: qsTr("Set key")
                    primaryColor: Theme.colorPrimary
                    fullColor: true

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
        }
    }

    ////////////////////////////////////////////////////////////////////////////
}
