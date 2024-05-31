cmake_minimum_required(VERSION 3.16)

project(SortManager-Music-v3 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

# cmake_policy(SET CMP0071 NEW)

find_package(Qt6 6.4 REQUIRED COMPONENTS Quick Sql Multimedia)

qt_standard_project_setup()

qt_add_executable(appSortManager-Music
    main.cpp
)

qt_add_qml_module(appSortManager-Music
    URI SortManager-Music
    VERSION 1.0


    # sources
    SOURCES cpp/Backend/Backend.h cpp/Backend/Backend.cpp
    SOURCES cpp/Database/Database.h cpp/Database/Database.cpp
    SOURCES cpp/DebugPrint/DebugPrint.h
    SOURCES cpp/Song/Song.h cpp/Song/Song.cpp
    SOURCES cpp/Song/SongDetails.h cpp/Song/SongDetails.cpp
    SOURCES cpp/Song/SongList.h cpp/Song/SongList.cpp
    SOURCES cpp/Tag/Tag.h cpp/Tag/Tag.cpp
    SOURCES cpp/Tag/TagDetails.h cpp/Tag/TagDetails.cpp
    SOURCES cpp/Tag/TagList.h cpp/Tag/TagList.cpp
    SOURCES cpp/Player/Player.h cpp/Player/Player.cpp
    SOURCES cpp/Playlist/Playlist.h cpp/Playlist/Playlist.cpp
    SOURCES cpp/Personalization/Personalization.h cpp/Personalization/Personalization.cpp
    SOURCES cpp/Filter/Filter.h cpp/Filter/Filter.cpp
    SOURCES cpp/Tag/TagWithComparator.h cpp/Tag/TagWithComparator.cpp
    SOURCES cpp/Song/SongDetailsList.h cpp/Song/SongDetailsList.cpp

    # QML:
    QML_FILES qml/Main.qml

    # pages
    QML_FILES qml/views/PagePlayer.qml
    QML_FILES qml/views/PagePlaylist.qml
    QML_FILES qml/views/PagePlaylist/Playlist.qml
    QML_FILES qml/views/PageSettings.qml
    QML_FILES qml/views/PageSettings/Settings.qml
    QML_FILES qml/views/PageSettings/AppSettings/Personalizations.qml
    QML_FILES qml/views/PageSettings/AppSettings/DBManagement.qml
    QML_FILES qml/views/PageSettings/Songs/Songs.qml
    QML_FILES qml/views/PageSettings/Songs/EditSong.qml
    QML_FILES qml/views/PageSettings/Songs/AddSong.qml
    QML_FILES qml/views/PageSettings/Tags/Tags.qml
    QML_FILES qml/views/PageSettings/Tags/EditTag.qml
    QML_FILES qml/views/PageSettings/Tags/AddTag.qml
    QML_FILES qml/views/PagePlaylist/Filters.qml
    QML_FILES qml/views/PagePlaylist/EditPlaylistSong.qml

    # components
    QML_FILES qml/components/BottomPlayer.qml
    QML_FILES qml/components/TriSwitch.qml
    QML_FILES qml/components/BetterButton.qml
    QML_FILES qml/components/ImageButton.qml
    QML_FILES qml/components/InfoToolTip.qml
    QML_FILES qml/components/ThinSeparator.qml
    QML_FILES qml/components/EmptyListInfo.qml
    QML_FILES qml/views/PagePlaylist/components/PlaylistHeader.qml

    # delegates
    QML_FILES qml/delegates/FileSelectField.qml
    QML_FILES qml/delegates/TriSwitchField.qml
    QML_FILES qml/delegates/StringField.qml
    QML_FILES qml/delegates/IntegerField.qml
    QML_FILES qml/delegates/NotChosenField.qml
    QML_FILES qml/delegates/SeparatorField.qml
    QML_FILES qml/delegates/ComboBoxField.qml
    QML_FILES qml/delegates/ButtonField.qml
    QML_FILES qml/delegates/DeleteField.qml
    QML_FILES qml/delegates/SwitchField.qml
    QML_FILES qml/delegates/PathSelectField.qml
    QML_FILES qml/delegates/ColorSelectField.qml
    QML_FILES qml/delegates/common/LeftText.qml
    QML_FILES qml/delegates/DBManagement/FileSaveButtonField.qml
    QML_FILES qml/delegates/DBManagement/FileSelectButtonField.qml
    QML_FILES qml/delegates/DBManagement/JustButtonField.qml
    QML_FILES qml/delegates/Playlist/ButtonField.qml
    QML_FILES qml/delegates/Playlist/Filters/CompareIntegerField.qml
    QML_FILES qml/delegates/Playlist/Filters/CompareStringField.qml
    QML_FILES qml/delegates/Playlist/Filters/CompareStateField.qml
    QML_FILES qml/delegates/Playlist/Filters/common/ComparatorComponent.qml

    # popup
    QML_FILES
    QML_FILES qml/popups/Popup1.qml
    QML_FILES qml/popups/Popup2.qml
    QML_FILES qml/popups/Popup3.qml
    QML_FILES qml/popups/PopupLoading.qml
    QML_FILES qml/popups/common/TextsArea.qml


    # resources
    RESOURCES assets/icons/player/end_64px.png
    RESOURCES assets/icons/player/pause_64px.png
    RESOURCES assets/icons/player/play_64px.png
    RESOURCES assets/icons/player/start_64px.png

    RESOURCES assets/icons/not_compare_64px.png
    RESOURCES assets/icons/approximately_equal_64px.png
    RESOURCES assets/icons/approximately_not_equal_64px.png
    RESOURCES assets/icons/equal_sign_64px.png
    RESOURCES assets/icons/less_or_equal_64px.png
    RESOURCES assets/icons/less_than_64px.png
    RESOURCES assets/icons/more_or_equal_64px.png
    RESOURCES assets/icons/more_than_64px.png
    RESOURCES assets/icons/not_equal_64px.png

    RESOURCES assets/icons/folder_open_64px.png
    RESOURCES assets/icons/filter_36px.png
    RESOURCES assets/icons/refresh_36px.png
    RESOURCES assets/icons/shuffle_36px.png
    RESOURCES assets/icons/info_24px.png
    RESOURCES assets/icons/trash_close_512px.png
    RESOURCES assets/icons/trash_open_512px.png

)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appSortManager-Music PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appSortManager-Music
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appSortManager-Music
    PRIVATE
    Qt6::Quick
    Qt6::Sql
    Qt6::Multimedia
)

include(GNUInstallDirs)
install(TARGETS appSortManager-Music
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
