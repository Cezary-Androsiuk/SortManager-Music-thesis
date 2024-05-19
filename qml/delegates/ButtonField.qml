import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material
import Qt5Compat.GraphicalEffects

// is used in Tags.qml and Songs.qml views, can open element details or delete element right in
Item{
    id: buttonField
    anchors.fill: parent

    property string dltText: ""
    property bool dltIsDeletable: true

    signal dltClickedElement()
    signal dltClickedDelete()

    TabButton{
        id: elementButton
        anchors{
            top: parent.top
            bottom: parent.bottom
            left: parent.left
            right: {
                if(dltIsDeletable)
                    deleteButton.left
                else
                    parent.right
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

    Item{
        id: deleteButton
        height: parent.height
        width: height

        anchors{
            top: parent.top
            right: parent.right
        }

        visible: dltIsDeletable

        Image{
            id: img
            fillMode: Image.PreserveAspectFit
            anchors{
                fill: parent
                margins: parent.width * 0.20
            }
            mipmap: true // smooths image

            source: "qrc:/SortManager-Music/assets/icons/trash_close.png"
        }

        ColorOverlay {
            id: colorOverlay
            anchors.fill: img
            source: img
            color: root.dark_theme ? rgb(96,96,96) : rgb(158,158,158)
        }

        MouseArea{
            anchors.fill: parent
            hoverEnabled: true

            onEntered: {
                img.source = "qrc:/SortManager-Music/assets/icons/trash_open.png";
                colorOverlay.color = root.rgb(180, 50, 50);
            }
            onExited: {
                img.source = "qrc:/SortManager-Music/assets/icons/trash_close.png";
                colorOverlay.color = root.dark_theme ? rgb(96,96,96) : rgb(158,158,158);
            }
            onPressed: dltClickedDelete()
        }

    }
}
