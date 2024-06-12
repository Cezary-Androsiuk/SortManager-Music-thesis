import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material
import Qt5Compat.GraphicalEffects

import "qrc:/SortManager-Music/qml/components" // ImageButton

Item{
    id: bottomPlayer
    anchors{
        left: parent.left
        right: parent.right
        bottom: parent.bottom
    }
    height: root._h * 0.2

    property bool isPlaying
    readonly property int bottomMainFieldHeight: bottomPlayer.height * 0.3
    readonly property bool showAreas: false

    Component.onCompleted: {
        // set isPlaying this way, because if isPlaying is constantly readed from
        // backend then within song change, play button blinks (playing is changed to pause for a moment)
        isPlaying = backend.player.isPlaying
    }


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
            id: leftMainField
            anchors{
                top: parent.top
                bottom: parent.bottom
                left: parent.left
            }
            width: height

            Item{
                id: thumbnailField
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
            id: rightMainField
            anchors{
                top: parent.top
                bottom: parent.bottom
                left: leftMainField.right
                right: parent.right
            }

            Item{
                id: controlsField
                anchors{
                    top: parent.top
                    right: parent.right
                    left: parent.left
                    bottom: sliderField.top
                    rightMargin: 40
                    leftMargin: 40
                }

                Item{
                    id: prevField
                    anchors{
                        top: parent.top
                        left: parent.left
                        bottom: parent.bottom
                    }
                    width: parent.width / 4

                    ImageButton{
                        id: prevImage
                        dltImageIdle: Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/player/start_64px.png")
                        dltImageHover: dltImageIdle
                        onUserClicked: {
                            backend.player.restartSong()
                        }
                    }
                }
                Item{
                    id: playField
                    anchors{
                        top: parent.top
                        bottom: parent.bottom
                        left: prevField.right
                        right: nextField.left
                    }

                    ImageButton{
                        id: playImage
                        dltImageIdle: {
                            if(isPlaying)
                                Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/player/pause_64px.png")
                            else
                                Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/player/play_64px.png")
                        }

                        dltImageHover: dltImageIdle
                        onUserClicked: {
                            backend.player.play();
                            bottomPlayer.isPlaying = !bottomPlayer.isPlaying
                        }
                    }
                }
                Item{
                    id: nextField
                    anchors{
                        top: parent.top
                        right: parent.right
                        bottom: parent.bottom
                        margins: 10
                    }
                    width: parent.width / 4

                    ImageButton{
                        id: nextImage
                        dltImageIdle: Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/player/end_64px.png")
                        dltImageHover: dltImageIdle
                        onUserClicked: {
                            backend.player.nextSong()
                        }
                    }
                }

                Rectangle{anchors.fill: parent; color: "blue"; opacity: 0.2; visible: showAreas}
            }

            Item{
                id: sliderField
                anchors{
                    right: parent.right
                    bottom: parent.bottom
                    left: parent.left
                }
                height: bottomPlayer.bottomMainFieldHeight

                Rectangle{anchors.fill: parent; color: "green"; opacity: 0.2; visible: showAreas}

                Slider{
                    id: slider
                    anchors{
                        fill: parent
                        leftMargin: parent.height * 0.2
                        rightMargin: parent.height * 0.2
                    }
                }
            }

        }
    }

}
