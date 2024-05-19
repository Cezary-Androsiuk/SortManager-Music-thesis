import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

import "qrc:/SortManager-Music/qml/components"
import "qrc:/SortManager-Music/qml/popups"

Page {
    anchors.fill: parent

    function loadPlaylistModel(){
        // var filters = backend.database.playlist.filter;
        backend.database.loadPlaylistModel();
    }

    Component.onCompleted: {
        loadPlaylistModel();
    }

    Connections{
        target: backend.database

        function onSignalPlaylistModelLoaded()
        {
            playlistLoader.active = true;
        }
        function onSignalPlaylistModelLoadError(desc)
        {
            pLoadPlaylistError.open();
            pLoadPlaylistError.dltDesc = desc;
        }
    }

    Popup2{
        id: pLoadPlaylistError
        dltText: "Playlist model load error!"
        // text description will be set in onSignalPlaylistModelLoadError() function

        dltTextLB: "Back"
        dltTextRB: "Retry"

        onDltClickedLB: {
            root.current_main_loader_page = path_page_player;
        }
        onDltClickedRB: {
            loadPlaylistModel();
        }
    }

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
