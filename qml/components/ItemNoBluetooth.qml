import QtQuick

import ThemeEngine

Item {
    id: itemNoBluetooth

    Rectangle {
        anchors.centerIn: parent
        anchors.verticalCenterOffset: -32

        width: appWindow.singleColumn ? (parent.width*0.5) : (parent.height*0.4)
        height: width
        radius: width
        color: Theme.colorForeground

        signal clicked()

        IconSvg {
            anchors.centerIn: parent
            width: parent.width*0.8
            height: width

            source: "qrc:/assets/icons/material-icons/outlined/bluetooth_disabled.svg"
            fillMode: Image.PreserveAspectFit
            color: Theme.colorSubText
            opacity: 0.9
            smooth: true
        }

        Text {
            anchors.top: parent.bottom
            anchors.topMargin: 24
            anchors.horizontalCenter: parent.horizontalCenter

            text: qsTr("Bluetooth is disabled...")
            textFormat: Text.PlainText
            font.pixelSize: Theme.fontSizeContentBig
            color: Theme.colorText

            ButtonWireframe {
                anchors.top: parent.bottom
                anchors.topMargin: 12
                anchors.horizontalCenter: parent.horizontalCenter

                fullColor: true
                text: (Qt.platform.os === "android") ? qsTr("Enable") : qsTr("Retry")
                onClicked: (Qt.platform.os === "android") ? deviceManager.enableBluetooth() : deviceManager.checkBluetooth()
            }
        }
    }
}
