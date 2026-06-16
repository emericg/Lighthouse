import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import ComponentLibrary
import LocalActions

Grid {
    id: virtualInput_touchpad

    rows: singleColumn ? 2 : 1
    columns: singleColumn ? 1 : 2
    spacing: 16

    property int gridSize: singleColumn ? width : Math.min(height, parent.width / 2 - 16)

    function backAction() {
        if (virtualtextfield.focus) {
            virtualtextfield.focus = false
            return
        }

        screenDeviceList.loadScreen()
    }

    ////////////////

    Column {
        width: gridSize
        height: gridSize
        spacing: -Theme.componentRadius*2

        ////////

        Rectangle { // touch surface
            id: padSurface
            anchors.left: parent.left
            anchors.right: parent.right
            height: parent.height - clickRow.height - parent.spacing

            clip: true

            radius: 8
            color: Theme.colorForeground
            border.width: 2
            border.color: Theme.colorSeparator

            ////

            MultiPointTouchArea {
                id: padArea
                anchors.fill: parent

                minimumTouchPoints: 1
                maximumTouchPoints: 2
                mouseEnabled: true // a desktop mouse acts as a single touch point

                touchPoints: [
                    TouchPoint { id: tp1 },
                    TouchPoint { id: tp2 }
                ]

                // tuning
                property real sensitivity: 1.6     // pointer speed multiplier
                property int  scrollStep: 18       // finger px per wheel notch
                property int  tapMs: 250           // max duration of a tap
                property int  tapSlopPx: 16        // max travel still counted as a tap
                property int  dragWindowMs: 320    // tap-then-press window for tap-drag

                // runtime state
                property int    _maxPoints: 0
                property real   _travel: 0         // path length, to tell tap from drag
                property double _startT: 0
                property real   _accDx: 0          // pending pointer move (fractional kept)
                property real   _accDy: 0
                property real   _accScroll: 0      // pending scroll (px, up = positive)
                property bool   _dragging: false   // left button held for a tap-drag
                property double _lastTapT: 0       // when the last 1-finger tap ended

                function fingerCount() { return (tp1.pressed ? 1 : 0) + (tp2.pressed ? 1 : 0) }
                function activePoint() { return tp1.pressed ? tp1 : (tp2.pressed ? tp2 : null) }

                onPressed: (points) => {
                    var n = fingerCount()
                    if (_maxPoints === 0) { // gesture start (first finger(s) down)
                        _startT = Date.now()
                        _travel = 0
                        _accDx = 0; _accDy = 0; _accScroll = 0
                        // single tap then quickly press again -> start a left-button drag
                        if (n === 1 && (_startT - _lastTapT) < dragWindowMs) {
                            _dragging = true
                            networkControls.sendMouseButton(0, true)
                        }
                        flushTimer.start()
                    }
                    _maxPoints = Math.max(_maxPoints, n)
                }

                onUpdated: (points) => {
                    var n = fingerCount()
                    if (n >= 2) { // two-finger scroll (vertical)
                        var dy = ((tp1.y - tp1.previousY) + (tp2.y - tp2.previousY)) / 2
                        _accScroll += -dy // finger up -> wheel up (traditional)
                        _travel += Math.abs(dy)
                    } else { // one-finger pointer move
                        var p = activePoint()
                        if (p) {
                            var mx = p.x - p.previousX
                            var my = p.y - p.previousY
                            _accDx += mx * sensitivity
                            _accDy += my * sensitivity
                            _travel += Math.hypot(mx, my)
                        }
                    }
                    _maxPoints = Math.max(_maxPoints, n)
                }

                onReleased: (points) => {
                    if (fingerCount() > 0) return // still a finger down

                    if (_dragging) {
                        networkControls.sendMouseButton(0, false)
                        _dragging = false
                    }
                    else if ((Date.now() - _startT) < tapMs && _travel < tapSlopPx) {
                        if (_maxPoints >= 2) { // two-finger tap -> right click
                            networkControls.sendMouseClick(1)
                        } else { // single tap -> left click, arm tap-drag
                            networkControls.sendMouseClick(0)
                            _lastTapT = Date.now()
                        }
                    }

                    flushTimer.stop()
                    flushTimer.flush() // push out any sub-notch remainder
                    _maxPoints = 0
                }

                Timer {
                    id: flushTimer
                    interval: 12 // ~90 Hz, coalesce moves so we don't flood TCP
                    repeat: true

                    function flush() {
                        var ix = Math.trunc(padArea._accDx)
                        var iy = Math.trunc(padArea._accDy)
                        if (ix !== 0 || iy !== 0) {
                            padArea._accDx -= ix; padArea._accDy -= iy
                            networkControls.sendMouseMove(ix, iy)
                        }
                        if (Math.abs(padArea._accScroll) >= padArea.scrollStep) {
                            var notches = Math.trunc(padArea._accScroll / padArea.scrollStep)
                            padArea._accScroll -= notches * padArea.scrollStep
                            networkControls.sendMouseScroll(0, notches)
                        }
                    }
                    onTriggered: flush()
                }
            }

            ////

            Rectangle { // touch pointer feedback (1)
                width: 64
                height: 64
                radius: 64

                x: tp1.x - (width / 2)
                y: tp1.y - (width / 2)

                color: Theme.colorMaterialDeepOrange
                opacity: tp1.pressed ? 1 : 0
                Behavior on opacity { NumberAnimation { duration: 133 } }
            }
            Rectangle { // touch pointer feedback (2)
                width: 64
                height: 64
                radius: 64

                x: tp2.x - (width / 2)
                y: tp2.y - (width / 2)

                color: Theme.colorMaterialDeepOrange
                opacity: tp2.pressed ? 1 : 0
                Behavior on opacity { NumberAnimation { duration: 133 } }
            }

            ////
        }

        ////////

        Rectangle {
            id: clickRowUnderlay
            anchors.left: parent.left
            anchors.right: parent.right

            height: Theme.componentHeight + Theme.componentRadius*2
            radius: Theme.componentRadius
            color: Theme.colorSeparator
            z: -1

            RowLayout { // physical click buttons
                id: clickRow
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                spacing: 0

                ButtonClear {
                    Layout.preferredWidth: 1
                    Layout.fillWidth: true
                    color: Theme.colorSeparator
                    colorText: "white"
                    colorHighlight: Qt.rgba(colorText.r, colorText.g, colorText.b, 0.2)

                    text: qsTr("Left")
                    onClicked: networkControls.sendMouseClick(0)
                }
                ButtonClear {
                    Layout.preferredWidth: 1
                    Layout.fillWidth: true
                    color: Theme.colorSeparator
                    colorText: "white"
                    colorHighlight: Qt.rgba(colorText.r, colorText.g, colorText.b, 0.2)

                    text: qsTr("Right")
                    onClicked: networkControls.sendMouseClick(1)
                }
            }
        }

        ////////
    }

    ////////////////

    Column { // virtual keyboard
        width: gridSize
        height: gridSize
        spacing: 16

        ////////

        ButtonClear { // open OS virtual keyboard
            anchors.left: parent.left
            anchors.right: parent.right

            text: qsTr("keyboard")
            source: "qrc:/assets/gfx/icons/keyboard-variant.svg"

            colorBackground: virtualtextfield.focus ? Theme.colorPrimary : "white"
            colorHighlight: virtualtextfield.focus ? "white" : Theme.colorPrimary
            colorBorder: virtualtextfield.focus ? Theme.colorPrimary : Theme.colorComponentBorder
            colorText: virtualtextfield.focus ? "white" : Theme.colorPrimary

            onClicked: {
                if (virtualtextfield.focus) {
                    virtualtextfield.focus = false
                    virtualtextfield.clear()
                } else {
                    virtualtextfield.forceActiveFocus()
                }
            }
        }

        ////////

        TextField { // virtual text field
            id: virtualtextfield
            anchors.left: parent.left
            anchors.right: parent.right

            visible: false // < set to invisible
            focus: false
            cursorVisible: false
            echoMode: TextInput.NoEcho

            background: Item {
                // older hack to fake invisibility
            }

            onDisplayTextChanged: {
                if (displayText) {
                    //console.log("virtualtextfield::onDisplayTextChanged(" + displayText + ")")

                    // send virtual event
                    networkControls.sendKey(displayText)
                    clear()
                }
            }
            onEditingFinished: {
                virtualtextfield.focus = false
                clear()
            }
            onAccepted: {
                virtualtextfield.focus = false
                clear()
            }
            Keys.onReturnPressed: {
                virtualtextfield.focus = false
                clear()
            }
        }

        ////////

        RowLayout { // buttons
            anchors.left: parent.left
            anchors.right: parent.right

            ButtonClear {
                Layout.preferredWidth: 1
                Layout.fillWidth: true

                PopupConfirmation {
                    id: confirmAction

                    property int action: 0

                    onConfirmed: {
                        if (action > 0) {
                            networkControls.sendAction(action)
                            confirmAction.action = 0
                        }
                    }
                    onClosed: {
                        confirmAction.action = 0
                    }
                }

                text: qsTr("Lock")
                onPressAndHold: {
                    confirmAction.action = LocalActions.ACTION_KEYBOARD_computer_lock
                    confirmAction.open()
                }
            }

            ButtonClear {
                Layout.preferredWidth: 1
                Layout.fillWidth: true

                text: qsTr("Sleep")
                onPressAndHold: {
                    confirmAction.action = LocalActions.ACTION_KEYBOARD_computer_sleep
                    confirmAction.open()
                }
            }

            ButtonClear {
                Layout.preferredWidth: 1
                Layout.fillWidth: true

                text: qsTr("Shutdown")
                onPressAndHold: {
                    confirmAction.action = LocalActions.ACTION_KEYBOARD_computer_poweroff
                    confirmAction.open()
                }
            }
        }

        ////////
    }

    ////////////////
}
