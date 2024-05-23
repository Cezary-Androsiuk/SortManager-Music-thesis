import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Dialogs

import "qrc:/SortManager-Music/qml/delegates"

Page {
    anchors.fill: parent

    property int delegateHeight: 60

    property var mdl: []

    Component.onCompleted: {
        mdl.push({id: 100, name: "Dark Theme"})
        mdl.push({id: 200, name: "Save Query to File"})
        mdl.push({id: 300, name: "Accent Color"})
        mdl.push({id: 400, name: "Show Constant Tags in List"})
        mdl.push({id: 500, name: "Default Add Tag Type"})
        mdl.push({id: 550, name: "Always keep a list position"})
        // mdl.push({id: 600, name: "Songs Open Path"})
        // mdl.push({id: 700, name: "Songs Main Path"})
        mdl.push({id: 800, name: "Show error description"})

        listViewLoader.active = true
    }

    Loader{
        id: listViewLoader
        anchors{
            left: parent.left
            top: listViewHeader.bottom
            right: parent.right
            bottom: parent.bottom
        }
        sourceComponent: listViewComponent
        active: false
    }

    Component{
        id: listViewComponent
        ScrollView{
            id: scrollView
            anchors.fill: parent
            ListView{
                id: personalisationsListView
                model: mdl

                clip: true

                boundsBehavior: Flickable.StopAtBounds

                footer: Item{
                    width: parent.width - 8 /*scrollbar offset*/
                    height: delegateHeight/2
                }

                // on this site scroll position will be saved allways or never cause do not contains any sub sites
                Component.onCompleted: {
                    contentY = root.last_pos_personalizations
                }
                Component.onDestruction: {
                    if(backend.personalization.alwaysKeepListPos)
                        root.last_pos_personalizations = contentY
                    else
                        root.last_pos_personalizations = 0
                }

                delegate: Loader{
                    width: parent.width - 8 /*scrollbar offset*/
                    height: delegateHeight
                    sourceComponent: {
                        if(false); // below lines are so beauty when equal <3
                        else if(modelData.id === 100) switchFieldDelegate;
                        else if(modelData.id === 200) switchFieldDelegate;
                        else if(modelData.id === 300) colorPickerDelegate;
                        else if(modelData.id === 400) switchFieldDelegate;
                        else if(modelData.id === 500) comboBoxDelegate;
                        else if(modelData.id === 550) switchFieldDelegate;
                        // else if(modelData.id === 600) pathSelectDelegate;
                        // else if(modelData.id === 700) pathSelectDelegate;
                        else if(modelData.id === 800) switchFieldDelegate;
                    }

                    Component {
                        id: switchFieldDelegate
                        SwitchField{
                            dltText: {
                                modelData.name
                            }
                            dltValue: {
                                if(false); // below lines are so beauty when equal <3
                                else if(modelData.id === 100) root.dark_theme
                                else if(modelData.id === 200) backend.personalization.saveExecQuery
                                else if(modelData.id === 400) backend.personalization.showConstantTags
                                else if(modelData.id === 550) backend.personalization.alwaysKeepListPos
                                else if(modelData.id === 800) backend.personalization.showErrorDesc
                            }
                            onDltValueChanged: {
                                if(false); // below lines are so beauty when equal <3
                                else if(modelData.id === 100) backend.personalization.app_theme = dltValue
                                else if(modelData.id === 200) backend.personalization.saveExecQuery = dltValue
                                else if(modelData.id === 400) backend.personalization.showConstantTags = dltValue
                                else if(modelData.id === 550)
                                {
                                    backend.personalization.alwaysKeepListPos = dltValue
                                    // reset all positions if keeping was turn off
                                    if(dltValue == false)
                                    {
                                        root.last_pos_settings = 0
                                        root.last_pos_personalizations = 0
                                        root.last_pos_db_management = 0
                                        root.last_pos_songs = 0
                                        root.last_pos_add_song = 0
                                        root.last_pos_edit_song = 0
                                        root.last_pos_tags = 0
                                        root.last_pos_add_tag = 0
                                        root.last_pos_edit_tag = 0
                                        root.last_pos_playlist = 0
                                        root.last_pos_filters = 0
                                    }
                                }
                                else if(modelData.id === 800) backend.personalization.showErrorDesc = dltValue
                            }
                        }
                    }

                    Component{
                        id: colorPickerDelegate
                        ColorSelectField{
                            dltText: modelData.name
                            dltColor: {
                                if(modelData.id === 300)
                                    root.color_accent2 // is dynamically changed in Main.qml
                            }
                            onDltColorChoosed: {
                                // reacting on dltColor changed wasn't update when user changed theme
                                //      so I changes local vairalble "choosed_color" and emit signal colorChoosed
                                //      and thats wokrs and when app_theme (that responds for a color_accent2)
                                //      is changed accent2 color is also changed
                                if(modelData.id === 300){
                                    if(root.dark_theme)
                                        backend.personalization.darkAccentColor = choosed_color
                                    else
                                        backend.personalization.lightAccentColor = choosed_color
                                }
                            }
                        }
                    }

                    Component{
                        id: comboBoxDelegate
                        ComboBoxField{
                            dltText: modelData.name
                            dltValue: backend.personalization.defaultAddTagType +1 // +1 because stored value is -1 or higher and in list is 0 or higher
                            dltModel: [
                                "",         // value -1 is Not selected
                                "Number",   // value  0 is TT_INTEGER in Database
                                "Text",     // value  1 is TT_TEXT in Database
                                "State"     // value  2 is TT_BOOL in Database
                            ]
                            onDltValueChanged: {
                                backend.personalization.defaultAddTagType = dltValue -1 // -1 because stored value is -1 or higher and in list is 0 or higher
                            }
                        }
                    }


                }

            }
        }
    }

    Rectangle{
        id: listViewHeader
        anchors{
            left: parent.left
            top: parent.top
            right: parent.right
        }
        height: tabBar.height
        color: root.color_background

        Component.onCompleted: {
            mainLoader.anchors.top = tabBar.top
        }

        TabButton{
            anchors{
                left: parent.left
                top: parent.top
                bottom: parent.bottom
            }
            width: height * 2

            text: "←"
            font.pixelSize: 30

            ToolTip.visible: hovered
            ToolTip.text: "Go Back"

            onClicked: {
                mainLoader.anchors.top = tabBar.bottom
                settingsLoader.source = root.pps_settings
            }
        }

        Text{
            text: "Personalisations"
            anchors.centerIn: parent
            font.pixelSize: 20
            color: root.color_accent1
        }
    }

}
