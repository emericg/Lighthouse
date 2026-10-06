import QtQuick
import QtQuick.Controls

import ComponentLibrary

Column {
    anchors.left: parent.left
    anchors.right: parent.right

    spacing: 20

    visible: isDesktop

    ListTitle { ////////////////////////////////////////////////////////////////
        anchors.leftMargin: devicesView.listMargin
        anchors.rightMargin: devicesView.listMargin
        text: qsTr("Local control(s)")
    }

    Grid {
        anchors.left: parent.left
        anchors.leftMargin: 8
        anchors.right: parent.right
        anchors.rightMargin: 8

        columns: singleColumn ? 1 : 4
        spacing: 12

        ////////////////

        Rectangle { // MPRIS
            width: singleColumn ? parent.width : 520
            height: 128
            radius: 4

            visible: mediaControls && mediaControls.available

            color: Theme.colorDeviceWidget
            border.width: 2
            border.color: singleColumn ? "transparent" : Theme.colorSeparator

            Column {
                anchors.left: parent.left
                anchors.leftMargin: 16
                anchors.right: thumbnail.left
                anchors.rightMargin: 16
                anchors.verticalCenter: parent.verticalCenter
                spacing: 6

                Text {
                    anchors.left: parent.left
                    anchors.right: parent.right

                    text: mediaControls.playerName + " / " + mediaControls.playbackStatus
                    font.pixelSize: Theme.fontSizeContentSmall
                    color: Theme.colorSubText
                    elide: Text.ElideRight
                }

                Text {
                    anchors.left: parent.left
                    anchors.right: parent.right

                    visible: (text.length > 1)

                    text: mediaControls.metaTitle + " " + mediaControls.metaAlbum
                    font.pixelSize: Theme.fontSizeContentBig
                    color: Theme.colorText
                    elide: Text.ElideRight
                }

                MediaButtonRow {
                    btnSize: 36
                    //visible: mediaControls.canControl
                    onMediaPrevious: mediaControls.media_prev()
                    onMediaPlayPause: mediaControls.media_playpause()
                    onMediaNext: mediaControls.media_next()
                }

                SliderThemed {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    visible: (mediaControls.position > 0 && mediaControls.metaDuration > 0)

                    from: 0
                    to: mediaControls.metaDuration
                    value: mediaControls.position
                }
            }

            IconSvg {
                anchors.top: parent.top
                anchors.topMargin: 8
                anchors.right: parent.right
                anchors.rightMargin: 8
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 8
                width: height

                source: {
                    if (mediaControls.playbackStatus === "paused")
                        return "qrc:/IconLibrary/material-symbols/media/slideshow.svg"
                    else
                        return "qrc:/IconLibrary/material-symbols/media/slideshow.svg"
                }
                color: Theme.colorSeparator
                opacity: 0.5
            }

            Image {
                id: thumbnail
                anchors.top: parent.top
                anchors.topMargin: 8
                anchors.right: parent.right
                anchors.rightMargin: 8
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 8
                width: height

                source: mediaControls.metaThumbnail
                sourceSize: Qt.size(width, height)
                fillMode: Image.PreserveAspectCrop
            }
        }

        ////////////////

        Rectangle { // KEYBOARD
            width: singleColumn ? parent.width : 420
            height: 128
            radius: 4

            color: Theme.colorDeviceWidget
            border.width: 2
            border.color: singleColumn ? "transparent" : Theme.colorSeparator

            Row {
                anchors.centerIn: parent
                spacing: 6

                MediaButtonRow {
                    anchors.verticalCenter: parent.verticalCenter
                    btnSize: 48
                    onMediaPrevious: localControls.keyboard_media_prev()
                    onMediaPlayPause: localControls.keyboard_media_playpause()
                    onMediaNext: localControls.keyboard_media_next()
                }

                IconSvg {
                    anchors.verticalCenter: parent.verticalCenter

                    width: 64
                    height: 64

                    color: Theme.colorSeparator
                    opacity: 0.5
                    source: "qrc:/assets/icons/keyboard-variant.svg"
                }

                VolumeButtonRow {
                    anchors.verticalCenter: parent.verticalCenter
                    btnSize: 48
                    onVolumeMute: localControls.keyboard_volume_mute()
                    onVolumeDown: localControls.keyboard_volume_down()
                    onVolumeUp: localControls.keyboard_volume_up()
                }
            }
        }

        ////////////////

        Rectangle { // VIRTUAL INPUTS
            width: singleColumn ? parent.width : 420
            height: visible ? 128 : 0
            radius: 4

            color: Theme.colorDeviceWidget
            border.width: 2
            border.color: singleColumn ? "transparent" : Theme.colorSeparator

            Column {
                anchors.left: parent.left
                anchors.leftMargin: 16
                anchors.verticalCenter: parent.verticalCenter

                Text {
                    text: "Virtual Controls"
                    textFormat: Text.PlainText
                    color: Theme.colorText
                    font.pixelSize: 22
                    verticalAlignment: Text.AlignVCenter
                }
                Text {
                    text: "click to open"
                    textFormat: Text.PlainText
                    color: Theme.colorSubText
                    font.pixelSize: 20
                    verticalAlignment: Text.AlignVCenter
                }
            }

            IconSvg {
                width: 80
                height: 80
                anchors.right: parent.right
                anchors.rightMargin: 16
                anchors.verticalCenter: parent.verticalCenter

                opacity: 0.66
                color: Theme.colorSeparator
                source: "qrc:/IconLibrary/material-icons/duotone/devices.svg"
            }

            MouseArea {
                anchors.fill: parent
                onClicked: screenVirtualInputs.loadScreen()
            }
        }

        ////////////////

        Rectangle { // CLAUDE CODE
            id: claudeWidget
            width: singleColumn ? parent.width : 420
            height: visible ? (ClaudeMonitor.cacheValid ? 164 : 128) : 0
            radius: 4

            color: Theme.colorDeviceWidget
            border.width: 2
            border.color: singleColumn ? "transparent" : Theme.colorSeparator

            visible: isDesktop && ClaudeMonitor.enabled && (ClaudeMonitor.available || ClaudeMonitor.probeAvailable)

            // Probing costs a little quota, so it is only offered when there is nothing fresh to show
            readonly property bool canProbe: ClaudeMonitor.probeAvailable &&
                                             ClaudeMonitor.probeState !== ClaudeMonitor.ProbeRunning &&
                                             (!ClaudeMonitor.available || ClaudeMonitor.stale)
            readonly property bool canInstallHook: ClaudeMonitor.enabled &&
                                                   !ClaudeMonitor.hookInstalled && !ClaudeMonitor.hookForeign

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

            function usageString(valid, percent) {
                return valid ? Math.round(percent) + "%" : "–"
            }

            function cacheString(seconds) {
                if (seconds < 0) return ""
                if (seconds === 0) return qsTr("cold", "prompt cache expired")

                // Time left before the prompt cache goes cold, as "1:02:05" / "4:32"
                var h = Math.floor(seconds / 3600)
                var m = Math.floor((seconds % 3600) / 60)
                var s = seconds % 60
                var mmss = (h > 0 && m < 10 ? "0" : "") + m + ":" + (s < 10 ? "0" : "") + s
                return (h > 0) ? h + ":" + mmss : mmss
            }

            function cacheColor(seconds, ttl) {
                if (seconds <= 0) return Theme.colorSeparator
                if (ttl > 0 && seconds < ttl * 0.2) return Theme.colorOrange
                return Theme.colorGreen
            }

            Column {
                anchors.left: parent.left
                anchors.leftMargin: 16
                anchors.right: parent.right
                anchors.rightMargin: 16
                anchors.verticalCenter: parent.verticalCenter
                spacing: 8

                Item { // header
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 22

                    Text {
                        id: claudeTitle
                        anchors.verticalCenter: parent.verticalCenter

                        text: "Claude Code"
                        textFormat: Text.PlainText
                        color: Theme.colorText
                        font.pixelSize: 18
                    }

                    Text {
                        anchors.left: claudeTitle.right
                        anchors.leftMargin: 8
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter

                        text: {
                            if (ClaudeMonitor.probeState === ClaudeMonitor.ProbeRunning) return qsTr("probing…")
                            if (ClaudeMonitor.probeState === ClaudeMonitor.ProbeAuthExpired && claudeWidget.canProbe)
                                return qsTr("log in to Claude Code again, then retry")
                            if (ClaudeMonitor.probeState === ClaudeMonitor.ProbeFailed && claudeWidget.canProbe)
                                return qsTr("probe failed, retry")
                            if (claudeWidget.canInstallHook) return qsTr("set up the statusline hook")
                            if (claudeWidget.canProbe) return ClaudeMonitor.available ? qsTr("stale, probe now") : qsTr("probe now")
                            if (ClaudeMonitor.hookForeign && !ClaudeMonitor.available) return qsTr("another statusline is configured")
                            if (!ClaudeMonitor.available) return qsTr("no statusline capture")
                            if (ClaudeMonitor.stale) return qsTr("stale")
                            if (ClaudeMonitor.source === ClaudeMonitor.SourceProbe) return qsTr("probed")
                            return ClaudeMonitor.modelName
                        }
                        textFormat: Text.PlainText
                        color: {
                            if (ClaudeMonitor.probeState === ClaudeMonitor.ProbeRunning) return Theme.colorSubText
                            if (ClaudeMonitor.probeState === ClaudeMonitor.ProbeAuthExpired && claudeWidget.canProbe) return Theme.colorOrange
                            if (claudeWidget.canInstallHook || claudeWidget.canProbe) return Theme.colorPrimary
                            return ClaudeMonitor.stale ? Theme.colorOrange : Theme.colorSubText
                        }
                        font.pixelSize: 13
                        horizontalAlignment: Text.AlignRight
                        elide: Text.ElideRight
                    }

                    MouseArea { // click to install the hook, or to probe
                        anchors.fill: parent
                        enabled: claudeWidget.canInstallHook || claudeWidget.canProbe
                        hoverEnabled: enabled
                        cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor

                        ToolTip.visible: containsMouse && !claudeWidget.canInstallHook
                        ToolTip.delay: 500
                        ToolTip.text: qsTr("Sends one small request through Claude Code to read the plan limits.\n" +
                                           "It uses a little of the quota, and starts a 5 hour window if none is active.")

                        onClicked: {
                            if (claudeWidget.canInstallHook) ClaudeMonitor.installStatuslineHook()
                            else ClaudeMonitor.probe()
                        }
                    }
                }

                Item { // 5 hour session window
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 28

                    Column {
                        id: claudeSessionLabel
                        width: 62
                        anchors.verticalCenter: parent.verticalCenter

                        Text {
                            text: qsTr("Session")
                            textFormat: Text.PlainText
                            color: Theme.colorText
                            font.pixelSize: 14
                        }
                        Text {
                            text: claudeWidget.resetString(ClaudeMonitor.fiveHourRemaining)
                            textFormat: Text.PlainText
                            color: Theme.colorSubText
                            font.pixelSize: 11
                        }
                    }

                    SliderValueSolid { // 5 hour session window
                        anchors.left: claudeSessionLabel.right
                        anchors.leftMargin: 8
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        height: 20
                        hhh: 16

                        from: 0
                        to: 100
                        unit: "%"
                        value: Math.max(0, ClaudeMonitor.fiveHourPercent)
                        enabled: false

                        //legend: "5 hour session window"
                        colorForeground: claudeWidget.usageColor(ClaudeMonitor.fiveHourPercent)
                        colorForegroundDisabled: colorForeground
                    }
                }

                Item { // 7 day window
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 28

                    Column {
                        id: claudeWeeklyLabel
                        width: 62
                        anchors.verticalCenter: parent.verticalCenter

                        Text {
                            text: qsTr("Weekly")
                            textFormat: Text.PlainText
                            color: Theme.colorText
                            font.pixelSize: 14
                        }
                        Text {
                            text: claudeWidget.resetString(ClaudeMonitor.sevenDayRemaining)
                            textFormat: Text.PlainText
                            color: Theme.colorSubText
                            font.pixelSize: 11
                        }
                    }

                    SliderValueSolid { // 7 day window
                        anchors.left: claudeWeeklyLabel.right
                        anchors.leftMargin: 8
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        height: 20
                        hhh: 16

                        from: 0
                        to: 100
                        unit: "%"
                        value: Math.max(0, ClaudeMonitor.sevenDayPercent)
                        enabled: false

                        //legend: "7 day window"
                        colorForeground: claudeWidget.usageColor(ClaudeMonitor.sevenDayPercent)
                        colorForegroundDisabled: colorForeground
                    }
                }

                Item { // prompt cache of the captured session
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 28
                    visible: ClaudeMonitor.cacheValid

                    Column {
                        id: claudeCacheLabel
                        width: 62
                        anchors.verticalCenter: parent.verticalCenter

                        Text {
                            text: qsTr("Cache")
                            textFormat: Text.PlainText
                            color: Theme.colorText
                            font.pixelSize: 14
                        }
                        Text {
                            text: claudeWidget.cacheString(ClaudeMonitor.cacheRemaining)
                            textFormat: Text.PlainText
                            color: Theme.colorSubText
                            font.pixelSize: 11
                        }
                    }

                    SliderValueSolid { // time left before the cache goes cold
                        anchors.left: claudeCacheLabel.right
                        anchors.leftMargin: 8
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        height: 20
                        hhh: 16

                        from: 0
                        to: Math.max(1, ClaudeMonitor.cacheTtl, ClaudeMonitor.cacheRemaining)
                        value: Math.max(0, ClaudeMonitor.cacheRemaining)
                        showvalue: false
                        enabled: false

                        colorForeground: claudeWidget.cacheColor(ClaudeMonitor.cacheRemaining, ClaudeMonitor.cacheTtl)
                        colorForegroundDisabled: colorForeground
                    }
                }
            }
        }

        ////////////////
    }
}
