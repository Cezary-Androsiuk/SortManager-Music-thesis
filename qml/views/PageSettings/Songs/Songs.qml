import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material

import "qrc:/SortManager-Music/qml/popups"
import "qrc:/SortManager-Music/qml/components" // EmptyListInfo
import "qrc:/SortManager-Music/qml/delegates"

Page {
    property var backend: Backend
    anchors.fill: parent

    property int delegateHeight: 60
    property int delegateWidth: width

    property var mdl: backend.database.all_songs_model.songs
    property int mdlLength: backend.database.all_songs_model.songsCount

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
        console.log("Songs.qml");
    }

    Connections{
        target: backend.database

        // add song
        function onSignalAddSongModelLoaded(){
            root.last_pos_songs = songsListView.contentY // set current scroll to restore after add song
            settingsLoader.source = root.ppsss_addsong
        }
        function onSignalAddSongModelLoadError(desc){
            pAddSongModelLoadError.open();
            pAddSongModelLoadError.dltDesc = desc;
        }

        // edit song
        function onSignalEditSongModelLoaded(){
            root.last_pos_songs = songsListView.contentY // set current scroll to restore after edit song
            settingsLoader.source = root.ppsss_editsong
        }
        function onSignalEditSongModelLoadError(desc){
            pEditSongModelLoadError.open();
            pEditSongModelLoadError.dltDesc = desc;
        }

        // delete song
        function onSignalDeletedSong(){
            backend.database.loadAllSongs()
        }
        function onSignalDeleteSongError(desc){
            pDeleteSongError.open();
            pDeleteSongError.dltDesc = desc;
        }

        // refresh page after any tag was deleted
        function onSignalAllSongsModelLoaded(){
            // force loader to refresh
            root.last_pos_songs = songsListView.contentY // set current scroll to restore after delete song
            settingsLoader.source = ""
            settingsLoader.source = root.ppss_songs
        }
        function onSignalAllSongsModelLoadError(desc){
            pAllSongsReloadError.open();
            pAllSongsReloadError.dltDesc = desc;
        }
    }

    // usefull properties for Popups
    property int just_used_id: 0
    property string just_used_title: ""

    Popup1{
        id: pAddSongModelLoadError
        dltText: "Error while loading add song model!"
        // text description will be set in onSignalAddSongModelLoadError() function
        dltTextMB: "OK"
    }
    Popup1{
        id: pEditSongModelLoadError
        dltText: "Error while loading edit song(id: " + just_used_id + ", title: " + just_used_title + ") model!"
        // text description will be set in onSignalEditSongModelLoadError() function
        dltTextMB: "OK"
    }
    Popup1{
        id: pDeleteSongError
        dltText: "Error while deleting song(id: " + just_used_id + ", title: " + just_used_title + ")!"
        // text description will be set in onSignalDeleteSongError() function
        dltTextMB: "OK"
    }
    Popup1{
        id: pAllSongsReloadError
        dltText: "Error while reloading all songs model!"
        // text description will be set in onSignalAllSongsModelLoadError() function
        dltTextMB: "OK"
    }

    Popup2{
        id: pDeleteConfirm
        dltText: "Sure you want to delete the '"+ just_used_title +"'(id: " + just_used_id + ") Song ?"

        dltTextLB: "Cancel"
        dltTextRB: "Delete"

        onDltClickedLB: {}
        onDltClickedRB: {
            backend.database.deleteSong(just_used_id);
        }
    }

    EmptyListInfo{
        id: emptyListInfo
        visible: (mdlLength === 0)
        text: "No Songs to Display"
    }

    ScrollView{
        id: scrollView
        anchors{
            left: parent.left
            top: listViewHeader.bottom
            right: parent.right
            bottom: parent.bottom
        }
        ListView{
            id: songsListView
            model: mdl
            clip: true

            boundsBehavior: Flickable.StopAtBounds

            footer: Item{
                width: parent.width - 15 /*scrollbar offset*/
                height: delegateHeight/2
            }

            Component.onCompleted: {
                contentY = root.last_pos_songs
                // scroll will be set before changing site
            }

            delegate: Item{
                width: delegateWidth - 15 /*scrollbar offset*/
                height: delegateHeight
                ButtonField{
                    dltText: modelData.title
                    onDltClickedElement: {
                        just_used_id = + modelData.id;
                        just_used_title = modelData.title;

                        // page is changed after signal was emited from loadEditSongModel()
                        backend.database.loadEditSongModel(+modelData.id) // + means parse int
                    }
                    onDltClickedDelete: {
                        // page refreshes after signal was emited from loadAllSongsModel()
                        if(!modelData.is_immutable)
                        {
                            just_used_id = + modelData.id;
                            just_used_title = modelData.title;

                            pDeleteConfirm.open();
                        }
                    }
                }
            }

        }
    }

    Rectangle{
        id: listViewHeader
        anchors{
            left: parent.left
            top: parent.top
            right: parent.right
        }
        height: delegateHeight * 4/5
        color: root.color_background

        Component.onCompleted: {
            mainLoader.anchors.top = tabBar.top;
        }

        TabButton{
            anchors{
                left: parent.left
                top: parent.top
                bottom: parent.bottom
            }
            width: height * 2

            text: "←"
            font.pixelSize: 30

            ToolTip.visible: hovered
            ToolTip.text: "Go Back"

            onClicked: {
                if(backend.personalization.alwaysKeepListPos)
                    root.last_pos_songs = songsListView.contentY // set current scroll to restore after back to page
                else
                    root.last_pos_songs = 0

                mainLoader.anchors.top = tabBar.bottom
                settingsLoader.source = root.pps_settings
            }
        }

        Text{
            text: "Songs"
            anchors.centerIn: parent
            font.pixelSize: 20
            color: root.color_accent1
        }

        TabButton{
            anchors{
                right: parent.right
                top: parent.top
                bottom: parent.bottom
            }
            width: height * 2

            text: "+"
            font.pixelSize: 30

            ToolTip.visible: hovered
            ToolTip.text: "Add Song"

            onClicked: {
                // page is changed after signal was emited from loadAddSongModel()
                backend.database.loadAddSongModel()
            }
        }
    }
}
