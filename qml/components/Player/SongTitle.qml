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
    required property bool isPlayerLarge
    required property bool isCurrentSong
    property double fadeAreaLenght
    property double stopPositionWhenAnimated
    property bool animated: false

    function loadPageOrChangeID(){
        // on song change reset everything, on initialize reset everything
        animationMoveLeft.stop() // not pause but stop
        timerAnimationDelay.stop()

        animatedTextContainer.x = 0
        animated = text.contentWidth > textContainer.width // need to be set here, because it wasn't refreshing properly

        if(isPlayerLarge){
            fadeAreaLenght = songTitle.animated ? songTitle.width * 0.1 : 0
            stopPositionWhenAnimated = songTitle.animated ? songTitle.width * 0.15 : 0
        }
        else{
            fadeAreaLenght = songTitle.animated ? songTitle.width * 0.05 : 0
            stopPositionWhenAnimated = songTitle.animated ? songTitle.width * 0.05 : 0
        }

        if(animated){
            // only when text.contentWidth > textContainer.width
            // console.log("######################################### timer will be started")
            animationMoveLeft.duration = songTitle.animationSpeed * (text.contentWidth / textContainer.width)
            timerAnimationDelay.start()
        }
        songTitle.visible = true;
    }

    onTitleChanged: {
        songTitle.visible = false; // to avoid blinking text while aligning
    }

    Component.onCompleted: {
        loadPageOrChangeID();
    }

    Connections{
        target: backend.player
        function onSongStarted(){
            loadPageOrChangeID();
        }
    }

    Timer{
        id: timerAnimationDelay
        interval: songTitle.delayBetweenAnimations
        onTriggered: {
            // console.log("######################################### animation triggered")
            animationMoveLeft.start()
        }
    }

    Item{
        id: textContainer
        anchors{
            top: parent.top
            bottom: parent.bottom
            left: parent.left
            leftMargin: songTitle.stopPositionWhenAnimated
            right: parent.right
            rightMargin: songTitle.stopPositionWhenAnimated
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
                    visible: songTitle.animated
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
            easing.type: Easing.InOutSine
            easing.period: 10 // idk if this changes anything
            onFinished: {
                // console.log("######################################### animation finished")
                timerAnimationDelay.start()
            }
        }
    }

    Rectangle{
        id: leftTextMask
        anchors{
            top: parent.top
            bottom: parent.bottom
            left: parent.left
        }
        width: songTitle.fadeAreaLenght
        visible: animated
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
        width: songTitle.fadeAreaLenght
        visible: animated
        gradient: Gradient{
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: "transparent" }
            GradientStop { position: 0.8; color: root.color_background }
            GradientStop { position: 1.0; color: root.color_background }
        }
    }
}
