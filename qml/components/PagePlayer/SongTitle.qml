import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material

Item {
    id: songTitle
    anchors.fill: parent

    readonly property int songID: backend.player.songID
    readonly property string title: backend.player.title
    readonly property int delayBetweenAnimations: 3500
    readonly property int animationSpeed: 4500
    readonly property double widthAnimationDelay: 50

    readonly property bool showAreas: !root.globalVisibleChanger
    property bool enableAnimation

    Connections{
        target: backend.player
        function onSongStarted(){
            // on song change reset everything
            animationMoveLeft.stop() // not pause but stop
            timerAnimationDelay.stop()

            animatedTextContainer.x = 0
            enableAnimation = text.contentWidth > textContainer.width


            if(enableAnimation){
                // only when text.contentWidth > textContainer.width
                console.log((text.contentWidth - textContainer.width))
                animationMoveLeft.duration = songTitle.animationSpeed * (text.contentWidth / textContainer.width)
                timerAnimationDelay.start()
            }
        }
    }

    Timer{
        id: timerAnimationDelay
        interval: songTitle.delayBetweenAnimations
        onTriggered: animationMoveLeft.start()
    }

    Item{
        id: textContainer
        anchors{
            top: parent.top
            bottom: parent.bottom
            left: parent.left
            leftMargin: 55
            right: parent.right
            rightMargin: 55
        }

        Rectangle{anchors.fill: parent; color: "blue"; opacity: 0.2; visible: showAreas}

        Item{
            id: animatedTextContainer
            width: parent.width
            height: parent.height

            Item{
                id: centerTextContainer
                anchors.horizontalCenter: parent.horizontalCenter
                width: {
                    if(parent.width > text.contentWidth)
                        text.contentWidth
                    else
                        parent.width
                }
                height: parent.height

                Text{
                    id: text
                    anchors{
                        top: parent.top
                        bottom: parent.bottom
                        left: parent.left
                    }

                    text: title
                    font.pixelSize: parent.height * 0.5
                    color: root.color_accent1
                }

                Text{
                    id: textClone
                    visible: songTitle.enableAnimation
                    width: text.width
                    height: text.height
                    x: text.contentWidth + textContainer.anchors.rightMargin + songTitle.widthAnimationDelay
                    y: text.y
                    text: text.text
                    font.pixelSize: text.font.pixelSize
                    color: text.color
                }
            }

        }

        PropertyAnimation{
            id: animationMoveLeft
            target: animatedTextContainer
            property: "x"
            from: 0
            to: 0 - text.contentWidth - textContainer.anchors.rightMargin - songTitle.widthAnimationDelay
            onFinished: timerAnimationDelay.start()
        }

    }



    Rectangle{
        id: leftTextMask
        anchors{
            top: parent.top
            bottom: parent.bottom
            left: parent.left
        }
        width: parent.width * 0.1
        visible: (text.contentWidth >= text.width) || true

        gradient: Gradient{
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: root.color_background }
            GradientStop { position: 0.2; color: root.color_background }
            GradientStop { position: 1.0; color: "transparent" }
        }
    }

    Rectangle{
        id: rightTextMask
        anchors{
            top: parent.top
            bottom: parent.bottom
            right: parent.right
        }
        width: parent.width * 0.1
        visible: (text.contentWidth >= text.width) || true

        gradient: Gradient{
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: "transparent" }
            GradientStop { position: 0.8; color: root.color_background }
            GradientStop { position: 1.0; color: root.color_background }
        }
    }
}
