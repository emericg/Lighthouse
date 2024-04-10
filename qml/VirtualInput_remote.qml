import QtQuick
import QtQuick.Effects
import QtQuick.Controls

import ThemeEngine

Item {
    id: virtualInput_remote

    signal buttonNext()
    signal buttonPrevious()

    Column {
        anchors.left: parent.left
        anchors.leftMargin: 0
        anchors.right: parent.right
        anchors.rightMargin: 0

        spacing: 64

        ////////////////

        SelectorMenuColorful {
            id: remoteSelector

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
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width * 0.66
            height: width

            visible: (remoteSelector.currentSelection === 1) // presentation

            RoundButtonClear {
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
                width: parent.width
                height: parent.width

                source: "qrc:/assets/icons/material-symbols/play_arrow-fill.svg"
            }
        }

        ////////////////

        Column {
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width * 0.8
            height: width

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

                RoundButtonClear {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    width: parent.width * 0.44
                    height: parent.height * 0.28
                    clip: true
                    shape: "square"
                    colorBackground: "transparent"
                    source: "qrc:/assets/icons/material-symbols/chevron_right.svg"
                    sourceSize: 40
                }
                RoundButtonClear {
                    anchors.left: parent.left
                    width: parent.width * 0.34
                    height: parent.height
                    clip: true
                    shape: "square"
                    colorBackground: "transparent"
                    source: "qrc:/assets/icons/material-symbols/chevron_left.svg"
                    sourceSize: 40
                }
                RoundButtonClear {
                    anchors.right: parent.right
                    width: parent.width * 0.34
                    height: parent.height
                    clip: true
                    shape: "square"
                    colorBackground: "transparent"
                    source: "qrc:/assets/icons/material-symbols/chevron_right.svg"
                    sourceSize: 40
                }
                RoundButtonClear {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
                    width: parent.width * 0.44
                    height: parent.height * 0.28
                    clip: true
                    shape: "square"
                    colorBackground: "transparent"
                    source: "qrc:/assets/icons/material-symbols/chevron_right.svg"
                    sourceSize: 40
                }

                RoundButtonFlat {
                    anchors.centerIn: parent
                    width: parent.width * 0.44
                    height: width

                    shape: "square"
                    color: Theme.colorPrimary
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

                RoundButtonFlat {
                    width: ((parent.width - 24) / 2)
                    height: 64
                    clip: true
                    shape: "square"
                    source: "qrc:/assets/icons/material-symbols/language.svg"
                    sourceSize: 40
                }

                RoundButtonFlat {
                    width: ((parent.width - 24) / 2)
                    height: 64
                    clip: true
                    shape: "square"
                    source: "qrc:/assets/icons/material-symbols/left_click.svg"
                    sourceSize: 40
                }
            }

            ////
        }

        ////////////////

        Item {
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width * 0.66
            height: width

            visible: (remoteSelector.currentSelection === 3) // keyboard
        }

        ////////////////
    }
}
