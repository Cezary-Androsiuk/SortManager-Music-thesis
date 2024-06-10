import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import Qt5Compat.GraphicalEffects

import "qrc:/SortManager-Music/qml/components" // ImageButton

Page {
    id: pagePlayer
    anchors.fill: parent

    property bool isPlaying
    readonly property double
    readonly property bool showAreas: true

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
            height: parent.height * 0.65

            Item{
                id: thumbnailField
                anchors{
                    horizontalCenter: parent.horizontalCenter
                    top: parent.top
                    topMargin: parent.height * 1/8
                }

                width: parent.width * 2/3
                height: width

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

            Rectangle{
                id: audioControlsTop
                anchors{
                    top: parent.top
                    left: parent.left
                    right: parent.right
                }
                height:

                anchors{
                    horizontalCenter: parent.horizontalCenter
                    top: parent.top
                    topMargin: 40
                }
                // color: "blue"
                color: color_background
                width: parent.width * 0.75

                Item{
                    id: prevField
                    anchors{
                        verticalCenter: playField.verticalCenter
                        right: playField.left
                        rightMargin: 30
                    }
                    width: 40
                    height: 40

                    ImageButton{
                        id: prevIcon
                        dltImageIdle: Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/player/start_64px.png")
                        dltImageHover: dltImageIdle
                        onUserClicked: {
                            backend.player.play();
                            pagePlayer.isPlaying = !pagePlayer.isPlaying
                        }
                    }
                }

                Item{
                    id: playField
                    anchors.centerIn: parent
                    width: 50
                    height: 50

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
                        leftMargin: 30
                    }
                    width: 40
                    height: 40

                    ImageButton{
                        id: nextIcon
                        dltImageIdle: Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/player/end_64px.png")
                        dltImageHover: dltImageIdle
                        onUserClicked: {
                            backend.player.play();
                            pagePlayer.isPlaying = !pagePlayer.isPlaying
                        }
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
