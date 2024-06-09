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

    readonly property int bottomMainFieldHeight: bottomPlayer.height * 0.3
    readonly property bool showAreas: false
    property bool isPlaying

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
            // contains multiple GradientStops, because trasition doesn't looks linear like i expected
            GradientStop { position: 0.00; color: root.color_background_opacity(0) }
            GradientStop { position: 0.30; color: root.color_background_opacity(90) }
            GradientStop { position: 0.50; color: root.color_background_opacity(127) }
            GradientStop { position: 0.80; color: root.color_background_opacity(210) }
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
                    margins: 5
                }
                width: height

                // Rectangle{
                //     id: imageMask
                //     anchors{
                //         fill: parent
                //         margins: 3 // removes white lines around
                //     }
                //     radius: width * 0.3
                // }

                Image{
                    fillMode: Image.PreserveAspectCrop
                    anchors.fill: parent

                    source: {
                        if(backend.player.thumbnail === "")
                            "qrc:/SortManager-Music/assets/noSongThumbnail.png"
                        else
                            backend.player.thumbnail
                    }

                    // layer.enabled: true
                    // layer.effect: OpacityMask {
                    //     maskSource: imageMask
                    //     cached: true
                    // }
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

// Rectangle {

//     // Colors
//     property color color_background: root.dark_theme ? rgb(50,50,50) : rgb(220,220,220)
//     property color color_controls: root.dark_theme ? rgb(255,255,255) : rgb(0,0,0)

//     anchors{
//         left: parent.left
//         right: parent.right
//         bottom: parent.bottom
//     }
//     height: root._h * 0.2
//     color: color_background
//     property bool isPlaying: backend.player.isPlaying

//     Rectangle{
//         id: topMask
//         anchors{
//             left: parent.left
//             bottom: parent.top
//             right: parent.right
//         }
//         height: 30

//         gradient: Gradient{
//             orientation: Gradient.Vertical
//             GradientStop { position: 0.0; color: "transparent" }
//             GradientStop { position: 1.0; color: root.color_background }
//         }
//     }
//     Rectangle{
//         anchors{
//             top: parent.top
//             left: parent.left
//             right: parent.right
//             bottom: sliderContainer.top
//         }
//         // color: Qt.rgba(.9, .1, .1, .5)
//         color: color_background

//         Item{
//             id: imageField
//             anchors{
//                 top: parent.top
//                 right: audioControls.left
//                 bottom: parent.bottom
//                 margins: parent.height * 0.1
//             }
//             width: height

//             Rectangle{
//                 id: imageMask
//                 anchors{
//                     fill: parent
//                     margins: 3 // removes white lines around
//                 }

//                 radius: width /3.4

//             }

//             Image{
//                 fillMode: Image.PreserveAspectCrop
//                 anchors.fill: parent

//                 source: "file:C:/0_Vigiland - Friday Night‬‬‬/Vigiland - Friday Night‬‬‬ (BQ).jpg"

//                 layer.enabled: true
//                 layer.effect: OpacityMask {
//                     maskSource: imageMask
//                     cached: true
//                 }
//             }
//         }

//         Rectangle{
//             id: audioControls
//             anchors{
//                 top: parent.top
//                 bottom: parent.bottom
//                 right: parent.right
//             }
//             // color: "blue"
//             color: color_background
//             width: parent.width * 0.75

//             Item{
//                 id: leftField
//                 anchors{
//                     verticalCenter: centerField.verticalCenter
//                     right: centerField.left
//                     rightMargin: 40
//                 }
//                 width: 30
//                 height: 30

//                 Rectangle{
//                     anchors.fill: parent
//                     color: "red"
//                 }
//             }
//             Item{
//                 id: centerField
//                 anchors{
//                     centerIn: parent
//                     topMargin: 20
//                 }

//                 width: 30
//                 height: 30

//                 Rectangle{
//                     anchors.fill: parent
//                     color: "green"
//                 }
//             }
//             Item{
//                 id: rightField
//                 anchors{
//                     verticalCenter: centerField.verticalCenter
//                     left: centerField.right
//                     leftMargin: 40
//                 }
//                 width: 30
//                 height: 30

//                 Rectangle{
//                     anchors.fill: parent
//                     color: "blue"
//                 }
//             }

//             // Image{
//             //     id: playIcon
//             //     source: isPlaying
//             //             ? Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/player/pause_64px.png")
//             //             : Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/player/play_64px.png")

//             //     anchors.centerIn: parent
//             //     width: 30
//             //     height: 30
//             //     fillMode: Image.PreserveAspectFit
//             //     MouseArea{
//             //         anchors.fill: parent
//             //         onClicked: {
//             //             isPlaying = !isPlaying
//             //             console.log( "is playing: " + isPlaying )
//             //         }
//             //     }
//             // }
//             Image{
//                 id: prevIcon
//                 source: Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/player/start_64px.png")

//                 anchors{
//                     verticalCenter: playIcon.verticalCenter
//                     right: playIcon.left
//                     rightMargin: 40
//                 }

//                 width: 30
//                 height: 30
//                 fillMode: Image.PreserveAspectFit
//                 MouseArea{
//                     anchors.fill: parent
//                     onClicked: {
//                         console.log( "previous song" )
//                     }
//                 }
//             }
//             Image{
//                 id: nextIcon
//                 source: Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/player/end_64px.png")

//                 anchors{
//                     verticalCenter: playIcon.verticalCenter
//                     left: playIcon.right
//                     leftMargin: 40
//                 }

//                 width: 30
//                 height: 30
//                 fillMode: Image.PreserveAspectFit
//                 MouseArea{
//                     anchors.fill: parent
//                     onClicked: {
//                         console1.log( "next song" )
//                     }
//                 }
//             }

//             ColorOverlay {
//                 anchors.fill: playIcon
//                 source: playIcon
//                 color: color_controls
//             }
//             ColorOverlay {
//                 anchors.fill: prevIcon
//                 source: prevIcon
//                 color: color_controls
//             }
//             ColorOverlay {
//                 anchors.fill: nextIcon
//                 source: nextIcon
//                 color: color_controls
//             }
//         }



//         Text{
//             id: title
//             anchors{
//                 bottom: audioControls.bottom
//                 left: audioControls.left
//                 right: audioControls.right
//             }
//             height: audioControls.height/3

//             font.pixelSize: 10
//             text: backend.player.title
//         }
//     }

//     Rectangle{
//         id: sliderContainer
//         anchors{
//             left: parent.left
//             right: parent.right
//             bottom: parent.bottom
//         }
//         height: parent.height * 0.35
//         // color: Qt.rgba(.1, .9, .1, .5)
//         color: color_background

//         Slider{
//             id: slider
//             anchors{
//                 fill: parent
//                 leftMargin: parent.width * 0.1
//                 rightMargin: parent.width * 0.1
//             }
//         }
//     }
// }


