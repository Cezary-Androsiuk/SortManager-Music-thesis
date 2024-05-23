import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material

import "qrc:/SortManager-Music/qml/popups"
import "qrc:/SortManager-Music/qml/delegates"

Page {
    id: pageAddSong
    anchors.fill: parent

    property int delegateHeight: 60
    property int delegateWidth: width

    property var mdl: [] // mdl shortcut for model

    /*
        received structure:

        backend.database.add_song_model - SongDetails:
            id - int: 0
            tags - TagList:
                tags - QList<Tag *>:
                    id - int: from SELECT
                    name - QString: from SELECT
                    value - QString: ""
                    type - int: from SELECT
                    is_immutable - bool: 0
                    is_editable - bool: 1
                    is_required - bool: from SELECT

        SELECT id, name, type, is_required FROM tags WHERE is_editable = 1;
    */

    Component.onCompleted: {
        console.log("AddSong.qml")
        if(backend.database.add_song_model.id !== 0){
            console.log("  given song model id should be 0 but is " + backend.database.add_song_model.id)
            root.close()
        }
        mainLoader.anchors.top = tabBar.top

        var tmp_mdl = backend.database.add_song_model.tags.tags

        for(var _tag of tmp_mdl){
            // set default values for an empty
            var _value;
            if(_tag.type === 0)      // type 0 is TT_INTEGER
                _value = "0";
            else if(_tag.type === 1) // type 1 is TT_TEXT
                _value = "";
            else if(_tag.type === 2) // type 2 is TT_BOOL
                _value = "0";

            pageAddSong.mdl.push(
                        {
                            delegate_type: "tag",
                            id: _tag.id,
                            name: _tag.name,
                            value: _value,
                            type: _tag.type,
                            is_required: _tag.is_required
                        });
        }


        listViewLoader.active = true
    }

    Connections{
        target: backend.database

        // save add
        function onSignalAddedSong(){
            backend.database.loadAllSongs()
        }
        function onSignalAddSongError(desc){
            pAddSongError.open()
            pAddSongError.dltDesc = desc
        }

        // back to songs page after song was saved or deleted
        function onSignalAllSongsModelLoaded(){
            settingsLoader.source = root.ppss_songs
        }
        function onSignalAllSongsModelLoadError(desc){
            pAllSongsModelLoadError.open()
            pAllSongsModelLoadError.dltDesc = desc
        }
    }

    Popup1{
        id: pAddSongError
        dltText: "Error while saving the song!"
        // text description will be set in onSignalAddSongError() function
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

    Loader{
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
        ScrollView{
            id: scrollView
            anchors.fill: parent
            ListView{
                id: addSongListView
                model: mdl

                clip: true

                boundsBehavior: Flickable.StopAtBounds

                footer: Item{
                    width: parent.width - 15 /*scrollbar offset*/
                    height: delegateHeight/2
                }

                // on this site scroll position will be saved allways or never cause do not contains any sub sites
                Component.onCompleted: {
                    contentY = root.last_pos_add_song
                }
                Component.onDestruction: {
                    if(backend.personalization.alwaysKeepListPos)
                        root.last_pos_add_song = contentY
                    else
                        root.last_pos_add_song = 0
                }

                delegate: Loader{
                    width: delegateWidth - 15 /*scrollbar offset*/
                    height: delegateHeight

                    // decide what style of row listView should choose
                    sourceComponent: {
                        var _id = modelData.id;
                        var _delegate_type = modelData.delegate_type

                        // console.log("  choosing delegate with: id:" + modelData.id + ", name:" + modelData.name +
                        //             ", value:" + modelData.value + ", type:" + modelData.type)

                        if(_delegate_type === "tag"){
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
                                switchDelagate
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
                            onDltValueChanged: {
                                pageAddSong.mdl[index].value = dltValue;
                                // modelData.value is just a copy and we need assign to original one
                            }
                        }
                    }

                    Component {
                        id: stringDelegate
                        StringField{
                            dltText: modelData.name
                            dltValue: modelData.value
                            onDltValueChanged: {
                                pageAddSong.mdl[index].value = dltValue;
                                // modelData.value is just a copy and we need assign to original one
                            }
                        }
                    }

                    Component{
                        id: fileDialogDelegate
                        FileSelectField{
                            dltText: modelData.name
                            dltValue: modelData.value
                            onDltValueChanged: {
                                pageAddSong.mdl[index].value = dltValue;
                                // modelData.value is just a copy and we need assign to original one
                            }
                            // delegate_start_folder: backend.personalization.songs_open_path
                        }
                    }

                    Component{
                        id: switchDelagate
                        TriSwitchField{
                            dltText: modelData.name
                            dltValue: modelData.value
                            onDltValueChanged: {
                                pageAddSong.mdl[index].value = dltValue;
                                // modelData.value is just a copy and we need assign to original one
                            }
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
        // test constant tags form validation (if all required fields are filled)
        var _element;
        for (_element of pageAddSong.mdl)
        {
            if(_element.is_required && ((""+_element.value) === ""))
            {
                // console.log(_element.value)
                openPopupEmptyField(_element.name)
                return;
            }
        }

        var result_list = []

        // console.log("  addSong return result:")
        for(_element of pageAddSong.mdl)
        {
            if(_element.delegate_type === "tag")
            {
                result_list.push({id: _element.id, value: _element.value})
                // console.log("    id: " + _element.id + ", value: " + _element.value)
            }
        }

        console.log("  saving song...")

        // page is changed after signal was emited from addSong()
        backend.database.addSong(result_list)
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
                settingsLoader.source = root.ppss_songs
            }
        }

        Text{
            text: "Add Song"
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
