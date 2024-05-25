import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material

import "qrc:/SortManager-Music/qml/popups"
import "qrc:/SortManager-Music/qml/delegates"

Page {
    anchors.fill: parent

/*

    tutaj użytkownik może dodać tag/usunąć/zobaczyć jakie piosenki go mają

    również z settings będzie analogiczna pozycja o nazwie muzyki która będzie powzwałała niemal analogicznie zobaczyć
    piosenki oraz tagi jakie są do nich dołączone (po kliknięciu na nie)
    Istotne tutaj będzie że w momencie dodawania piosenki będzie można ustawić tagi które są oznaczone jako:
    "is_immutable: true;" oraz ustawić tagi dodane przez użytkownika, w momencie gdy nie będzie
    tagu jaki nas interesuje to nie będzie trzeba wychodzić tylko będzie można do z tamtego miejsca dodać

    w PlaylistView nie będzie można nic edytować lecz tylko wybierać playlistę

    Wybieranie tagów w playlist będzie się odbywać poprzez wiele wierszów (gdzie każdy to inny tag)
    a każdy z nich będzie miał 3-fazowy swich decydujący [ chcę w playlist | nie istotne | nie chcę w playlist ]
    gdzie dodatkowo tagi z zakresu "userAdded: false;" będą miały dodatkowe pola z wyborem (np Tytuł pole tekstowe które
        wskaże do czego się odnosi chcę/nie chcę w playlist )

    ? wtedy będzie można utworzyć na nowo ten tag i będzie jak wcześniej, w settings będzie przycisk do optymalizowania bazy.
    ? ta operacja usunie z muzyk tagi których nie ma w tabeli tagów.

*/

    property var backend: Backend

    property int delegateHeight: 60
    property int delegateWidth: width

    // original shortcut from model
    property var mdl: backend.database.all_tags_model.tags

    /*
        received structure:

        backend.database.all_tags_model - TagList:
            tags - QList<Tag *>:
                id - int: from SELECT
                name - QString: from SELECT
                value - QString: ""
                type - int: -1
                is_immutable - bool: from SELECT
                is_editable - bool: false
                is_required - bool: false


        SELECT id, name, is_immutable FROM tags;
        OR
        SELECT id, name, is_immutable FROM tags WHERE is_immutable = 0;
    */

    Component.onCompleted: {
        console.log("Tags.qml");
    }

    Connections{
        target: backend.database

        // add tag
        function onSignalAddTagModelLoaded(){
            root.last_pos_tags = tagsListView.contentY // set current scroll to restore after add tag
            settingsLoader.source = root.ppsst_addtag
        }
        function onSignalAddTagModelLoadError(desc){
            pAddTagLoadError.open();
            pAddTagLoadError.dltDesc = desc;
        }

        // edit tag
        function onSignalEditTagModelLoaded(){
            root.last_pos_tags = tagsListView.contentY // set current scroll to restore after edit tag
            settingsLoader.source = root.ppsst_edittag
        }
        function onSignalEditTagModelLoadError(desc){
            pEditTagLoadError.open();
            pEditTagLoadError.dltDesc = desc
        }

        // delete tag
        function onSignalDeletedTag(){
            backend.database.loadAllTags()
        }
        function onSignalDeleteTagError(desc){
            pDeleteTagError.open();
            pDeleteTagError.dltDesc = desc;
        }

        // refresh page after any tag was deleted
        function onSignalAllTagsModelLoaded(){
            // force loader to refresh
            root.last_pos_tags = tagsListView.contentY // set current scroll to restore after delete tag
            settingsLoader.source = ""
            settingsLoader.source = root.ppss_tags
        }
        function onSignalAllTagsModelLoadError(desc){
            pAllTagsReloadError.open();
            pAllTagsReloadError.dltDesc = desc;
        }
    }

    // usefull properties for Popups
    property int just_used_id: 0
    property string just_used_name: ""

    Popup1{
        id: pAddTagModelLoadError
        dltText: "Error while loading add tag model!"
        // text description will be set in onSignalAddTagModelLoadError() function
        dltTextMB: "OK"
    }
    Popup1{
        id: pEditTagModelLoadError
        dltText: "Error while loading edit tag(id: " + just_used_id + ", name: " + just_used_name + ") model!"
        // text description will be set in onSignalEditTagModelLoadError() function
        dltTextMB: "OK"
    }
    Popup1{
        id: pDeleteTagError
        dltText: "Error while deleting tag(id: " + just_used_id + ", name: " + just_used_name + ")!"
        // text description will be set in onSignalDeleteTagError() function
        dltTextMB: "OK"
    }
    Popup1{
        id: pAllTagsReloadError
        dltText: "Error while reloading all tags model!"
        // text description will be set in onSignalAllTagsModelLoadError() function
        dltTextMB: "OK"
    }

    Popup2{
        id: pDeleteConfirm
        dltText: "Sure you want to delete the '"+ just_used_name +"'(id: " + just_used_id + ") Song ?"

        dltTextLB: "Cancel"
        dltTextRB: "Delete"

        onDltClickedLB: {}
        onDltClickedRB: {
            backend.database.deleteTag(just_used_id);
        }
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
            id: tagsListView
            model: mdl
            clip: true

            boundsBehavior: Flickable.StopAtBounds

            footer: Item{
                width: parent.width - 15 /*scrollbar offset*/
                height: delegateHeight/2
            }

            Component.onCompleted: {
                contentY = root.last_pos_tags
                // scroll will be set before changing site
            }

            delegate: Item{
                width: delegateWidth - 15 /*scrollbar offset*/
                height: delegateHeight
                ButtonField{
                    dltText: modelData.name
                    dltIsDeletable: !modelData.is_immutable
                    onDltClickedElement: {
                        just_used_id = + modelData.id;
                        just_used_name = modelData.name;

                        // page is changed after signal was emited from loadEditTagModel()
                        backend.database.loadEditTagModel(+modelData.id) // + means parse int
                    }
                    onDltClickedDelete: {
                        // page is refreshed after signal was emited from loadAllTagsModel()
                        if(!modelData.is_immutable)
                        {
                            just_used_id = + modelData.id;
                            just_used_name = modelData.name;

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
        height: tabBar.height
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
                    root.last_pos_tags = tagsListView.contentY // set current scroll to restore after back to page
                else
                    root.last_pos_tags = 0

                mainLoader.anchors.top = tabBar.bottom
                settingsLoader.source = root.pps_settings
            }
        }

        Text{
            text: "Tags"
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
            ToolTip.text: "Add Tag"

            onClicked: {
                // page is changed after signal was emited from loadAddTagModel()
                backend.database.loadAddTagModel()
            }
        }
    }
}

