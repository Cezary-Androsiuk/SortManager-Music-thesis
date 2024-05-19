import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Dialogs

Item {    
    id: comparatorComponent
    anchors{
        top: parent.top
        bottom: parent.bottom
        right: dltAnchorRight
    }
    width: height * 3/2

    property var dltModel: [
        {text: "example", desc: "what you have on your mind"},
        {text: "data", desc: "anything about data"},
        {text: "in", desc: "random word"},
        {text: "list", desc: "use your own data"}
    ]
    property int dltIndex: 0

    // right handle for the item
    property var dltAnchorRight: null

    readonly property int popup_case_height: selectedValueField.height * 0.8;

    // describe if texts in button list will be equal size but they receive common pixel
    //    size that will be the largest possible for the langest text (to fit in given area)
    //    or all texts in button list will have own pixel size but if one text is short and the
    //    other is longer pixel sizes can be defferent
    property bool individualPixelSize: false
    property int lowestPixelSize: 5
    property int globalButtonPixelSize: 100 // start pixel size

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

    function computePixelSize(___text, ___field)
    {
        if(individualPixelSize)
        {
            // will find perfect font size for current delegate text
            var lw = ___text.width // protector "last width"
            var lh = ___text.height // protector "last height"
            while(___text.width <= ___field.width && ___text.height <= ___field.height)
            {
                ___text.font.pixelSize += 0.1
                if(lw === ___text.width && lh === ___text.height)
                {
                    // console.log("comparator component pixel size protected!");
                    break;
                }
            }
            while(___text.width > ___field.width || ___text.height > ___field.height)
            {
                ___text.font.pixelSize -= 1
                if(___text.font.pixelSize < lowestPixelSize) {
                    console.log("reached lowest pixelSize: " + lowestPixelSize)
                    break;
                }
            }
        }
        else
        {
            // will decrease size of all texts in list if any text reqiore smaller pixelSize
            while(___text.width > ___field.width || ___text.height > ___field.height)
            {
                globalButtonPixelSize -= 1
                if(globalButtonPixelSize < lowestPixelSize) {
                    console.log("reached lowest pixelSize: " + lowestPixelSize)
                    break;
                }
            }
        }
    }
    Popup{
        id: modelList
        // height set in parent's onCompleted
        width: selectedValueField.width
        x: selectedValueField.x
        y: selectedValueField.y

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

                delegate: TabButton{
                    id: _button
                    height: popup_case_height
                    width: parent.width
                    Text{
                        id: _text
                        Component.onCompleted: {
                            computePixelSize(_text, _button)
                        }

                        anchors.centerIn: parent
                        text: dltModel[index].text

                        color: root.color_accent1
                        font.pixelSize: individualPixelSize ? 10 : globalButtonPixelSize
                        verticalAlignment: Text.AlignVCenter
                    }

                    ToolTip{
                        visible: parent.hovered && modelList.opened
                        text: dltModel[index].desc
                        delay: 500
                    }




                    onClicked: {
                        modelList.close()
                        dltIndex = index
                    }
                }
            }
        }
    }
    // Rectangle{
    //     anchors.fill: selectedValueField
    //     color: "white"
    //     opacity: 0.01
    //     // radius: width*0.1
    // }
    TabButton{
        id: selectedValueField
        anchors{
            fill: parent
            margins: 5
        }
        onClicked: {
            modelList.open()
        }
        opacity: 0.4

    }
    Text{
        id: _text

        anchors.centerIn: selectedValueField
        text: dltModel[dltIndex].text

        color: root.color_accent1
        font.pixelSize: individualPixelSize ? 10 : globalButtonPixelSize
        verticalAlignment: Text.AlignVCenter

        opacity: 0.8

        Component.onCompleted: {
            computePixelSize(_text, selectedValueField)
        }

        onTextChanged: {
            computePixelSize(_text, selectedValueField)
        }
    }
}
