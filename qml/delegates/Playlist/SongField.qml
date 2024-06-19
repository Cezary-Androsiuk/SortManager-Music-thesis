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

    // signal dltClickedElement() // limited for june
    signal dltClickedPlay()

    LeftText{
        id: leftText
        dltText: listButtonField.dltText
        // dltDesc: "some text"
        dltAnchorRight: playButtonContainer.left
        // onClicked: dltClickedElement()
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
            // Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/player/pause_64px.png")
            dltImageIdle: Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/player/play_64px.png")
            dltImageHover: dltImageIdle
            onUserClicked: {
                dltClickedPlay()
            }
            dltImageMarginsRatio: 0.12
        }
    }

}
