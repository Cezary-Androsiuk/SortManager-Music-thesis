import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

import "qrc:/SortManager-Music/qml/components" // BottomPlayer

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


    // will be hidden behind bottom player
    Rectangle {
        id: footer
        anchors{
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        height: 30
        color: root.color_background

        Label {
            id: footerText
            anchors.centerIn: parent
            text: qsTr("Cezary Androsiuk (2024) UwB")
        }
    }

    BottomPlayer {
        id: bottomPlayer
    }

}
