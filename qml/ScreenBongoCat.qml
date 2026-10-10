import QtQuick
import QtQuick.Controls

import ComponentLibrary

Loader {
    id: screenBongoCat
    anchors.fill: parent

    ////////

    function loadScreen() {
        screenBongoCat.active = true
        appContent.state = "ScreenBongoCat"
    }

    function backAction() {
        if (screenBongoCat.status === Loader.Ready)
            screenBongoCat.item.backAction()
    }

    ////////////////////////////////////////////////////////////////////////////

    active: false
    asynchronous: false

    sourceComponent: Item {
        id: bongoScreen
        anchors.fill: parent

        focus: parent.focus
        Keys.onPressed: (event) => {
            if (!event.isAutoRepeat) bongoCat.tap()
        }

        property bool editing: false

        function backAction() {
            if (editing) editing = false
            else screenDeviceList.loadScreen()
        }

        readonly property bool remote: (networkClient !== null && networkClient.connected)

        readonly property var faces: ["cute", "asia", "dead", "nerd", "harry_potter", "pilot", "pixel_cool", "fosure"]
        readonly property var hats: ["", "crown", "propeller_hat", "banana", "lil_duck", "heart", "timer"]

        ////////////////

        Flickable {
            anchors.fill: parent

            contentWidth: -1
            contentHeight: contentColumn.height + 32
            interactive: bongoScreen.editing
            boundsBehavior: Theme.isDesktop ? Flickable.OvershootBounds : Flickable.DragAndOvershootBounds

            Column {
                id: contentColumn
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.topMargin: 16
                spacing: 16

                Item { // stage
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: isMobile ? parent.width : Math.round(parent.width * 0.5)
                    height: Math.round(width * bongoCat.implicitHeight / bongoCat.implicitWidth)

                    BongoCat {
                        id: bongoCat
                        anchors.fill: parent
                        anchors.margins: 8

                        typing: bongoScreen.remote ? networkClient.typing : InputMonitor.typing
                        face: SettingsManager.bongoFace
                        hat: SettingsManager.bongoHat
                        pawInterval: SettingsManager.bongoPawInterval
                    }

                    MouseArea {
                        anchors.fill: parent
                        onPressed: bongoCat.tap()
                    }
                }

                Column { // customization
                    anchors.left: parent.left
                    anchors.right: parent.right
                    spacing: 16

                    visible: bongoScreen.editing

                    ////

                    ListTitle {
                        text: qsTr("Face")
                        source: "qrc:/IconLibrary/material-symbols/face-fill.svg"
                    }

                    Flow {
                        anchors.left: parent.left
                        anchors.leftMargin: 16
                        anchors.right: parent.right
                        anchors.rightMargin: 16
                        spacing: 12

                        Repeater {
                            model: bongoScreen.faces

                            KitTile {
                                required property string modelData

                                face: modelData
                                hat: SettingsManager.bongoHat
                                selected: (SettingsManager.bongoFace === modelData)
                                onPicked: SettingsManager.bongoFace = modelData
                            }
                        }
                    }

                    ////

                    ListTitle {
                        text: qsTr("Hat")
                        source: "qrc:/IconLibrary/material-symbols/face_4-fill.svg"
                    }

                    Flow {
                        anchors.left: parent.left
                        anchors.leftMargin: 16
                        anchors.right: parent.right
                        anchors.rightMargin: 16
                        spacing: 12

                        Repeater {
                            model: bongoScreen.hats

                            KitTile {
                                required property string modelData

                                face: SettingsManager.bongoFace
                                hat: modelData
                                selected: (SettingsManager.bongoHat === modelData)
                                onPicked: SettingsManager.bongoHat = modelData
                            }
                        }
                    }

                    ////

                    ListTitle {
                        text: qsTr("Paw speed")
                        source: "qrc:/IconLibrary/material-symbols/speed.svg"
                    }

                    SliderValueSolid {
                        anchors.left: parent.left
                        anchors.leftMargin: 16
                        anchors.right: parent.right
                        anchors.rightMargin: 16

                        from: 100
                        to: 400
                        stepSize: 10
                        unit: "ms"
                        floatprecision: 0

                        value: SettingsManager.bongoPawInterval
                        onMoved: SettingsManager.bongoPawInterval = Math.round(value)
                    }

                    ////
                }
            }
        }

        ////////////////

        ButtonFab {
            anchors.right: parent.right
            anchors.rightMargin: Theme.componentMarginXL
            anchors.bottom: parent.bottom
            anchors.bottomMargin: Theme.componentMarginXL

            source: bongoScreen.editing ?
                        "qrc:/IconLibrary/material-symbols/check.svg" :
                        "qrc:/IconLibrary/material-symbols/edit.svg"
            onClicked: bongoScreen.editing = !bongoScreen.editing
        }

        ////////////////
    }

    ////////////////////////////////////////////////////////////////////////////

    component KitTile: Rectangle {
        id: kitTile

        property alias face: tileCat.face
        property alias hat: tileCat.hat
        property bool selected: false
        signal picked()

        width: 96
        height: 72
        radius: 8

        color: Theme.colorForeground
        border.width: 2
        border.color: selected ? Theme.colorPrimary : Theme.colorSeparator

        BongoCat {
            id: tileCat
            anchors.fill: parent
            anchors.margins: 4
        }

        MouseArea {
            anchors.fill: parent
            onClicked: kitTile.picked()
        }
    }

    ////////////////////////////////////////////////////////////////////////////
}
