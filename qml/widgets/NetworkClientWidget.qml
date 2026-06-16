import QtQuick
import QtQuick.Layouts

import ComponentLibrary
import Lighthouse

Item {
    required property var modelData

    width: 512
    height: Theme.componentHeightXL

    ////////

    Rectangle { // background
        anchors.fill: parent
        color: Theme.colorForeground
        opacity: 0.33
    }

    ////////

    RowLayout {
        anchors.left: parent.left
        anchors.leftMargin: Theme.componentMargin
        anchors.right: parent.right
        anchors.rightMargin: Theme.componentMargin
        anchors.verticalCenter: parent.verticalCenter
        spacing: Theme.componentMargin

        ////

        Rectangle { // connection indicator
            Layout.alignment: Qt.AlignVCenter
            width: 8
            height: 8
            radius: 8
            color: modelData.connected ? Theme.colorGreen : Theme.colorSubText
            opacity: modelData.connected ? 1 : 0.5
        }

        ////

        Column {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            spacing: 0

            Text {
                width: parent.width
                text: modelData.name
                textFormat: Text.PlainText
                elide: Text.ElideRight
                font.pixelSize: Theme.fontSizeContent
                color: Theme.colorText
            }
            Text {
                width: parent.width
                text: {
                    if (modelData.connected) return qsTr("Connected")
                    if (modelData.lastSeen && !isNaN(modelData.lastSeen.getTime()))
                        return qsTr("Last seen %1").arg(Qt.formatDateTime(modelData.lastSeen, "yyyy-MM-dd HH:mm"))
                    return qsTr("Never connected")
                }
                textFormat: Text.PlainText
                elide: Text.ElideRight
                font.pixelSize: Theme.fontSizeContentSmall
                color: Theme.colorSubText
            }
        }

        ////

        Row { // buttons
            Layout.alignment: Qt.AlignVCenter
            spacing: Theme.componentMargin
            z: 1

            IconSvg { // secure (authenticated) indicator
                anchors.verticalCenter: parent.verticalCenter
                width: 20
                height: 20

                visible: modelData.connected
                color: modelData.secure ? Theme.colorGreen : Theme.colorGrey
                source: modelData.secure ? "qrc:/IconLibrary/material-symbols/lock.svg"
                                         : "qrc:/IconLibrary/material-symbols/lock_open.svg"
            }

            SwitchThemed { // enable client
                anchors.verticalCenter: parent.verticalCenter

                checked: modelData.enabled
                onClicked: modelData.enabled = checked
            }

            RoundButtonSunken { // revoke client
                anchors.verticalCenter: parent.verticalCenter

                source: "qrc:/IconLibrary/material-symbols/delete.svg"
                onClicked: networkServer.forgetClient(modelData)
            }
        }

        ////
    }

    ////////
}
