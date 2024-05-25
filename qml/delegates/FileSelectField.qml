import QtCore // StandardPaths
import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Dialogs
import Qt5Compat.GraphicalEffects

import "qrc:/SortManager-Music/qml/delegates/common" // LeftText
import "qrc:/SortManager-Music/qml/components" // ImageButton

// is used in EditPlaylistSong.qml (change song path), AddSong.qml (select song path),
//    EditSong.qml (change song path) and EditTag.qml (change one of the songs song path)
Item{
    id: fileSelectField
    anchors.fill: parent

    property var dltText: null
    property string dltDesc: ""
    property string dltValue: ""
    property bool dltEnabled: true
    property string dltStartFolder: ""


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
            id: fileDialogText
            anchors{
                top: parent.top
                left: parent.left
                bottom: parent.bottom
                right: fileDialogButton.left
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
            id: fileDialogButton
            anchors{
                top: parent.top
                topMargin: 10 // to align with fileDialogText field
                bottom: parent.bottom
                right: parent.right
            }

            // height: fileDialogText.height
            width: height * 0.7

            ImageButton{
                dltDescription: "Select File"
                dltImageIdle: Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/folder_open_64px.png")
                dltImageHover: dltImageIdle
                onUserClicked: fileDialog.open()
            }

        }

        FileDialog {
            id: fileDialog
            title: "Select a File"
            fileMode: FileDialog.OpenFile

            currentFolder: dltStartFolder

            onCurrentFolderChanged: {}

            onAccepted: {
                fileDialogText.text = fileDialog.selectedFile;
                dltValue = fileDialog.selectedFile;
            }
            onRejected: fileDialog.close()
        }

    }
}
