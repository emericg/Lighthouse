import QtQuick
import QtQuick.Effects
import QtQuick.Controls

import ThemeEngine

Item {
    id: virtualInput_remote

    signal buttonNext()
    signal buttonPrevious()

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

        Item {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: -(btn111.height/2)

            width: parent.width * 0.66
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

                source: "qrc:/assets/icons/material-symbols/play_arrow-fill.svg"
                sourceRotation: -180
            }
            RoundButtonFlat {
                id: btn222
                width: parent.width
                height: parent.width

                source: "qrc:/assets/icons/material-symbols/play_arrow-fill.svg"
            }
        }

        ////////

        Column {
            anchors.centerIn: parent
            width: parent.width * 0.8

            visible: (remoteSelector.currentSelection === 2) // media
            spacing: 32

            ////

            Rectangle {
                id: mediaRectangle
                width: parent.width
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
                    source: "qrc:/assets/icons/material-symbols/add.svg"
                    sourceSize: 40
                }
                SquareButtonClear {
                    anchors.left: parent.left
                    width: parent.width * 0.32
                    height: parent.height
                    radius: 0

                    colorBackground: "transparent"
                    source: "qrc:/assets/icons/material-symbols/chevron_left.svg"
                    sourceSize: 40
                }
                SquareButtonClear {
                    anchors.right: parent.right
                    width: parent.width * 0.32
                    height: parent.height

                    clip: false
                    radius: 0

                    colorBackground: "transparent"
                    source: "qrc:/assets/icons/material-symbols/chevron_right.svg"
                    sourceSize: 40
                }
                SquareButtonClear {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: parent.height * 0.32
                    radius: 0

                    colorBackground: "transparent"
                    source: "qrc:/assets/icons/material-symbols/remove.svg"
                    sourceSize: 40
                }

                SquareButtonFlat {
                    anchors.centerIn: parent
                    width: parent.width * 0.44
                    height: width

                    //color: Theme.colorPrimary
                    source: "qrc:/assets/icons/material-symbols/play_pause.svg"
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
                    source: "qrc:/assets/icons/material-symbols/language.svg"
                    sourceSize: 40
                }

                SquareButtonClear {
                    width: ((parent.width - 24) / 2)
                    height: 64
                    clip: true
                    source: "qrc:/assets/icons/material-symbols/left_click.svg"
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
