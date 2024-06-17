import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import Qt5Compat.GraphicalEffects

import "qrc:/SortManager-Music/qml/components" // ImageButton, VolumeSlider
import "qrc:/SortManager-Music/qml/components/Player" // SongTitle

Page {
    id: pagePlayer
    anchors.fill: parent

    property bool isPlaying
    // thumbnail
    readonly property double thumbnailFieldTopMarginRatio: 0.075
    readonly property double thumbnailFieldHeightRatio: 0.54
    // title
    readonly property double titleFieldTopMarginRatio: 0.035
    readonly property double titleFieldHeightRatio: 0.08
    // slider
    readonly property double sliderFieldTopMarginRatio: 0.016
    readonly property double sliderFieldHeightRatio: 0.05
    // controls
    readonly property double controlsFieldTopMarginRatio: 0.02
    readonly property double controlsFieldHeightRatio: 0.11
    readonly property int controlsDistanceBetweenControls: 25
    readonly property double nonPlayTopControlsSizeRatio: 0.8

    property bool stoppedBySlider: false // useful when user decided that song should stop, while changing song position

    readonly property bool showAreas: !root.globalVisibleChanger

    Component.onCompleted: {
        // set isPlaying this way, because if isPlaying is constantly readed from
        // backend then within song change, play button blinks (playing is changed to pause for a moment)
        isPlaying = backend.player.isPlaying
        // in bottomPlayer is readed constantly because song can be stopped by reloading (and this mess up the play button)
    }

    Item{
        id: mainField
        anchors{
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: footer.top
        }

        Item{
            id: thumbnailField
            anchors{
                top: parent.top
                topMargin: parent.height * pagePlayer.thumbnailFieldTopMarginRatio
                left: parent.left
                right: parent.right
            }
            height: parent.height * pagePlayer.thumbnailFieldHeightRatio

            Rectangle{anchors.fill: parent; color: "red"; opacity: 0.2; visible: showAreas}

            Item{
                anchors{
                    top: parent.top
                    bottom: parent.bottom
                    horizontalCenter: parent.horizontalCenter
                    margins: 8
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
        }

        Item{
            id: titleField
            anchors{
                top: thumbnailField.bottom
                topMargin: parent.height * pagePlayer.titleFieldTopMarginRatio
                left: parent.left
                right: parent.right
            }
            height: parent.height * pagePlayer.titleFieldHeightRatio

            Rectangle{anchors.fill: parent; color: "green"; opacity: 0.2; visible: showAreas}

            SongTitle{
                isPlayerLarge: true
            }
        }

        Item{
            id: sliderField
            anchors{
                top: titleField.bottom
                topMargin: parent.height * pagePlayer.sliderFieldTopMarginRatio
                left: parent.left
                right: parent.right
            }
            height: parent.height * pagePlayer.sliderFieldHeightRatio

            Rectangle{anchors.fill: parent; color: "blue"; opacity: 0.2; visible: showAreas}

            Slider{
                id: slider
                anchors{
                    left: parent.left
                    leftMargin: parent.width * 0.12
                    right: parent.right
                    rightMargin: parent.width * 0.12
                    verticalCenter: parent.verticalCenter
                }
                from: 0
                to: backend.player.duration
                value: backend.player.position
                onPressedChanged: {
                    if(!backend.personalization.stopSongWhileSeek)
                        return

                    if(!isPlaying)
                        return;

                    if(pressed)
                    {
                        pagePlayer.stoppedBySlider = true;
                        backend.player.play();
                    }
                    else
                    {
                        if(pagePlayer.stoppedBySlider)
                        {
                            pagePlayer.stoppedBySlider = false;
                            backend.player.play();
                        }
                    }
                }
                onMoved: {
                    backend.player.position = value
                }
            }
        }

        Item{
            id: controlsField
            anchors{
                top: sliderField.bottom
                topMargin: parent.height * pagePlayer.controlsFieldTopMarginRatio
                left: parent.left
                right: parent.right
            }
            height: parent.height * pagePlayer.controlsFieldHeightRatio

            Rectangle{anchors.fill: parent; color: "red"; opacity: 0.2; visible: showAreas}

            Item{
                id: prevField
                anchors{
                    verticalCenter: parent.verticalCenter
                    right: playField.left
                    rightMargin: pagePlayer.controlsDistanceBetweenControls
                }
                height: parent.height * pagePlayer.nonPlayTopControlsSizeRatio
                width: height

                ImageButton{
                    id: prevIcon
                    dltImageIdle: Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/player/start_64px.png")
                    dltImageHover: dltImageIdle
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

                ImageButton{
                    id: playIcon
                    dltImageIdle: {
                        if(isPlaying)
                            Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/player/pause_64px.png")
                        else
                            Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/player/play_64px.png")
                    }
                    dltImageHover: dltImageIdle
                    onUserClicked: {
                        backend.player.play();
                        pagePlayer.isPlaying = !pagePlayer.isPlaying
                    }
                }
            }

            Item{
                id: nextField
                anchors{
                    verticalCenter: parent.verticalCenter
                    left: playField.right
                    leftMargin: pagePlayer.controlsDistanceBetweenControls
                }
                height: parent.height * pagePlayer.nonPlayTopControlsSizeRatio
                width: height

                ImageButton{
                    id: nextIcon
                    dltImageIdle: Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/player/end_64px.png")
                    dltImageHover: dltImageIdle
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
                    rightMargin: 20
                }
                height: parent.height * pagePlayer.nonPlayTopControlsSizeRatio
                width: height
                Rectangle{anchors.fill: parent; color: "red"; opacity: 0.2; visible: showAreas}
                VolumeSlider{

                }
            }
        }
    }

    Rectangle {
        id: footer
        anchors{
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        height: 30
        color: root.color_background

        Label {
            id: footerText
            anchors.centerIn: parent
            text: qsTr("Cezary Androsiuk (2024) UwB")
        }
    }
}
