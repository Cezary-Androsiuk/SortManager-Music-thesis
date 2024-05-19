import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material

Item{
    anchors.fill: parent

    property string delegate_text: ""

    signal buttonClicked()

    TabButton{
        width: parent.width
        height: parent.height

        ToolTip.visible: hovered && (text.contentWidth > text.width)
        ToolTip.text: delegate_text

        onClicked: {
            buttonClicked()
        }
    }

    Text{
        id: text
        anchors{
            verticalCenter: parent.verticalCenter
            left: parent.left
            leftMargin: 20
            right: parent.right
            rightMargin: 10
        }
        text: delegate_text
        font.pixelSize: 15
        color: root.color_accent1
        verticalAlignment: Text.AlignVCenter

        clip: true
    }

}
