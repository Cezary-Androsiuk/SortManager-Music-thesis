import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material
import Qt5Compat.GraphicalEffects

import "../common" // LeftText
import "qrc:/SortManager-Music/qml/components" // ImageButton

Item{
    id: listButtonField
    anchors.fill: parent

    property string dltText: ""
    property bool isCurrentSong: false
    property bool isNextSong: false

    // signal dltClickedElement() // limited for june
    signal dltClickedPlay()

    LeftText{
        id: leftText
        dltText: listButtonField.dltText
        // dltDesc: "some text"
        dltAnchorRight: playButtonContainer.left
        // onClicked: dltClickedElement()
    }

    Rectangle{
        anchors.fill: leftText
        color: root.color_accent2
        opacity: {
            if(listButtonField.isCurrentSong)
                0.5
            else if(listButtonField.isNextSong)
                0.2
            else
                0.0
        }
    }

    Item{
        id: playButtonContainer
        anchors{
            top: parent.top
            bottom: parent.bottom
            right: parent.right
        }
        width: height

        ImageButton{
            id: headImage
            dltDescription: "play"
            dltImageIdle: {
                if(listButtonField.isCurrentSong)
                {
                    if(listButtonField.isNextSong)
                        Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/player/play_64px.png")// current and next song icon
                    else
                        Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/player/play_64px.png")// current song icon
                }
                else if(listButtonField.isNextSong)
                    Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/player/play_64px.png")// next song icon
                else
                    Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/player/play_64px.png")
            }

            dltImageHover: dltImageIdle
            onUserClicked: {
                dltClickedPlay()
            }
            dltImageMarginsRatio: 0.12
        }
    }

}
