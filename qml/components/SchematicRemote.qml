import QtQuick
import QtQuick.Controls

import ThemeEngine

Rectangle {
    id: remoteSchematics
    anchors.centerIn: parent

    width: height / 3
    height: parent.height*0.8
    radius: width / 2

    color: Theme.colorForeground
    border.width: 2
    border.color: Theme.colorSeparator

    property int mmm: (width / 8)
    property int bbb: (width * 0.32)

    ////////////////////////////////////////////////////////////////////////////

    Connections {
        target: currentDevice
        function onButtonPressed() { ledAnimation.start() }
        function onButton1Pressed() { btn1Animation.start() }
        function onButton1LongPressed() { btn1Animation.start() }
        function onButton2Pressed() { btn2Animation.start() }
        function onButton2LongPressed() { btn2Animation.start() }
        function onButton3Pressed() { btn3Animation.start() }
        function onButton3LongPressed() { btn3Animation.start() }
        function onButton4Pressed() { btn4Animation.start() }
        function onButton4LongPressed() { btn4Animation.start() }
        function onButton5Pressed() { btn5Animation.start() }
        function onButton5LongPressed() { btn5Animation.start() }
        function onButton6Pressed() { btn6Animation.start() }
        function onButton6LongPressed() { btn6Animation.start() }
    }

    ////////////////////////////////////////////////////////////////////////////

    Rectangle {
        id: led
        anchors.top: parent.top
        anchors.topMargin: remoteSchematics.mmm
        anchors.horizontalCenter: parent.horizontalCenter

        width: (parent.width / 24)
        height: width
        radius: width

        color: Theme.colorSeparator

        ColorAnimation on color {
            id: ledAnimation
            loops: 1
            running: false

            duration: 333
            from: Theme.colorPrimary
            to: Theme.colorSeparator
        }
    }

    ////

    Row {
        anchors.top: parent.top
        anchors.topMargin: parent.radius - remoteSchematics.mmm
        anchors.left: parent.left
        anchors.leftMargin: remoteSchematics.mmm
        anchors.right: parent.right
        anchors.rightMargin: remoteSchematics.mmm
        spacing: remoteSchematics.mmm

        Column {
            width: remoteSchematics.bbb
            spacing: (remoteSchematics.mmm * 0.666)

            Rectangle { // #1 // button "on"
                width: remoteSchematics.bbb
                height: width
                radius: width
                color: Theme.colorBackground
                border.width: 2
                border.color: (itemDeviceRemote.btnClicked === 1) ? Theme.colorPrimary: Theme.colorSeparator

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (itemDeviceRemote.btnClicked === 1)
                            itemDeviceRemote.btnClicked = 0
                        else
                            itemDeviceRemote.btnClicked = 1
                    }
                }

                Text {
                    anchors.centerIn: parent

                    text: "ON"
                    font.pixelSize: 22
                    color: (itemDeviceRemote.btnClicked === 1) ? Theme.colorPrimary: Theme.colorSeparator
                }

                ColorAnimation on color {
                    id: btn1Animation
                    loops: 1
                    running: false

                    duration: 333
                    from: Theme.colorPrimary
                    to: Theme.colorBackground
                }
            }

            ////

            Rectangle { // #3 // button "sun"
                width: remoteSchematics.bbb
                height: width
                radius: width
                color: Theme.colorBackground
                border.width: 2
                border.color: (itemDeviceRemote.btnClicked === 3) ? Theme.colorPrimary: Theme.colorSeparator

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (itemDeviceRemote.btnClicked === 3)
                            itemDeviceRemote.btnClicked = 0
                        else
                            itemDeviceRemote.btnClicked = 3
                    }
                }

                IconSvg {
                    anchors.centerIn: parent
                    width: parent.width*0.5
                    height: parent.width*0.5
                    source: "qrc:/assets/icons_material/duotone-wb_sunny-24px.svg"
                    color: (itemDeviceRemote.btnClicked === 3) ? Theme.colorPrimary: Theme.colorSeparator
                }

                ColorAnimation on color {
                    id: btn3Animation
                    loops: 1
                    running: false

                    duration: 333
                    from: Theme.colorPrimary
                    to: Theme.colorBackground
                }
            }

            ////

            Rectangle { // #5 // button "M"
                width: remoteSchematics.bbb
                height: width
                radius: width
                color: Theme.colorBackground
                border.width: 2
                border.color: (itemDeviceRemote.btnClicked === 5) ? Theme.colorPrimary: Theme.colorSeparator

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (itemDeviceRemote.btnClicked === 5)
                            itemDeviceRemote.btnClicked = 0
                        else
                            itemDeviceRemote.btnClicked = 5
                    }
                }

                Text {
                    anchors.centerIn: parent

                    text: "M"
                    font.pixelSize: 24
                    color: (itemDeviceRemote.btnClicked === 5) ? Theme.colorPrimary: Theme.colorSeparator
                }

                ColorAnimation on color {
                    id: btn5Animation
                    loops: 1
                    running: false

                    duration: 333
                    from: Theme.colorPrimary
                    to: Theme.colorBackground
                }
            }
        }

        ////////

        Column {
            width: remoteSchematics.bbb
            spacing: (remoteSchematics.mmm * 0.666)

            Rectangle { // #2 // button "off"
                width: remoteSchematics.bbb
                height: width
                radius: width
                color: Theme.colorBackground
                border.width: 2
                border.color: (itemDeviceRemote.btnClicked === 2) ? Theme.colorPrimary: Theme.colorSeparator

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (itemDeviceRemote.btnClicked === 2)
                            itemDeviceRemote.btnClicked = 0
                        else
                            itemDeviceRemote.btnClicked = 2
                    }
                }

                Text {
                    anchors.centerIn: parent

                    text: "OFF"
                    font.pixelSize: 22
                    color: (itemDeviceRemote.btnClicked === 2) ? Theme.colorPrimary: Theme.colorSeparator
                }

                ColorAnimation on color {
                    id: btn2Animation
                    loops: 1
                    running: false

                    duration: 333
                    from: Theme.colorPrimary
                    to: Theme.colorBackground
                }
            }

            ////

            Rectangle { // volume buttons
                width: remoteSchematics.bbb
                height: width*2+parent.spacing
                radius: width
                color: Theme.colorBackground
                border.width: 2
                border.color: Theme.colorSeparator

                Rectangle { // #4 // button "+"
                    anchors.top: parent.top
                    anchors.topMargin: 2
                    anchors.horizontalCenter: parent.horizontalCenter

                    width: remoteSchematics.bbb-4
                    height: width
                    radius: width
                    color: Theme.colorBackground

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (itemDeviceRemote.btnClicked === 4)
                                itemDeviceRemote.btnClicked = 0
                            else
                                itemDeviceRemote.btnClicked = 4
                        }
                    }

                    Rectangle {
                        anchors.centerIn: parent
                        width: parent.width*0.4
                        height: 2
                        color: (itemDeviceRemote.btnClicked === 4) ? Theme.colorPrimary: Theme.colorSeparator
                    }
                    Rectangle {
                        anchors.centerIn: parent
                        width: 2
                        height: parent.width*0.4
                        color: (itemDeviceRemote.btnClicked === 4) ? Theme.colorPrimary: Theme.colorSeparator
                    }

                    ColorAnimation on color {
                        id: btn4Animation
                        loops: 1
                        running: false

                        duration: 333
                        from: Theme.colorPrimary
                        to: Theme.colorBackground
                    }
                }


                Rectangle { // #6 // button "-"
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 2
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: remoteSchematics.bbb-4
                    height: width
                    radius: width
                    color: Theme.colorBackground

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (itemDeviceRemote.btnClicked === 6)
                                itemDeviceRemote.btnClicked = 0
                            else
                                itemDeviceRemote.btnClicked = 6
                        }
                    }

                    Rectangle {
                        anchors.centerIn: parent
                        width: parent.width*0.4
                        height: 2
                        color: (itemDeviceRemote.btnClicked === 6) ? Theme.colorPrimary: Theme.colorSeparator
                    }

                    ColorAnimation on color {
                        id: btn6Animation
                        loops: 1
                        running: false

                        duration: 333
                        from: Theme.colorPrimary
                        to: Theme.colorBackground
                    }
                }
            }
        }
    }
}
