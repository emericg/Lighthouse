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

                source: networkControls.metaThumbnail
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

        Column {
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: 2

            Text {
                anchors.left: parent.left
                anchors.right: parent.right

                visible: (text.length > 0)
                text: networkControls.metaTitle
                font.pixelSize: Theme.fontSizeContentBig
                font.bold: true
                color: Theme.colorText
                elide: Text.ElideRight
            }

            Text {
                anchors.left: parent.left
                anchors.right: parent.right

                visible: (text.length > 0)
                text: {
                    var sub = networkControls.metaArtist
                    if (networkControls.metaAlbum.length > 0)
                        sub += (sub.length > 0 ? " — " : "") + networkControls.metaAlbum
                    return sub
                }
                font.pixelSize: Theme.fontSizeContentSmall
                color: Theme.colorSubText
                elide: Text.ElideRight
            }
        }

        ////

        SliderValueSolid {
            anchors.left: parent.left
            anchors.right: parent.right

            // read-only playback progress (seeking over the network is not supported)
            enabled: false

            from: 0
            to: 100
            value: networkControls.position >= 0 ? networkControls.position : 0
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

                muted: networkControls.volumeMuted

                onVolumeMute: networkControls.volume_mute()
                onVolumeDown: networkControls.volume_down()
                onVolumeUp: networkControls.volume_up()
            }
        }

        ////

        SliderValueSolid {
            id: volumeSlider
            anchors.left: parent.left
            anchors.right: parent.right

            from: 0
            to: 100
            stepSize: 1
            unit: "%"
            floatprecision: 0

            enabled: networkControls.connected && networkControls.volume >= 0

            // reflect the desktop state, but never overwrite what the user is setting
            property real remoteVolume: networkControls.volume >= 0 ? networkControls.volume * 100 : 0
            onRemoteVolumeChanged: if (!pressed) value = remoteVolume
            Component.onCompleted: value = remoteVolume

            // apply the chosen level once, on release
            onPressedChanged: if (!pressed) networkControls.volume_set(Math.round(value))
        }

        ////
    }

    ////////
}
