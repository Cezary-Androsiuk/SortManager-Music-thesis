import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material

import "qrc:/SortManager-Music/qml/delegates/common" // LeftText
import "qrc:/SortManager-Music/qml/delegates/Playlist/Filters/common" // ComparatorComponent

Item{
    id: compareStateField
    anchors.fill: parent

    property string dltText: ""
    property string dltValue: ""

    LeftText{
        id: textElement
        dltAnchorRight: textMiddleElement.left
        dltText: compareStateField.dltText
        dltRightMargin: 5
    }

    Item{
        id: textMiddleElement
        anchors{
            top: parent.top
            bottom: parent.bottom
            right: textRightElement.left
        }
        width: height
    }

    Item{
        id: textRightElement
        anchors{
            top: parent.top
            bottom: parent.bottom
            right: parent.right
            rightMargin: 20
        }
        width: parent.width * 0.3

    }
}
