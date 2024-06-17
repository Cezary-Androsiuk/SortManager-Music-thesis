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
    }

    BottomPlayer {
        id: bottomPlayer
    }
}
