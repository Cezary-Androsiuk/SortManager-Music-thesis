import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

import "qrc:/SortManager-Music/qml/views"
import "qrc:/SortManager-Music/qml/popups"

ApplicationWindow {
    id: root
    property var backend: Backend
    property int _w: 400
    property int _h: 600
    minimumWidth: _w
    maximumWidth: _w
    minimumHeight: _h
    maximumHeight: _h

    visible: true
    title: qsTr("SortManager Music")

    function rgb(r, g, b, a=255){
        return Qt.rgba(r/255, g/255, b/255, a/255);
    }




    Component.onCompleted: {
        console.log("qml initialized")
        Backend.checkPersonalization() // first step to check initialization
        // this way allow to display popup when anything failed in loading backend
    }



    Connections{
        target: Backend
        function onPersonalizationLoaded(){
            // do nothing cause this was the last step
        }

        function onPersonalizationLoadError(errorCode){
            pPersonalizationLoadError.open()
            if(errorCode === "10")
                pPersonalizationLoadError.dltDesc = "file personalization.json not found!"
            else
                pPersonalizationLoadError.dltDesc = "error code: " + errorCode
        }

        function onBackendInitialized(){
            console.log("backend initialized")
        }
    }

    Connections{
        target: Backend.database
        // initialize database on start
        function onSignalInitializedOnStart()
        {
            console.log("received signal onSignalInitializedOnStart from Database");
        }
        function onSignalInitializeOnStartFailed(desc)
        {
            console.log("received signal onSignalInitializeOnStartFailed from Database with arg: " + desc);
            // console.log("DEBUG ONLY! ----- database example data Main.qml")
            // backend.database.initializeWithTags();
            // backend.database.createExampleData();
            pDBInitializeOnStartFailed.open();
            pDBInitializeOnStartFailed.dltDesc = desc
        }

        // initialize database with tags
        function onSignalInitializedWithTags()
        {
            console.log("received signal onSignalInitializedWithTags from Database");
        }
        function onSignalInitializeWithTagsFailed(desc)
        {
            console.log("received signal onSignalInitializeWithTagsFailed from Database with arg: " + desc);
            pDBInitializeWithTagsFailed.open();
            pDBInitializeWithTagsFailed.dltDesc = desc
        }

        // --------------------------------------------- Loading Info --------------------------------------------- //
        function onSignalLoadingStarted(whatStarted)
        {
            pLoading.open()
            pLoading.dltText = whatStarted
        }
        function onSignalLoadingProgress(infoList)
        {
            var callerID = infoList[0]
            if(callerID === "importSongsToDatabase")
            {
                // for example: Loading Songs 123/456
                pLoading.dltText = infoList[1] + " " + infoList[2] + "/" + infoList[3]
            }
            else
                console.log("unknown caller with ID: " + callerID + "!")
        }
        function onSignalLoadingFinished()
        {
            // this signal is emited by all things like finished, loaded or error
            pLoading.close()
        }
    }


    PopupLoading{
        id: pLoading // can be global (in Main.qml)
        // text message will be set in caller function
        onDltClickedMB: Backend.database.cancelLoading()
    }

    Popup3{
        id: pPersonalizationLoadError
        dltText: "Error while loading personalizations!"
        // text description set in onPersonalizationLoadError

        dltTextLB: "Retry"
        dltTextMB: "Default"
        dltTextRB: "Exit"
        dltJea: false

        onDltClickedLB: {
            backend.reinitializePersonalization();
        }

        onDltClickedMB: {
            backend.useDefaultPersonalization();
        }

        onDltClickedRB: {
            root.close();
        }
    }

    Popup3{
        id: pDBInitializeOnStartFailed
        dltText: "Boot initialization database failed!"
        // text description set in onPersonalizationLoadError

        dltTextLB: "Retry"
        dltTextMB: "Recreate"
        dltTextRB: "Exit"
        dltJea: false

        onDltClickedLB: {
            backend.database.initializeOnStart();
        }

        onDltClickedMB: {
            backend.database.initializeWithTags();
        }

        onDltClickedRB: {
            root.close();
        }
    }

    Popup2{
        id: pDBInitializeWithTagsFailed
        dltText: "Initialization with tags failed!"
        // text description set in onPersonalizationLoadError

        dltTextLB: "Retry"
        dltTextRB: "Exit"
        dltJea: false

        onDltClickedLB: {
            backend.database.initializeWithTags();
        }

        onDltClickedRB: {
            root.close();
        }
    }



    // Colors
    /*
      dark_theme, color_dark_accent and color_light_accent takes info from backend
      when backend.db_app_theme is changed then dark_theme is refreshed
      backend.db_app_theme is changed by PageSettings/Personalisations
      when is changed information goes to backend and there are handled and saved to conf.json
      at this point backend emits signal that backend.db_app_theme was changed and so on..
      */
    property bool dark_theme: backend.personalization.isDarkTheme
    property color color_dark_accent: backend.personalization.darkAccentColor
    property color color_light_accent: backend.personalization.lightAccentColor

    property color color_accent1: dark_theme ? rgb(255,255,255) : rgb(0,0,0)
    property color color_accent2: dark_theme ? color_dark_accent : color_light_accent
    property color color_mouse_hover: dark_theme ? rgb(56,56,59) : rgb(225,221,224)
    property color color_background: dark_theme ? rgb(28,27,31) : rgb(255,251,254)
    function color_background_opacity(opacity){
        return dark_theme ? rgb(28,27,31,opacity) : rgb(255,251,254,opacity)
    }

    property color color_element_idle: color_accent1
    property color color_element_hover: dark_theme ? rgb(140, 140, 140) : rgb(110, 110, 110)
    property color color_element_press: color_accent2
    property color color_button_hover: dark_theme ? rgb(48,47,51) : rgb(195,191,194)

    property color color_mouse_press: dark_theme ? rgb(81,81,84) : rgb(195,191,194)

    Material.theme: dark_theme ? Material.Dark : Material.Light
    Material.accent: root.color_accent2

    /*
      mainLoader switch between [PageSettings | PagePlayer | PagePlaylist] their are something like virtual pages (not visual)
      each page contains and switched between their sub pages

      prefix for names are just acronyms for pages you need to go through for this page
      ex: ppsst == path_page_settings_settings_tags
      */

    // Pages prefix
    property string page_prefix:                "qrc:/SortManager-Music/qml/views"
    // Player
    property string path_page_player:           page_prefix + "/PagePlayer.qml"
    // Playlist
    property string path_page_playlist:         page_prefix + "/PagePlaylist.qml"
    property string ppp_playlist:               page_prefix + "/PagePlaylist/Playlist.qml"
    property string pppp_editplaylistsong:      page_prefix + "/PagePlaylist/EditPlaylistSong.qml"
    property string pppp_filters:               page_prefix + "/PagePlaylist/Filters.qml"
    // Settings
    property string path_page_settings:         page_prefix + "/PageSettings.qml"
    property string pps_settings:               page_prefix + "/PageSettings/Settings.qml"
    property string ppss_personalizations:      page_prefix + "/PageSettings/AppSettings/Personalizations.qml"
    property string ppss_dbmanagement:          page_prefix + "/PageSettings/AppSettings/DBManagement.qml"
    property string ppss_songs:                 page_prefix + "/PageSettings/Songs/Songs.qml"
    property string ppsss_addsong:              page_prefix + "/PageSettings/Songs/AddSong.qml"
    property string ppsss_editsong:             page_prefix + "/PageSettings/Songs/EditSong.qml"
    property string ppss_tags:                  page_prefix + "/PageSettings/Tags/Tags.qml"
    property string ppsst_addtag:               page_prefix + "/PageSettings/Tags/AddTag.qml"
    property string ppsst_edittag:              page_prefix + "/PageSettings/Tags/EditTag.qml"

    property string current_main_loader_page: path_page_playlist//path_page_player


    // ListView last positions
    property double last_pos_settings: 0
    property double last_pos_personalizations: 0
    property double last_pos_db_management: 0
    property double last_pos_songs: 0
    property double last_pos_add_song: 0
    property double last_pos_edit_song: 0
    property double last_pos_tags: 0
    property double last_pos_add_tag: 0
    property double last_pos_edit_tag: 0
    property double last_pos_playlist: 0
    property double last_pos_edit_playlist_song: 0
    property double last_pos_filters: 0

    Item{
        id: rootContainer
        anchors.fill: parent

        TabBar {
            id: tabBar

            anchors{
                top: parent.top
                left: parent.left
                right: parent.right
            }
            // height: 300
            currentIndex: 1


            // onFocusChanged: {
            //     if(focus)
            //         rootContainer.focus = true
            // }

            TabButton {
                text: qsTr("Settings")
                font.pixelSize: 20
            }
            TabButton {
                text: qsTr("Player")
                font.pixelSize: 20
            }
            TabButton {
                text: qsTr("Playlist")
                font.pixelSize: 20
            }

            onCurrentIndexChanged: {
                switch (tabBar.currentIndex) {
                case 0:
                    current_main_loader_page = path_page_settings
                    break;
                case 1:
                    current_main_loader_page = path_page_player
                    break;
                case 2:
                    current_main_loader_page = path_page_playlist
                    break;
                }
            }
        }

        Loader{
            id: mainLoader
            anchors{
                top: tabBar.bottom
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }

            source: current_main_loader_page
        }

        focus: true
        // onFocusChanged: {
        //     // keep focus at any cost
        //     rootContainer.focus = true
        // }

        Keys.onEscapePressed: root.close()
        Keys.onSpacePressed: console.log("")
        Keys.onDeletePressed: backend.database.createExampleData();
        // Keys.onLeftPressed: {
        //     if(tabBar.currentIndex > 0)
        //         tabBar.currentIndex --;
        // }
        // Keys.onRightPressed: {
        //     if(tabBar.currentIndex < 2)
        //         tabBar.currentIndex ++;
        // }
    }


    onClosing: {
        console.log("closing...")
    }
}
