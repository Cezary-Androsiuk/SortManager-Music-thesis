import QtQuick 2.15

Item{
    property string dltText
    property string dltDesc
    readonly property bool noDesc: (dltDesc === "")
    readonly property int pwidth: parent.width

    id: areaTexts
    anchors{
        top: parent.top
        left: parent.left
        right: parent.right
    }
    height: parent.height * 4/6

    Item{
        id: areaMessage
        anchors{
            top: parent.top
            left: parent.left
            right: parent.right
        }
        height: noDesc ? parent.height : parent.height * 3/6

        Text{
            id: message
            anchors.centerIn: parent
            text: dltText
            clip: true
            font.pixelSize: 14
            color: root.color_accent1
            width: pwidth - 50
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
        }
    }
    Item{
        id: areaDescription
        anchors{
            top: areaMessage.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        visible: !noDesc

        Rectangle{
            anchors.fill: parent
            color: "white"
            opacity: 0.05
        }

        Text{
            anchors{
                top: parent.top
                left: parent.left
                right: parent.right
            }
            text: "Description"
            color: root.color_accent1
            font.pixelSize: 10
            horizontalAlignment: Text.AlignHCenter
        }

        Text{
            anchors{
                fill: parent
                leftMargin: 10
                rightMargin: 10
                topMargin: 15
            }

            text: dltDesc
            color: root.color_accent1
            wrapMode: Text.Wrap
            clip: true
            font.pixelSize: 9
        }
    }
}
