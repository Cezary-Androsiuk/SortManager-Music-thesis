import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material

import "qrc:/SortManager-Music/qml/popups"
import "qrc:/SortManager-Music/qml/delegates"

Page {
    id: pageEditTag
    property var backend: Backend
    anchors.fill: parent

    property int delegateHeight: 60
    property int delegateWidth: width

    property var mdl: [] // mdl means model
    property int glob_type: -1 // init tag type (State)

    /*
        received structure:

        backend.database.edit_tag_model - TagDetails:
            id - int: from SELECT
            name - QString: from SELECT
            description - QString: from SELECT
            add_date - QString: from SELECT
            update_date - QString: from SELECT
            type - int: from SELECT
            is_immutable - bool: from SELECT
            is_editable - bool: from SELECT
            is_required - bool: from SELECT
            songs - SongList:
                songs - QList<Song *>:
                    id - int: from SELECT
                    title - QString: from SELECT
                    value - QString: from SELECT

        SELECT * FROM tags WHERE id = %1;

        SELECT song_id,
            MAX(CASE WHEN tag_id = 2 THEN value END) AS title,
            MAX(CASE WHEN tag_id = 4 THEN value END) AS song_value
        FROM songs_tags
        WHERE tag_id IN (2, 4)
        GROUP BY song_id;
    */

    Component.onCompleted: {
        console.log("EditTag.qml")
        mainLoader.anchors.top = tabBar.top

        // set tag parameters to model
        pageEditTag.mdl.push({delegate_type: "param", name: "ID",
                                 value: backend.database.edit_tag_model.id,
                                 is_editable: false});

        pageEditTag.mdl.push({delegate_type: "param", name: "Name",
                                 value: backend.database.edit_tag_model.name,
                                 is_editable: true});

        pageEditTag.mdl.push({delegate_type: "param", name: "Description",
                                 value: backend.database.edit_tag_model.description,
                                 is_editable: true});

        pageEditTag.mdl.push({delegate_type: "param", name: "Add Date",
                                 value: backend.database.edit_tag_model.add_date,
                                 is_editable: false});

        pageEditTag.mdl.push({delegate_type: "param", name: "Update Date",
                                 value: backend.database.edit_tag_model.update_date,
                                 is_editable: false});

        pageEditTag.mdl.push({delegate_type: "param", name: "Type",
                                 value: backend.database.edit_tag_model.type,
                                 is_editable: false});
        pageEditTag.glob_type = backend.database.edit_tag_model.type;

        pageEditTag.mdl.push({delegate_type: "param", name: "Is Immutable",
                                 value: backend.database.edit_tag_model.is_immutable,
                                 is_editable: false});

        pageEditTag.mdl.push({delegate_type: "param", name: "Is Editable",
                                 value: backend.database.edit_tag_model.is_editable,
                                 is_editable: false});

        pageEditTag.mdl.push({delegate_type: "param", name: "Is Required",
                                 value: backend.database.edit_tag_model.is_required,
                                 is_editable: false});


        pageEditTag.mdl.push({delegate_type: "sep"});

        // rewrite songs to model
        var tmp_song_mdl = backend.database.edit_tag_model.songs.songs;
        for(var _song of tmp_song_mdl)
        {
            pageEditTag.mdl.push(
                        {
                            delegate_type: "song",
                            id: _song.id,
                            title: _song.title,
                            value: _song.value,
                            is_editable: backend.database.edit_tag_model.is_editable
                            // if this tag is editable then values of the sogns can be changed
                        });
        }

        if(!backend.database.edit_tag_model.is_immutable)
            pageEditTag.mdl.push({delegate_type: "delete"});

        listViewLoader.active = true;
    }

    Connections{
        target: backend.database

        // save add
        function onSignalEditedTag(){
            backend.database.loadAllTags()
        }
        function onSignalEditTagError(desc){
            pEditTagError.open();
            pEditTagError.dltDesc = desc;
        }

        // delete song
        function onSignalDeletedTag(){
            Backend.database.loadAllTags()
        }
        function onSignalDeleteTagError(desc){
            pDeleteTagError.open();
            pDeleteTagError.dltDesc = desc;
        }

        // back to tags page after tag was saved or deleted
        function onSignalAllTagsModelLoaded(){
            settingsLoader.source = root.ppss_tags
        }
        function onSignalAllTagsModelLoadError(desc){
            pAllTagsModelLoadError.open();
            pAllTagsModelLoadError.dltDesc = desc;
        }
    }

    Popup1{
        id: pEditTagError
        dltText: "Error while saving the tag!"
        // text description will be set in onSignalEditTagError() function
        dltTextMB: "OK"
    }

    Popup1{
        id: pDeleteTagError
        dltText: "Error while deleting the tag!"
        // text description will be set in onSignalDeleteTagError() function
        dltTextMB: "OK"
    }

    Popup1{
        id: pAllTagsModelLoadError
        dltText: "Error while loading all tags!"
        // text description will be set in onSignalAllTagsModelLoadError() function
        dltTextMB: "OK"
    }


    Popup1{
        id: pFormSubmitError
        // text message will be set in openPopupEmptyField() function
        dltTextMB: "OK"

        onDltClickedMB: {}
    }

    Popup2{
        id: pDeleteConfirm
        dltText: "Sure you want to delete the '" +
                     backend.database.edit_tag_model.name +
                     "' Tag (id: " +
                     backend.database.edit_tag_model.id + ")?"

        dltTextLB: "Cancel"
        dltTextRB: "Delete"

        onDltClickedLB: {}
        onDltClickedRB: {
            backend.database.deleteTag(backend.database.edit_tag_model.id);
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

        ScrollView{
            id: scrollView
            anchors.fill: parent
            ListView{
                id: addTagsListView
                model: mdl

                clip: true

                boundsBehavior: Flickable.StopAtBounds

                footer: Item{
                    width: parent.width - 15 /*scrollbar offset*/
                    height: delegateHeight/2
                }

                // on this site scroll position will be saved allways or never cause do not contains any sub sites
                Component.onCompleted: {
                    contentY = root.last_pos_edit_tag
                }
                Component.onDestruction: {
                    if(backend.personalization.alwaysKeepListPos)
                        root.last_pos_edit_tag = contentY
                    else
                        root.last_pos_edit_tag = 0
                }

                delegate: Loader{
                    width: delegateWidth - 15 /*scrollbar offset*/
                    height: delegateHeight

                    // decide what style of row listView should choose
                    sourceComponent: {

                        // console.log("  choosing delegate with: delegate_type:" + modelData.delegate_type + ", id:"
                        //             + modelData.id + ", name:" + modelData.name + ", value:" + modelData.value +
                        //             ", title:" + modelData.title + ", is_editable:" + modelData.is_editable)

                        if(modelData.delegate_type === "param")
                        {
                            if(modelData.name === "Type")
                                paramComboBoxDelegate;
                            else
                                paramStringDelegate;
                        }
                        else if(modelData.delegate_type === "song")
                        {
                            if(pageEditTag.glob_type === -1) // none type selected
                                songNotChosenDelegate;
                            else if(backend.database.edit_tag_model.id === 9) // tag describe Song Path tag
                                songFileDialogDelegate // all songs fields will be path type
                            else if(backend.database.edit_tag_model.id === 10) // tag describe Thumbnail Path tag
                                songFileDialogDelegate // all songs fields will be path type
                            else if (pageEditTag.glob_type === 0) // type 0 is TT_INTEGER
                                songIntegerDelegate;
                            else if (pageEditTag.glob_type === 1) // type 1 is TT_TEXT
                                songStringDelegate;
                            else if (pageEditTag.glob_type === 2) // type 2 is TT_BOOL
                                songTriSwitchDelegate;

                        }
                        else if(modelData.delegate_type === "sep")
                        {
                            separatorDelegate;
                        }
                        else if(modelData.delegate_type === "delete")
                        {
                            deleteTagDelegate;
                        }
                    }


                    Component{
                        id: paramStringDelegate
                        StringField{
                            dltText: modelData.name
                            dltValue: modelData.value
                            dltEnabled: modelData.is_editable
                            onDltValueChanged: {
                                pageEditTag.mdl[index].value = dltValue;
                                // modelData.value is just a copy and we need assign to original one
                            }
                        }
                    }

                    Component{
                        id: paramComboBoxDelegate
                        ComboBoxField{
                            dltText: modelData.name
                            dltValue: pageEditTag.glob_type
                            dltEnabled: modelData.is_editable
                            onDltValueChanged: {
                                pageEditTag.glob_type = dltValue
                            }

                            dltModel: [
                                "Number",   // value  0 is TT_INTEGER in Database
                                "Text",     // value  1 is TT_TEXT in Database
                                "State"     // value  2 is TT_BOOL in Database
                            ]
                        }
                    }


                    Component{
                        id: separatorDelegate
                        SeparatorField{

                        }
                    }


                    Component{
                        id: songNotChosenDelegate;
                        NotChosenField{
                            dltText: modelData.title
                        }
                    }

                    Component{
                        id: songIntegerDelegate;
                        IntegerField{
                            dltText: modelData.title
                            dltValue: modelData.value
                            dltEnabled: modelData.is_editable
                            onDltValueChanged: {
                                pageEditTag.mdl[index].value = dltValue;
                                // modelData.value is just a copy and we need assign to original one
                            }
                        }
                    }

                    Component{
                        id: songFileDialogDelegate
                        FileSelectField{
                            dltText: modelData.title
                            dltValue: modelData.value
                            dltEnabled: modelData.is_editable
                            onDltValueChanged: {
                                pageEditTag.mdl[index].value = dltValue;
                                // modelData.value is just a copy and we need assign to original one
                            }
                        }
                    }

                    Component{
                        id: songStringDelegate;
                        StringField{
                            dltText: modelData.title
                            dltValue: modelData.value
                            dltEnabled: modelData.is_editable
                            onDltValueChanged: {
                                pageEditTag.mdl[index].value = dltValue;
                                // modelData.value is just a copy and we need assign to original one
                            }
                        }
                    }

                    Component{
                        id: songTriSwitchDelegate;
                        TriSwitchField{
                            dltText: modelData.title
                            dltValue: modelData.value
                            dltEnabled: modelData.is_editable
                            onDltValueChanged: {
                                pageEditTag.mdl[index].value = dltValue;
                                // modelData.value is just a copy and we need assign to original one
                            }
                        }
                    }

                    Component {
                        id: deleteTagDelegate
                        DeleteField{
                            onDltClicked: {
                                pDeleteConfirm.open();
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
        // glob_type is constant

        for (_element of pageEditTag.mdl)
        {
            if(_element.delegate_type === "param")
            {
                if(_element.name === "Name" && _element.value === "")
                {
                    openPopupEmptyField(_element.name);
                    return;
                }
            }
            // songs will be always not required

        }
        var result_list = []

        // console.log("  editTag return result:")
        for(_element of pageEditTag.mdl)
        {
            if(_element.delegate_type === "param" && _element.is_editable === true)
            {
                result_list.push({delegate_type: "param", name: _element.name, value: _element.value})
                // console.log("    type: param, name: " + _element.name + ", value: " + _element.value);

            }
            else if(_element.delegate_type === "song" && _element.is_editable === true)
            {
                result_list.push({delegate_type: "song", id: _element.id, value: _element.value})
                // console.log("    type: song, id: " + _element.id + ", title: " + _element.title + ", value: " + _element.value);
            }
        }

        console.log("  saving tag...")

        // page is changed after signal was emited from addTag()
        backend.database.editTag(backend.database.edit_tag_model.id, result_list)
    }

    Rectangle{
        id: listViewHeader
        anchors{
            left: parent.left
            top: parent.top
            right: parent.right
        }
        height: tabBar.height
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

            onClicked: settingsLoader.source = root.ppss_tags
        }

        Text{
            text: "Edit Tag"
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
