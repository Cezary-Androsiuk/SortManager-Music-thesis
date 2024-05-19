import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material

import "qrc:/SortManager-Music/qml/popups"
import "qrc:/SortManager-Music/qml/delegates"

Page {
    id: pageAddTag
    property var backend: Backend
    anchors.fill: parent

    property int delegateHeight: 60
    property int delegateWidth: width

    property var mdl: [] // mdl means model
    property int glob_type: -1 // init tag type (State)

    /*
        received structure:

        backend.database.add_tag_model - TagDetails:
            id - int: 0
            name - QString: ""
            description - QString: ""
            add_date - QString: ""
            update_date - QString: ""
            type - int: -1
            is_immutable - bool: false
            is_editable - bool: false
            is_required - bool: false
            songs - SongList:
                songs - QList<Song *>:
                    id - int: from SELECT
                    title - QString: from SELECT
                    value - QString: ""

        SELECT song_id, value AS title FROM songs_tags WHERE tag_id = 2; // Title id is 2
    */

    Component.onCompleted: {
        console.log("AddTag.qml")
        mainLoader.anchors.top = tabBar.top

        var tmp_glob_type = backend.personalization.defaultAddTagType

        pageAddTag.glob_type = tmp_glob_type;

        // set tag parameters to model
        // id - not editable
        pageAddTag.mdl.push({delegate_type: "param", name: "Name", value: ""});
        pageAddTag.mdl.push({delegate_type: "param", name: "Description", value: ""});
        // add_date - not editable
        // update_date - not editable
        pageAddTag.mdl.push({delegate_type: "param", name: "Type", value: tmp_glob_type});
        // is_immutable - not editable
        // is_editable - not editable
        // is_required - not editable

        pageAddTag.mdl.push({delegate_type: "sep"});

        // rewrite songs to model
        var tmp_song_mdl = backend.database.add_tag_model.songs.songs;
        for(var _song of tmp_song_mdl)
        {
            var _value;
            if(pageAddTag.glob_type === -1) // none type selected
                _value = null;
            else if (pageAddTag.glob_type === 0) // type 0 is TT_INTEGER
                _value = "0";
            else if (pageAddTag.glob_type === 1) // type 1 is TT_TEXT
                _value = "";
            else if (pageAddTag.glob_type === 2) // type 2 is TT_BOOL
                _value = "0";

            pageAddTag.mdl.push(
                        {
                            delegate_type: "song",
                            id: _song.id,
                            title: _song.title,
                            value: _value
                        });
        }

        listViewLoader.active = true
    }

    Connections{
        target: backend.database

        // save add
        function onSignalAddedTag(){
            backend.database.loadAllTags()
        }
        function onSignalAddTagError(desc){
            pAddTagError.open()
            pAddTagError.dltDesc = desc
        }

        // back to tags page after tag was saved or deleted
        function onSignalAllTagsModelLoaded(){
            settingsLoader.source = root.ppss_tags
        }
        function onSignalAllTagsModelLoadError(desc){
            pAllTagsModelLoadError.open()
            pAllTagsModelLoadError.dltDesc = desc
        }
    }

    Popup1{
        id: pAddTagError
        dltText: "Error while saving the tag!"
        // text description will be set in onSignalAddTagError() function
    }

    Popup1{
        id: pAllTagsModelLoadError
        dltText: "Error while loading all tags!"
        // text description will be set in onSignalAllTagsModelLoadError() function
    }


    Popup1{
        id: pFormSubmitError
        // text message will be set in openPopupEmptyField() function

        dltTextMB: "Ok"

        onDltClickedMB: {}
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
                    contentY = root.last_pos_add_tag
                }
                Component.onDestruction: {
                    if(backend.personalization.alwaysKeepListPos)
                        root.last_pos_add_tag = contentY
                    else
                        root.last_pos_add_tag = 0
                }

                delegate: Loader{
                    width: delegateWidth - 15 /*scrollbar offset*/
                    height: delegateHeight

                    // decide what style of row listView should choose
                    sourceComponent: {

                        // console.log("  choosing delegate with: delegate_type:" + modelData.delegate_type + ", id:"
                        //             + modelData.id + ", name:" + modelData.name + ", value:" + modelData.value +
                        //             ", title:" + modelData.title)

                        if(modelData.delegate_type === "param")
                        {
                            if(modelData.name === "Type")
                                paramComboBoxDelegate;
                            else
                                paramStringDelegate;
                        }
                        else if(modelData.delegate_type === "song")
                        {
                            if(pageAddTag.glob_type === -1) // none type selected
                                songNotChosenDelegate;
                            else if (pageAddTag.glob_type === 0) // type 0 is TT_INTEGER
                                songIntegerDelegate;
                            else if (pageAddTag.glob_type === 1) // type 1 is TT_TEXT
                                songStringDelegate;
                            else if (pageAddTag.glob_type === 2) // type 2 is TT_BOOL
                                songTriSwitchDelegate;
                        }
                        else if(modelData.delegate_type === "sep"){
                            separatorDelegate;
                        }
                    }


                    Component{
                        id: paramStringDelegate
                        StringField{
                            dltText: modelData.name
                            dltValue: modelData.value
                            onDltValueChanged: {
                                pageAddTag.mdl[index].value = dltValue;
                                // modelData.value is just a copy and we need assign to original one
                            }
                        }
                    }

                    Component{
                        id: paramComboBoxDelegate
                        ComboBoxField{
                            dltText: modelData.name
                            dltValue: pageAddTag.glob_type
                            onDltValueChanged: {
                                pageAddTag.glob_type = dltValue
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
                            onDltValueChanged: {
                                pageAddTag.mdl[index].value = dltValue;
                                // modelData.value is just a copy and we need assign to original one
                            }
                        }
                    }

                    Component{
                        id: songStringDelegate;
                        StringField{
                            dltText: modelData.title
                            dltValue: modelData.value
                            onDltValueChanged: {
                                pageAddTag.mdl[index].value = dltValue;
                                // modelData.value is just a copy and we need assign to original one
                            }
                        }
                    }

                    Component{
                        id: songTriSwitchDelegate;
                        TriSwitchField{
                            dltText: modelData.title
                            dltValue: modelData.value
                            onDltValueChanged: {
                                pageAddTag.mdl[index].value = dltValue;
                                // modelData.value is just a copy and we need assign to original one
                            }
                        }
                    }

                }
            }
        }
    }

    function openPopupEmptyField(message)
    {
        pFormSubmitError.open();
        pFormSubmitError.dltText = message;
    }

    function submitMethod(){
        // test constant tags form validation (if all required fields are filled)
        var _element
        if(pageAddTag.glob_type === -1)
        {
            openPopupEmptyField("You must select the type of tag!");
            return;
        }

        for (_element of pageAddTag.mdl)
        {
            if(_element.delegate_type === "param")
            {
                if(_element.name === "Name" && _element.value === "")
                {
                    openPopupEmptyField("Field \"" + _element.name + "\" can't be empty");
                    return;
                }
            }
            // songs will be always not required
        }

        var result_list = []

        // console.log("  addTag return result:")
        for(_element of pageAddTag.mdl)
        {
            if(_element.delegate_type === "param")
            {
                result_list.push({delegate_type: "param", name: _element.name, value: _element.value})
                // console.log("    type: param, name: " + _element.name + ", value: " + _element.value);

            }
            else if(_element.delegate_type === "song")
            {
                result_list.push({delegate_type: "song", id: _element.id, value: _element.value})
                // console.log("    type: song, id: " + _element.id + ", title: " + _element.title + ", value: " + _element.value);
            }
        }

        console.log("  saving tag...")

        // page is changed after signal was emited from addTag()
        backend.database.addTag(result_list)
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
            text: "Add Tag"
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
