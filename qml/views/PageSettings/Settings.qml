import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material

import "qrc:/SortManager-Music/qml/popups"

Page {
    anchors.fill: parent

    property int delegateHeight: 60
    property int delegateWidth: width

    ListModel{
        id: settingsModel
        ListElement{ name: "Personalisations" }
        ListElement{ name: "Database Management" }
        ListElement{ name: "Songs" }
        ListElement{ name: "Tags" }
    }

    Connections{
        target: backend.database

        // all songs
        function onSignalAllSongsModelLoaded(){
            settingsLoader.source = root.ppss_songs
        }
        function onSignalAllSongsModelLoadError(desc){
            pAllSongsModelLoadError.open();
            pAllSongsModelLoadError.dltDesc = desc;
        }

        // all tags
        function onSignalAllTagsModelLoaded(){
            settingsLoader.source = root.ppss_tags
        }
        function onSignalAllTagsModelLoadError(desc){
            pAllTagsModelLoadError.open();
            pAllTagsModelLoadError.dltDesc = desc;
        }
    }

    Popup1{
        id: pAllSongsModelLoadError
        dltText: "Error while loading all songs model!"
        // text description will be set in onSignalAllSongsModelLoadError() function
    }

    Popup1{
        id: pAllTagsModelLoadError
        dltText: "Error while loading all songs model!"
        // text description will be set in onSignalAllTagsModelLoadError() function
    }

    ListView{
        id: settingsListView
        model: settingsModel
        anchors.fill: parent

        boundsBehavior: Flickable.StopAtBounds

        // on this site scroll position will be saved allways or never cause do not contains any sub sites
        Component.onCompleted: {
            contentY = root.last_pos_settings
        }
        Component.onDestruction: {
            if(backend.personalization.alwaysKeepListPos)
                root.last_pos_settings = contentY
            else
                root.last_pos_settings = 0
        }

        delegate: TabButton{
            height: delegateHeight
            width: delegateWidth
            Text{
                anchors{
                    verticalCenter: parent.verticalCenter
                    left: parent.left
                    leftMargin: 20
                }
                text: model.name
                font.pixelSize: 15
                color: root.color_accent1
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
            onClicked: {
                if(name === "Personalisations"){
                    settingsLoader.source = root.ppss_personalizations
                }
                else if(name === "Database Management"){
                    settingsLoader.source = root.ppss_dbmanagement
                }
                else if(name === "Songs"){
                    // page is changed after signal was emited from loadAllSongs()
                    Backend.database.loadAllSongs()
                }
                else if(name === "Tags"){
                    // page is changed after signal was emited from loadAllTags()
                    Backend.database.loadAllTags()
                }
            }
        }
    }
}
