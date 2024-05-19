import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material

Popup {
    id: popupDeleteSongConfirm

    property int delegate_id: 0
    property string delegate_text: ""
    property string delegate_element_type: ""
    property string message: "Sure you want to delete the '" + delegate_text + "' " +
                             delegate_element_type + " (id: " + delegate_id + ")?"

    height: 160
    width: 300

    x: parent.width/2 - width/2
    y: parent.height/2 - height/2

    dim: true
    modal: true
    // closePolicy: Popup.NoAutoClose

    signal deleteConfirmed(int element_id)

    Item{
        Component.onCompleted: {
            popupDeleteSongConfirm.focus = true
        }
        Keys.onEscapePressed: {
            console.log(delegate_element_type + " deletion cancelled")
            popupDeleteSongConfirm.close()
        }
    }


    Item{
        id: textArea
        anchors{
            top: parent.top
            left: parent.left
            right: parent.right
        }
        height: parent.height * 3/5

        Text{
            id: textInfo
            anchors.centerIn: parent
            text: message
            color: root.dark_theme ? "white" : "black"
            width: popupDeleteSongConfirm.width - 50
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
        }
    }


    Item{
        id: lbuttonArea
        anchors{
            top: textArea.bottom
            left: parent.left
            bottom: parent.bottom
        }
        width: parent.width/2

        Button{
            id: lbutton
            anchors.centerIn: parent
            text: "Cancel"
            onClicked: {
                console.log(delegate_element_type + " deletion cancelled")
                close()
            }
            width: 100
        }
    }

    Item{
        id: rbuttonArea
        anchors{
            top: textArea.bottom
            right: parent.right
            bottom: parent.bottom
        }
        width: parent.width/2

        Button{
            id: rbutton
            anchors.centerIn: parent
            text: "Delete"
            onClicked: {
                console.log(delegate_element_type + " deletion confirmed");
                close();
                popupDeleteSongConfirm.deleteConfirmed(delegate_id);
            }
            width: 100
        }
    }

}
