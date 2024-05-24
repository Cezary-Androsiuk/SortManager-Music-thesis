import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Dialogs
import Qt5Compat.GraphicalEffects

import "qrc:/SortManager-Music/qml/delegates/common" // LeftText
import "qrc:/SortManager-Music/qml/components" // ImageButton

Item {
    id: pathSelectField
    anchors.fill: parent

    property var dltText: null
    property string dltDesc: ""
    property string dltValue: ""
    property bool dltEnabled: true

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
            top: parent.top
            bottom: parent.bottom
            right: parent.right
            rightMargin: 20
        }
        width: parent.width * 0.4

        TextField{
            id: folderDialogText
            anchors{
                top: parent.top
                left: parent.left
                bottom: parent.bottom
                right: folderDialogButton.left
            }
            x: -10

            font.pixelSize: 15
            clip: true

            enabled: dltEnabled

            text: dltValue;

            onEditingFinished: {
                text = dltValue;
            }

            ToolTip.visible: hovered && (width < (contentWidth * 1.3))
            ToolTip.text: text
        }

        Item{
            id: folderDialogButton
            anchors{
                top: parent.top
                bottom: parent.bottom
                right: parent.right
            }

            // height: folderDialogText.height
            width: height// * 0.7


            // ImageButton{
            //     dltDescription: "Shuffle Songs"
            //     dltImageIdle: Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/shuffle_36px.png")
            //     dltImageHover: dltImageIdle
            //     onUserClicked: {}
            // }

            // Image{
            //     id: img
            //     fillMode: Image.PreserveAspectFit
            //     anchors{
            //         fill: parent
            //         topMargin: 15
            //         margins: parent.width * 0.15
            //     }

            //     source: "qrc:/SortManager-Music/assets/icons/select_file-idle.png"
            // }

            // ColorOverlay {
            //     anchors.fill: img
            //     source: img
            //     color: root.dark_theme ? rgb(96,96,96) : rgb(158,158,158)
            //     opacity: dltEnabled ? 1.0 : 0.4
            // }

            // MouseArea{
            //     anchors.fill: parent
            //     hoverEnabled: true

            //     enabled: dltEnabled

            //     onEntered: {
            //         img.source = "qrc:/SortManager-Music/assets/icons/select_file-hover.png"
            //     }
            //     onExited: {
            //         img.source = "qrc:/SortManager-Music/assets/icons/select_file-idle.png"
            //     }
            //     onPressed: folderDialog.open()
            // }

            // ToolTip.visible: hovered
            // ToolTip.text: "Select File"
        }

        FolderDialog {
            id: folderDialog
            title: "Select a Path"
            options: FolderDialog.ShowDirsOnly

            currentFolder: dltValue

            onAccepted: {
                folderDialogText.text = folderDialog.selectedFolder;
                dltValue = folderDialog.selectedFolder
            }
            onRejected: folderDialog.close()
        }

    }
}
