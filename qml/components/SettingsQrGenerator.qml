import QtQuick

import ZXingQt

Image {
    id: settingsQrGenerator

    width: 256
    height: 256

    ////////

    cache: true
    smooth: false
    fillMode: Image.PreserveAspectFit

    source: "image://ZXingQt/encode/" + barcode_string + barcode_settings
    sourceSize.width: width
    sourceSize.height: height

    ////////

    property string barcode_string: "lighthouse://" + networkServer.serverAddress + "/"
                                                    + networkServer.serverPort

    property string barcode_settings: "?" + "format=" + format + "&"
                                          + "eccLevel=" + eccLevel + "&"
                                          + "margins=" + margins + "&"
                                          + "backgroundColor=" + colorBg + "&"
                                          + "foregroundColor=" + colorFg

    property string format: "qrcode"
    property int eccLevel: 0
    property int margins: 12
    property color colorBg: "#fff"
    property color colorFg: "#000"

    ////////
}
