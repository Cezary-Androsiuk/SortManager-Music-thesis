import QtQuick 2.15

Item {
    // just redefine anchors
    anchors{
        left: parent.left
        right: parent.right
    }
    height: 1
    width: 1

    Rectangle{
        anchors.fill: parent
        opacity: 0.3
        color: root.color_accent2
    }
}
