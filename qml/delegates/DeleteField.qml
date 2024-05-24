import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material
import Qt5Compat.GraphicalEffects

// is used in EditSong.qml, EditTag.qml and EditPlaylistSong.qml to delete element
Item{
    id: deleteSongField
    anchors.fill: parent

    signal dltClicked()

    Item{
        anchors.fill: parent

        Image{
            id: img
            fillMode: Image.PreserveAspectFit
            anchors{
                fill: parent
                margins: parent.height * 0.05
            }
            mipmap: true // smooths image

            source: Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/trash_close_512px.png")
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
                img.source = Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/trash_open_512px.png");
                colorOverlay.color = root.rgb(180, 50, 50);
            }
            onExited: {
                img.source = Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/trash_close_512px.png");
                colorOverlay.color = root.dark_theme ? rgb(96,96,96) : rgb(158,158,158);
            }
            onPressed: dltClicked()
        }
    }

    // TabButton{
    //     anchors.fill: parent

    //     text: delegate_button_text
    //     font.pixelSize: 20

    //     onClicked: {
    //         dltClicked();
    //     }

    //     // background: Rectangle {
    //     //     property color idle: root.rgb(220, 30, 30, 50)
    //     //     property color hover: root.rgb(220, 30, 30, 150)
    //     //     property color press: root.rgb(220, 30, 30, 120)
    //     //         color: parent.down ? press : (parent.hovered ?  hover : idle)
    //     // }
    // }


    // Rectangle{
    //     anchors.fill: parent
    //     border.width: 3
    //     border.color: root.rgb(230, 20, 20)
    //     color: root.rgb(0, 0, 0, 0)
    // }
}
