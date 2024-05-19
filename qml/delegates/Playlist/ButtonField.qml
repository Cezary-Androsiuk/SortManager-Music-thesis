import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material
import Qt5Compat.GraphicalEffects

import "qrc:/SortManager-Music/qml/components"

Item{
    id: listButtonField
    anchors.fill: parent

    property string dltText: ""

    signal dltClickedElement()
    signal dltClickedPlay()

    TabButton{
        id: elementButton
        anchors{
            top: parent.top
            bottom: parent.bottom
            left: parent.left
            right: {
                playSongButton.left
            }
        }

        ToolTip.visible: hovered && (text.contentWidth > text.width)
        ToolTip.text: dltText

        onClicked: dltClickedElement()

        Text{
            id: text
            anchors{
                verticalCenter: parent.verticalCenter
                left: parent.left
                leftMargin: 20
                right: parent.right
                rightMargin: 10
            }
            text: dltText
            font.pixelSize: 15
            color: root.color_accent1
            verticalAlignment: Text.AlignVCenter

            // elide: Text.ElideRight
            clip: true
        }
    }




    TabButton{
        id: playSongButton
        height: parent.height
        width: height

        anchors{
            top: parent.top
            right: parent.right
        }

        text: "▶" //(index !== 2 ? "▶" : "")
        font.pixelSize: 30
        onClicked: dltClickedPlay()
    }

}
