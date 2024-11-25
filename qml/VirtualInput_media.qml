import QtQuick
import QtQuick.Effects
import QtQuick.Controls

import ComponentLibrary
import Lighthouse

Grid {
    id: virtualInput_media

    rows: singleColumn ? 2 : 1
    columns: singleColumn ? 1 : 2
    spacing: 16

    property int gridSize: singleColumn ? width : Math.min(height, parent.width / 2 - 16)

    function backAction() {
        screenDeviceList.loadScreen()
    }

    ////////

    Column {
        width: gridSize
        height: gridSize
        spacing: 16

        ////

        Item {
            id: thumbnail
            anchors.left: parent.left
            anchors.leftMargin: 0
            anchors.right: parent.right
            anchors.rightMargin: 0
            height: width

            Rectangle {
                anchors.fill: parent
                color: Theme.colorForeground

                IconSvg { // generic thumbnail
                    anchors.fill: parent

                    source: "qrc:/IconLibrary/material-symbols/media/slideshow.svg"
                    color: Theme.colorIcon
                    opacity: 0.33
                }
            }
            Image { // actual thumbnail
                anchors.fill: parent

                source: mprisControls.metaThumbnail
                sourceSize: Qt.size(width, height)
                fillMode: Image.PreserveAspectCrop
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
                        x: thumbnail.x
                        y: thumbnail.y
                        width: thumbnail.width
                        height: thumbnail.height
                        radius: 16
                    }
                }
            }
        }
    }

    ////////

    Column {
        width: gridSize
        height: gridSize
        spacing: 16

        ////

        SliderValueSolid {
            anchors.left: parent.left
            anchors.right: parent.right
        }

        ////

        Row {
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: 16

            property int btnSize: singleColumn ? ((gridSize - 16) / 6) : 64

            MediaButtonRow {
                btnSize: parent.btnSize

                onMediaPrevious: networkControls.media_prev()
                onMediaPlayPause: networkControls.media_playpause()
                onMediaNext: networkControls.media_next()
            }

            VolumeButtonRow {
                btnSize: parent.btnSize

                onVolumeMute: networkControls.volume_mute()
                onVolumeDown: networkControls.volume_down()
                onVolumeUp: networkControls.volume_up()
            }
        }

        ////
    }

    ////////
}
