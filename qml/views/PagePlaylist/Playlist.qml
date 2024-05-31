import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material
import Qt5Compat.GraphicalEffects

import "qrc:/SortManager-Music/qml/popups"
import "qrc:/SortManager-Music/qml/components" // ImageButton
import "qrc:/SortManager-Music/qml/delegates/Playlist"
import "components" // PlaylistHeader

Page {
    id: pagePlaylist
    anchors.fill: parent

    property int delegateHeight: 40
    property int headerHeight: 45
    property int headerSeparatorSpace: 1
    property int delegateWidth: width

    property var mdl: []
    property int mdlLength: 0

    property double startListPosition: root.last_pos_playlist - headerHeight
    property bool saveListPosition: false
    property bool alwaysSaveListPosition: backend.personalization.alwaysKeepListPos

    /*
        received structure:

        backend.database.all_songs_model - SongList:
            songs - QList<Song *>:
                id - int: from SELECT
                title - QString: from SELECT
                value - QString: ""

        SELECT song_id, value AS title FROM songs_tags WHERE tag_id = 2;
    */

    Component.onCompleted: {
        console.log("Playlist.qml")

        // for(var _song of backend.database.playlist_model.songs)
        // {
        //     mdl.push(_song);
        //     mdlLength += 1
        // }
        console.log("looped over songs and pushed to array")

        listViewLoader.active = true
        console.log("actived loader")
    }


    Connections{
        target: backend.database

        // edit playlist song
        function onSignalEditPlaylistSongModelLoaded()
        {
            saveListPosition = true;
            // change page to EditPlaylistSong
            playlistLoader.source = root.pppp_editplaylistsong
        }
        function onSignalEditPlaylistSongModelLoadError(desc)
        {
            pLoadEditPlaylistSongError.open()
            pLoadEditPlaylistSongError.dltDesc = desc
        }

        // filters
        function onSignalFiltersModelLoaded()
        {
            saveListPosition = true;
            // change page to Filters
            playlistLoader.source = root.pppp_filters
        }
        function onSignalFiltersModelLoadError(desc)
        {
            pLoadFilterError.open();
            pLoadFilterError.dltDesc = desc;
        }
    }

    property int just_used_id: 0
    property string just_used_title: ""

    Popup2{
        id: pLoadFilterError
        dltText: "Error while loading filters model!"
        // text description will be set in onSignalFilterModelLoadError() function

        dltTextLB: "OK"
        dltTextRB: "Retry"

        onDltClickedLB: {

        }
        onDltClickedRB: {
            backend.database.loadFiltersModel();
        }
    }

    Popup2{
        id: pLoadEditPlaylistSongError
        dltText:  "Error while loading edit playlist song(id: " +
                      just_used_id + ", title: " + just_used_title + ") model!"
        // text description will be set in onSignalEditPlaylistSongModelLoadError() function

        dltTextLB: "OK"
        dltTextRB: "Retry"

        onDltClickedLB: {

        }
        onDltClickedRB: {
            backend.database.loadEditPlaylistSongModel(+just_used_id)
        }
    }

    Loader{
        id: listViewLoader
        anchors{
            left: parent.left
            top: parent.top
            right: parent.right
            bottom: parent.bottom
        }
        sourceComponent: listViewComponent
        active: false
    }

    Component{
        id: listViewComponent
        Item{
            anchors.fill: parent
            EmptyListInfo{
                id: emptyListInfo
                visible: (mdlLength === 0)
                text: "No Songs to Display"
            }

            ScrollView{
                id: scrollView
                anchors.fill: parent

                ListView{
                    id: playlistListView
                    model: mdl

                    clip: true

                    boundsBehavior: Flickable.StopAtBounds

                    header: PlaylistHeader{}

                    footer: Item{
                        width: parent.width - 15 /*scrollbar offset*/
                        height: delegateHeight/2
                    }


                    Component.onCompleted: {
                        console.log("completed listview")
                        contentY = startListPosition
                    }
                    Component.onDestruction: {
                        console.log("destroing listview")
                        // test if action wants to save position
                        if(saveListPosition)
                        {
                            root.last_pos_playlist = contentY + headerHeight;
                        }
                        else
                        {
                            // test if app is forced to keep list position
                            if(alwaysSaveListPosition)
                                root.last_pos_playlist = contentY + headerHeight;
                            else
                                root.last_pos_playlist = 0;
                        }

                    }

                    delegate: Loader{
                        width: delegateWidth - 15 /*scrollbar offset*/
                        height: delegateHeight

                        sourceComponent: Item{
                            id: buttonFieldDelegate
                            ButtonField{
                                dltText: modelData.title
                                onDltClickedElement: {
                                    just_used_id = + modelData.id;
                                    just_used_title = modelData.title;

                                    backend.database.loadEditPlaylistSongModel(+modelData.id)
                                }
                                onDltClickedPlay: {
                                    console.log("play: " + modelData.title)
                                }
                            }
                        }
                    }
                }
            }
        }

    }
}
