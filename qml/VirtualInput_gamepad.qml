import QtQuick
import QtQuick.Controls

import ThemeEngine

Item {
    id: virtualInput_gamepad

    function backAction() {
        screenDeviceList.loadScreen()
    }

    ////////

    Column {
        anchors.centerIn: parent
        spacing: 24

        AnalogPad { }

        AnalogPad { }
    }

    ////////
}
