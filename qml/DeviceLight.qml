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

        if (clickedDevice === currentDevice) {
            deviceLight.item.updateData()
        } else {
            // set device
            currentDevice = clickedDevice

            // load screen
            deviceLight.active = true
            deviceLight.item.loadDevice()
        }
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

            btnLightMode.visible = false

            currentDevice.actionConnect()

            updateHeader()
        }

        function updateHeader() {
            if (typeof currentDevice === "undefined" || !currentDevice) return
            if (!currentDevice.isLight) return
            //console.log("DeviceLight // updateHeader() >> " + currentDevice)
        }

        function updateData() {
            if (typeof currentDevice === "undefined" || !currentDevice) return
            if (!currentDevice.isLight) return
            if (!currentDevice.connected) return
            //console.log("DeviceLight // updateData() >> " + currentDevice)

            var a = currentDevice.getLuminosity()
            var r = currentDevice.getColor_r()
            var g = currentDevice.getColor_g()
            var b = currentDevice.getColor_b()

            slider_luminosity.value = a / 255;
            slider_color.value = Qt.rgba(r / 255, g / 255, b / 255, 1).hsvHue

            if (r !== 0 || g !== 0 || b !== 0) {
                btnLightMode.currentSelection = 4 // color
            } else if (a !== 0) {
                btnLightMode.currentSelection = 2 // light
            } else {
                btnLightMode.currentSelection = 1 // off
            }
            btnLightMode.visible = true

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
            return false
        }
        function resetHistoryMode() {
            //
        }

        ////////////////////////////////////////////////////////////////////////

        ColorDialog {
            id: colorDialog
            title: "Please choose a color"

            currentColor: selectedDevice.colorSaved
            onAccepted: {
                //console.log("colorDialog::onAccepted(" + color + ")")
                selectedDevice.setColorSaved(color)

                colorSaved.selection = 12
                currentDevice.setColors_rgb_hex(color)
                slider_color.value = Qt.rgba(color.r / 255, color.g / 255, color.b / 255, 1).hsvHue
            }
        }

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

                        text: UtilsDeviceSensors.getDeviceStatusText(currentDevice.status)
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

                ItemLoadData {
                    id: itemLoadData
                    anchors.centerIn: parent
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
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            height: 36

                            radius: 16
                            color: Theme.colorComponentBackground

                            Text {
                                anchors.centerIn: parent

                                text: qsTr("Loading...")
                                textFormat: Text.PlainText
                                color: Theme.colorText
                                font.pixelSize: Theme.fontSizeComponent
                            }
                        }

                        SelectorMenu {
                            id: btnLightMode
                            anchors.centerIn: parent
                            height: 36

                            currentSelection: 0
                            model: ListModel {
                                id: lmSelectorMenuTxt2
                                ListElement { idx: 1; txt: "off"; src: ""; sz: 0; }
                                ListElement { idx: 2; txt: "light"; src: ""; sz: 0; }
                                ListElement { idx: 3; txt: "warmth"; src: ""; sz: 0; }
                                ListElement { idx: 4; txt: "colors"; src: ""; sz: 0; }
                            }

                            onMenuSelected: (index) => {
                                //console.log("SelectorMenu clicked #" + index)
                                currentSelection = index

                                if (currentSelection === 1) {
                                    currentDevice.setOff()
                                }
                                else if (currentSelection === 2) {
                                    currentDevice.setColors_float(slider_luminosity.value, 0, 0, 0)
                                }
                                else if (currentSelection === 4) {
                                    var clr = Qt.hsva(slider_color.value, 1, 1, 1)
                                    currentDevice.setColors_float(slider_luminosity.value, clr.r, clr.g, clr.b)
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
                            visible: (btnLightMode.currentSelection === 2 ||
                                      btnLightMode.currentSelection === 3 ||
                                      btnLightMode.currentSelection === 4)

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

                            visible: (btnLightMode.currentSelection === 4)

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

                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 12

                        visible: (btnLightMode.currentSelection === 4)

                        Grid {
                            id: colorSaved

                            width: ww*columns + spacing*(columns-1)
                            height: hh*rows + spacing*(rows-1)

                            columns: 6
                            rows: 2
                            spacing: 12

                            property int ww: isPhone ? 48 : 80
                            property int hh: 48
                            property int rr: 6

                            property int selection: -1

                            Rectangle {
                                width: colorSaved.ww; height: colorSaved.hh; radius: colorSaved.rr;

                                property int cid: 0
                                color: "#f2b101"

                                border.width: (colorSaved.selection === cid) ? 3 : 2
                                border.color: (colorSaved.selection === cid) ? Theme.colorPrimary : Qt.darker(color, 1.05)

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

                                border.width: (colorSaved.selection === cid) ? 3 : 2
                                border.color: (colorSaved.selection === cid) ? Theme.colorPrimary : Qt.darker(color, 1.05)

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

                                border.width: (colorSaved.selection === cid) ? 3 : 2
                                border.color: (colorSaved.selection === cid) ? Theme.colorPrimary : Qt.darker(color, 1.05)

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

                                border.width: (colorSaved.selection === cid) ? 3 : 2
                                border.color: (colorSaved.selection === cid) ? Theme.colorPrimary : Qt.darker(color, 1.05)

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

                                border.width: (colorSaved.selection === cid) ? 3 : 2
                                border.color: (colorSaved.selection === cid) ? Theme.colorPrimary : Qt.darker(color, 1.05)

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

                                border.width: (colorSaved.selection === cid) ? 3 : 2
                                border.color: (colorSaved.selection === cid) ? Theme.colorPrimary : Qt.darker(color, 1.05)

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

                                border.width: (colorSaved.selection === cid) ? 3 : 2
                                border.color: (colorSaved.selection === cid) ? Theme.colorPrimary : Qt.darker(color, 1.05)

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

                                border.width: (colorSaved.selection === cid) ? 3 : 2
                                border.color: (colorSaved.selection === cid) ? Theme.colorPrimary : Qt.darker(color, 1.05)

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

                                border.width: (colorSaved.selection === cid) ? 3 : 2
                                border.color: (colorSaved.selection === cid) ? Theme.colorPrimary : Qt.darker(color, 1.05)

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

                                border.width: (colorSaved.selection === cid) ? 3 : 2
                                border.color: (colorSaved.selection === cid) ? Theme.colorPrimary : Qt.darker(color, 1.05)

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

                                border.width: (colorSaved.selection === cid) ? 3 : 2
                                border.color: (colorSaved.selection === cid) ? Theme.colorPrimary : Qt.darker(color, 1.05)

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

                                border.width: (colorSaved.selection === cid) ? 3 : 2
                                border.color: (colorSaved.selection === cid) ? Theme.colorPrimary : Qt.darker(color, 1.05)

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

                        Rectangle {
                            id: colorPickerButton
                            width: colorSaved.ww*1.6 + colorSaved.spacing
                            height: colorSaved.hh*2 + colorSaved.spacing
                            radius: colorSaved.rr

                            color: selectedDevice && selectedDevice.colorSaved
                            border.width: (colorSaved.selection === cid) ? 3 : 2
                            border.color: (colorSaved.selection === cid) ? Theme.colorPrimary : Qt.darker(color, 1.05)

                            visible: isDesktop
                            property int cid: 12

                            IconSvg {
                                anchors.centerIn: parent
                                width: parent.width * 0.33
                                height: width
                                color: utilsApp.isQColorLight(parent.color) ? Theme.colorHighContrast : Theme.colorLowContrast
                                opacity: 0.5
                                source: "qrc:/assets/icons_material/duotone-style-24px.svg"
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    colorDialog.open()
                                }
                            }
                        }

                        ////////
                    }
                }
            }
        }

        ////////////////////////////////////////////////////////////////////////
    }
}
