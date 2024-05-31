import QtQuick 2.15

Item {
    id: emptyListInfo
    anchors.centerIn: parent

    required property string text

    Text{
        anchors.centerIn: parent
        text: emptyListInfo.text
        font.pixelSize: 20
        color: root.color_accent2
    }
}
