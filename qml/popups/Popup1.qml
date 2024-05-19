import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material

import "qrc:/SortManager-Music/qml/components" // BetterButton
import "qrc:/SortManager-Music/qml/popups/common" // TextsArea

Popup {
    id: popup1
    property string dltText: "no message has been set"
    property string dltDesc: ""


    // - LB
    property string dltTextMB: "MB" // middle button text
    // - RB

    // - LB
    property int dltFontSizeMB: 14 // middle button font size
    // - RB

    property bool dltJea: true // just exit action

    // - LB
    signal dltClickedMB() // clicked middle button
    // - RB

    readonly property bool noDesc: (dltDesc === "")

    height: noDesc ? 160 : 240
    width: 310

    x: root._w/2 - width/2
    y: (root._h/2 - height/4) - height/2

    // background: Rectangle {
    //     color: root.color_background
    //     radius: 10
    // }

    dim: true
    modal: true
    closePolicy: dltJea ?
                     Popup.CloseOnEscape | Popup.CloseOnPressOutside :
                     Popup.NoAutoClose

    Item{
        Component.onCompleted: {
            popup1.focus = true
        }
        Keys.onEscapePressed: {
            if(dltJea)
                popup1.close()
            else
                root.close()
        }
    }

    Item{
        id: globArea
        anchors{
            fill: parent
            topMargin: noDesc ? 0 : -10
            bottomMargin: noDesc ? 0 : -10
        }

        TextsArea{
            id: areaTexts
            dltText: popup1.dltText
            dltDesc: popup1.dltDesc
        }

        Item{
            id: areaButtons
            anchors{
                top: areaTexts.bottom
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
                    dltText: dltTextMB
                    dltFontSize: dltFontSizeMB
                    dltBorderVisible: true
                    dltUsePopupColor: true
                    onUserClicked:{
                        popup1.close();
                        dltClickedMB();
                    }
                }
            }

        }

    }



}
