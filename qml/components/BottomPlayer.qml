import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material
import Qt5Compat.GraphicalEffects

// ImageButton
// VolumeSlider
import "Player" // SongTitle

Item{
    id: bottomPlayer
    anchors{
        left: parent.left
        right: parent.right
        bottom: parent.bottom
    }
    height: root._h * 0.2

    // title
    readonly property double titleFieldTopMarginRatio: 0.05
    readonly property double titleFieldHeightRatio: 0.3
    // slider
    readonly property double sliderFieldTopMarginRatio: -0.05
    readonly property double sliderFieldHeightRatio: 0.25
    // controls
    readonly property double controlsFieldBottomMarginRatio: 0.1
    readonly property double controlsFieldHeightRatio: 0.35
    readonly property int controlsDistanceBetweenControls: 15
    readonly property double nonPlayTopControlsSizeRatio: 0.8

    property bool stoppedBySlider: false // useful when user decided that song should stop, while changing song position

    readonly property bool showAreas: !root.globalVisibleChanger


    Rectangle{
        id: topSmoothFadeIn
        anchors{
            left: background.left
            bottom: background.top
            right: background.right
        }
        height: 20
        
        gradient: Gradient{
            orientation: Gradient.Vertical
            GradientStop { position: 0.00; color: root.color_background_opacity(0) }
            GradientStop { position: 1.00; color: root.color_background_opacity(255) }
        }
    }

    Rectangle{
        id: background
        anchors.fill: parent
        color: root.color_background
    }


    Item{
        id: mainField
        anchors.fill: parent

        Item{
            id: thumbnailField
            anchors{
                top: parent.top
                bottom: parent.bottom
                left: parent.left
            }
            width: height

            Item{
                anchors{
                    top: parent.top
                    bottom: parent.bottom
                    left: parent.left
                    margins: 10
                }
                width: height

                RectangularGlow{
                    id: fadeInImage
                    anchors{
                        fill: parent
                        margins: 10
                    }
                    glowRadius: 15
                    spread: 0.2
                    color: root.color_accent1
                    cornerRadius: glowRadius + thumbnailMask.radius - 1.8*anchors.margins
                }

                Rectangle{
                    id: thumbnailMask
                    anchors{
                        fill: parent
                        margins: 3 // removes white lines around
                    }
                    radius: width * 0.1
                }

                Image{
                    id: thumbnail
                    fillMode: Image.PreserveAspectCrop
                    anchors.fill: parent
                    source: {
                        if(backend.player.thumbnail === "")
                        {
                            if(root.dark_theme)
                                "qrc:/SortManager-Music/assets/noSongThumbnailDark.png"
                            else
                                "qrc:/SortManager-Music/assets/noSongThumbnailLight.png"
                        }
                        else
                            backend.player.thumbnail
                    }
                    layer.enabled: true
                    layer.effect: OpacityMask {
                        maskSource: thumbnailMask
                        cached: true
                    }
                }
            }

            Rectangle{anchors.fill: parent; color: "red"; opacity: 0.2; visible: showAreas}
        }

        Item{
            id: titleField
            anchors{
                top: parent.top
                topMargin: parent.height * bottomPlayer.titleFieldTopMarginRatio
                left: thumbnailField.right
                leftMargin: parent.width * 0
                right: parent.right
                rightMargin: parent.width * 0.01
            }
            height: parent.height * bottomPlayer.titleFieldHeightRatio
            clip: true
            Rectangle{anchors.fill: parent; color: "green"; opacity: 0.2; visible: showAreas}
            SongTitle{
                isPlayerLarge: false
            }
        }

        Item{
            id: sliderField
            anchors{
                top: titleField.bottom
                topMargin: parent.height * bottomPlayer.sliderFieldTopMarginRatio
                left: thumbnailField.right
                leftMargin: parent.width * 0.01
                right: parent.right
                rightMargin: parent.width * 0.01
            }
            height: parent.height * bottomPlayer.sliderFieldHeightRatio
            Rectangle{anchors.fill: parent; color: "blue"; opacity: 0.2; visible: showAreas}
            Slider{
                id: slider
                anchors{
                    fill: parent
                    leftMargin: parent.height * 0.2
                    rightMargin: parent.height * 0.2
                    topMargin: -10
                }
                from: 0
                to: backend.player.duration
                value: backend.player.position
                onPressedChanged: {
                    if(!backend.personalization.stopSongWhileSeek)
                        return

                    if(!backend.player.isPlaying)
                        return;

                    if(pressed)
                    {
                        bottomPlayer.stoppedBySlider = true;
                        backend.player.play();
                    }
                    else
                    {
                        if(bottomPlayer.stoppedBySlider)
                        {
                            bottomPlayer.stoppedBySlider = false;
                            backend.player.play();
                        }
                    }
                }
                onMoved: {
                    backend.player.position = value
                }
            }

            Text{
                id: positionText
                anchors{
                    top: slider.bottom
                    topMargin: -12
                    left: slider.left
                    leftMargin: 5
                }
                height: parent.height/2
                width: 40
                text: backend.player.displayPosition
                font.pixelSize: 10
                color: root.color_accent1
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                Rectangle{anchors.fill: parent; color: "yellow"; opacity: 0.2; visible: showAreas}
            }

            Text{
                id: durationText
                anchors{
                    top: slider.bottom
                    topMargin: -12
                    right: slider.right
                    rightMargin: 5
                }
                height: parent.height/2
                width: 40
                text: backend.player.displayDuration
                font.pixelSize: 10
                color: root.color_accent1
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                Rectangle{anchors.fill: parent; color: "yellow"; opacity: 0.2; visible: showAreas}
            }
        }

        Item{
            id: controlsField
            anchors{
                left: thumbnailField.right
                leftMargin: parent.width * 0.01
                right: parent.right
                rightMargin: parent.width * 0.01
                bottom: parent.bottom
                bottomMargin: parent.height * bottomPlayer.controlsFieldBottomMarginRatio
            }
            height: parent.height * bottomPlayer.controlsFieldHeightRatio
            Rectangle{anchors.fill: parent; color: "pink"; opacity: 0.2; visible: showAreas}
            Item{
                id: prevField
                anchors{
                    verticalCenter: parent.verticalCenter
                    right: playField.left
                    rightMargin: bottomPlayer.controlsDistanceBetweenControls
                }
                height: parent.height * bottomPlayer.nonPlayTopControlsSizeRatio
                width: height

                Rectangle{anchors.fill: parent; color: "blue"; opacity: 0.2; visible: showAreas}
                ImageButton{
                    id: prevImage
                    dltImageIdle: Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/player/start_64px.png")
                    dltImageHover: dltImageIdle
                    dltImageMarginsRatio: 0.12
                    onUserClicked: {
                        backend.player.restartSong()
                    }
                }
            }
            Item{
                id: playField
                anchors.centerIn: parent
                height: parent.height
                width: height

                Rectangle{anchors.fill: parent; color: "blue"; opacity: 0.2; visible: showAreas}
                ImageButton{
                    id: playImage
                    dltImageIdle: {
                        if(backend.player.isPlaying)
                            Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/player/pause_64px.png")
                        else
                            Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/player/play_64px.png")
                    }
                    dltImageHover: dltImageIdle
                    dltImageMarginsRatio: 0.12
                    onUserClicked: {
                        backend.player.play();
                    }
                }
            }
            Item{
                id: nextField
                anchors{
                    verticalCenter: parent.verticalCenter
                    left: playField.right
                    leftMargin: bottomPlayer.controlsDistanceBetweenControls
                }
                height: parent.height * bottomPlayer.nonPlayTopControlsSizeRatio
                width: height

                Rectangle{anchors.fill: parent; color: "blue"; opacity: 0.2; visible: showAreas}
                ImageButton{
                    id: nextImage
                    dltImageIdle: Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/player/end_64px.png")
                    dltImageHover: dltImageIdle
                    dltImageMarginsRatio: 0.12
                    onUserClicked: {
                        backend.player.nextSong()
                    }
                }
            }

            Item{
                id: volumeSliderField
                anchors{
                    verticalCenter: parent.verticalCenter
                    right: parent.right
                    rightMargin: 10
                }
                height: parent.height * bottomPlayer.nonPlayTopControlsSizeRatio * 1.2
                width: height
                Rectangle{anchors.fill: parent; color: "red"; opacity: 0.2; visible: showAreas}
                VolumeSlider{

                }
            }
        }

    }
}
