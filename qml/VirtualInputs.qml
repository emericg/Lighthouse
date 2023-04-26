import QtQuick
import QtQuick.Controls

import ThemeEngine 1.0

Loader {
    id: virtualInputs

    ////////

    function loadScreen(clickedDevice) {
        virtualInputs.active = true
        appContent.state = "VirtualInputs"
    }

    function backAction() {
        if (virtualInputs.status === Loader.Ready)
            virtualInputs.item.backAction()
    }

    ////////////////////////////////////////////////////////////////////////////

    active: false
    asynchronous: false

    sourceComponent: Column {
        id: itemVirtualInputs
        anchors.top: parent.top
        anchors.topMargin: 16
        anchors.left: parent.left
        anchors.leftMargin: 16
        anchors.right: parent.right
        anchors.rightMargin: 16
        spacing: 16

        focus: parent.focus

        function backAction() {
            if (virtualtextfield.focus) {
                virtualtextfield.focus = false
                return
            }

            appContent.state = "DeviceList"
        }

        ////////////////

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            height: width
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

            Rectangle { // mousePointer
                id: mouseBackground
                width: 32; height: 32; radius: 32;
                x: mousearea.mouseX - (width / 2)
                y: mousearea.mouseY - (width / 2)

                color: "red"
                opacity: mousearea.containsMouse ? 0.16 : 0
                Behavior on opacity { NumberAnimation { duration: 333 } }
                Behavior on width { NumberAnimation { duration: 200 } }
            }
        }

        ////////////////

        ButtonWireframeIcon {
            anchors.left: parent.left
            anchors.right: parent.right

            text: qsTr("keyboard")
            fullColor: virtualtextfield.focus
            source: "qrc:/assets/icons_material/keyboard-variant.svg"

            onClicked: {
                if (virtualtextfield.focus) {
                    virtualtextfield.focus = false
                    virtualtextfield.clear()
                } else {
                    virtualtextfield.forceActiveFocus()
                }
            }
        }

        ////////////////

        TextField {
            id: virtualtextfield
            anchors.left: parent.left
            anchors.right: parent.right

            visible: false
            cursorVisible: false
            echoMode: TextInput.NoEcho

            onDisplayTextChanged: {
                if (displayText) {
                    console.log("TEXT CHANGED : " + displayText)
                    // send virtual event
                    clear()
                }
            }
            onEditingFinished: {
                virtualtextfield.focus = false
                clear()
            }
        }

        ////////////////
    }

    ////////////////////////////////////////////////////////////////////////////
}