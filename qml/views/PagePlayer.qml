import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import Qt5Compat.GraphicalEffects

import "qrc:/SortManager-Music/qml/components" // ImageButton

Page {
    id: pagePlayer
    anchors.fill: parent

    property bool isPlaying
    readonly property double thumbnailAreaHeightRatio: 0.6
    readonly property double audioControlsTopAreaHeightRatio: 0.26
    readonly property int audioControlsTopTopMargin: 15
    readonly property int audioControlsDistanceBetweenControls: 25
    readonly property double nonPlayTopControlsSizeRatio: 0.8
    readonly property bool showAreas: false

    Component.onCompleted: {
        // set isPlaying this way, because if isPlaying is constantly readed from
        // backend then within song change, play button blinks (playing is changed to pause for a moment)
        isPlaying = backend.player.isPlaying
    }

    Item{
        id: mainField
        anchors.fill: parent

        Item{
            id: topMainField
            anchors{
                top: parent.top
                left: parent.left
                right: parent.right
            }
            height: parent.height * pagePlayer.thumbnailAreaHeightRatio

            Item{
                id: thumbnailField
                anchors{
                    horizontalCenter: parent.horizontalCenter
                    top: parent.top
                    topMargin: parent.height * 0.16
                }

                width: parent.width * 0.67
                height: width

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
            id: bottomMainField
            anchors{
                top: topMainField.bottom
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }

            Item{
                Rectangle{anchors.fill: parent; color: "green"; opacity: 0.2; visible: showAreas}
                id: audioControlsTop
                anchors{
                    top: parent.top
                    left: parent.left
                    right: parent.right
                    topMargin: pagePlayer.audioControlsTopTopMargin
                }
                height: parent.height * pagePlayer.audioControlsTopAreaHeightRatio

                Item{
                    id: prevField
                    anchors{
                        verticalCenter: playField.verticalCenter
                        right: playField.left
                        rightMargin: pagePlayer.audioControlsDistanceBetweenControls
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
                        verticalCenter: playField.verticalCenter
                        left: playField.right
                        leftMargin: pagePlayer.audioControlsDistanceBetweenControls
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
            }
            Item{
                id: audioControlsBottom
                anchors{
                    bottom: parent.bottom
                    left: parent.left
                    right: parent.right
                    top: audioControlsTop.bottom

                    // i know, i konw, but i don't want oto reorganize layout again just because i didn't noticed anchors.fill: parent in mainField
                    bottomMargin: footer.height
                }

                Rectangle{anchors.fill: parent; color: "blue"; opacity: 0.2; visible: showAreas}

                Slider{
                    id: slider
                    anchors{
                        left: parent.left
                        leftMargin: parent.width * 0.08
                        right: parent.right
                        rightMargin: parent.width * 0.08
                        top: parent.top
                        topMargin: parent.height * 0.08
                    }
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
