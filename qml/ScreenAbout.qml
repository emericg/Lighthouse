import QtQuick
import QtQuick.Controls

import ThemeEngine
import "qrc:/utils/UtilsNumber.js" as UtilsNumber

Loader {
    id: screenAbout
    anchors.fill: parent

    ////////////////////////////////////////////////////////////////////////////

    function loadScreen() {
        // load screen
        screenAbout.active = true

        // change screen
        appContent.state = "ScreenAbout"
    }

    ////////

    function backAction() {
        if (screenAbout.status === Loader.Ready)
            screenAbout.item.backAction()
    }

    ////////////////////////////////////////////////////////////////////////////

    active: false
    asynchronous: false

    sourceComponent: Flickable {
        anchors.fill: parent
        contentWidth: -1
        contentHeight: contentColumn.height

        boundsBehavior: isDesktop ? Flickable.OvershootBounds : Flickable.DragAndOvershootBounds
        ScrollBar.vertical: ScrollBar { visible: false }

        function backAction() {
            screenDeviceList.loadScreen()
        }

        Column {
            id: contentColumn
            anchors.left: parent.left
            anchors.right: parent.right

            ////////////////

            Rectangle { // header
                anchors.left: parent.left
                anchors.right: parent.right

                height: 128
                color: headerUnicolor ? Theme.colorBackground : Theme.colorForeground

                Row { // logo
                    anchors.left: parent.left
                    anchors.leftMargin: Theme.componentMargin
                    anchors.verticalCenter: parent.verticalCenter

                    z: 2
                    height: 128
                    spacing: Theme.componentMargin

                    IconSvg {
                        id: imageLogo
                        width: 128
                        height: 128
                        anchors.verticalCenter: parent.verticalCenter

                        source: "qrc:/assets/gfx/icons/desk-lamp-logo.svg"
                        //sourceSize: Qt.size(width, height)
                        color: Theme.colorIcon
                    }

                    Column { // title
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.verticalCenterOffset: 2

                        Text {
                            text: "Lighthouse"
                            color: Theme.colorText
                            font.pixelSize: 28
                        }
                        Text {
                            color: Theme.colorSubText
                            text: qsTr("version %1 %2").arg(utilsApp.appVersion()).arg(utilsApp.appBuildMode())
                            font.pixelSize: Theme.fontSizeContentBig
                        }
                    }
                }

                ////////

                Row { // desktop buttons row
                    anchors.right: parent.right
                    anchors.rightMargin: Theme.componentMargin
                    anchors.verticalCenter: parent.verticalCenter

                    visible: wideWideMode
                    spacing: Theme.componentMargin

                    ButtonSolid {
                        width: 160
                        height: 40

                        //color: (Theme.currentTheme === ThemeEngine.THEME_NIGHT) ? Theme.colorHeader : "#5483EF"

                        text: qsTr("WEBSITE")
                        source: "qrc:/assets/icons/material-symbols/link.svg"
                        sourceSize: 28

                        onClicked: Qt.openUrlExternally("https://emeric.io/Lighthouse")
                    }

                    ButtonSolid {
                        width: 160
                        height: 40

                        //color: (Theme.currentTheme === ThemeEngine.THEME_NIGHT) ? Theme.colorHeader : "#5483EF"

                        text: qsTr("SUPPORT")
                        source: "qrc:/assets/icons/material-symbols/support.svg"
                        sourceSize: 22

                        onClicked: Qt.openUrlExternally("https://github.com/emericg/Lighthouse/issues")
                    }

                    ButtonSolid {
                        visible: (appWindow.width > 800)
                        width: 160
                        height: 40

                        //color: (Theme.currentTheme === ThemeEngine.THEME_NIGHT) ? Theme.colorHeader : "#5483EF"

                        text: qsTr("GitHub")
                        source: "qrc:/assets/gfx/logos/github.svg"
                        sourceSize: 22

                        onClicked: Qt.openUrlExternally("https://github.com/emericg/Lighthouse")
                    }
                }

                ////////

                Rectangle { // bottom separator
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: 2
                    visible: isDesktop
                    color: Theme.colorSeparator
                }
            }

            ////////////////

            Row { // mobile buttons row
                height: 72

                anchors.left: parent.left
                anchors.leftMargin: screenPaddingLeft + Theme.componentMargin
                anchors.right: parent.right
                anchors.rightMargin: screenPaddingRight + Theme.componentMargin

                visible: !wideWideMode
                spacing: Theme.componentMargin

                ButtonFlat {
                    anchors.verticalCenter: parent.verticalCenter
                    width: ((parent.width - parent.spacing) / 2)
                    height: 40

                    text: qsTr("WEBSITE")
                    color: (Theme.currentTheme === ThemeEngine.THEME_NIGHT) ? Theme.colorHeader : "#5483EF"
                    source: "qrc:/assets/icons/material-symbols/link.svg"
                    sourceSize: 28

                    onClicked: Qt.openUrlExternally("https://emeric.io/Lighthouse")
                }
                ButtonFlat {
                    anchors.verticalCenter: parent.verticalCenter
                    width: ((parent.width - parent.spacing) / 2)
                    height: 40

                    text: qsTr("SUPPORT")
                    color: (Theme.currentTheme === ThemeEngine.THEME_NIGHT) ? Theme.colorHeader : "#5483EF"
                    source: "qrc:/assets/icons/material-symbols/support.svg"
                    sourceSize: 22

                    onClicked: Qt.openUrlExternally("https://github.com/emericg/Lighthouse/issues")
                }
            }

            ////////////////

            ListItem { // description
                width: parent.width
                text: qsTr("Control your computer from anywhere.")
                source: "qrc:/assets/icons/material-symbols/info.svg"
            }

            ListItemClickable { // authors
                width: parent.width

                text: qsTr("Application by <a href=\"https://emeric.io\">Emeric Grange</a>")
                source: "qrc:/assets/icons/material-symbols/supervised_user_circle.svg"
                indicatorSource: "qrc:/assets/icons/material-icons/duotone/launch.svg"

                onClicked: Qt.openUrlExternally("https://emeric.io")
            }

            ListItemClickable { // rate
                width: parent.width
                visible: (Qt.platform.os === "android" || Qt.platform.os === "ios")

                text: qsTr("Rate the application")
                source: "qrc:/assets/icons/material-symbols/stars-fill.svg"
                indicatorSource: "qrc:/assets/icons/material-icons/duotone/launch.svg"

                onClicked: {
                    if (Qt.platform.os === "android")
                        Qt.openUrlExternally("market://details?id=io.emeric.lighthouse")
                    else if (Qt.platform.os === "ios")
                        Qt.openUrlExternally("itms-apps://itunes.apple.com/app/1476046123")
                    else
                        Qt.openUrlExternally("https://github.com/emericg/Lighthouse/stargazers")
                }
            }
/*
            ListItemClickable { // tutorial
                width: parent.width

                text: qsTr("Open the tutorial")
                source: "qrc:/assets/icons/material-symbols/import_contacts.svg"
                iconSize: 28
                indicatorSource: "qrc:/assets/icons/material-symbols/chevron_right.svg"

                onClicked: screenTutorial.loadScreenFrom("ScreenAbout")
            }
*/

            ListItemClickable { // release notes
                width: parent.width

                text: qsTr("Release notes")
                source: "qrc:/assets/icons/material-symbols/new_releases.svg"
                sourceSize: 28
                indicatorSource: "qrc:/assets/icons/material-icons/duotone/launch.svg"

                onClicked: Qt.openUrlExternally("https://github.com/emericg/Lighthouse/releases")
            }

            ////////

            ListSeparator { visible: (Qt.platform.os === "android") }

            ListItemClickable { // permissions
                width: parent.width
                visible: (Qt.platform.os === "android")

                text: qsTr("About app permissions")
                source: "qrc:/assets/icons/material-symbols/flaky.svg"
                sourceSize: 28
                indicatorSource: "qrc:/assets/icons/material-symbols/chevron_right.svg"

                onClicked: screenAboutPermissions.loadScreenFrom("ScreenAbout")
            }

            ////////

            ListSeparator { }

            Item { // list dependencies
                anchors.left: parent.left
                anchors.leftMargin: screenPaddingLeft + Theme.componentMargin
                anchors.right: parent.right
                anchors.rightMargin: screenPaddingRight + Theme.componentMargin

                height: 40 + dependenciesText.height + dependenciesColumn.height

                IconSvg {
                    width: 24
                    height: 24
                    anchors.left: parent.left
                    anchors.leftMargin: 4
                    anchors.verticalCenter: dependenciesText.verticalCenter

                    source: "qrc:/assets/icons/material-symbols/settings.svg"
                    color: Theme.colorSubText
                }

                Text {
                    id: dependenciesText
                    anchors.top: parent.top
                    anchors.topMargin: 16
                    anchors.left: parent.left
                    anchors.leftMargin: appHeader.headerPosition - parent.anchors.leftMargin
                    anchors.right: parent.right
                    anchors.rightMargin: 8

                    text: qsTr("This application is made possible thanks to a couple of third party open source projects:")
                    textFormat: Text.PlainText
                    color: Theme.colorSubText
                    font.pixelSize: Theme.fontSizeContent
                    wrapMode: Text.WordWrap
                }

                Column {
                    id: dependenciesColumn
                    anchors.top: dependenciesText.bottom
                    anchors.topMargin: 8
                    anchors.left: parent.left
                    anchors.leftMargin: appHeader.headerPosition - parent.anchors.leftMargin
                    anchors.right: parent.right
                    anchors.rightMargin: 8
                    spacing: 4

                    Repeater {
                        model: [
                            "Qt6 (LGPL v3)",
                            "MobileUI (MIT)",
                            "MobileSharing (MIT)",
                            "SingleApplication (MIT)",
                            "Google Material Icons (MIT)",
                        ]
                        delegate: Text {
                            width: parent.width
                            text: "- " + modelData
                            textFormat: Text.PlainText
                            color: Theme.colorSubText
                            font.pixelSize: Theme.fontSizeContent
                            wrapMode: Text.WordWrap
                        }
                    }
                }
            }

            ////////

            ListSeparatorPadded { }

            Item { // list debug stuff
                anchors.left: parent.left
                anchors.leftMargin: screenPaddingLeft + Theme.componentMargin
                anchors.right: parent.right
                anchors.rightMargin: screenPaddingRight + Theme.componentMargin

                height: 16 + debugColumn.height

                IconSvg {
                    width: 24
                    height: 24
                    anchors.top: debugColumn.top
                    anchors.topMargin: 4
                    anchors.left: parent.left
                    anchors.leftMargin: 4

                    source: "qrc:/assets/icons/material-symbols/info.svg"
                    color: Theme.colorSubText
                }

                Column {
                    id: debugColumn
                    anchors.left: parent.left
                    anchors.leftMargin: appHeader.headerPosition - parent.anchors.leftMargin
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter

                    spacing: Theme.componentMargin * 0.4

                    Text {
                        color: Theme.colorSubText
                        text: "App name: %1".arg(utilsApp.appName())
                        font.pixelSize: Theme.fontSizeContent
                    }
                    Text {
                        color: Theme.colorSubText
                        text: "App version: %1".arg(utilsApp.appVersion())
                        font.pixelSize: Theme.fontSizeContent
                    }
                    Text {
                        color: Theme.colorSubText
                        text: "Build mode: %1".arg(utilsApp.appBuildModeFull())
                        font.pixelSize: Theme.fontSizeContent
                    }
                    Text {
                        color: Theme.colorSubText
                        text: "Build date: %1".arg(utilsApp.appBuildDateTime())
                        font.pixelSize: Theme.fontSizeContent
                    }
                    Text {
                        color: Theme.colorSubText
                        text: "Qt version: %1".arg(utilsApp.qtVersion())
                        font.pixelSize: Theme.fontSizeContent
                    }
                }
            }

            ListSeparatorPadded { }

            ////////
        }
    }
}
