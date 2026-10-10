import QtQuick
import QtQuick.Controls

import ComponentLibrary

Column {
    anchors.left: parent.left
    anchors.right: parent.right
    spacing: 8

    visible: isMobile

    ////////////////

    Rectangle { // NETWORK CONTROLS
        width: singleColumn ? parent.width : 480
        height: visible ? (singleColumn ? 112 : 128) : 0
        radius: 4

        visible: (networkClient !== null && networkClient.connected) || (SettingsManager.fakeIt)

        color: Theme.colorDeviceWidget
        border.width: 2
        border.color: singleColumn ? "transparent" : Theme.colorSeparator

        Rectangle { // Actual background
            anchors.fill: parent
            anchors.leftMargin: -8
            anchors.rightMargin: -8
            color: Theme.colorForeground
        }

        Column {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            spacing: 16

            ////////

            Row { // controls
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 16

                MediaButtonRow {
                    btnSize: 52

                    onMediaPrevious: networkControls.media_prev()
                    onMediaPlayPause: networkControls.media_playpause()
                    onMediaNext: networkControls.media_next()
                }
                VolumeButtonRow {
                    btnSize: 52

                    onVolumeMute: networkControls.volume_toggle_mute()
                    onVolumeDown: networkControls.volume_down()
                    onVolumeUp: networkControls.volume_up()
                }
            }

            ////////
        }
    }

    ////////////////

    Rectangle { // MEDIA CONTROLS
        width: singleColumn ? parent.width : 480
        height: visible ? (singleColumn ? 96 : 96) : 0
        radius: 4

        visible: (networkClient !== null && networkClient.connected && networkClient.metaTitle !== "") || (SettingsManager.fakeIt)

        color: Theme.colorForeground
        border.width: 2
        border.color: singleColumn ? "transparent" : Theme.colorSeparator

        MouseArea {
            anchors.fill: parent
            onClicked: screenMedia.loadScreen()
        }

        Item { // now playing
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width
            height: 80

            ////////

            Rectangle {
                id: mediaThumbnail
                anchors.left: parent.left
                anchors.leftMargin: 12
                anchors.verticalCenter: parent.verticalCenter

                width: 72
                height: 72
                radius: 4

                clip: false
                color: Theme.colorSeparator

                Image {
                    anchors.fill: parent

                    visible: (networkClient.metaThumbnail !== "")
                    source: networkClient.metaThumbnail
                    fillMode: Image.PreserveAspectCrop
                }

                IconSvg { // no artwork with this track
                    anchors.centerIn: parent
                    width: 40
                    height: 40

                    visible: (networkClient.metaThumbnail === "")
                    color: Theme.colorSubText
                    source: "qrc:/IconLibrary/material-symbols/media/album.svg"
                }

                Rectangle { // playback status
                    anchors.left: parent.left
                    anchors.leftMargin: -4
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: -4
                    width: 24
                    height: 24
                    radius: width / 2

                    visible: (networkClient.playbackStatus !== "")
                    color: (networkClient.playbackStatus === "Playing") ? Theme.colorPrimary : Theme.colorComponentBorder

                    IconSvg {
                        anchors.centerIn: parent
                        width: 16
                        height: 16

                        color: "white"
                        source: (networkClient.playbackStatus === "Playing") ?
                                    "qrc:/IconLibrary/material-symbols/media/play_arrow-fill.svg" :
                                    "qrc:/IconLibrary/material-symbols/media/pause-fill.svg"
                    }
                }
            }

            ////////

            Column {
                anchors.left: mediaThumbnail.right
                anchors.leftMargin: 12
                anchors.right: parent.right
                anchors.rightMargin: 12
                anchors.verticalCenter: parent.verticalCenter
                spacing: 4

                Text {
                    width: parent.width

                    text: networkClient.metaTitle
                    textFormat: Text.PlainText
                    color: Theme.colorText
                    font.pixelSize: 18
                    elide: Text.ElideRight
                }
                Text {
                    width: parent.width

                    text: (networkClient.metaAlbum !== "") ? networkClient.metaAlbum : networkClient.metaArtist
                    textFormat: Text.PlainText
                    color: Theme.colorSubText
                    font.pixelSize: 16
                    elide: Text.ElideRight
                }

                ProgressBarThemed { // playback position
                    width: parent.width
                    height: 12

                    from: 0
                    to: 100
                    value: Math.max(0, networkClient.position)
                }
            }

            ////////
        }
    }

    ////////////////

    Rectangle { // CLAUDE CODE
        id: claudeWidget

        width: singleColumn ? parent.width : 480
        height: visible ? (singleColumn ? 96 : 96) : 0
        radius: 4

        visible: (networkClient !== null && networkClient.connected && networkClient.claudeState !== ClaudeMonitor.CaptureNone)

        color: Theme.colorForeground
        border.width: 2
        border.color: singleColumn ? "transparent" : Theme.colorSeparator

        readonly property bool relayed: (networkClient !== null && networkClient !== undefined)
        readonly property int claudeState: relayed ? networkClient.claudeState : ClaudeMonitor.CaptureNone

        readonly property real fiveHourPercent: relayed ? networkClient.claudeFiveHourPercent : -1
        readonly property int fiveHourRemaining: relayed ? networkClient.claudeFiveHourRemaining : -1
        readonly property real sevenDayPercent: relayed ? networkClient.claudeSevenDayPercent : -1

        readonly property int probeState: relayed ? networkClient.claudeProbeState : ClaudeMonitor.ProbeNone

        readonly property bool canProbe: relayed && networkClient.claudeProbeAvailable &&
                                         probeState !== ClaudeMonitor.ProbeRunning &&
                                         claudeState !== ClaudeMonitor.CaptureLive

        function resetString(seconds) {
            if (seconds < 0) return ""

            var d = Math.floor(seconds / 86400)
            var h = Math.floor((seconds % 86400) / 3600)
            var m = Math.floor((seconds % 3600) / 60)

            // Time left before a window resets, as "4d 6h" / "2h 14m" / "12m"
            if (d > 0) return d + qsTr("d", "short for days") + " " + h + qsTr("h", "short for hours")
            if (h > 0) return h + qsTr("h", "short for hours") + " " + m + qsTr("m", "short for minutes")
            return m + qsTr("m", "short for minutes")
        }

        function usageColor(percent) {
            if (percent < 0) return Theme.colorSeparator
            if (percent >= 95) return Theme.colorRed
            if (percent >= 75) return Theme.colorOrange
            return Theme.colorGreen
        }

        ////////

        MouseArea {
            anchors.fill: parent
            enabled: claudeWidget.canProbe

            onPressAndHold: {
                UtilsOS.hapticFeedback()
                networkClient.claude_probe()
            }
        }

        Column {
            anchors.left: parent.left
            anchors.leftMargin: 12
            anchors.right: parent.right
            anchors.rightMargin: 12
            anchors.verticalCenter: parent.verticalCenter
            spacing: 8

            Item { // header
                anchors.left: parent.left
                anchors.right: parent.right
                height: 18

                Text {
                    id: claudeMobileTitle
                    anchors.verticalCenter: parent.verticalCenter

                    text: "Claude Code"
                    textFormat: Text.PlainText
                    color: Theme.colorText
                    font.pixelSize: Theme.fontSizeContent
                }

                Text {
                    anchors.left: claudeMobileTitle.right
                    anchors.leftMargin: 8
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter

                    text: {
                        if (claudeWidget.probeState === ClaudeMonitor.ProbeRunning) return qsTr("probing…")
                        if (claudeWidget.canProbe) {
                            if (claudeWidget.probeState === ClaudeMonitor.ProbeAuthExpired) return qsTr("log in to Claude Code again")
                            if (claudeWidget.probeState === ClaudeMonitor.ProbeFailed) return qsTr("probe failed, hold to retry")
                            return qsTr("stale, hold to probe")
                        }
                        if (claudeWidget.claudeState === ClaudeMonitor.CaptureStale) return qsTr("stale")
                        return claudeWidget.resetString(claudeWidget.fiveHourRemaining)
                    }
                    textFormat: Text.PlainText
                    color: {
                        if (claudeWidget.probeState === ClaudeMonitor.ProbeRunning) return Theme.colorSubText
                        if (claudeWidget.claudeState === ClaudeMonitor.CaptureStale) return Theme.colorOrange
                        return Theme.colorSubText
                    }
                    font.pixelSize: Theme.fontSizeContent
                    horizontalAlignment: Text.AlignRight
                    elide: Text.ElideRight
                }
            }

            SliderValueSolid { // 5 hour session window
                anchors.left: parent.left
                anchors.right: parent.right
                height: 18
                hhh: 16

                from: 0
                to: 100
                unit: "%"
                value: Math.max(0, claudeWidget.fiveHourPercent)
                enabled: false

                //legend: "5 hour session window"
                colorForeground: claudeWidget.usageColor(claudeWidget.fiveHourPercent)
                colorForegroundDisabled: colorForeground
            }

            SliderValueSolid { // 7 day window
                anchors.left: parent.left
                anchors.right: parent.right
                height: 18
                hhh: 16

                from: 0
                to: 100
                unit: "%"
                value: Math.max(0, claudeWidget.sevenDayPercent)
                enabled: false

                //legend: "7 day window"
                colorForeground: claudeWidget.usageColor(claudeWidget.sevenDayPercent)
                colorForegroundDisabled: colorForeground
            }
        }

        ////////
    }

    ////////////////

    Rectangle { // VIRTUAL INPUTS
        width: singleColumn ? parent.width : 480
        height: visible ? (singleColumn ? 96 : 96) : 0
        radius: 4

        visible: (isMobile && networkClient.connected) || (SettingsManager.fakeIt)

        color: Theme.colorForeground
        border.width: 2
        border.color: singleColumn ? "transparent" : Theme.colorSeparator

        ////////

        Column {
            anchors.left: parent.left
            anchors.leftMargin: 12
            anchors.verticalCenter: parent.verticalCenter

            Text {
                text: "Virtual Controls"
                textFormat: Text.PlainText
                color: Theme.colorText
                font.pixelSize: 20
                verticalAlignment: Text.AlignVCenter
            }
            Text {
                text: "click to open"
                textFormat: Text.PlainText
                color: Theme.colorSubText
                font.pixelSize: 18
                verticalAlignment: Text.AlignVCenter
            }
        }

        IconSvg {
            width: 48
            height: 48
            anchors.right: parent.right
            anchors.rightMargin: 48
            anchors.verticalCenter: parent.verticalCenter

            color: Theme.colorHighContrast
            source: "qrc:/IconLibrary/material-icons/duotone/devices.svg"
        }
        IconSvg {
            width: 32
            height: 32
            anchors.right: parent.right
            anchors.rightMargin: singleColumn ? 4 : 12
            anchors.verticalCenter: parent.verticalCenter

            color: Theme.colorHighContrast
            source: "qrc:/IconLibrary/material-symbols/chevron_right.svg"
        }

        MouseArea {
            anchors.fill: parent
            onClicked: screenVirtualInputs.loadScreen()
        }

        ////////
    }

    ////////////////

    Rectangle { // BONGO CAT
        width: singleColumn ? parent.width : 480
        height: visible ? 96 : 0
        radius: 4

        visible: (networkClient !== null && networkClient.connected && networkClient.typingAvailable)

        color: Theme.colorForeground
        border.width: 2
        border.color: singleColumn ? "transparent" : Theme.colorSeparator

        BongoCat {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: height * implicitWidth / implicitHeight

            typing: networkClient.typing
            face: SettingsManager.bongoFace
            hat: SettingsManager.bongoHat
            pawInterval: SettingsManager.bongoPawInterval
        }

        MouseArea {
            anchors.fill: parent
            onClicked: screenBongoCat.loadScreen()
        }
    }

    ////////////////
}
