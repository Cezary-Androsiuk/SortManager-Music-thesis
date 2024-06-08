import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material
import Qt5Compat.GraphicalEffects

Rectangle {

    // Colors
    property color color_background: root.dark_theme ? rgb(50,50,50) : rgb(220,220,220)
    property color color_controls: root.dark_theme ? rgb(255,255,255) : rgb(0,0,0)

    anchors{
        left: parent.left
        right: parent.right
        bottom: parent.bottom
    }
    height: root._h * 0.2
    color: color_background
    property bool isPlaying: false

    Rectangle{
        anchors{
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: sliderContainer.top
        }
        // color: Qt.rgba(.9, .1, .1, .5)
        color: color_background

        Rectangle{
            id: imageContainer
            anchors{
                top: parent.top
                right: audioControls.left
                bottom: parent.bottom
                margins: parent.height * 0.1
            }
            width: height
            radius: width /3.4

            Image{
                fillMode: Image.PreserveAspectCrop
                anchors.fill: parent

                source: "file:C:/0_Vigiland - Friday Night‬‬‬/Vigiland - Friday Night‬‬‬ (BQ).jpg"

                layer.enabled: true
                layer.effect: OpacityMask {
                    maskSource: parent
                    cached: true
                }
            }
        }

        Rectangle{
            id: audioControls
            anchors{
                top: parent.top
                bottom: parent.bottom
                right: parent.right
            }
            // color: "blue"
            color: color_background
            width: parent.width * 0.75

            Image{
                id: playIcon
                source: isPlaying
                        ? Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/player/pause_64px.png")
                        : Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/player/play_64px.png")

                anchors.centerIn: parent
                width: 30
                height: 30
                fillMode: Image.PreserveAspectFit
                MouseArea{
                    anchors.fill: parent
                    onClicked: {
                        isPlaying = !isPlaying
                        console.log( "is playing: " + isPlaying )
                    }
                }
            }
            Image{
                id: prevIcon
                source: Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/player/start_64px.png")

                anchors{
                    verticalCenter: playIcon.verticalCenter
                    right: playIcon.left
                    rightMargin: 40
                }

                width: 30
                height: 30
                fillMode: Image.PreserveAspectFit
                MouseArea{
                    anchors.fill: parent
                    onClicked: {
                        console.log( "previous song" )
                    }
                }
            }
            Image{
                id: nextIcon
                source: Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/player/end_64px.png")

                anchors{
                    verticalCenter: playIcon.verticalCenter
                    left: playIcon.right
                    leftMargin: 40
                }

                width: 30
                height: 30
                fillMode: Image.PreserveAspectFit
                MouseArea{
                    anchors.fill: parent
                    onClicked: {
                        console1.log( "next song" )
                    }
                }
            }

            ColorOverlay {
                anchors.fill: playIcon
                source: playIcon
                color: color_controls
            }
            ColorOverlay {
                anchors.fill: prevIcon
                source: prevIcon
                color: color_controls
            }
            ColorOverlay {
                anchors.fill: nextIcon
                source: nextIcon
                color: color_controls
            }
        }

        Text{
            id: title
            anchors{
                bottom: audioControls.bottom
                left: audioControls.left
                right: audioControls.right
            }
            height: audioControls.height/3

            font.pixelSize: 10
            text: backend.player.title
        }
    }

    Rectangle{
        id: sliderContainer
        anchors{
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        height: parent.height * 0.35
        // color: Qt.rgba(.1, .9, .1, .5)
        color: color_background

        Slider{
            id: slider
            anchors{
                fill: parent
                leftMargin: parent.width * 0.1
                rightMargin: parent.width * 0.1
            }
        }
    }


}
