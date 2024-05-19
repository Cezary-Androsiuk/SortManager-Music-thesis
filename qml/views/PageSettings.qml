import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

import "qrc:/SortManager-Music/qml/components"

Page {
    anchors.fill: parent

    // StackView{
    //     id: settingsStackView
    //     anchors{
    //         top: parent.top
    //         left: parent.left
    //         right: parent.right
    //         bottom: bottomPlayer.top
    //     }
    //     initialItem: root._SourcePathPageSettingsSettings
    // }

    Loader{
        id: settingsLoader
        anchors{
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: bottomPlayer.top
        }
        source: root.pps_settings
    }

    BottomPlayer {
        id: bottomPlayer
    }

}
