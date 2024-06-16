import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material
import Qt5Compat.GraphicalEffects

Item{
    id: imageButton
    anchors.fill: parent

    property string dltDescription: ""
    required property string dltImageIdle
    required property string dltImageHover
    property bool dltBackgroundVisible: false
    property bool dltUsePopupColor: false
    property double dltImageMarginsRatio: 0.2
    onDltImageIdleChanged: {
        img.source = dltImageIdle // sometimes was not refresh it self
    }

    // text colors
    property color dltTextIdleColor: root.color_element_idle
    property color dltTextHoverColor: root.color_element_hover
    property color dltTextPressColor: root.color_element_press

    readonly property color fixedTextIdleColor: dltTextIdleColor
    readonly property color fixedTextHoverColor: dltBackgroundVisible ? dltTextIdleColor : dltTextHoverColor
    readonly property color fixedTextPressColor: dltTextPressColor

    // background colors
    property color dltBacgroundIdleColor: root.color_background
    property color dltBackgroundHoverColor: root.color_button_hover

    property color dltBacgroundIdleColorPopup: root.dark_theme ? root.rgb(66, 66, 66) : root.rgb(255,255,255)
    property color dltBackgroundHoverColorPopup: root.dark_theme ? root.rgb(96, 96, 96) : root.rgb(215,215,215)

    readonly property color fixedBackgroundIdleColor: dltUsePopupColor ? dltBacgroundIdleColorPopup : dltBacgroundIdleColor
    readonly property color fixedBackgroundHoverColor: dltUsePopupColor ? dltBackgroundHoverColorPopup : dltBackgroundHoverColor

    readonly property color fixedBackgroundColor:
        msArea.containsMouse ? fixedBackgroundHoverColor : fixedBackgroundIdleColor




    property bool dltDarkThemeRefresh: root.dark_theme
    onDltDarkThemeRefreshChanged: {
        // force to refresh because can't find reason why this isn't
        //    allways refreshing after changing dark_theme state
        // i think that is caused by some interaction with containsMouse
        colorOverlay.color = fixedTextIdleColor
    }

    Rectangle{
        id: background
        anchors.fill: parent
        visible: dltBackgroundVisible
        color: fixedBackgroundColor
    }

    signal userClicked()
    Image{
        id: img
        fillMode: Image.PreserveAspectFit
        anchors{
            fill: parent
            margins: {
                var pw = parent.width * imageButton.dltImageMarginsRatio;
                var ph = parent.height * imageButton.dltImageMarginsRatio;

                (pw < ph) ? pw : ph;
            }
        }
        // mipmap: true // smooths image

        source: dltImageIdle
    }

    ColorOverlay {
        id: colorOverlay
        anchors.fill: img
        source: img
        color: fixedTextIdleColor
    }


    // below mouse area need to be copied to every component that emulate button
    // extracting it to the new component might now workout
    MouseArea{
        id: msArea
        anchors.fill: parent
        hoverEnabled: true

        // following code is emulating button fine and don't need any changes
        onEntered: {
            // change image to hover
            img.source = dltImageHover;

            // change color to hover or to press color, if following action happend:
            // button was pressed, left area, and enter area again (constantly being pressed)
            colorOverlay.color = msArea.containsPress ? fixedTextPressColor : fixedTextHoverColor
        }
        onExited: {
            // change image to idle
            img.source = dltImageIdle;

            // change color to idle
            colorOverlay.color = fixedTextIdleColor
        }
        onPressed: {
            // change color to press
            colorOverlay.color = fixedTextPressColor
        }
        onReleased: {
            // change color to hover if mouse was relesed on area or if was relesed
            //     outside area to the idle color
            colorOverlay.color = msArea.containsMouse ? fixedTextHoverColor : fixedTextIdleColor

            // if was relesed, still containing the mouse activate click
            if(msArea.containsMouse)
            {
                tooltip.visible = false // cause sometimes in popup, tooltip doesn't turn off
                userClicked()
            }
        }

        ToolTip{
            id: tooltip
            visible: msArea.containsMouse && (dltDescription !== "") // && Backend.personalization.showTooltips
            text: dltDescription
            delay: 800
        }
    }

}
