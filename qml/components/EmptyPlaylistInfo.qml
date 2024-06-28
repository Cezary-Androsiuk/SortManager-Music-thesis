import QtQuick 2.15

Item {
    id: emptyPlaylistInfo
    anchors.fill: parent

    property bool showAreas: false
    property double topMargin: 0
    property double leftMargin: 0
    property double rightMargin: 0
    property double bottomMargin: 0
    property double distanceBetweenTexts: 4 // margin between infoText1 and infoText2

    property string largeTextString: "Playlist is empty"
    property int largeTextPixelSize: 23
    property string smallTextString: "Add songs to Playlist to listen to music"
    property int smallTextPixelSize: 16

    property double scale: 1

    anchors{
        fill: parent
        // change position of text by adjusting margins
        topMargin: emptyPlaylistInfo.topMargin
        leftMargin: emptyPlaylistInfo.leftMargin
        rightMargin: emptyPlaylistInfo.rightMargin
        bottomMargin: emptyPlaylistInfo.bottomMargin
    }

    Item{
        id: centeredAnchor
        anchors.centerIn: parent
        width: parent.width * 0.6 // aesthetic reason
        height: distanceBetweenTexts
        Rectangle{anchors.fill: parent; color: "red"; opacity: 0.2; visible: showAreas}
    }

    Text{
        id: infoText1
        anchors{
            bottom: centeredAnchor.top
            horizontalCenter: parent.horizontalCenter
        }

        text: largeTextString

        font.pixelSize: largeTextPixelSize * scale
        color: root.color_accent2
    }

    Text{
        id: infoText2
        anchors{
            top: centeredAnchor.bottom
            horizontalCenter: parent.horizontalCenter
        }

        text: smallTextString

        font.pixelSize: smallTextPixelSize * scale
        color: root.color_accent2
    }

    Rectangle{anchors.fill: parent; color: "blue"; opacity: 0.2; visible: showAreas}
}
