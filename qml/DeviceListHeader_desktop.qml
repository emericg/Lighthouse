import QtQuick
import QtQuick.Controls

import ComponentLibrary

Column {
    anchors.left: parent.left
    anchors.right: parent.right

    spacing: 20

    ListTitle { ////////////////////////////////////////////////////////////////
        anchors.leftMargin: devicesView.listMargin
        anchors.rightMargin: devicesView.listMargin
        visible: isDesktop
        text: qsTr("Local control(s)")
    }

    Grid {
        anchors.left: parent.left
        anchors.leftMargin: 8
        anchors.right: parent.right
        anchors.rightMargin: 8

        visible: isDesktop
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

            visible: isDesktop
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
            height: visible ? 128 : 0
            radius: 4

            visible: isDesktop && ClaudeMonitor.enabled && ClaudeMonitor.available
            color: Theme.colorDeviceWidget
            border.width: 2
            border.color: singleColumn ? "transparent" : Theme.colorSeparator

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
                            if (ClaudeMonitor.hookForeign) return qsTr("another statusline is configured")
                            if (!ClaudeMonitor.hookInstalled) return qsTr("set up the statusline hook")
                            if (!ClaudeMonitor.available) return qsTr("no statusline capture")
                            if (ClaudeMonitor.stale) return qsTr("stale")
                            return ClaudeMonitor.modelName
                        }
                        textFormat: Text.PlainText
                        color: {
                            if (ClaudeMonitor.hookForeign) return Theme.colorSubText
                            if (!ClaudeMonitor.hookInstalled) return Theme.colorPrimary
                            return ClaudeMonitor.stale ? Theme.colorOrange : Theme.colorSubText
                        }
                        font.pixelSize: 13
                        horizontalAlignment: Text.AlignRight
                        elide: Text.ElideRight
                    }

                    MouseArea { // click to install hook
                        anchors.fill: parent
                        enabled: ClaudeMonitor.enabled && !ClaudeMonitor.hookInstalled && !ClaudeMonitor.hookForeign
                        cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                        onClicked: ClaudeMonitor.installStatuslineHook()
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

                    Text {
                        id: claudeSessionValue
                        width: 40
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter

                        text: claudeWidget.usageString(ClaudeMonitor.fiveHourValid,
                                                       ClaudeMonitor.fiveHourPercent)
                        textFormat: Text.PlainText
                        color: Theme.colorSubText
                        font.pixelSize: 14
                        horizontalAlignment: Text.AlignRight
                    }

                    ProgressBarThemed {
                        anchors.left: claudeSessionLabel.right
                        anchors.leftMargin: 8
                        anchors.right: claudeSessionValue.left
                        anchors.rightMargin: 8
                        anchors.verticalCenter: parent.verticalCenter
                        height: 10

                        from: 0
                        to: 100
                        value: Math.max(0, ClaudeMonitor.fiveHourPercent)
                        colorForeground: claudeWidget.usageColor(ClaudeMonitor.fiveHourPercent)
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

                    Text {
                        id: claudeWeeklyValue
                        width: 40
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter

                        text: claudeWidget.usageString(ClaudeMonitor.sevenDayValid,
                                                       ClaudeMonitor.sevenDayPercent)
                        textFormat: Text.PlainText
                        color: Theme.colorSubText
                        font.pixelSize: 14
                        horizontalAlignment: Text.AlignRight
                    }

                    ProgressBarThemed {
                        anchors.left: claudeWeeklyLabel.right
                        anchors.leftMargin: 8
                        anchors.right: claudeWeeklyValue.left
                        anchors.rightMargin: 8
                        anchors.verticalCenter: parent.verticalCenter
                        height: 10

                        from: 0
                        to: 100
                        value: Math.max(0, ClaudeMonitor.sevenDayPercent)
                        colorForeground: claudeWidget.usageColor(ClaudeMonitor.sevenDayPercent)
                    }
                }
            }
        }

        ////////////////
    }
}
