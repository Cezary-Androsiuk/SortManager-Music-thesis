import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material

import "qrc:/SortManager-Music/qml/delegates/common" // LeftText

Item{
    id: notChosenField
    anchors.fill: parent

    property string dltText: ""
    property string dltDesc: ""

    LeftText{
        id: textElement
        dltAnchorRight: parent.right
        dltRightMargin: 10
        dltText: parent.dltText
        dltDesc: parent.dltDesc
    }
}
