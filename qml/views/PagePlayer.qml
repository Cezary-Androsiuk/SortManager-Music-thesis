import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import Qt5Compat.GraphicalEffects

Page {
    property var backend: Backend
    anchors.fill: parent
    // Label {
    //     id: label
    //     text: qsTr("Middle Player")
    //     anchors.bottom: imageContainer.top
    //     anchors.horizontalCenter: parent.horizontalCenter
    // }
    // OptionButton{
    //     anchors.fill: label
    //     idle: root.color_background
    //     hover: root.color_mouse_hover
    //     press: root.color_mouse_press
    //     onClicked:{
    //         console.log("hi")
    //     }
    // }


    property color color_controls: root.dark_theme ? rgb(255,255,255) : rgb(0,0,0)
    property bool isPlaying: false
    property bool isPlaying2: false
    // Rectangle{
    //     id: imageContainer
    //     anchors{
    //         horizontalCenter: parent.horizontalCenter
    //         top: parent.top
    //         topMargin: parent.height * 1/8
    //     }

    //     width: parent.width * 2/3
    //     height: width
    //     radius: width /3.4

    //     Image{
    //         fillMode: Image.PreserveAspectCrop
    //         anchors.fill: parent

    //         source: "file:C:/0_Vigiland - Friday Night‬‬‬/Vigiland - Friday Night‬‬‬ (BQ).jpg"

    //         layer.enabled: true
    //         layer.effect: OpacityMask {
    //             maskSource: parent
    //             cached: true
    //         }
    //     }
    // }

    Item{
        id: imageContainer
        anchors{
            horizontalCenter: parent.horizontalCenter
            top: parent.top
            topMargin: parent.height * 1/8
        }

        width: parent.width * 2/3
        height: width

        Image{
            fillMode: Image.PreserveAspectCrop
            anchors.fill: parent

            source: "file:C:/0_Vigiland - Friday Night‬‬‬/Vigiland - Friday Night‬‬‬ (BQ).jpg"
        }
    }

    Rectangle{
        id: audioControls
        anchors{
            horizontalCenter: parent.horizontalCenter
            top: imageContainer.bottom
            topMargin: 40
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
                    if(isPlaying)
                        backend.player.pause();
                    else
                        backend.player.play();
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
                    if(isPlaying2)
                        backend.player.pause2();
                    else
                        backend.player.play2();
                    isPlaying2 = !isPlaying2
                    console.log( "is playing2: " + isPlaying2 )

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
                    console.log( "next song" )
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
