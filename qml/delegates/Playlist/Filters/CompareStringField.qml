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
    property int dltValue: 0

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
            {text: "...", desc: "do not compare"},
            // {text: "⋅", desc: "do not compare"},
            // {text: "~", desc: "do not compare"},
            // {text: "⋯", desc: "do not compare"},
            // {text: "⋮", desc: "do not compare"},
            {text: "=", desc: "is equal to"},
            {text: "≠", desc: "is different than"},
            {text: "≈", desc: "as case insensitive, is equal to"},
            {text: "≉", desc: "as case insensitive, is different than"},
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
