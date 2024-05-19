import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material

import "qrc:/SortManager-Music/qml/delegates/common" // LeftText

// is used in Personalizations.qml (to select default add tag type), AddTag.qml and EditTag.qml to view or change tag type
Item{
    id: comboBoxField
    anchors.fill: parent

    property string dltText: ""
    property string dltDesc: ""
    property int dltValue: -1
    property bool dltEnabled: true
    property var dltModel: null

    LeftText{
        id: textElement
        dltAnchorRight: textRightElement.left
        dltRightMargin: 10
        dltText: parent.dltText
        dltDesc: parent.dltDesc
    }

    Item{
        id: textRightElement
        anchors{
            top: parent.top
            bottom: parent.bottom
            right: parent.right
            rightMargin: 20
        }
        width: parent.width * 0.4

        ComboBox{
            anchors.fill: parent
            anchors.topMargin: 3 // top and bottom are not equal :c
            anchors.bottomMargin: -3 // top and bottom are not equal :c
            model: dltModel
            enabled: dltEnabled
            currentIndex: dltValue
            font.pixelSize: 15

            onCurrentIndexChanged: {
                dltValue = currentIndex;
            }
        }
    }
}
