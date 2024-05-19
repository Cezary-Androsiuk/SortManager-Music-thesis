import QtCore // StandardPaths
import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Dialogs
import Qt5Compat.GraphicalEffects

import "qrc:/SortManager-Music/qml/delegates/common" // LeftText

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
                bottom: parent.bottom
                right: parent.right
            }

            // height: fileDialogText.height
            width: height * 0.7

            Image{
                id: img
                fillMode: Image.PreserveAspectFit
                anchors{
                    fill: parent
                    topMargin: 15
                    margins: parent.width * 0.15
                }

                source: "qrc:/SortManager-Music/assets/icons/select_file-idle.png"
            }

            ColorOverlay {
                anchors.fill: img
                source: img
                color: root.dark_theme ? rgb(96,96,96) : rgb(158,158,158)
                opacity: dltEnabled ? 1.0 : 0.4
            }

            MouseArea{
                anchors.fill: parent
                hoverEnabled: true

                enabled: dltEnabled

                onEntered: {
                    img.source = "qrc:/SortManager-Music/assets/icons/select_file-hover.png"
                }
                onExited: {
                    img.source = "qrc:/SortManager-Music/assets/icons/select_file-idle.png"
                }
                onPressed: fileDialog.open()
            }

            // ToolTip.visible: hovered
            // ToolTip.text: "Select File"
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
