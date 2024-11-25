import QtQuick
import QtQuick.Controls

import ZXingCpp

import ComponentLibrary

Image {
    id: settingsQrCode

    width: 256
    height: 256

    ////////

    cache: true
    smooth: false

    sourceSize.width: width
    sourceSize.height: height
    fillMode: Image.PreserveAspectFit

    source: "image://ZXingCpp/encode/" + barcode_string + barcode_settings

    ////////

    property string barcode_string: "lighthouse://" + networkServer.serverAddress +
                                    "/" + networkServer.serverPort

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
