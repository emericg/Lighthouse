import QtQuick
import QtQuick.Effects
import QtQuick.Controls

import QtMultimedia
import ZXingCpp

import ComponentLibrary

Rectangle {
    id: qrreader
    anchors.left: parent.left
    anchors.right: parent.right

    height: width / 1.333
    radius: 8
    color: Theme.colorForeground

    CaptureSession {
        id: captureSession

        videoOutput: videoOutput

        camera: Camera {
            id: camera

            active: true
            focusMode: Camera.FocusModeAutoNear

            onErrorOccurred: (errorString) => {
                console.log("CaptureSession::onErrorOccurred() " + errorString)
            }
        }
    }

    VideoOutput {
        id: videoOutput
        width: parent.width
        height: parent.height
        orientation: 0

        fillMode: VideoOutput.PreserveAspectCrop
        //fillMode: VideoOutput.PreserveAspectFit

        // Capture rectangle
        property double captureRectStartFactorX: 0.05
        property double captureRectStartFactorY: 0.20
        property double captureRectFactorWidth: 0.9
        property double captureRectFactorHeight: 0.45

        // Capture rectangles
        property rect captureRect
        property rect captureRect_full: Qt.rect(0, 0, videoOutput.sourceRect.width, videoOutput.sourceRect.height)
        property rect captureRect_mapped
        property rect captureRect_mapped_str

        property rect captureRect_mobile_top: Qt.rect(0.05, 0.20, 0.9, 0.45) // default
        property rect captureRect_mobile_mid: Qt.rect(0.05, 0.25, 0.9, 0.5)
        property rect captureRect_wide_left: Qt.rect(0.10, 0.12, 0.5, 0.76)
        property rect captureRect_wide_right: Qt.rect(0.4, 0.12, 0.5, 0.76)
    }

    ZXingCppVideoFilter {
        id: barcodeReader

        videoSink: videoOutput.videoSink
        captureRect: Qt.rect(videoOutput.sourceRect.x, videoOutput.sourceRect.y,
                             videoOutput.sourceRect.width, videoOutput.sourceRect.height)
                     //Qt.rect(videoOutput.sourceRect.width * videoOutput.captureRectStartFactorX,
                     //        videoOutput.sourceRect.height * videoOutput.captureRectStartFactorY,
                     //        videoOutput.sourceRect.width * videoOutput.captureRectFactorWidth,
                     //        videoOutput.sourceRect.height * videoOutput.captureRectFactorHeight)

        tryHarder: true
        tryRotate: true
        tryInvert: true
        tryDownscale: true

        formats: ZXingCpp.QRCode

        onTagFound: (result) => {
            //console.log("onTagFound : " + result)

            if (result.isValid && result.text !== "") {
                //result.text, result.formatName, result.contentType

                utilsApp.vibrate(33)

                if (settingsManager.setNetCtrlSettings(result.text)) {
                    buttonReader.highlighted = false
                    networkClient.connectToServer()
                }
            }
        }

        onDecodingFinished: (result) => {
            //console.log("ZXingCpp::onDecodingFinished(" + result.isValid + " / " + result.runTime + " ms)")
        }
    }

    Text {
        anchors.left: parent.left
        anchors.leftMargin: Theme.componentMargin*2
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Theme.componentMargin*1.5

        text: "Scanning..."
        color: "white"

        Rectangle {
            anchors.fill: parent
            anchors.margins: -Theme.componentMargin*0.5
            anchors.leftMargin: -Theme.componentMargin
            anchors.rightMargin: -Theme.componentMargin
            radius: height
            z: -1
            color: "black"
            opacity: 0.66
        }
    }

    layer.enabled: true
    layer.effect: MultiEffect {
        maskEnabled: true
        maskInverted: false
        maskThresholdMin: 0.5
        maskSpreadAtMin: 1.0
        maskSpreadAtMax: 0.0
        maskSource: ShaderEffectSource {
            sourceItem: Rectangle {
                x: qrreader.x
                y: qrreader.y
                width: qrreader.width
                height: qrreader.height
                radius: qrreader.radius
            }
        }
    }
}
