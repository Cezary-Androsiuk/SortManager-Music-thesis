import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Dialogs

import "qrc:/SortManager-Music/qml/popups"
import "qrc:/SortManager-Music/qml/delegates/DBManagement"

Page {
    property var backend: Backend
    anchors.fill: parent

    property int delegateHeight: 60

    property var mdl: []

    Component.onCompleted: {
        mainLoader.anchors.top = tabBar.top

        mdl.push({id: 100, name: "Export database"})
        mdl.push({id: 200, name: "Import Songs to database"})
        mdl.push({id: 300, name: "Import Tags to database"})
        mdl.push({id: 400, name: "Reeset database"})

        listViewLoader.active = true;
    }

    Connections{
        target: backend.database

        function onSignalExportedDatabase()
        {
            pGlobalOk.open();
            pGlobalOk.dltText = "Database exported successfully!"
            pGlobalOk.dltDesc = "" // need to be set because something might left last after error
        }
        function onSignalExportDatabaseError(desc)
        {
            console.log(desc);
            pGlobalOk.open();
            pGlobalOk.dltText = "Error while exporting database!"
            pGlobalOk.dltDesc = desc
        }


        function onSignalImportedDatabase()
        {
            // TO DELETE
            pGlobalOk.open();
            pGlobalOk.dltText = "Database imported successfully!"
            pGlobalOk.dltDesc = "" // need to be set because something might left last after error
            // TO DELETE
        }
        function onSignalImportDatabaseError(desc)
        {
            // TO DELETE
            console.log(desc);
            pGlobalOk.open();
            pGlobalOk.dltText = "Error while importing database!"
            pGlobalOk.dltDesc = desc
            // TO DELETE
        }

        function onSignalImportedTagsToDatabase()
        {
            pGlobalOk.open();
            pGlobalOk.dltText = "Tags imported successfully!"
            pGlobalOk.dltDesc = "" // need to be set because something might left last after error
        }
        function onSignalImportTagsToDatabaseError(desc)
        {
            console.log(desc);
            pGlobalOk.open();
            pGlobalOk.dltText = "Error while importing Tags to database!"
            pGlobalOk.dltDesc = desc
        }


        function onSignalImportedSongsToDatabase()
        {
            pGlobalOk.open();
            pGlobalOk.dltText = "Songs imported successfully!"
            pGlobalOk.dltDesc = "" // need to be set because something might left last after error
        }
        function onSignalImportSongsToDatabaseError(desc)
        {
            console.log(desc);
            pGlobalOk.open();
            pGlobalOk.dltText = "Error while importing Songs to database!"
            pGlobalOk.dltDesc = desc
        }


        function onSignalDeletedDatabase()
        {
            backend.database.initializeWithTags();
        }
        function onSignalDeleteDatabaseError(desc)
        {
            console.log(desc);
            pGlobalOk.open();
            pGlobalOk.dltText = "Resetting database: Error while deleting database!"
            pGlobalOk.dltDesc = desc
        }


        function onSignalInitializedWithTags()
        {
            pGlobalOk.open();
            pGlobalOk.dltText = "Database reseted successfully!"
            pGlobalOk.dltDesc = "" // need to be set because something might left last after error
        }
        function onSignalInitializeWithTagsFailed(desc)
        {
            console.log(desc);
            pGlobalOk.open();
            pGlobalOk.dltText = "Resetting database: Error while initializing database with tags!"
            pGlobalOk.dltDesc = desc
        }


    }

    Popup1{
        id: pGlobalOk
    }

    Popup2{
        id: pDeleteDatabaseConfirm
        dltText: "Are you sure to delete the database?"

        dltTextLB: "Cancel"
        dltTextRB: "Delete"

        onDltClickedRB: {
            backend.database.deleteDatabase();
            // initialized after signal
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
        ListView{
            id: dbManagementListView
            anchors.fill: parent
            model: mdl

            clip: true

            boundsBehavior: Flickable.StopAtBounds

            footer: Item{
                width: parent.width
                height: delegateHeight/2
            }

            // on this site scroll position will be saved allways or never cause do not contains any sub sites
            Component.onCompleted: {
                contentY = root.last_pos_db_management
            }
            Component.onDestruction: {
                if(backend.personalization.alwaysKeepListPos)
                    root.last_pos_db_management = contentY
                else
                    root.last_pos_db_management = 0
            }

            delegate: Loader{
                width: parent.width
                height: delegateHeight

                sourceComponent: {
                    if(false);
                    else if(modelData.id === 100) delegateFileSaveButtonField;
                    else if(modelData.id === 200) delegateFileSelectButtonField;
                    else if(modelData.id === 300) delegateFileSelectButtonField;
                    else if(modelData.id === 400) delegateDeleteDatabase;
                }

                Component {
                    id: delegateFileSaveButtonField

                    FileSaveButtonField{
                        delegate_text: modelData.name
                        onOwnSelectedFileChanged: {
                            if(modelData.id === 100)
                                backend.database.exportDatabase(ownSelectedFile);
                        }
                    }
                }

                Component {
                    id: delegateFileSelectButtonField

                    FileSelectButtonField{
                        delegate_text: modelData.name
                        onOwnSelectedFileChanged: {
                            if(modelData.id === 200) // tags
                                backend.database.importTagsToDatabase(ownSelectedFile);
                            else if(modelData.id === 300) // songs
                                backend.database.importSongsToDatabase(ownSelectedFile);
                        }
                    }
                }

                Component{
                    id: delegateDeleteDatabase

                    JustButtonField{
                        delegate_text: modelData.name
                        onButtonClicked: {
                            if(modelData.id === 400)
                                pDeleteDatabaseConfirm.open();
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

        TabButton{
            anchors{
                left: parent.left
                top: parent.top
                bottom: parent.bottom
            }
            width: height * 5/3 // height + height/2 // is golden ratio

            text: "←"
            font.pixelSize: 30

            ToolTip.visible: hovered
            ToolTip.text: "Go Back"

            onClicked: {
                mainLoader.anchors.top = tabBar.bottom
                settingsLoader.source = root.pps_settings
            }
        }

        Text{
            text: "Database Management"
            anchors.centerIn: parent
            font.pixelSize: 18
            color: root.color_accent1
        }
    }

}
