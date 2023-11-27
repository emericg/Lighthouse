import QtQuick
import QtQuick.Controls

import ThemeEngine
import "qrc:/js/UtilsNumber.js" as UtilsNumber

Loader {
    id: screenAbout

    ////////////////////////////////////////////////////////////////////////////

    function loadScreen() {
        // load screen
        screenAbout.active = true

        // change screen
        appContent.state = "About"
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

                        source: "qrc:/assets/icons_material/desk-lamp.svg"
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

                    ButtonWireframeIconCentered {
                        width: 160
                        sourceSize: 28
                        fullColor: true
                        primaryColor: (Theme.currentTheme === ThemeEngine.THEME_NIGHT) ? Theme.colorHeader : "#5483EF"

                        text: qsTr("WEBSITE")
                        source: "qrc:/assets/icons_material/baseline-insert_link-24px.svg"
                        onClicked: Qt.openUrlExternally("https://emeric.io/Lighthouse")
                    }

                    ButtonWireframeIconCentered {
                        width: 160
                        sourceSize: 22
                        fullColor: true
                        primaryColor: (Theme.currentTheme === ThemeEngine.THEME_NIGHT) ? Theme.colorHeader : "#5483EF"

                        text: qsTr("SUPPORT")
                        source: "qrc:/assets/icons_material/baseline-support-24px.svg"
                        onClicked: Qt.openUrlExternally("https://github.com/emericg/Lighthouse/issues")
                    }

                    ButtonWireframeIconCentered {
                        visible: (appWindow.width > 800)
                        width: 160
                        sourceSize: 22
                        fullColor: true
                        primaryColor: (Theme.currentTheme === ThemeEngine.THEME_NIGHT) ? Theme.colorHeader : "#5483EF"

                        text: qsTr("GitHub")
                        source: "qrc:/assets/logos/github.svg"
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

                ButtonWireframeIconCentered {
                    anchors.verticalCenter: parent.verticalCenter
                    width: ((parent.width - parent.spacing) / 2)

                    sourceSize: 28
                    fullColor: true
                    primaryColor: (Theme.currentTheme === ThemeEngine.THEME_NIGHT) ? Theme.colorHeader : "#5483EF"

                    text: qsTr("WEBSITE")
                    source: "qrc:/assets/icons_material/baseline-insert_link-24px.svg"
                    onClicked: Qt.openUrlExternally("https://emeric.io/Lighthouse")
                }
                ButtonWireframeIconCentered {
                    anchors.verticalCenter: parent.verticalCenter
                    width: ((parent.width - parent.spacing) / 2)

                    sourceSize: 22
                    fullColor: true
                    primaryColor: (Theme.currentTheme === ThemeEngine.THEME_NIGHT) ? Theme.colorHeader : "#5483EF"

                    text: qsTr("SUPPORT")
                    source: "qrc:/assets/icons_material/baseline-support-24px.svg"
                    onClicked: Qt.openUrlExternally("https://github.com/emericg/Lighthouse/issues")
                }
            }

            ////////////////

            ListItem { // description
                width: parent.width
                text: qsTr("Control your computer from anywhere.")
                iconSource: "qrc:/assets/icons_material/outline-info-24px.svg"
            }

            ListItemClickable { // authors
                width: parent.width

                text: qsTr("Application by <a href=\"https://emeric.io\">Emeric Grange</a>")
                iconSource: "qrc:/assets/icons_material/baseline-supervised_user_circle-24px.svg"
                indicatorSource: "qrc:/assets/icons_material/duotone-launch-24px.svg"

                onClicked: Qt.openUrlExternally("https://emeric.io")
            }

            ListItemClickable { // rate
                width: parent.width
                visible: (Qt.platform.os === "android" || Qt.platform.os === "ios")

                text: qsTr("Rate the application")
                iconSource: "qrc:/assets/icons_material/baseline-stars-24px.svg"
                indicatorSource: "qrc:/assets/icons_material/duotone-launch-24px.svg"

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
                iconSource: "qrc:/assets/icons_material/baseline-import_contacts-24px.svg"
                iconSize: 28
                indicatorSource: "qrc:/assets/icons_material/baseline-chevron_right-24px.svg"

                onClicked: screenTutorial.loadScreenFrom("About")
            }
*/
            ////////

            ListSeparator { }

            ListItemClickable { // release notes
                width: parent.width

                text: qsTr("Release notes")
                iconSource: "qrc:/assets/icons_material/outline-new_releases-24px.svg"
                iconSize: 28
                indicatorSource: "qrc:/assets/icons_material/duotone-launch-24px.svg"

                onClicked: Qt.openUrlExternally("https://github.com/emericg/Lighthouse/releases")
            }

            ListSeparator { visible: (Qt.platform.os === "android") }

            ListItemClickable { // permissions
                width: parent.width
                visible: (Qt.platform.os === "android")

                text: qsTr("About app permissions")
                iconSource: "qrc:/assets/icons_material/baseline-flaky-24px.svg"
                iconSize: 28
                indicatorSource: "qrc:/assets/icons_material/baseline-chevron_right-24px.svg"

                onClicked: screenAboutPermissions.loadScreenFrom("About")
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
                    width: 28
                    height: 28
                    anchors.left: parent.left
                    anchors.leftMargin: 2
                    anchors.verticalCenter: dependenciesText.verticalCenter

                    source: "qrc:/assets/icons_material/baseline-settings-20px.svg"
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

            ListSeparator { }

            Column { // list debug stuff
                anchors.left: parent.left
                anchors.leftMargin: screenPaddingLeft + appHeader.headerPosition
                anchors.right: parent.right
                anchors.rightMargin: screenPaddingRight + Theme.componentMargin

                visible: settingsManager.showDebug
                spacing: Theme.componentMargin * 0.4

                Item { width: 8; height: 8; } // padding

                Text {
                    color: Theme.colorSubText
                    text: "build mode: %1".arg(utilsApp.appBuildModeFull())
                    font.pixelSize: Theme.fontSizeContent
                }
                Text {
                    color: Theme.colorSubText
                    text: "build date: %1".arg(utilsApp.appBuildDateTime())
                    font.pixelSize: Theme.fontSizeContent
                }
                Text {
                    color: Theme.colorSubText
                    text: "build mode: %1".arg(utilsApp.appBuildModeFull())
                    font.pixelSize: Theme.fontSizeContent
                }
                Text {
                    color: Theme.colorSubText
                    text: "Qt version: %1".arg(utilsApp.qtVersion())
                    font.pixelSize: Theme.fontSizeContent
                }
            }

            ////////
        }
    }
}
