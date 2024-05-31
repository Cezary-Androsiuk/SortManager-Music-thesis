import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material

import "qrc:/SortManager-Music/qml/components" // BetterButton
import "qrc:/SortManager-Music/qml/popups/common" // TextsArea

Popup {
    id: popupLoading
    property string dltText: "no message has been set"
    property string dltTextProgress: "" // "13/21"
    property int dltTextFontSize: 12

    // - LB
    property string dltTextMB: "Cancel" // middle button text
    // - RB

    // - LB
    property int dltFontSizeMB: 17 // middle button font size
    // - RB

    // - LB
    signal dltClickedMB() // clicked middle button
    // - RB

    height: 160
    width: 310

    x: root._w/2 - width/2
    y: (root._h/2 - height/4) - height/2

    dim: true
    modal: true
    closePolicy: Popup.NoAutoClose

    Item{
        id: globArea
        anchors.fill: parent

        Item{
            id: areaBusyIndicator
            anchors{
                top: parent.top
                left: parent.left
                right: parent.right
            }
            height: parent.height * 3/5

            BusyIndicator{
                id: bi
                anchors{
                    horizontalCenter: parent.horizontalCenter
                    top: parent.top
                    topMargin: 10
                }

                width: 50
                height: 50
            }
            Text{
                anchors{
                    horizontalCenter: bi.horizontalCenter
                    top: bi.bottom
                }
                text: dltText + dltTextProgress
                color: root.color_accent1
                font.pixelSize: dltTextFontSize
            }
        }

        Item{
            id: areaButtons
            anchors{
                top: areaBusyIndicator.bottom
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }

            Item{
                id: areaMB
                anchors.centerIn: parent
                height: 46
                width: 80

                BetterButton{
                    id: buttonM
                    dltText: popupLoading.dltTextMB
                    dltFontSize: popupLoading.dltFontSizeMB
                    dltBorderVisible: true
                    dltUsePopupColor: true
                    onUserClicked:{
                        popupLoading.close();
                        dltClickedMB();
                    }
                }
            }

        }

    }



}
