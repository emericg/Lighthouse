import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import ComponentLibrary

Grid {
    id: virtualInput_touchpad

    rows: singleColumn ? 2 : 1
    columns: singleColumn ? 1 : 2
    spacing: 16

    property int gridSize: singleColumn ? width : Math.min(height, parent.width / 2 - 16)

    function backAction() {
        if (virtualtextfield.focus) {
            virtualtextfield.focus = false
            return
        }

        screenDeviceList.loadScreen()
    }

    ////////

    Column { // touchpad
        width: gridSize
        height: gridSize
        spacing: 16

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            height: width

            clip: true

            radius: 8
            color: Theme.colorForeground
            border.width: 2
            border.color: Theme.colorSeparator

            MouseArea {
                id: mousearea
                anchors.fill: parent

                hoverEnabled: true

                //onPressed: { }
                //onClicked: { }
                //onDoubleClicked: { }
            }

            Rectangle { // touch pointer
                id: mouseBackground

                width: 64
                height: 64
                radius: 64

                x: mousearea.mouseX - (width / 2)
                y: mousearea.mouseY - (width / 2)

                color: Theme.colorMaterialDeepOrange
                opacity: mousearea.containsMouse ? 1 : 0
                Behavior on opacity { NumberAnimation { duration: 133 } }
            }
        }
    }

    ////////

    Column { // virtual keyboard
        width: gridSize
        height: gridSize
        spacing: 16

        ButtonClear { // open OS keyboard
            anchors.left: parent.left
            anchors.right: parent.right

            text: qsTr("keyboard")
            source: "qrc:/assets/gfx/icons/keyboard-variant.svg"

            colorBackground: virtualtextfield.focus ? Theme.colorPrimary : "white"
            colorHighlight: virtualtextfield.focus ? "white" : Theme.colorPrimary
            colorBorder: virtualtextfield.focus ? Theme.colorPrimary : Theme.colorComponentBorder
            colorText: virtualtextfield.focus ? "white" : Theme.colorPrimary

            onClicked: {
                if (virtualtextfield.focus) {
                    virtualtextfield.focus = false
                    virtualtextfield.clear()
                } else {
                    virtualtextfield.forceActiveFocus()
                }
            }
        }

        TextField { // virtual text field
            id: virtualtextfield
            anchors.left: parent.left
            anchors.right: parent.right

            //visible: false // < set to invisible
            cursorVisible: false
            echoMode: TextInput.NoEcho

            background: Rectangle { // fake invisibility
                border.width: 0
                color: Theme.colorBackground
                border.color: Theme.colorBackground
            }

            onDisplayTextChanged: {
                if (displayText) {
                    //console.log("virtualtextfield::onDisplayTextChanged(" + displayText + ")")

                    // send virtual event
                    networkControls.sendKey(displayText)
                    clear()
                }
            }
            onEditingFinished: {
                virtualtextfield.focus = false
                clear()
            }
        }
    }

    ////////
}
