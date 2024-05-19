import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material

import "qrc:/SortManager-Music/qml/components" // BetterButton
import "qrc:/SortManager-Music/qml/popups/common" // TextsArea

Popup {
    id: popup2
    property string dltText: "no message has been set"
    property string dltDesc: ""


    property string dltTextLB: "LB" // left button text
    // - MB
    property string dltTextRB: "RB" // right button text

    property int dltFontSizeLB: 14 // left button font size
    // - MB
    property int dltTontSizeRB: 14 // right button font size

    property bool dltJea: true // just exit action

    signal dltClickedLB() // clicked left button
    // - MB
    signal dltClickedRB() // clicked right button

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
            popup2.focus = true
        }
        Keys.onEscapePressed: {
            if(dltJea)
                popup2.close()
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
            dltText: popup2.dltText
            dltDesc: popup2.dltDesc
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
                width: parent.width /2

                Item{
                    anchors{
                        verticalCenter: parent.verticalCenter
                        right: parent.right
                        rightMargin: (parent.width - width) * 1/3
                    }
                    height: 46
                    width: 80

                    BetterButton{
                        id: buttonL
                        dltText: dltTextLB
                        dltFontSize: dltFontSizeLB
                        dltBorderVisible: true
                        dltBorderOpacity: 0.1
                        dltUsePopupColor: true
                        onUserClicked:{
                            popup2.close();
                            dltClickedLB();
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
                width: parent.width /2

                Item{
                    anchors{
                        verticalCenter: parent.verticalCenter
                        left: parent.left
                        leftMargin: (parent.width - width) * 1/3
                    }
                    height: 46
                    width: 80

                    BetterButton{
                        id: buttonR
                        dltText: dltTextRB
                        dltFontSize: dltTontSizeRB
                        dltBorderVisible: true
                        dltBorderOpacity: 0.1
                        dltUsePopupColor: true
                        onUserClicked:{
                            popup2.close();
                            dltClickedRB();
                        }
                    }
                }

            }
        }

    }



}
