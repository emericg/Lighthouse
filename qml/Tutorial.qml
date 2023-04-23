import QtQuick
import QtQuick.Controls

import ThemeEngine 1.0

Rectangle {
    width: 480
    height: 720
    anchors.fill: parent

    color: Theme.colorHeader

    property string entryPoint: "DeviceList"

    ////////////////////////////////////////////////////////////////////////////

    function loadScreen() {
        entryPoint = "DeviceList"
        appContent.state = "Tutorial"
    }

    function loadScreenFrom(screenname) {
        entryPoint = screenname
        appContent.state = "Tutorial"
    }

    ////////////////////////////////////////////////////////////////////////////

    Loader {
        id: tutorialLoader
        anchors.fill: parent

        active: (appContent.state === "Tutorial")
        asynchronous: true

        sourceComponent: Item {
            id: itemTutorial

            function reset() {
                tutorialPages.disableAnimation()
                tutorialPages.currentIndex = 0
                tutorialPages.enableAnimation()
            }

            ////////////////

            SwipeView {
                id: tutorialPages
                anchors.fill: parent
                anchors.leftMargin: screenPaddingLeft
                anchors.rightMargin: screenPaddingRight
                anchors.bottomMargin: 56
                property int margins: isPhone ? 24 : 40

                currentIndex: 0
                onCurrentIndexChanged: {
                    if (currentIndex < 0) currentIndex = 0
                    if (currentIndex > count-1) {
                        currentIndex = 0 // reset
                        appContent.state = entryPoint
                    }
                }

                function enableAnimation() {
                    contentItem.highlightMoveDuration = 333
                }
                function disableAnimation() {
                    contentItem.highlightMoveDuration = 0
                }

                ////////

                Item {
                    id: page1

                    Text {
                        anchors.centerIn: parent

                        text: qsTr("There is no tutorial.")
                        textFormat: Text.PlainText
                        color: Theme.colorHeaderContent
                        font.bold: true
                        font.pixelSize: Theme.fontSizeContent
                    }
                }

                Item {
                    id: page2

                    Text {
                        anchors.centerIn: parent

                        text: qsTr("I lied.")
                        textFormat: Text.PlainText
                        color: Theme.colorHeaderContent
                        font.bold: true
                        font.pixelSize: Theme.fontSizeContent
                    }
                }
            }

            ////////////////

            Text {
                id: pagePrevious
                anchors.left: parent.left
                anchors.leftMargin: tutorialPages.margins
                anchors.verticalCenter: pageIndicator.verticalCenter

                visible: (tutorialPages.currentIndex !== 0)

                text: qsTr("Previous")
                textFormat: Text.PlainText
                color: Theme.colorHeaderContent
                font.bold: true
                font.pixelSize: Theme.fontSizeContent

                opacity: 0.8
                Behavior on opacity { OpacityAnimator { duration: 133 } }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: parent.opacity = 1
                    onExited: parent.opacity = 0.8
                    onCanceled: parent.opacity = 0.8
                    onClicked: tutorialPages.currentIndex--
                }
            }

            PageIndicatorThemed {
                id: pageIndicator
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: tutorialPages.margins/2

                count: tutorialPages.count
                currentIndex: tutorialPages.currentIndex
            }

            Text {
                id: pageNext
                anchors.right: parent.right
                anchors.rightMargin: tutorialPages.margins
                anchors.verticalCenter: pageIndicator.verticalCenter

                text: (tutorialPages.currentIndex === tutorialPages.count-1) ? qsTr("Start") : qsTr("Next")
                textFormat: Text.PlainText
                color: Theme.colorHeaderContent
                font.bold: true
                font.pixelSize: Theme.fontSizeContent

                opacity: 0.8
                Behavior on opacity { OpacityAnimator { duration: 133 } }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: parent.opacity = 1
                    onExited: parent.opacity = 0.8
                    onCanceled: parent.opacity = 0.8
                    onClicked: tutorialPages.currentIndex++
                }
            }

            ////////////////
        }
    }
}
