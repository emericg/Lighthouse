import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects

import ThemeEngine 1.0
import LocalActions 1.0
import "qrc:/js/UtilsActions.js" as UtilsActions

Rectangle {
    id: remoteButton
    width: pw
    height: ph/2
    radius: 12

    color: Theme.colorBackground
    border.width: 2
    border.color: Theme.colorSeparator

    property var currentButton: null

    property int pw: parent.www
    property int ph: parent.hhh
    property int layoutDirection: Qt.LeftToRight

    ////////////////////////////////////////////////////////////////////////////

    Row {
        anchors.fill: parent
        spacing: 12
        layoutDirection: remoteButton.layoutDirection

        ////////

        Rectangle {
            width: parent.height
            height: parent.height
            radius: 12

            color: (itemDeviceRemote.btnClicked === currentButton.id) ? Theme.colorPrimary : Theme.colorSeparator

            Text {
                anchors.centerIn: parent
                text: "#" + currentButton.id
                font.pixelSize: 32
                font.bold: true
                color: "white"
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (itemDeviceRemote.btnClicked === currentButton.id)
                        itemDeviceRemote.btnClicked = 0
                    else
                        itemDeviceRemote.btnClicked = currentButton.id
                }
            }
        }

        ////////

        Column {
            width: parent.width - parent.height
            anchors.verticalCenter: parent.verticalCenter

            ////

            Row {
                anchors.left: parent.left
                anchors.right: parent.right
                layoutDirection: remoteButton.layoutDirection

                height: (ph / 6)
                spacing: 12

                visible: currentDevice.hasPress

                ItemTag {
                    anchors.verticalCenter: parent.verticalCenter
                    height: 36
                    width: 92

                    color: Theme.colorForeground
                    text: "press"
                }

                ButtonWireframeIcon {
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width - 92 - 12*3
                    height: 36

                    fullColor: true
                    fulltextColor: Theme.colorText
                    primaryColor: Theme.colorForeground

                    text: UtilsActions.getActionName(currentButton.singleAction)
                    source: UtilsActions.getActionIcon(currentButton.singleAction)

                    onClicked: popupActionsChooser.openAction(currentButton, 0)
                }
            }

            ////

            Row {
                anchors.left: parent.left
                anchors.right: parent.right
                layoutDirection: remoteButton.layoutDirection

                height: (ph / 6)
                spacing: 12

                visible: currentDevice.hasDoublePress

                ItemTag {
                    anchors.verticalCenter: parent.verticalCenter
                    height: 36
                    width: 92

                    color: Theme.colorForeground
                    text: "double"
                }

                ButtonWireframeIcon {
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width - 92 - 12*3
                    height: 36

                    fullColor: true
                    fulltextColor: Theme.colorText
                    primaryColor: Theme.colorForeground

                    text: UtilsActions.getActionName(currentButton.doubleAction)
                    source: UtilsActions.getActionIcon(currentButton.doubleAction)

                    onClicked: popupActionsChooser.openAction(currentButton, 1)
                }
            }

            ////

            Row {
                anchors.left: parent.left
                anchors.right: parent.right
                layoutDirection: remoteButton.layoutDirection

                height: (ph / 6)
                spacing: 12

                visible: currentDevice.hasLongPress

                ItemTag {
                    anchors.verticalCenter: parent.verticalCenter
                    height: 36
                    width: 92

                    color: Theme.colorForeground
                    text: "long"
                }

                ButtonWireframeIcon {
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width - 92 - 12*3
                    height: 36

                    fullColor: true
                    fulltextColor: Theme.colorText
                    primaryColor: Theme.colorForeground

                    text: UtilsActions.getActionName(currentButton.longAction)
                    source: UtilsActions.getActionIcon(currentButton.longAction)

                    onClicked: popupActionsChooser.openAction(currentButton, 2)
                }
            }

            ////
        }

        ////////
    }
}

/*
Item { // collapsable button
    id: btn
    width: pw
    height: extended ? ph : 48

    property bool extended: false

    Behavior on height { NumberAnimation { duration: 333 } }

    MouseArea {
        anchors.fill: parent
        onClicked: parent.extended = !parent.extended
    }

    Rectangle {
        id: background
        width: pw
        height: parent.height
        radius: 8
        color: Theme.colorBackground
        border.width: 2
        border.color: Theme.colorSeparator

        Rectangle {
            width: pw
            height: 48
            color: (itemDeviceRemote.btnClicked === 1) ? Theme.colorPrimary : Theme.colorSeparator

            Text {
                anchors.left: parent.left
                anchors.leftMargin: 12
                anchors.verticalCenter: parent.verticalCenter

                text: "Button 1"
                font.bold: true
                color: "white"
            }
        }

        layer.enabled: true
        layer.effect: OpacityMask {
            maskSource: Rectangle {
                x: background.x
                y: background.y
                width: background.width
                height: background.height
                radius: background.radius
            }
        }
    }
}
*/
