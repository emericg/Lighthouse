import QtQuick
import QtQuick.Controls

import ComponentLibrary
import Lighthouse

Item {
    required property var modelData

    width: 512
    height: Theme.componentHeight

    Rectangle { // connection indicator
        id: dot_client
        anchors.left: parent.left
        anchors.leftMargin: contentColumn.padIcon + 8
        anchors.verticalCenter: parent.verticalCenter
        width: 8
        height: 8
        radius: 8
        color: modelData.connected ? Theme.colorGreen : Theme.colorSubText
        opacity: modelData.connected ? 1 : 0.5
    }

    IconSvg { // secure (authenticated) indicator
        id: lock_secure
        anchors.right: switch_client.left
        anchors.rightMargin: visible ? Theme.componentMargin : 0
        anchors.verticalCenter: parent.verticalCenter
        width: visible ? 20 : 0
        height: 20

        visible: modelData.secure
        color: Theme.colorGreen
        source: "qrc:/IconLibrary/material-symbols/lock-fill.svg"
    }

    Column {
        anchors.left: parent.left
        anchors.leftMargin: contentColumn.padText
        anchors.right: lock_secure.left
        anchors.rightMargin: Theme.componentMargin
        anchors.verticalCenter: parent.verticalCenter
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
            text: modelData.connected ? qsTr("Connected")
                                      : (modelData.lastSeen && !isNaN(modelData.lastSeen.getTime())
                                            ? qsTr("Last seen %1").arg(Qt.formatDateTime(modelData.lastSeen, "yyyy-MM-dd HH:mm"))
                                            : qsTr("Never connected"))
            textFormat: Text.PlainText
            elide: Text.ElideRight
            font.pixelSize: Theme.fontSizeContentSmall
            color: Theme.colorSubText
        }
    }

    SwitchThemed {
        id: switch_client
        anchors.right: button_revoke.left
        anchors.rightMargin: Theme.componentMargin
        anchors.verticalCenter: parent.verticalCenter
        z: 1

        checked: modelData.enabled
        onClicked: modelData.enabled = checked
    }

    RoundButtonSunken {
        id: button_revoke
        anchors.right: parent.right
        anchors.rightMargin: 8
        anchors.verticalCenter: parent.verticalCenter

        source: "qrc:/IconLibrary/material-symbols/delete.svg"
        onClicked: networkServer.forgetClient(modelData)
    }
}
