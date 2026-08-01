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

                    onVolumeMute: networkControls.volume_mute()
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
                clip: true

                color: Theme.colorSeparator

                Image {
                    anchors.fill: parent

                    visible: (networkClient.metaThumbnail !== "")
                    source: networkClient.metaThumbnail
                    fillMode: Image.PreserveAspectCrop
                    asynchronous: true
                }

                IconSvg { // no artwork with this track
                    anchors.centerIn: parent
                    width: 40
                    height: 40

                    visible: (networkClient.metaThumbnail === "")
                    color: Theme.colorSubText
                    source: "qrc:/IconLibrary/material-symbols/media/album.svg"
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
                        if (claudeWidget.claudeState === ClaudeMonitor.CaptureStale) return qsTr("stale")
                        return claudeWidget.resetString(claudeWidget.fiveHourRemaining)
                    }
                    textFormat: Text.PlainText
                    color: (claudeWidget.claudeState === ClaudeMonitor.CaptureStale)
                           ? Theme.colorOrange : Theme.colorSubText
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

                //legend: "5 hour session window"
                colorForeground: claudeWidget.usageColor(claudeWidget.fiveHourPercent)
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

                //legend: "7 day window"
                colorForeground: claudeWidget.usageColor(claudeWidget.sevenDayPercent)
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
}
