import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material

import "qrc:/SortManager-Music/qml/popups"
import "qrc:/SortManager-Music/qml/delegates/Playlist/Filters"


Page {
    id: pageFilters
    anchors.fill: parent

    property int delegateHeight: 60
    property int delegateWidth: width

    property var mdl: [] // mdl shortcut for model

    property double startListPosition: root.last_pos_filters
    property bool saveListPosition: false
    property bool alwaysSaveListPosition: backend.personalization.alwaysKeepListPos

    function closeFiltersPage(){
        mainLoader.anchors.top = tabBar.bottom
        playlistLoader.source = root.ppp_playlist
    }

    /*
        received structure:

        backend.database.filters_model - TagList:
            tags - QList<TagWithComparator *>:
                id - int: from SELECT
                name - QString: from SELECT
                value - QString: ""
                type - int:  from SELECT
                is_immutable - bool: false
                is_editable - bool: false
                is_required - bool: false
                comparison_way - int: from JSON
                comparison_value - QString: from JSON


        SELECT id, name, type FROM tags; + filters content from json
    */

    Component.onCompleted: {
        console.log("Filters.qml")

        for(var _tag of backend.database.filters_model.tags)
        {
            // console.log( "id:" +_tag.id+
            //             ", name:" +_tag.name+ ", type:" +_tag.type+
            //             ", comparison_way:" +_tag.comparison_way+ ", comparison_value:" +_tag.comparison_value)
            mdl.push({ id: _tag.id, name: _tag.name, type: _tag.type,
                         comparison_way: _tag.comparison_way, comparison_value: _tag.comparison_value })
        }

        listViewLoader.active = true
    }

    Popup2{
        id: pConfirmSave
        dltText: "Are you sure to save Filters? Player will be stopped and reseted!"
        // text description will be set in onSignalFilterModelLoadError() function

        dltTextLB: "Cancel"
        dltTextRB: "Save"

        onDltClickedRB: {
            submitMethod()
        }
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
                id: filtersListView
                model: mdl
                anchors.fill: parent
                clip: true

                boundsBehavior: Flickable.StopAtBounds

                delegate: Loader{
                    width: delegateWidth
                    height: delegateHeight

                    // decide what style of row listView should choose
                    sourceComponent: {
                        if(false);
                        else if(modelData.type === 0) compareIntegerFieldComponent
                        else if(modelData.type === 1) compareStringFieldComponent
                        else if(modelData.type === 2) compareStateFieldComponent
                    }


                    Component{
                        id: compareIntegerFieldComponent
                        CompareIntegerField{
                            dltText: modelData.name
                            dltComboboxValue: modelData.comparison_way
                            onDltComboboxValueChanged: {
                                pageFilters.mdl[index].comparison_way = dltComboboxValue
                            }
                            dltValue: modelData.comparison_value
                            onDltValueChanged: {
                                pageFilters.mdl[index].comparison_value = dltValue
                            }
                        }
                    }
                    Component{
                        id: compareStringFieldComponent
                        CompareStringField{
                            dltText: modelData.name
                            dltComboboxValue: modelData.comparison_way
                            onDltComboboxValueChanged: {
                                pageFilters.mdl[index].comparison_way = dltComboboxValue
                            }
                            dltValue: modelData.comparison_value
                            onDltValueChanged: {
                                pageFilters.mdl[index].comparison_value = dltValue
                            }
                        }
                    }
                    Component{
                        id: compareStateFieldComponent
                        CompareStateField{
                            dltText: modelData.name
                            dltValue: modelData.comparison_value
                            onDltValueChanged: {
                                pageFilters.mdl[index].comparison_value = dltValue
                            }
                        }
                    }

                }
            }
        }
    }

    function submitMethod(){
        // for(var _tag of pageFilters.mdl)
        // {
        //     console.log( "id:" +_tag.id+
        //                 ", name:" +_tag.name+ ", type:" +_tag.type+
        //                 ", comparison_way:" +_tag.comparison_way+ ", comparison_value:" +_tag.comparison_value)
        // }

        Backend.database.updateFilters(pageFilters.mdl);
        // page is closed if filters update was completed succesfully
        closeFiltersPage();
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

            onClicked: closeFiltersPage() // pConfirmLeave.open()
        }

        Text{
            text: "Filters"
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

            onClicked: {
                if(backend.personalization.showFiltersSave)
                    pConfirmSave.open()
                else
                    submitMethod()
            }
        }
    }
}
