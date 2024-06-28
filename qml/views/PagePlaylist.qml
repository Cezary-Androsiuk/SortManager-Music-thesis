import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

import "qrc:/SortManager-Music/qml/components" // BottomPlayer
import "qrc:/SortManager-Music/qml/popups"

Page {
    anchors.fill: parent

    Component.onCompleted: {
        console.log("PlaylistPage.qml")

        playlistLoader.active = true;
    }

    // Connections{
    //     target: backend.playlist

    //     function onPlaylistModelLoaded()
    //     {
    //         playlistLoader.active = true;
    //     }
    //     function onPlaylistModelLoadError(desc)
    //     {
    //         pLoadPlaylistError.open();
    //         pLoadPlaylistError.dltDesc = desc;
    //         playlistLoader.visible = false;
    //     }
    // }

    // Popup1{
    //     id: pLoadPlaylistError
    //     dltText: "Playlist model load error!"
    //     // text description will be set in onSignalPlaylistModelLoadError() function

    //     dltTextMB: "OK"

    //     onDltClickedMB: {}
    // }

    Loader{
        id: playlistLoader
        anchors{
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: bottomPlayer.top
        }
        source: root.ppp_playlist
        active: false
        onLoaded: {
            console.log("playlist loader loaded, error is fine (cause deleting objects is not working properly or something)")
            // swich play button to pause if needed
            playlistLoader.item.reloadPressed.connect(
                        function (){
                            bottomPlayer.switchIsPlayingToFalse()
                        })
        }
    }


    // will be hidden behind bottom player
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

    BottomPlayer {
        id: bottomPlayer
    }
}
