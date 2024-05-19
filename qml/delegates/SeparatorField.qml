import QtQuick 2.15

Item{
    id: separatorField
    anchors.fill: parent

    Component.onCompleted: {
        parent.height = delegateHeight/5 // loader height // used
    }

    Rectangle{
        anchors{
            top: parent.top
            topMargin: 10 // top and bottom are not equal :c
            left: parent.left
            leftMargin: 20
            right: parent.right
            rightMargin: 20
        }
        height: 1
        opacity: 0.3
        color: root.color_accent1
    }
}
