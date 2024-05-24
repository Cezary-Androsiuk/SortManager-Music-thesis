import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material

import "qrc:/SortManager-Music/qml/delegates/common" // LeftText
import "qrc:/SortManager-Music/qml/delegates/Playlist/Filters/common" // ComparatorComponent

Item{
    id: compareStringField
    anchors.fill: parent

    property string dltText: ""
    property int dltComboboxValue: 0
    property string dltValue: ""

    LeftText{
        id: textElement
        dltAnchorRight: textMiddleElement.left
        dltText: compareStringField.dltText
        dltRightMargin: 5
    }

    ComparatorComponent{
        id: textMiddleElement
        dltAnchorRight: textRightElement.left
        dltModel: [
            // {image: "...", desc: "do not compare"},
            // {image: "=", desc: "is equal to"},
            // {image: "≠", desc: "is different than"},
            // {image: "≈", desc: "as case insensitive, is equal to"},
            // {image: "≉", desc: "as case insensitive, is different than"}
            {image: Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/more_64px.png"), desc: "do not compare"},
            {image: Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/equal_sign_64px.png"), desc: "is equal to"},
            {image: Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/not_equal_64px.png"), desc: "is equal to"},
            {image: Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/approximately_equal_64px.png"), desc: "is equal to"},
            {image: Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/approximately_not_equal_64px.png"), desc: "is equal to"}
        ]
        width: height
        dltIndex: dltComboboxValue
        onDltIndexChanged: {
            if(dltComboboxValue !== dltIndex)
                dltComboboxValue = dltIndex
        }
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

        TextField{
            anchors{
                fill: parent
                bottomMargin: 10
            }

            font.pixelSize: 15
            clip: true

            opacity:{
                if(dltComboboxValue === 0)
                    0.1
                else 1
            }

            Component.onCompleted: {
                // init component
                // this will be destroyed after scroll
                text = dltValue;
            }

            onEditingFinished: {
                dltValue = text;
            }

            ToolTip.visible: hovered && (width < (contentWidth * 1.7))
            ToolTip.text: text
        }
    }
}
