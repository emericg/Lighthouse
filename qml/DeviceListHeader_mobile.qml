import QtQuick
import QtQuick.Controls

import ComponentLibrary

Column {
    anchors.left: parent.left
    anchors.right: parent.right

    spacing: 12

    ////////////////

    Rectangle { // NETWORK
        width: singleColumn ? parent.width : 480
        height: visible ? (singleColumn ? 112 : 128) : 0
        radius: 4

        visible: (isMobile && networkClient.connected) || (SettingsManager.fakeIt)

        color: Theme.colorDeviceWidget
        border.width: 2
        border.color: singleColumn ? "transparent" : Theme.colorSeparator

        Rectangle {
            anchors.fill: parent
            anchors.leftMargin: -8
            anchors.rightMargin: -8
            color: Theme.colorForeground
        }

        Row {
            anchors.centerIn: parent
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
    }

    ////////////////

    Rectangle { // CLAUDE CODE
        id: claudeWidgetMobile
        width: singleColumn ? parent.width : 480
        height: visible ? (singleColumn ? 96 : 96) : 0
        radius: 4

        visible: (isMobile && networkClient.connected &&
                  networkClient.claudeState !== ClaudeMonitor.CaptureNone)

        color: Theme.colorForeground
        border.width: 2
        border.color: singleColumn ? "transparent" : Theme.colorSeparator

        readonly property bool relayed: (networkClient !== null && networkClient !== undefined)
        readonly property int claudeState: relayed ? networkClient.claudeState
                                                   : ClaudeMonitor.CaptureNone

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

        function usageString(percent) {
            return (percent >= 0) ? Math.round(percent) + "%" : "–"
        }

        Column {
            anchors.left: parent.left
            anchors.leftMargin: 12
            anchors.right: parent.right
            anchors.rightMargin: 12
            anchors.verticalCenter: parent.verticalCenter
            spacing: 6

            Item { // header
                anchors.left: parent.left
                anchors.right: parent.right
                height: 20

                Text {
                    id: claudeMobileTitle
                    anchors.verticalCenter: parent.verticalCenter

                    text: "Claude Code"
                    textFormat: Text.PlainText
                    color: Theme.colorText
                    font.pixelSize: 16
                }

                Text {
                    anchors.left: claudeMobileTitle.right
                    anchors.leftMargin: 8
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter

                    text: {
                        if (claudeWidgetMobile.claudeState === ClaudeMonitor.CaptureStale) return qsTr("stale")
                        return claudeWidgetMobile.resetString(claudeWidgetMobile.fiveHourRemaining)
                    }
                    textFormat: Text.PlainText
                    color: (claudeWidgetMobile.claudeState === ClaudeMonitor.CaptureStale)
                           ? Theme.colorOrange : Theme.colorSubText
                    font.pixelSize: 13
                    horizontalAlignment: Text.AlignRight
                    elide: Text.ElideRight
                }
            }

            Item { // 5 hour session window
                anchors.left: parent.left
                anchors.right: parent.right
                height: 18

                Text {
                    id: claudeMobileSessionLabel
                    width: 54
                    anchors.verticalCenter: parent.verticalCenter

                    text: qsTr("Session")
                    textFormat: Text.PlainText
                    color: Theme.colorSubText
                    font.pixelSize: 13
                }

                Text {
                    id: claudeMobileSessionValue
                    width: 36
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter

                    text: claudeWidgetMobile.usageString(claudeWidgetMobile.fiveHourPercent)
                    textFormat: Text.PlainText
                    color: Theme.colorSubText
                    font.pixelSize: 13
                    horizontalAlignment: Text.AlignRight
                }

                ProgressBarThemed {
                    anchors.left: claudeMobileSessionLabel.right
                    anchors.leftMargin: 8
                    anchors.right: claudeMobileSessionValue.left
                    anchors.rightMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    height: 8

                    from: 0
                    to: 100
                    value: Math.max(0, claudeWidgetMobile.fiveHourPercent)
                    colorForeground: claudeWidgetMobile.usageColor(claudeWidgetMobile.fiveHourPercent)
                }
            }

            Item { // 7 day window
                anchors.left: parent.left
                anchors.right: parent.right
                height: 18

                Text {
                    id: claudeMobileWeeklyLabel
                    width: 54
                    anchors.verticalCenter: parent.verticalCenter

                    text: qsTr("Weekly")
                    textFormat: Text.PlainText
                    color: Theme.colorSubText
                    font.pixelSize: 13
                }

                Text {
                    id: claudeMobileWeeklyValue
                    width: 36
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter

                    text: claudeWidgetMobile.usageString(claudeWidgetMobile.sevenDayPercent)
                    textFormat: Text.PlainText
                    color: Theme.colorSubText
                    font.pixelSize: 13
                    horizontalAlignment: Text.AlignRight
                }

                ProgressBarThemed {
                    anchors.left: claudeMobileWeeklyLabel.right
                    anchors.leftMargin: 8
                    anchors.right: claudeMobileWeeklyValue.left
                    anchors.rightMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    height: 8

                    from: 0
                    to: 100
                    value: Math.max(0, claudeWidgetMobile.sevenDayPercent)
                    colorForeground: claudeWidgetMobile.usageColor(claudeWidgetMobile.sevenDayPercent)
                }
            }
        }
    }

    ////////////////
}
