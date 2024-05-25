import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Dialogs

import "qrc:/SortManager-Music/qml/components" // ImageButton

Item {    
    id: comparatorComponent
    anchors{
        top: parent.top
        bottom: parent.bottom
        right: dltAnchorRight
    }
    width: height * 3/2

    property var dltModel: [
        {image: "example", desc: "what you have on your mind"},
        {image: "data", desc: "anything about data"},
        {image: "in", desc: "random word"},
        {image: "list", desc: "use your own data"}
    ]
    property int dltIndex: 0

    // right handle for the item
    property var dltAnchorRight: null

    readonly property int popup_case_height: comboBoxHead.height * 0.8;

    Component.onCompleted: {
        var popup_height = 0;
        for(var _element of dltModel)
            popup_height += popup_case_height;
        if(popup_height === 0)
            popup_height = 10;

        var max_height = popup_case_height * 3.8; // 3.8 button will be shown
        if(popup_height > max_height)
            popup_height = max_height;

        modelList.height = popup_height;

        listLoader.active = true;
    }

    Popup{
        id: modelList
        // height set in parent's onCompleted
        width: comboBoxHead.width
        x: comboBoxHead.x
        y: comboBoxHead.y

        padding: 4
        focus: true
        Loader{
            id: listLoader
            anchors.centerIn: parent
            width: parent.width
            height: parent.height
            sourceComponent: listComponent
            active: false
        }

        Component{
            id: listComponent
            ListView{
                anchors.fill: parent
                model: dltModel
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                opacity: 0.5
                currentIndex: dltIndex
                highlight: Rectangle{
                    color: root.color_accent2
                    opacity: 0.3
                }

                delegate: Item{
                    width: parent.width
                    height: comparatorComponent.popup_case_height

                    ImageButton{
                        dltDescription: modelData.desc
                        dltImageIdle: modelData.image
                        dltImageHover: dltImageIdle
                        onUserClicked: {
                            modelList.close()
                            dltIndex = index
                        }
                        dltUsePopupColor: true
                        dltBackgroundVisible: true
                    }

                }
            }
        }
    }

    Item{
        id: comboBoxHead
        anchors{
            fill: parent
            margins: 5
        }

        opacity: (dltIndex === 0) ? 0.2 : 0.4

        ImageButton{
            id: comboBoxImage
            dltDescription: dltModel[dltIndex].desc
            dltImageIdle: dltModel[dltIndex].image
            dltImageHover: dltImageIdle
            onUserClicked: {
                modelList.open()
            }
            dltBackgroundVisible: true
        }
    }
}
