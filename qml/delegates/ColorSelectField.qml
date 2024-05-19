import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Dialogs

import "qrc:/SortManager-Music/qml/delegates/common" // LeftText

// is used in Personalizations.qml view to select color (accent color)
Item{
    id: colorSelectField
    anchors.fill: parent

    property string dltText: ""
    property string dltDesc: ""
    property var dltColor: null
    property var dltChoosedColor: null
    signal dltColorChoosed()
    // reacting on dltColor changed wasn't update when user changed theme
    //      so I changes local vairalble "dltChoosedColor" and emit signal dltColorChoosed
    //      and thats wokrs and when app_theme (that responds for a color_accent2)
    //      is changed accent2 color is also changed

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

        Rectangle{
            id: colorPicker
            color: "transparent"
            anchors{
                verticalCenter: parent.verticalCenter
                right: parent.right
                rightMargin: 32
            }
            width: 54
            height: colorSelectField.height * 3/5

            border.color: {
                dltColor
            }
            border.width: 2

            Rectangle{
                id: colorArea
                color: parent.border.color
                anchors{
                    margins: parent.border.width
                    top: parent.top
                    bottom: colorIDText.top
                    left: parent.left
                    right: parent.right
                }
                height: parent.height * 3/6
            }
            Text{
                id: colorIDText
                color: parent.border.color
                font.pixelSize: 10
                anchors{
                    horizontalCenter: parent.horizontalCenter
                    bottom: parent.bottom
                    bottomMargin: parent.border.width
                }
                text: parent.border.color
            }
            MouseArea{
                anchors.fill: parent
                onClicked: colorDialog.open()
            }
        }
        ColorDialog {
            id: colorDialog
            title: "Please choose a color"
            onAccepted: {
                dltChoosedColor = colorDialog.selectedColor
                dltColorChoosed()
            }
            onRejected: {
                colorDialog.close()
            }
        }
    }
}
