import QtQuick
import QtQuick.Controls

import ThemeEngine

Loader {
    id: virtualInputs
    anchors.fill: parent

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

    sourceComponent: Item {
        anchors.fill: parent
        anchors.margins: 16

        focus: parent.focus

        function backAction() {
            if (inputSelector.currentSelection === 0) {
                virtualInput_touchpad.backAction()
            } else if (inputSelector.currentSelection === 1) {
                virtualInput_gamepad.backAction()
            } else if (inputSelector.currentSelection === 2) {
                virtualInput_remote.backAction()
            } else if (inputSelector.currentSelection === 3) {
                virtualInput_media.backAction()
            } else {
                screenDeviceList.loadScreen()
            }
        }

        ////////////////

        SelectorMenuColorful {
            id: inputSelector

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 40

            model: ListModel {
                ListElement { idx: 1; txt: qsTr("touchpad"); src: ""; sz: 16; }
                ListElement { idx: 2; txt: qsTr("gamepad"); src: ""; sz: 16; }
                ListElement { idx: 3; txt: qsTr("remote"); src: ""; sz: 16; }
                ListElement { idx: 4; txt: qsTr("media"); src: ""; sz: 16; }
            }

            currentSelection: 1
            onMenuSelected: (index) => {
                currentSelection = index
            }
        }

        ////////////////

        VirtualInput_touchpad {
            id: virtualInput_touchpad

            anchors.top: inputSelector.bottom
            anchors.topMargin: 16
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            visible: (inputSelector.currentSelection === 1)
        }

        ////////////////

        VirtualInput_gamepad {
            id: virtualInput_gamepad

            anchors.top: inputSelector.bottom
            anchors.topMargin: 16
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            visible: (inputSelector.currentSelection === 2)
        }

        ////////////////

        VirtualInput_remote {
            id: virtualInput_remote

            anchors.top: inputSelector.bottom
            anchors.topMargin: 16
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            visible: (inputSelector.currentSelection === 3)
        }

        ////////////////

        VirtualInput_media {
            id: virtualInput_media

            anchors.top: inputSelector.bottom
            anchors.topMargin: 16
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            visible: (inputSelector.currentSelection === 4)
        }

        ////////////////
    }

    ////////////////////////////////////////////////////////////////////////////
}
