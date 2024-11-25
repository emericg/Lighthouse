import QtQuick
import QtQuick.Effects
import QtQuick.Controls

import ComponentLibrary
import LocalActions
import Lighthouse

Popup {
    id: popupActions

    x: (appWindow.width / 2) - (width / 2)
    y: ((appWindow.height / 2) - (height / 2) - (appHeader.height))

    width: singleColumn ? parent.width : parent.width * 0.8
    height: singleColumn ? columnContent.height + padding*2 : parent.height * 0.8
    padding: 0

    dim: true
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    signal confirmed()

    property var currentButton: null

    property int currentButtonMode: 0 // single click, double click, or long press
    property int currentButtonAction: 0 // LocalActions.Actions
    property string currentButtonParams: ""

    function openAction(button, mode) {
        if (typeof button === "undefined" || !button) return
        //console.log("> openAction(" + button + " / " + mode + ")")

        currentButton = button

        buttonId.text = qsTr("BUTTON #%1").arg(currentButton.id)

        if (mode === 0) {
            currentButtonMode = 0
            currentButtonAction = button.singleAction
            currentButtonParams = button.singleParameters
            buttonMode.text = qsTr("SINGLE PRESS")
        } else if (mode === 1) {
            currentButtonMode = 1
            currentButtonAction = button.doubleAction
            currentButtonParams = button.doubleParameters
            buttonMode.text = qsTr("DOUBLE PRESS")
        } else if (mode === 2) {
            currentButtonMode = 2
            currentButtonAction = button.longAction
            currentButtonParams = button.longParameters
            buttonMode.text = qsTr("LONG PRESS")
        }

        if (currentButtonAction == LocalActions.ACTION_NO_ACTION) {
            //name = "keyboard shortcut"
            menuArea.mode = qsTr("actions")
        }
        if (currentButtonAction == LocalActions.ACTION_keys) {
            //name = "keyboard shortcut"
            menuArea.mode = qsTr("keyboard")
        }
        if (currentButtonAction == LocalActions.ACTION_commands) {
            //name = "launch app"
            menuArea.mode = qsTr("command")
            fileinput.text = currentButtonParams
        }

        if (currentButtonAction > LocalActions.ACTION_KEYBOARD_START &&
            currentButtonAction < LocalActions.ACTION_KEYBOARD_STOP) {
            //name = "keyboard action"
            menuArea.mode = qsTr("actions")
        }
        if (currentButtonAction > LocalActions.ACTION_MPRIS_START &&
            currentButtonAction < LocalActions.ACTION_MPRIS_STOP) {
            //name = "media action"
            menuArea.mode = qsTr("actions")
        }

        open()
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
            spacing: 0

            Rectangle { // titleArea
                anchors.left: parent.left
                anchors.leftMargin: 0
                anchors.right: parent.right
                anchors.rightMargin: 0

                height: 64
                radius: Theme.componentRadius
                color: Theme.colorPrimary

                Rectangle {
                    anchors.left: parent.left
                    anchors.leftMargin: 0
                    anchors.right: parent.right
                    anchors.rightMargin: 0
                    anchors.bottom: parent.bottom
                    height: parent.radius
                    color: parent.color
                }

                Row {
                    anchors.left: parent.left
                    anchors.leftMargin: 24
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 24

                    Text {
                        id: buttonId
                        anchors.verticalCenter: parent.verticalCenter

                        text: qsTr("BUTTON #1")
                        font.pixelSize: Theme.fontSizeTitle
                        font.bold: true

                        color: "white"
                    }

                    TagFlat {
                        id: buttonMode
                        anchors.verticalCenter: parent.verticalCenter
                        height: Theme.componentHeight

                        text: qsTr("SINGLE PRESS")
                        color: Theme.colorHeaderHighlight
                    }
                }
            }

            ////////////////

            Rectangle { // menuArea
                id: menuArea
                anchors.left: parent.left
                anchors.leftMargin: 0
                anchors.right: parent.right
                anchors.rightMargin: 0

                height: 48
                color: Theme.colorSecondary

                property string mode: qsTr("actions")

                Row {
                    DesktopHeaderItem {
                        height: menuArea.height

                        source: "qrc:/IconLibrary/material-symbols/hardware/nest_remote.svg"
                        colorContent: Theme.colorHeaderContent
                        colorHighlight: Theme.colorHeaderHighlight

                        text: qsTr("actions")

                        highlighted: (menuArea.mode === qsTr("actions"))
                        onClicked: menuArea.mode = qsTr("actions")
                    }
                    DesktopHeaderItem {
                        height: menuArea.height

                        source: "qrc:/IconLibrary/material-symbols/media/play_pause.svg"
                        colorContent: Theme.colorHeaderContent
                        colorHighlight: Theme.colorHeaderHighlight

                        text: qsTr("media")

                        highlighted: (menuArea.mode === qsTr("media"))
                        onClicked: menuArea.mode = qsTr("media")
                    }
                    DesktopHeaderItem {
                        height: menuArea.height

                        source: "qrc:/assets/gfx/icons/keyboard-variant.svg"
                        colorContent: Theme.colorHeaderContent
                        colorHighlight: Theme.colorHeaderHighlight

                        text: qsTr("keyboard")

                        highlighted: (menuArea.mode === qsTr("keyboard"))
                        onClicked: menuArea.mode = qsTr("keyboard")
                    }
                    DesktopHeaderItem {
                        height: menuArea.height

                        source: "qrc:/IconLibrary/material-icons/duotone/launch.svg"
                        colorContent: Theme.colorHeaderContent
                        colorHighlight: Theme.colorHeaderHighlight

                        text: qsTr("command")

                        highlighted: (menuArea.mode === qsTr("command"))
                        onClicked: menuArea.mode = qsTr("command")
                    }
                }
            }

            ////////////////

            Column { // content
                anchors.left: parent.left
                anchors.leftMargin: 24
                anchors.right: parent.right
                anchors.rightMargin: 24

                topPadding: 24
                bottomPadding: 24
                spacing: 16

                visible: menuArea.mode === qsTr("actions") || menuArea.mode === qsTr("media")

                Flow {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    spacing: 16

                    Repeater {
                        model: [LocalActions.ACTION_NO_ACTION]

                        TagFlat {
                            text: UtilsActions.getActionName(modelData)

                            color: (modelData === currentButtonAction) ? Theme.colorPrimary : Theme.colorForeground
                            colorText: (modelData === currentButtonAction) ? "white" : Theme.colorSubText

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    currentButtonAction = modelData
                                }
                            }
                        }
                    }
                }

                Text { // subtitle
                    text: qsTr("Hardware:")
                    color: Theme.colorText
                    font.bold: true
                    font.pixelSize: Theme.fontSizeContentBig
                }

                Flow {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    spacing: 16

                    Repeater {
                        model: [LocalActions.ACTION_KEYBOARD_computer_lock,
                                LocalActions.ACTION_KEYBOARD_computer_sleep,
                                LocalActions.ACTION_KEYBOARD_computer_poweroff,
                                LocalActions.ACTION_KEYBOARD_monitor_brightness_up,
                                LocalActions.ACTION_KEYBOARD_monitor_brightness_down,
                                LocalActions.ACTION_KEYBOARD_keyboard_brightness_onoff,
                                LocalActions.ACTION_KEYBOARD_keyboard_brightness_up,
                                LocalActions.ACTION_KEYBOARD_keyboard_brightness_down]

                        TagFlat {
                            text: UtilsActions.getActionName(modelData)

                            color: (modelData === currentButtonAction) ? Theme.colorPrimary : Theme.colorForeground
                            colorText: (modelData === currentButtonAction) ? "white" : Theme.colorSubText

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    currentButtonAction = modelData
                                }
                            }
                        }
                    }
                }

                Text { // subtitle
                    text: qsTr("Actions:")
                    color: Theme.colorText
                    font.bold: true
                    font.pixelSize: Theme.fontSizeContentBig
                }

                Flow {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    spacing: 16

                    Repeater {
                        model: [LocalActions.ACTION_KEYBOARD_desktop_backward,
                                LocalActions.ACTION_KEYBOARD_desktop_forward,
                                LocalActions.ACTION_KEYBOARD_desktop_refresh,
                                LocalActions.ACTION_KEYBOARD_desktop_stop,
                                LocalActions.ACTION_KEYBOARD_desktop_fullscreen,
                                LocalActions.ACTION_KEYBOARD_desktop_calculator,
                                LocalActions.ACTION_KEYBOARD_desktop_web,
                                LocalActions.ACTION_KEYBOARD_desktop_mail,
                                LocalActions.ACTION_KEYBOARD_desktop_calendar,
                                LocalActions.ACTION_KEYBOARD_desktop_files]

                        TagFlat {
                            text: UtilsActions.getActionName(modelData)

                            color: (modelData === currentButtonAction) ? Theme.colorPrimary : Theme.colorForeground
                            colorText: (modelData === currentButtonAction) ? "white" : Theme.colorSubText

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    currentButtonAction = modelData
                                }
                            }
                        }
                    }
                }

                Text { // subtitle
                    text: qsTr("Media:")
                    color: Theme.colorText
                    font.bold: true
                    font.pixelSize: Theme.fontSizeContentBig
                }

                Flow {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    spacing: 16

                    Repeater {
                        model: [LocalActions.ACTION_KEYBOARD_media_playpause,
                                LocalActions.ACTION_KEYBOARD_media_stop,
                                LocalActions.ACTION_KEYBOARD_media_next,
                                LocalActions.ACTION_KEYBOARD_media_prev,
                                LocalActions.ACTION_KEYBOARD_volume_mute,
                                LocalActions.ACTION_KEYBOARD_volume_up,
                                LocalActions.ACTION_KEYBOARD_volume_down]

                        TagFlat {
                            text: UtilsActions.getActionName(modelData)

                            color: (modelData === currentButtonAction) ? Theme.colorPrimary : Theme.colorForeground
                            colorText: (modelData === currentButtonAction) ? "white" : Theme.colorSubText

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    currentButtonAction = modelData
                                }
                            }
                        }
                    }
                }
            }

            ////

            Column { // content
                anchors.left: parent.left
                anchors.leftMargin: 24
                anchors.right: parent.right
                anchors.rightMargin: 24

                topPadding: 24
                bottomPadding: 24
                spacing: 16
                visible: menuArea.mode === qsTr("keyboard")

                Text { // subtitle
                    text: qsTr("Keyboard shortcut:")
                    color: Theme.colorText
                    font.bold: true
                    font.pixelSize: Theme.fontSizeContentBig
                }

                Text {
                    text: qsTr("Select a keyboard shortcut.")
                    color: Theme.colorText
                    font.pixelSize: Theme.fontSizeContent
                }

                Row {
                    spacing: 16

                    ComboBoxThemed {
                        width: 96
                        model: ["ctrl", "alt", "opt", "maj"]
                    }

                    ComboBoxThemed {
                        width: 96
                        model: ["", "ctrl", "alt", "opt", "maj"]
                    }

                    TextFieldThemed {
                        width: 96
                        text: "a"
                    }
                }
            }

            ////

            Column { // content
                anchors.left: parent.left
                anchors.leftMargin: 24
                anchors.right: parent.right
                anchors.rightMargin: 24

                topPadding: 24
                bottomPadding: 24
                spacing: 16
                visible: menuArea.mode === qsTr("command")

                Text { // subtitle
                    text: qsTr("Application launcher:")
                    color: Theme.colorText
                    font.bold: true
                    font.pixelSize: Theme.fontSizeContentBig
                }

                Text {
                    text: qsTr("Input a command to launch, or select a binary with the file chooser.")
                    color: Theme.colorText
                    font.pixelSize: Theme.fontSizeContent
                }

                FileInputArea {
                    id: fileinput
                    anchors.left: parent.left
                    anchors.right: parent.right
                }
            }
        }

        ////////

        Row {
            anchors.right: parent.right
            anchors.rightMargin: Theme.componentMarginXL
            anchors.bottom: parent.bottom
            anchors.bottomMargin: Theme.componentMarginXL
            spacing: Theme.componentMarginL

            ButtonFlat {
                anchors.verticalCenter: parent.verticalCenter
                color: Theme.colorGrey

                text: qsTr("Cancel")
                onClicked: popupActions.close()
            }
            ButtonFlat {
                anchors.verticalCenter: parent.verticalCenter
                color: Theme.colorSuccess

                text: qsTr("Save")
                source: "qrc:/IconLibrary/material-symbols/save.svg"
                onClicked: {
                    var newAction
                    var newParameter

                    if (menuArea.mode === qsTr("actions") || menuArea.mode === qsTr("media")) {
                        newAction = currentButtonAction
                        newParameter = ""
                    } else if (menuArea.mode === qsTr("keyboard")) {
                        newAction = LocalActions.ACTION_keys
                        newParameter = ""
                    } else if (menuArea.mode === qsTr("command")) {
                        newAction = LocalActions.ACTION_commands
                        newParameter = fileinput.text
                    }

                    if (currentButtonMode == 0) {
                        currentButton.singleAction = newAction
                        currentButton.singleParameters = newParameter
                    } else if (currentButtonMode == 1) {
                        currentButton.doubleAction = newAction
                        currentButton.doubleParameters = newParameter
                    } else if (currentButtonMode == 2) {
                        currentButton.longAction = newAction
                        currentButton.longParameters = newParameter
                    }

                    popupActions.confirmed()
                    popupActions.close()
                }
            }
        }

        ////////
    }

    ////////////////////////////////////////////////////////////////////////////
}
