import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Dialogs

Item{
    id: fileSaveButtonField
    anchors.fill: parent

    property string delegate_text: ""

    property var ownSelectedFile: null

    TabButton{
        width: parent.width
        height: parent.height

        ToolTip.visible: hovered && (text.contentWidth > text.width)
        ToolTip.text: delegate_text

        onClicked: {
            fileDialog.open();
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

    FileDialog {
        id: fileDialog
        // currentFolder: StandardPaths.standardLocations("C:/0_Vigiland - Friday Night‬‬‬")[0] //new URL("C:\\")
        fileMode: FileDialog.SaveFile

        onAccepted: {
            ownSelectedFile = fileDialog.selectedFile;

            // if(model.name === "Export database")
            //     backend.database.exportDatabase(fileDialog.selectedFile);
            // else
            //     backend.database.importDatabase(fileDialog.selectedFile);
            // fileDialogText.text = fileDialog.selectedFile;
            // console.log("file: " + delegate_value)
        }
        onRejected: fileDialog.close()
    }
}
