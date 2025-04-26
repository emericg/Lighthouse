import QtQuick
import QtQuick.Controls

import ComponentLibrary
import Lighthouse

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
/*
        SelectorMenuColorful { // top menu selector
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
*/
        Item { // bottom menu selector
            id: inputSelector
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: mobileMenu.hhh

            property int hhh: 72
            property int hhi: (hhh * 0.4)
            property int www: 80

            property int currentSelection: 1

            Row {
                anchors.centerIn: parent
                spacing: Theme.componentMargin

                MobileMenuItem_vertical {
                    width: mobileMenu.www
                    height: mobileMenu.hhh
                    sourceSize: mobileMenu.hhi

                    colorContent: Theme.colorTabletmenuContent
                    colorHighlight: Theme.colorTabletmenuHighlight

                    text: qsTr("touchpad")
                    source: "qrc:/IconLibrary/material-icons/duotone/touch_app.svg"
                    highlighted: (inputSelector.currentSelection === 1)
                    onClicked: inputSelector.currentSelection = 1
                }
                MobileMenuItem_vertical {
                    width: mobileMenu.www
                    height: mobileMenu.hhh
                    sourceSize: mobileMenu.hhi

                    colorContent: Theme.colorTabletmenuContent
                    colorHighlight: Theme.colorTabletmenuHighlight

                    text: qsTr("gamepad")
                    source: highlighted ?
                                "qrc:/IconLibrary/material-symbols/hardware/joystick-fill.svg" :
                                "qrc:/IconLibrary/material-symbols/hardware/joystick.svg"
                    highlighted: (inputSelector.currentSelection === 2)
                    onClicked: inputSelector.currentSelection = 2
                }
                MobileMenuItem_vertical {
                    width: mobileMenu.www
                    height: mobileMenu.hhh
                    sourceSize: mobileMenu.hhi

                    colorContent: Theme.colorTabletmenuContent
                    colorHighlight: Theme.colorTabletmenuHighlight

                    text: qsTr("remote")
                    source: highlighted ?
                                "qrc:/IconLibrary/material-symbols/hardware/nest_remote-fill.svg" :
                                "qrc:/IconLibrary/material-symbols/hardware/nest_remote.svg"
                    highlighted: (inputSelector.currentSelection === 3)
                    onClicked: inputSelector.currentSelection = 3
                }
                MobileMenuItem_vertical {
                    width: mobileMenu.www
                    height: mobileMenu.hhh
                    sourceSize: mobileMenu.hhi

                    colorContent: Theme.colorTabletmenuContent
                    colorHighlight: Theme.colorTabletmenuHighlight

                    text: qsTr("media")
                    source: highlighted ?
                                "qrc:/IconLibrary/material-symbols/media/album-fill.svg" :
                                "qrc:/IconLibrary/material-symbols/media/album.svg"
                    highlighted: (inputSelector.currentSelection === 4)
                    onClicked: inputSelector.currentSelection = 4
                }
            }
        }

        ////////////////

        Item {
            anchors.top: parent.top
            anchors.topMargin: 0 + 8
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 72

            ////

            VirtualInput_touchpad {
                id: virtualInput_touchpad
                anchors.fill: parent

                visible: (inputSelector.currentSelection === 1)
            }

            ////

            VirtualInput_gamepad {
                id: virtualInput_gamepad
                anchors.fill: parent

                visible: (inputSelector.currentSelection === 2)
            }

            ////

            VirtualInput_remote {
                id: virtualInput_remote
                anchors.fill: parent

                visible: (inputSelector.currentSelection === 3)
            }

            ////

            VirtualInput_media {
                id: virtualInput_media
                anchors.fill: parent

                visible: (inputSelector.currentSelection === 4)
            }

            ////
        }

        ////////////////
    }

    ////////////////////////////////////////////////////////////////////////////
}
