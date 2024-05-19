import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material

import "qrc:/SortManager-Music/qml/popups"
import "qrc:/SortManager-Music/qml/delegates"
import "qrc:/SortManager-Music/qml/components"

Page {
    id: pageEditPlaylistSong
    anchors.fill: parent

    property int delegateHeight: 60
    property int delegateWidth: width

    property var mdl: [] // mdl shortcut for model

    property double startListPosition: root.last_pos_edit_playlist_song - headerHeight
    property bool saveListPosition: false
    property bool alwaysSaveListPosition: Backend.personalization.alwaysKeepListPos

    /*
        received structure:

        Backend.database.m_edit_song_model - SongDetails:
            id - int: %1
            tags - TagList:
                tags - QList<Tag *>:
                    id - int: from SELECT
                    name - QString: from SELECT
                    value - QString: ""
                    type - int: from SELECT
                    is_immutable - bool: null (0)
                    is_editable - bool: from SELECT
                    is_required - bool: from SELECT

        SELECT tags.id, tags.name, tags.type, tags.is_editable, tags.is_required, songs_tags.value
        FROM songs_tags
        JOIN tags ON songs_tags.tag_id = tags.id
        WHERE songs_tags.song_id = %1
    */

    Component.onCompleted: {
        console.log("EditPlaylistSong.qml")
        if(Backend.database.m_edit_song_model.id === 0){
            console.log("  given song model id shouldn't be 0 but it is")
            root.close()
        }
        mainLoader.anchors.top = tabBar.top

        var tmp_mdl = Backend.database.m_edit_song_model.tags.tags

        for(var _tag of tmp_mdl){
            pageEditPlaylistSong.mdl.push(
                        {
                            delegate_type: "tag",
                            id: _tag.id,
                            name: _tag.name,
                            value: _tag.value,
                            type: _tag.type,
                            is_required: _tag.is_required,
                            is_editable: _tag.is_editable
                        });
        }

        pageEditPlaylistSong.mdl.push({delegate_type: "delete"});

        listViewLoader.active = true
    }

    Connections{
        target: Backend.database

        // save edit
        function onSignalEditedPlaylistSong(){
            Backend.database.loadPlaylistModel()
        }
        function onSignalEditPlaylistSongError(desc){
            pEditSongError.open()
            pEditSongError.dltDesc = desc
        }

        // delete song
        function onSignalDeletedPlaylistSong(){
            Backend.database.loadPlaylistModel()
        }
        function onSignalDeletePlaylistSongError(desc){
            pDeleteSongError.open()
            pDeleteSongError.dltDesc = desc
        }

        // back to playlist page after song was saved or deleted
        function onSignalPlaylistModelLoaded(){
            playlistLoader.source = root.ppp_playlist
        }
        function onSignalPlaylistModelLoadError(desc){
            pAllSongsModelLoadError.open()
            pAllSongsModelLoadError.dltDesc = desc
        }
    }

    Popup1{
        id: pEditSongError
        dltText: "Error while saving the song!"
        // text description will be set in onSignalEditSongError() function
    }

    Popup1{
        id: pDeleteSongError
        dltText: "Error while deleting the song!"
        // text description will be set in onSignalDeleteSongError() function
    }

    Popup1{
        id: pAllSongsModelLoadError
        dltText: "Error while loading all songs!"
        // text description will be set in onSignalAllSongsModelLoadError() function
    }


    Popup1{
        id: pFormSubmitError
        // text message will be set in openPopupEmptyField() function

        dltTextMB: "Ok"

        onDltClickedMB: {}
    }

    Popup2{
        id: pDeleteConfirm
        dltText: "Sure you want to delete the '" +
                     Backend.database.m_edit_song_model.tags.tags[1].value /* song title */ +
                     "' Song (id: " +
                     Backend.database.m_edit_song_model.id + ")?"

        dltTextLB: "Cancel"
        dltTextRB: "Delete"

        onDltClickedLB: {}
        onDltClickedRB: {
            Backend.database.deletePlaylistSong(Backend.database.m_edit_song_model.id);
        }
    }

    Loader {
        id: listViewLoader
        anchors{
            left: parent.left
            top: listViewHeader.bottom
            right: parent.right
            bottom: parent.bottom
        }
        sourceComponent: listViewComponent
        active: false
    }

    Component{
        id: listViewComponent
        ListView{
            id: addSongListView
            anchors.fill: parent
            model: mdl

            clip: true

            boundsBehavior: Flickable.StopAtBounds

            footer: Item{
                width: parent.width
                height: delegateHeight/2
            }


            Component.onCompleted: {
                contentY = startListPosition
            }
            Component.onDestruction: {
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
                width: delegateWidth
                height: delegateHeight

                // decide what style of row listView should choose
                sourceComponent: {
                    var _id = modelData.id;
                    var _delegate_type = modelData.delegate_type

                    // console.log("  choosing delegate with: id:" + modelData.id + ", name:" + modelData.name +
                    //             ", value:" + modelData.value + ", type:" + modelData.type + ", is_required:"+
                    //             modelData.is_required + ", is_editable:" + modelData.is_editable)

                    if(_delegate_type === "delete") // delete song button
                        deleteSongDelegate;
                    else if(_delegate_type === "tag"){
                        var _type = modelData.type;
                        var is_path_type = _id === 9 || _id === 10; // id=9 is equal to "Song Path", id=10 is equal to "Thumbnail Path"

                        if(is_path_type){
                            fileDialogDelegate
                        }
                        else if(_type === 0){ // type 0 is TT_INTEGER
                            integerDelegate
                        }
                        else if(_type === 1){ // type 1 is TT_TEXT
                            stringDelegate
                        }
                        else if(_type === 2){ // type 2 is TT_BOOL
                            triSwitchDelagate
                        }
                        else {
                            console.log ("  !!!!!     Unknown Type " + _type + "     !!!!!");
                        }
                    }
                }

                Component{
                    id: integerDelegate
                    IntegerField{
                        dltText: modelData.name
                        dltValue: modelData.value
                        dltEnabled: modelData.is_editable
                        onDltValueChanged: {
                            pageEditPlaylistSong.mdl[index].value = dltValue;
                            // modelData.value is just a copy and we need assign to original one
                        }
                    }
                }

                Component {
                    id: stringDelegate
                    StringField{
                        dltText: modelData.name
                        dltValue: modelData.value
                        dltEnabled: modelData.is_editable
                        onDltValueChanged: {
                            pageEditPlaylistSong.mdl[index].value = dltValue;
                            // modelData.value is just a copy and we need assign to original one
                        }
                    }
                }

                Component{
                    id: fileDialogDelegate
                    FileSelectField{
                        dltText: modelData.name
                        dltValue: modelData.value
                        dltEnabled: modelData.is_editable
                        onDltValueChanged: {
                            pageEditPlaylistSong.mdl[index].value = dltValue;
                            // modelData.value is just a copy and we need assign to original one
                        }
                    }
                }

                Component{
                    id: triSwitchDelagate
                    TriSwitchField{
                        dltText: modelData.name
                        dltValue: modelData.value
                        dltEnabled: modelData.is_editable
                        onDltValueChanged: {
                            pageEditPlaylistSong.mdl[index].value = dltValue;
                            // modelData.value is just a copy and we need assign to original one
                        }
                    }
                }

                Component {
                    id: deleteSongDelegate
                    DeleteField{
                        onDltClicked: {
                            pDeleteConfirm.open()
                        }
                    }
                }

            }
        }
    }

    function openPopupEmptyField(empty_field_name)
    {
        pFormSubmitError.open();
        pFormSubmitError.dltText = "Field \"" + empty_field_name + "\" can't be empty";
    }

    function submitMethod(){
        var _element;
        // test constant tags form validation (if all required fields are filled)
        for (_element of pageEditPlaylistSong.mdl)
        {
            if(_element.is_required && ((""+_element.value) === ""))
            {
                openPopupEmptyField(_element.name)
                return;
            }
        }

        var result_list = []

        // console.log("  editPlaylistSong return result:")
        for(_element of pageEditPlaylistSong.mdl)
        {
            if(_element.delegate_type === "tag")
            {
                if(_element.is_editable === true)
                {
                    result_list.push({id: _element.id, value: _element.value})
                    // console.log("    id: " + _element.id + ", value: " + _element.value)
                }
            }
        }

        console.log("  saving song...")

        // page is changed after signal was emited from editSong()
        Backend.database.editPlaylistSong(Backend.database.m_edit_song_model.id, result_list)
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

                mainLoader.anchors.top = tabBar.bottom
                playlistLoader.source = root.ppp_playlist
            }
        }

        Text{
            text: "Edit Song"
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

            text: "SAVE"
            font.pixelSize: 15

            onClicked: submitMethod()
        }
    }

}
