import QtQuick
import QtQuick.Effects
import QtQuick.Dialogs

import QtQuick.Controls.impl
import QtQuick.Templates as T

import ThemeEngine
import "qrc:/js/UtilsPath.js" as UtilsPath

T.TextField {
    id: control

    implicitWidth: implicitBackgroundWidth + leftInset + rightInset
                   || Math.max(contentWidth, placeholder.implicitWidth) + leftPadding + rightPadding
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding,
                             placeholder.implicitHeight + topPadding + bottomPadding)

    clip: false
    padding: 12
    leftPadding: padding + 4

    text: ""
    color: colorText
    font.pixelSize: Theme.componentFontSize

    placeholderText: ""
    placeholderTextColor: colorPlaceholderText

    selectByMouse: true
    selectionColor: colorSelection
    selectedTextColor: colorSelectedText

    onEditingFinished: focus = false

    property alias folder: control.text
    property alias file: fileArea.text
    property string path: folder

    // settings
    property int fileMode: FileDialog.OpenFile
    property int buttonWidth: (buttonChange.visible ? buttonChange.width : 0)

    // colors
    property string colorText: Theme.colorComponentText
    property string colorPlaceholderText: Theme.colorSubText
    property string colorBorder: Theme.colorComponentBorder
    property string colorBackground: Theme.colorComponentBackground
    property string colorSelectedText: Theme.colorHighContrast
    property string colorSelection: Theme.colorPrimary

    ////////////////////////////////////////////////////////////////////////////

    Loader {
        id: fileDialogLoader

        active: false
        asynchronous: false
        sourceComponent: FileDialog {
            title: qsTr("Please choose a file!")
            currentFolder: UtilsPath.makeUrl(control.text)
            fileMode: FileDialog.OpenFile

            //fileMode: FileDialog.OpenFile / FileDialog.OpenFiles / FileDialog.SaveFile
            //flags: FileDialog.HideNameFilterDetails

            onAccepted: {
                //console.log("fileDialog URL: " + selectedFile)

                var f = UtilsPath.cleanUrl(selectedFile)
                //if (f.slice(0, -1) !== "/") f += "/"

                control.text = f
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    PlaceholderText {
        id: placeholder
        x: control.leftPadding
        y: control.topPadding
        width: control.width - (control.leftPadding + control.rightPadding)
        height: control.height - (control.topPadding + control.bottomPadding)

        text: control.placeholderText
        font: control.font
        color: control.placeholderTextColor
        verticalAlignment: control.verticalAlignment
        visible: !control.length && !control.preeditText && (!control.activeFocus || control.horizontalAlignment !== Qt.AlignHCenter)
        elide: Text.ElideRight
        renderType: control.renderType
    }

    ////////////////////////////////////////////////////////////////////////////

    background: Rectangle {
        implicitWidth: 256
        implicitHeight: Theme.componentHeight

        radius: Theme.componentRadius
        color: control.colorBackground

        Rectangle {
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            width: buttonWidth
            color: Theme.colorComponent
        }

        Rectangle {
            anchors.fill: parent
            color: "transparent"
            radius: Theme.componentRadius
            border.width: 2
            border.color: (control.activeFocus || fileArea.activeFocus) ? Theme.colorPrimary : control.colorBorder
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
                    x: background.x
                    y: background.y
                    width: background.width
                    height: background.height
                    radius: background.radius
                }
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    TextInput {
        id: fileArea
        anchors.top: control.top
        anchors.bottom: control.bottom

        x: control.leftPadding + control.contentWidth
        width: control.width - control.buttonWidth - x - 12

        clip: true
        autoScroll: false
        color: Theme.colorSubText
        verticalAlignment: Text.AlignVCenter

        selectByMouse: true
        selectionColor: control.colorSelection
        selectedTextColor: control.colorSelectedText

        onTextChanged: {
            control.textChanged()
        }
        onEditingFinished: {
            focus = false
        }
    }

    ButtonThemed {
        id: buttonChange
        anchors.top: parent.top
        anchors.topMargin: 2
        anchors.right: parent.right
        anchors.rightMargin: 2
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 2

        height: control.height
        visible: control.enabled
        text: qsTr("choose")

        onClicked: {
            fileDialogLoader.active = true
            fileDialogLoader.item.open()
        }
    }
}
