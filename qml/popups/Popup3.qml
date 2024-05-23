import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material

import "qrc:/SortManager-Music/qml/components" // BetterButton
import "qrc:/SortManager-Music/qml/popups/common" // TextsArea

Popup {
    id: popup3
    property string dltText: "no message has been set"
    property string dltDesc: ""


    property string dltTextLB: "LB" // left button text
    property string dltTextMB: "MB" // middle button text
    property string dltTextRB: "RB" // right button text

    property int dltFontSizeLB: 12 // left button font size
    property int dltFontSizeMB: 12 // middle button font size
    property int dltFontSizeRB: 12 // right button font size

    property bool dltJea: true // just exit action

    signal dltClickedLB() // clicked left button
    signal dltClickedMB() // clicked middle button
    signal dltClickedRB() // clicked right button

    readonly property bool noDesc:
        ((dltDesc === "") || (backend.personalization.showErrorDesc === false))

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
            popup3.focus = true
        }
        Keys.onEscapePressed: {
            if(dltJea)
                popup3.close()
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
            dltText: popup3.dltText
            dltDesc: popup3.dltDesc
            dltNoDesc: popup3.noDesc
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
                id: areaLB
                anchors{
                    top: parent.top
                    bottom: parent.bottom
                    left: parent.left
                }
                width: parent.width /3

                Item{
                    anchors.centerIn: parent
                    height: 46
                    width: 80

                    BetterButton{
                        id: buttonL
                        dltText: dltTextLB
                        dltFontSize: dltFontSizeLB
                        dltBorderVisible: true
                        dltUsePopupColor: true
                        onUserClicked:{
                            popup3.close();
                            dltClickedLB();
                        }
                    }
                }
            }

            Item{
                id: areaMB
                anchors{
                    top: parent.top
                    bottom: parent.bottom
                    left: areaLB.right
                    right: areaRB.left
                }

                Item{
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
                            popup3.close();
                            dltClickedMB();
                        }
                    }
                }
            }

            Item{
                id: areaRB
                anchors{
                    top: parent.top
                    bottom: parent.bottom
                    right: parent.right
                }
                width: parent.width /3

                Item{
                    anchors.centerIn: parent
                    height: 46
                    width: 80

                    BetterButton{
                        id: buttonR
                        dltText: dltTextRB
                        dltFontSize: dltFontSizeRB
                        dltBorderVisible: true
                        dltUsePopupColor: true
                        onUserClicked:{
                            popup3.close();
                            dltClickedRB();
                        }
                    }
                }
            }
        }

    }



}
