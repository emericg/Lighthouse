import QtQuick
import QtQuick.Controls
import Qt.labs.platform

import ThemeEngine 1.0
import DeviceUtils 1.0
import "qrc:/js/UtilsDeviceSensors.js" as UtilsDeviceSensors

Loader {
    id: deviceLight

    property var currentDevice: null

    ////////

    function loadDevice(clickedDevice) {
        if (typeof clickedDevice === "undefined" || !clickedDevice) return
        if (!clickedDevice.isLight) return

        appContent.state = "DeviceLight"

        // set device
        if (clickedDevice === currentDevice) return
        currentDevice = clickedDevice

        // load screen
        deviceLight.active = true
        deviceLight.item.loadDevice()
    }

    ////////

    function backAction() {
        if (deviceLight.status === Loader.Ready)
            deviceLight.item.backAction()
    }

    active: false

    asynchronous: false
    sourceComponent: Item {
        id: itemDeviceLight
        implicitWidth: 480
        implicitHeight: 720

        focus: parent.focus

        // 1: single column (single column view or portrait tablet)
        // 2: wide mode (wide view)
        property int uiMode: (singleColumn || (isTablet && screenOrientation === Qt.PortraitOrientation)) ? 1 : 2

        property string cccc: headerUnicolor ? Theme.colorHeaderContent : "white"

        ////////

        Connections {
            target: currentDevice
            function onStatusUpdated() { updateHeader() }
            function onDataUpdated() { updateData() }
        }

        Connections {
            target: appHeader
            // desktop only
            function onDeviceDataButtonClicked() {
                appHeader.setActiveDeviceData()
            }
            function onDeviceSettingsButtonClicked() {
                appHeader.setActiveDeviceSettings()
            }
            // mobile only
            function onRightMenuClicked() {
                //
            }
        }

        Keys.onPressed: (event) => {
            if (event.key === Qt.Key_Backspace) {
                event.accepted = true
                backAction()
            }
        }

        ////////

        function loadDevice() {
            //console.log("DeviceLight // loadDevice() >> " + currentDevice)

            currentDevice.actionConnect()

            updateHeader()
            updateData()
        }

        function updateHeader() {
            if (typeof currentDevice === "undefined" || !currentDevice) return
            if (!currentDevice.isLight) return
            //console.log("DeviceLight // updateHeader() >> " + currentDevice)

            // Status
            updateStatusText()
        }

        function updateStatusText() {
            if (typeof currentDevice === "undefined" || !currentDevice) return
            if (!currentDevice.isLight) return
            //console.log("DeviceLight // updateStatusText() >> " + currentDevice)

            // Status
            textStatus.text = UtilsDeviceSensors.getDeviceStatusText(currentDevice.status)
        }

        function updateData() {
            if (typeof currentDevice === "undefined" || !currentDevice) return
            if (!currentDevice.isLight) return
            //console.log("DeviceLight // updateData() >> " + currentDevice)

            var a = currentDevice.getLuminosity()
            var r = currentDevice.getColor_r()
            var g = currentDevice.getColor_g()
            var b = currentDevice.getColor_b()

            slider_luminosity.value = a / 255;
            slider_color.value = Qt.rgba(r / 255, g / 255, b / 255, 1).hsvHue

            if (r !== 0 || g !== 0 || b !== 0) {
                btnLightMode.selected = qsTr("colors")
            } else if (a !== 0) {
                btnLightMode.selected = qsTr("light")
            } else {
                btnLightMode.selected = qsTr("off")
            }

            colorSaved.selection = -1
        }

        ////////

        function backAction() {
            if (textInputLocation.focus) {
                textInputLocation.focus = false
                return
            }
            if (isHistoryMode()) {
                resetHistoryMode()
                return
            }

            currentDevice.deviceDisconnect()

            appContent.state = "DeviceList"
        }

        function isHistoryMode() {
            //return deviceScreenChart.isIndicator()
        }
        function resetHistoryMode() {
            //deviceScreenChart.resetIndicator()
        }

        ////////////////////////////////////////////////////////////////////////
/*
        ColorDialog {
            id: colorDialog
            title: "Please choose a color"
        }
*/
        ////////////////////////////////////////////////////////////////////////

        Flow {
            anchors.fill: parent

            Rectangle {
                id: headerBox

                property int dimboxw: Math.min(deviceLight.width * 0.4, isPhone ? 320 : 600)
                property int dimboxh: Math.max(deviceLight.height * 0.333, isPhone ? 180 : 256)

                width: (uiMode === 1) ? parent.width : dimboxw
                height: (uiMode === 1) ? dimboxh : parent.height

                color: Theme.colorHeader
                z: 5

                MouseArea { anchors.fill: parent } // prevent clicks below this area

                // center
                Column {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.verticalCenterOffset: -(appHeader.height / 2) + (imageBattery.visible ? (imageBattery.width / 2) : 0)
                    spacing: 2

                    IconSvg {
                        id: imageLightbulb
                        width: 180; height: 180;
                        anchors.horizontalCenter: parent.horizontalCenter
                        source: "qrc:/assets/icons_material/light_FILL0_wght400_GRAD0_opsz48.svg"
                        color: Theme.colorHeaderContent
                    }

                    IconSvg {
                        id: imageBattery
                        width: isPhone ? 20 : 24
                        height: isPhone ? 32 : 36
                        rotation: 90
                        anchors.horizontalCenter: parent.horizontalCenter

                        visible: (currentDevice.hasBattery && currentDevice.deviceBattery >= 0)
                        source: UtilsDeviceSensors.getDeviceBatteryIcon(currentDevice.deviceBattery)
                        fillMode: Image.PreserveAspectCrop
                        color: cccc
                    }
                }

                ////////

                Row {
                    id: status
                    anchors.left: parent.left
                    anchors.leftMargin: 8
                    anchors.right: itemLocation.left
                    anchors.rightMargin: 8
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 8

                    height: 24
                    spacing: 8

                    IconSvg {
                        id: imageStatus
                        width: 24
                        height: 24
                        anchors.verticalCenter: parent.verticalCenter

                        source: "qrc:/assets/icons_material/duotone-schedule-24px.svg"
                        color: cccc
                    }
                    Text {
                        id: textStatus
                        width: status.width - status.spacing - imageStatus.width
                        anchors.verticalCenter: parent.verticalCenter

                        text: qsTr("Loading...")
                        color: cccc
                        font.bold: false
                        font.pixelSize: 17
                        elide: Text.ElideRight
                    }
                }

                ////////

                Row {
                    id: itemLocation
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 8
                    anchors.right: parent.right
                    anchors.rightMargin: 8

                    height: 24
                    spacing: 4

                    IconSvg {
                        id: imageEditLocation
                        width: 20
                        height: 20
                        anchors.verticalCenter: parent.verticalCenter

                        source: "qrc:/assets/icons_material/duotone-edit-24px.svg"
                        color: cccc

                        opacity: (isMobile || !textInputLocation.text || textInputLocation.focus || textInputLocationArea.containsMouse) ? 0.9 : 0
                        Behavior on opacity { OpacityAnimator { duration: 133 } }
                    }
                    TextInput {
                        id: textInputLocation
                        anchors.verticalCenter: parent.verticalCenter

                        padding: 4
                        font.pixelSize: 17
                        font.bold: false
                        color: cccc

                        text: currentDevice ? currentDevice.deviceLocationName : ""
                        onEditingFinished: {
                            currentDevice.deviceLocationName = text
                            focus = false
                        }

                        MouseArea {
                            id: textInputLocationArea
                            anchors.fill: parent
                            anchors.topMargin: -4
                            anchors.leftMargin: -24
                            anchors.rightMargin: -4
                            anchors.bottomMargin: -4

                            hoverEnabled: true
                            propagateComposedEvents: true

                            onPressed: (mouse) => {
                                textInputLocation.forceActiveFocus()
                                mouse.accepted = false
                            }
                        }
                    }
                    IconSvg {
                        id: imageLocation
                        width: 24
                        height: 24
                        anchors.verticalCenter: parent.verticalCenter

                        source: "qrc:/assets/icons_material/duotone-pin_drop-24px.svg"
                        color: cccc
                    }
                }

                ////////

                Rectangle {
                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom

                    visible: !singleColumn
                    width: 2
                    opacity: 0.33
                    color: Theme.colorHeaderHighlight
                }
                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom

                    visible: singleColumn
                    height: 2
                    opacity: 0.33
                    color: Theme.colorHeaderHighlight
                }
            }

            ////////////////////////////////////////////////////////////////////

            Item {
                width: {
                    if (isTablet && screenOrientation == Qt.PortraitOrientation) return parent.width
                    return singleColumn ? parent.width : (parent.width - headerBox.width)
                }
                height: {
                    if (isTablet && screenOrientation == Qt.PortraitOrientation) return (parent.height - headerBox.height)
                    return singleColumn ? (parent.height - headerBox.height) : parent.height
                }

                ////////

                Rectangle {
                    id: itemLoadData
                    anchors.centerIn: parent

                    width: singleColumn ? (parent.width*0.26) : (parent.height*0.26)
                    height: width
                    radius: width
                    color: Theme.colorForeground
                    opacity: 0.8

                    visible: (currentDevice.status !== DeviceUtils.DEVICE_CONNECTED)

                    IconSvg {
                        anchors.centerIn: parent
                        width: parent.width*0.8
                        height: width

                        source: {
                            if (currentDevice.status === DeviceUtils.DEVICE_CONNECTED)
                                return ""
                            else if (currentDevice.status === DeviceUtils.DEVICE_CONNECTING)
                                return "qrc:/assets/icons_material/duotone-bluetooth_connected-24px.svg"
                            else
                                return "qrc:/assets/icons_material/baseline-bluetooth_disabled-24px.svg"
                        }

                        fillMode: Image.PreserveAspectFit
                        color: Theme.colorSubText
                        smooth: true
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            currentDevice.actionConnect()
                        }
                    }
                }

                ////////

                Column {
                    id: columnLightbulb
                    anchors.fill: parent
                    anchors.margins: isPhone ? 12 : 24

                    spacing: 16
                    visible: (currentDevice && currentDevice.status >= DeviceUtils.DEVICE_CONNECTED)

                    Item {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        height: 40

                        Rectangle {
                            width: btnLightMode.width+32
                            height: 32
                            anchors.centerIn: parent

                            radius: 16
                            color: "white"
                            border.color: Theme.colorComponentDown
                            border.width: 2

                            Row {
                                id: btnLightMode
                                anchors.left: parent.left
                                anchors.leftMargin: 16
                                anchors.verticalCenter: parent.verticalCenter
                                height: 32
                                spacing: 32

                                property string selected
                                property int value: {
                                    if (selected === qsTr("off")) return 0
                                    if (selected === qsTr("light")) return 1
                                    if (selected === qsTr("colors")) return 2
                                    else return 2
                                }

                                function clicked() {
                                    if (selected === qsTr("off")) {
                                        currentDevice.setOff()
                                    }
                                    if (selected === qsTr("light")) {
                                        currentDevice.setColors_float(slider_luminosity.value, 0, 0, 0)
                                    }
                                    if (selected === qsTr("colors")) {
                                        var clr = Qt.hsva(slider_color.value, 1, 1, 1)
                                        currentDevice.setColors_float(slider_luminosity.value, clr.r, clr.g, clr.b)
                                    }
                                }

                                Text {
                                    id: btnOff
                                    anchors.verticalCenter: parent.verticalCenter

                                    property bool selected: false
                                    text: qsTr("off")
                                    font.pixelSize: Theme.fontSizeComponent
                                    //font.bold: (btnLightMode.selected === btnOff.text) ? true : false
                                    color: (btnLightMode.selected === btnOff.text) ? "white" : Theme.colorSubText

                                    Rectangle {
                                        anchors.fill: parent
                                        anchors.topMargin: -8
                                        anchors.leftMargin: -16
                                        anchors.rightMargin: -16
                                        anchors.bottomMargin: -8
                                        z: -1
                                        radius: 32
                                        color: (btnLightMode.selected === btnOff.text) ? Theme.colorPrimary : "transparent"

                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {
                                                btnLightMode.selected = btnOff.text
                                                btnLightMode.clicked()
                                            }
                                        }
                                    }
                                }
                                Text {
                                    id: btnLight
                                    anchors.verticalCenter: parent.verticalCenter

                                    property bool selected: false
                                    text: qsTr("light")
                                    font.pixelSize: Theme.fontSizeComponent
                                    //font.bold: (btnLightMode.selected === btnLight.text) ? true : false
                                    color: (btnLightMode.selected === btnLight.text) ? "white" : Theme.colorSubText

                                    Rectangle {
                                        anchors.fill: parent
                                        anchors.topMargin: -8
                                        anchors.leftMargin: -16
                                        anchors.rightMargin: -16
                                        anchors.bottomMargin: -8
                                        z: -1
                                        radius: 32
                                        color: (btnLightMode.selected === btnLight.text) ? Theme.colorPrimary : "transparent"

                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {
                                                btnLightMode.selected = btnLight.text
                                                btnLightMode.clicked()
                                            }
                                        }
                                    }
                                }
                                Text {
                                    id: btnColors
                                    anchors.verticalCenter: parent.verticalCenter

                                    property bool selected: false
                                    text: qsTr("colors")
                                    font.pixelSize: Theme.fontSizeComponent
                                    //font.bold: (btnLightMode.selected === btnColors.text) ? true : false
                                    color: (btnLightMode.selected === btnColors.text) ? "white" : Theme.colorSubText

                                    Rectangle {
                                        anchors.fill: parent
                                        anchors.topMargin: -8
                                        anchors.leftMargin: -16
                                        anchors.rightMargin: -16
                                        anchors.bottomMargin: -8
                                        z: -1
                                        radius: 32
                                        color: (btnLightMode.selected === btnColors.text) ? Theme.colorPrimary : "transparent"

                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {
                                                btnLightMode.selected = btnColors.text
                                                btnLightMode.clicked()
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    ////////

                    Item {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        height: 40

                        SliderGradientLuminosity {
                            id: slider_luminosity
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter

                            height: 32
                            radius: 8

                            from: 0.0
                            to: 1.0
                            stepSize: 0.05

                            visible: (btnLightMode.selected === "light" ||
                                      btnLightMode.selected === "colors")

                            onMoved: {
                                var l = slider_luminosity.value
                                var c = Qt.hsva(slider_color.value, 1, 1, 1)

                                currentDevice.setColors_float(l, c.r, c.g, c.b)
                            }
                        }
                    }

                    ////////////////

                    Item {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        height: 40

                        SliderGradientColor {
                            id: slider_color
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter

                            height: 32
                            radius: 8

                            from: 0.0
                            to: 1.0
                            stepSize: 0.05

                            visible: (btnLightMode.selected === "colors")

                            onMoved: {
                                var lum = slider_luminosity.value
                                var clr = Qt.hsva(slider_color.value, 1, 1, 1)

                                colorSaved.selection = -1
                                currentDevice.setColors_float(lum, clr.r, clr.g, clr.b)
                            }
                        }
                    }

                    ////////

                    Item { width: 16; height: 16; } // spacer

                    ////////

                    Grid {
                        id: colorSaved
                        //anchors.left: parent.left
                        //anchors.right: parent.right
                        anchors.horizontalCenter: parent.horizontalCenter

                        width: ww*columns + spacing*(columns-1)
                        height: hh*rows + spacing*(rows-1)

                        columns: 6
                        rows: 2
                        spacing: 12

                        visible: (btnLightMode.selected === "colors")

                        property int ww: isPhone ? 48 : 80
                        property int hh: 48
                        property int rr: 6

                        property int selection: -1

                        Rectangle {
                            width: colorSaved.ww; height: colorSaved.hh; radius: colorSaved.rr;

                            property int cid: 0
                            color: "#f2b101"

                            border.width: 2
                            border.color: (colorSaved.selection === cid) ? Theme.colorHighContrast : Qt.darker(color, 1.05)

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    colorSaved.selection = parent.cid
                                    currentDevice.setColors_rgb_hex(parent.color)
                                    slider_color.value = Qt.rgba(parent.color.r / 255, parent.color.g / 255, parent.color.b / 255, 1).hsvHue
                                }
                            }
                        }

                        Rectangle {
                            width: colorSaved.ww; height: colorSaved.hh; radius: colorSaved.rr;

                            property int cid: 1
                            color: "#f32f09"

                            border.width: 2
                            border.color: (colorSaved.selection === cid) ? Theme.colorHighContrast : Qt.darker(color, 1.05)

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    colorSaved.selection = parent.cid
                                    currentDevice.setColors_rgb_hex(parent.color)
                                    slider_color.value = Qt.rgba(parent.color.r / 255, parent.color.g / 255, parent.color.b / 255, 1).hsvHue
                                }
                            }
                        }

                        Rectangle {
                            width: colorSaved.ww; height: colorSaved.hh; radius: colorSaved.rr;

                            property int cid: 2
                            color: "#f24175"

                            border.width: 2
                            border.color: (colorSaved.selection === cid) ? Theme.colorHighContrast : Qt.darker(color, 1.05)

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    colorSaved.selection = parent.cid
                                    currentDevice.setColors_rgb_hex(parent.color)
                                    slider_color.value = Qt.rgba(parent.color.r / 255, parent.color.g / 255, parent.color.b / 255, 1).hsvHue
                                }
                            }
                        }

                        Rectangle {
                            width: colorSaved.ww; height: colorSaved.hh; radius: colorSaved.rr;

                            property int cid: 3
                            color: "#02b6eb"

                            border.width: 2
                            border.color: (colorSaved.selection === cid) ? Theme.colorHighContrast : Qt.darker(color, 1.05)

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    colorSaved.selection = parent.cid
                                    currentDevice.setColors_rgb_hex(parent.color)
                                    slider_color.value = Qt.rgba(parent.color.r / 255, parent.color.g / 255, parent.color.b / 255, 1).hsvHue
                                }
                            }
                        }

                        Rectangle {
                            width: colorSaved.ww; height: colorSaved.hh; radius: colorSaved.rr;

                            property int cid: 4
                            color: "#01ec9d"

                            border.width: 2
                            border.color: (colorSaved.selection === cid) ? Theme.colorHighContrast : Qt.darker(color, 1.05)

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    colorSaved.selection = parent.cid
                                    currentDevice.setColors_rgb_hex(parent.color)
                                    slider_color.value = Qt.rgba(parent.color.r / 255, parent.color.g / 255, parent.color.b / 255, 1).hsvHue
                                }
                            }
                        }

                        Rectangle {
                            width: colorSaved.ww; height: colorSaved.hh; radius: colorSaved.rr;

                            property int cid: 5
                            color: "#01e802"

                            border.width: 2
                            border.color: (colorSaved.selection === cid) ? Theme.colorHighContrast : Qt.darker(color, 1.05)

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    colorSaved.selection = parent.cid
                                    currentDevice.setColors_rgb_hex(parent.color)
                                    slider_color.value = Qt.rgba(parent.color.r / 255, parent.color.g / 255, parent.color.b / 255, 1).hsvHue
                                }
                            }
                        }

                        ////

                        Rectangle {
                            width: colorSaved.ww; height: colorSaved.hh; radius: colorSaved.rr;

                            property int cid: 6
                            color: "#f5df02"

                            border.width: 2
                            border.color: (colorSaved.selection === cid) ? Theme.colorHighContrast : Qt.darker(color, 1.05)

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    colorSaved.selection = parent.cid
                                    currentDevice.setColors_rgb_hex(parent.color)
                                    slider_color.value = Qt.rgba(parent.color.r / 255, parent.color.g / 255, parent.color.b / 255, 1).hsvHue
                                }
                            }
                        }

                        Rectangle {
                            width: colorSaved.ww; height: colorSaved.hh; radius: colorSaved.rr;

                            property int cid: 7
                            color: "#73c300"

                            border.width: 2
                            border.color: (colorSaved.selection === cid) ? Theme.colorHighContrast : Qt.darker(color, 1.05)

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    colorSaved.selection = parent.cid
                                    currentDevice.setColors_rgb_hex(parent.color)
                                    slider_color.value = Qt.rgba(parent.color.r / 255, parent.color.g / 255, parent.color.b / 255, 1).hsvHue
                                }
                            }
                        }

                        Rectangle {
                            width: colorSaved.ww; height: colorSaved.hh; radius: colorSaved.rr;

                            property int cid: 8
                            color: "#00c0bb"

                            border.width: 2
                            border.color: (colorSaved.selection === cid) ? Theme.colorHighContrast : Qt.darker(color, 1.05)

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    colorSaved.selection = parent.cid
                                    currentDevice.setColors_rgb_hex(parent.color)
                                    slider_color.value = Qt.rgba(parent.color.r / 255, parent.color.g / 255, parent.color.b / 255, 1).hsvHue
                                }
                            }
                        }

                        Rectangle {
                            width: colorSaved.ww; height: colorSaved.hh; radius: colorSaved.rr;

                            property int cid: 9
                            color: "#265799"

                            border.width: 2
                            border.color: (colorSaved.selection === cid) ? Theme.colorHighContrast : Qt.darker(color, 1.05)

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    colorSaved.selection = parent.cid
                                    currentDevice.setColors_rgb_hex(parent.color)
                                    slider_color.value = Qt.rgba(parent.color.r / 255, parent.color.g / 255, parent.color.b / 255, 1).hsvHue
                                }
                            }
                        }

                        Rectangle {
                            width: colorSaved.ww; height: colorSaved.hh; radius: colorSaved.rr;

                            property int cid: 10
                            color: "#af1e55"

                            border.width: 2
                            border.color: (colorSaved.selection === cid) ? Theme.colorHighContrast : Qt.darker(color, 1.05)

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    colorSaved.selection = parent.cid
                                    currentDevice.setColors_rgb_hex(parent.color)
                                    slider_color.value = Qt.rgba(parent.color.r / 255, parent.color.g / 255, parent.color.b / 255, 1).hsvHue
                                }
                            }
                        }

                        Rectangle {
                            width: colorSaved.ww; height: colorSaved.hh; radius: colorSaved.rr;

                            property int cid: 11
                            color: "#f47caa"

                            border.width: 2
                            border.color: (colorSaved.selection === cid) ? Theme.colorHighContrast : Qt.darker(color, 1.05)

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    colorSaved.selection = parent.cid
                                    currentDevice.setColors_rgb_hex(parent.color)
                                    slider_color.value = Qt.rgba(parent.color.r / 255, parent.color.g / 255, parent.color.b / 255, 1).hsvHue
                                }
                            }
                        }
                    }

                    ////////
                }
            }
        }

        ////////////////////////////////////////////////////////////////////////
    }
}
