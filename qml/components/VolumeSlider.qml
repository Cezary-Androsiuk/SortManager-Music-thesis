import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Dialogs

// ImageButton

Item {
    id: volumeSlider
    anchors.fill: parent

    property int playerVolume: backend.personalization.playerVolume
    readonly property int popupHeight: 200

    readonly property var speakerIcon100: Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/not_compare_64px.png")
    readonly property var speakerIcon050: Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/not_compare_64px.png")
    readonly property var speakerIcon000: Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/not_compare_64px.png")
    readonly property var speakerIcon: {
        if(playerVolume > 50)
            speakerIcon100
        else if(playerVolume > 0)
            speakerIcon050
        else
            speakerIcon000
    }

    Popup{
        id: popup
        // height set in parent's onCompleted
        width: headButton.width
        x: headButton.x
        y: headButton.y - (height - headButton.height)
        height: volumeSlider.popupHeight

        padding: 4
        focus: true

        Item{
            id: textArea
            anchors{
                top: parent.top
                left: parent.left
                right: parent.right
                margins: - popup.padding
            }
            height: 20
            Text{
                anchors.fill: parent
                color: root.color_accent1
                font.pixelSize: 12
                text: "" + playerVolume + "%"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }

        Item{
            id: sliderArea
            anchors{
                top: textArea.bottom
                left: parent.left
                right: parent.right
                bottom: hideButtonArea.top
            }
            Slider{
                id: slider
                anchors.fill: parent
                orientation: Qt.Vertical
                from: 0
                to: 100
                stepSize: 1
                value: playerVolume
                onMoved: {
                    backend.personalization.playerVolume = value
                    console.log("new volume: " + backend.personalization.playerVolume)
                }
            }
        }

        Item{
            id: hideButtonArea
            anchors{
                bottom: parent.bottom
                left: parent.left
                right: parent.right
            }
            height: width

            ImageButton{
                dltDescription: "" + playerVolume + "%"
                dltImageIdle: volumeSlider.speakerIcon
                dltImageHover: dltImageIdle
                dltUsePopupColor: true
                onUserClicked: popup.close()
                dltImageMarginsRatio: 0
            }
        }
    }

    Item{
        id: headButton
        anchors{
            fill: parent
            margins: 5
        }

        opacity: 0.4

        ImageButton{
            id: headImage
            dltDescription: "" + playerVolume + "%"
            dltImageIdle: volumeSlider.speakerIcon
            dltImageHover: dltImageIdle
            onUserClicked: {
                popup.open()
            }
            // dltBackgroundVisible: true
            dltImageMarginsRatio: 0.12
        }
    }
}
