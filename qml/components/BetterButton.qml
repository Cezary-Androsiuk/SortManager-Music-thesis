import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material
import Qt5Compat.GraphicalEffects

Item{
    id: betterButton
    anchors.fill: parent

    required property string dltText
    property int dltFontSize: 14

    property bool dltBorderVisible: false
    property double dltBorderOpacity: 0.3
    property bool dltUsePopupColor: false

    property color dltIdleColor: root.color_element_idle
    property color dltHoverColor: root.color_element_hover
    property color dltPressColor: root.color_element_press

    property color dltBacgroundIdleColor: root.color_background
    property color dltBackgroundHoverColor: root.color_button_hover

    property color dltBacgroundIdleColorPopup: root.dark_theme ? root.rgb(66, 66, 66) : root.rgb(255,255,255)
    property color dltBackgroundHoverColorPopup: root.dark_theme ? root.rgb(96, 96, 96) : root.rgb(215,215,215)

    property bool dltDarkThemeRefresh: root.dark_theme
    onDltDarkThemeRefreshChanged: {
        // force to refresh because can't find reason why this isn't
        //    allways refreshing after changing dark_theme state
        // i think that is caused by some interaction with containsMouse
        text.color = dltIdleColor
    }

    signal userClicked()

    readonly property color fixedBackgroundIdleColor: dltUsePopupColor ? dltBacgroundIdleColorPopup : dltBacgroundIdleColor
    readonly property color fixedBackgroundHoverColor: dltUsePopupColor ? dltBackgroundHoverColorPopup : dltBackgroundHoverColor

    readonly property color fixedBackgroundColor:
        msArea.containsMouse ? fixedBackgroundHoverColor : fixedBackgroundIdleColor

    Rectangle{
        id: background
        anchors.fill: parent
        color: fixedBackgroundColor
    }

    Item{
        id: textContainer
        anchors{
            fill: parent
            leftMargin: 10
            rightMargin: 10
        }
        clip: true

        Text{
            id: text
            width: parent.width
            height: parent.height

            readonly property bool textNotFit: text.contentWidth >= text.width

            text: dltText
            color: root.color_accent1
            font.pixelSize: dltFontSize
            horizontalAlignment: textNotFit ? Text.AlignLeft : Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        Rectangle{
            id: endTextMask
            anchors{
                top: parent.top
                bottom: parent.bottom
                right: parent.right
            }
            width: parent.width * 0.2
            visible: text.textNotFit

            gradient: Gradient{
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: "transparent" }
                GradientStop { position: 1.0; color: fixedBackgroundColor }
            }
        }
    }

    Rectangle{
        id: border
        anchors.fill: parent

        visible: dltBorderVisible
        color: "transparent"
        border.color: root.color_accent1
        border.width: 1
        opacity: dltBorderOpacity
    }


    // below mouse area need to be copied to every component that emulate button
    // extracting it to the new component might now workout
    MouseArea{
        id: msArea
        anchors.fill: parent
        hoverEnabled: true

        // following code is emulating button fine and don't need any changes
        onEntered: {
            // change color to hover or to press color, if following action happend:
            // button was pressed, left area, and enter area again (constantly being pressed)
            text.color = msArea.containsPress ? dltPressColor : dltIdleColor
        }
        onExited: {
            // change color to idle
            text.color = dltIdleColor
        }
        onPressed: {
            // change color to press
            text.color = dltPressColor
        }
        onReleased: {
            // change color to hover if mouse was relesed on area or if was relesed
            //     outside area to the idle color
            text.color = msArea.containsMouse ? dltHoverColor : dltIdleColor

            // if was relesed, still containing the mouse activate click
            if(msArea.containsMouse)
            {
                console.log("text")
                userClicked()
            }
        }

        ToolTip{
            visible: msArea.containsMouse && (text.contentWidth >= text.width)
            text: dltText
            delay: 400
        }
    }
}
