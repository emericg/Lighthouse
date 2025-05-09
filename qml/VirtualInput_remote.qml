import QtQuick
import QtQuick.Effects
import QtQuick.Controls

import ComponentLibrary

Item {
    id: virtualInput_remote

    signal buttonNext()
    signal buttonPrevious()

    function backAction() {
        screenDeviceList.loadScreen()
    }

    ////////////////

    SelectorMenuColorful {
        id: remoteSelector

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        height: 40

        model: ListModel {
            ListElement { idx: 1; txt: qsTr("presentation"); src: ""; sz: 16; }
            ListElement { idx: 2; txt: qsTr("media"); src: ""; sz: 16; }
            ListElement { idx: 3; txt: qsTr("keyboard"); src: ""; sz: 16; }
        }

        currentSelection: 1
        onMenuSelected: (index) => {
            currentSelection = index
        }
    }

    ////////////////

    Item {
        anchors.top: remoteSelector.bottom
        anchors.topMargin: 16
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        ////////

        Item { //// SLIDE REMOTE
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: -(btn111.height/2)

            width: singleColumn ? parent.width * 0.66 : parent.height * 0.6
            height: width

            visible: (remoteSelector.currentSelection === 1) // presentation

            RoundButtonClear {
                id: btn111
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.bottom
                anchors.topMargin: -width * 0.16
                width: parent.width * 0.66
                height: width
                z: -1

                source: "qrc:/IconLibrary/material-symbols/media/play_arrow-fill.svg"
                sourceRotation: -180

                onClicked: networkControls.key_left()
            }
            RoundButtonFlat {
                id: btn222
                width: parent.width
                height: parent.width

                source: "qrc:/IconLibrary/material-symbols/media/play_arrow-fill.svg"

                onClicked: networkControls.key_right()
            }
        }

        ////////

        Column { // MEDIA REMOTE
            anchors.centerIn: parent

            width: singleColumn ? parent.width * 0.66 : parent.height * 0.5

            visible: (remoteSelector.currentSelection === 2) // media
            spacing: 32

            ////

            Rectangle {
                id: mediaRectangle
                anchors.left: parent.left
                anchors.right: parent.right
                height: width

                radius: 32
                color: "transparent"
                border.width: 3
                border.color: Theme.colorPrimary

                Rectangle {
                    anchors.fill: parent
                    radius: 32
                    color: Theme.colorPrimary
                    opacity: 0.2
                }

                SquareButtonClear {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    width: parent.width
                    height: parent.height * 0.32
                    radius: 0

                    colorBackground: "transparent"
                    source: "qrc:/IconLibrary/material-symbols/add.svg"
                    sourceSize: 40

                    onClicked: networkControls.volume_up()
                }
                SquareButtonClear {
                    anchors.left: parent.left
                    width: parent.width * 0.32
                    height: parent.height
                    radius: 0

                    colorBackground: "transparent"
                    source: "qrc:/IconLibrary/material-symbols/chevron_left.svg"
                    sourceSize: 40

                    onClicked: networkControls.media_prev()
                }
                SquareButtonClear {
                    anchors.right: parent.right
                    width: parent.width * 0.32
                    height: parent.height

                    clip: false
                    radius: 0

                    colorBackground: "transparent"
                    source: "qrc:/IconLibrary/material-symbols/chevron_right.svg"
                    sourceSize: 40

                    onClicked: networkControls.media_next()
                }
                SquareButtonClear {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: parent.height * 0.32
                    radius: 0

                    colorBackground: "transparent"
                    source: "qrc:/IconLibrary/material-symbols/remove.svg"
                    sourceSize: 40

                    onClicked: networkControls.volume_down()
                }

                SquareButtonFlat {
                    anchors.centerIn: parent
                    width: parent.width * 0.44
                    height: width

                    //color: Theme.colorPrimary
                    source: "qrc:/IconLibrary/material-symbols/media/play_pause.svg"

                    onClicked: networkControls.media_playpause()
                }

                layer.enabled: true
                layer.effect: MultiEffect {
                    maskEnabled: true
                    maskInverted: false
                    maskThresholdMin: 0.5
                    maskSpreadAtMin: 1.0
                    maskSpreadAtMax: 0.0
                    maskSource: ShaderEffectSource {
                        sourceItem: Rectangle {
                            x: mediaRectangle.x
                            y: mediaRectangle.y
                            width: mediaRectangle.width
                            height: mediaRectangle.height
                            radius: 32
                        }
                    }
                }
            }

            ////

            Row {
                anchors.left: parent.left
                anchors.right: parent.right
                spacing: 24

                SquareButtonClear {
                    width: ((parent.width - 24) / 2)
                    height: 64
                    source: "qrc:/IconLibrary/material-symbols/language.svg"
                    sourceSize: 40
                }

                SquareButtonClear {
                    width: ((parent.width - 24) / 2)
                    height: 64
                    clip: true
                    source: "qrc:/IconLibrary/material-symbols/left_click.svg"
                    sourceSize: 40
                }
            }

            ////
        }

        ////////

        Item {
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width * 0.66
            height: width

            visible: (remoteSelector.currentSelection === 3) // keyboard
        }

        ////////
    }
}
