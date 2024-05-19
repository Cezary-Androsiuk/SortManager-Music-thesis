import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material

import "qrc:/SortManager-Music/qml/components" // TriSwitch
import "qrc:/SortManager-Music/qml/delegates/common" // LeftText

Item{
    id: switchField
    anchors.fill: parent

    property string dltText: ""
    property string dltDesc: ""
    property var dltValue: null
    property bool dltEnabled: true

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
            right: parent.right
            top: parent.top
            bottom: parent.bottom
        }
        width: parent.width * 0.25

        TriSwitch{
            anchors{
                verticalCenter: parent.verticalCenter
                right: parent.right
                rightMargin: 60
            }

            enabled: dltEnabled

            state: 0

            Component.onCompleted: {
                // init component
                // this will be destroyed after scroll
                state = + dltValue;
            }
            onStateChanged: {
                dltValue = ""+state;
            }
        }

    }

}
