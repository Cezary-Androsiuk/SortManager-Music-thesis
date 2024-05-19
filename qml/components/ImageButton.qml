import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material
import Qt5Compat.GraphicalEffects

Item{
    id: imageButton
    anchors.fill: parent

    required property string dltDescription
    required property string dltImageIdle
    required property string dltImageHover

    property color dltIdleColor: root.color_element_idle
    property color dltHoverColor: root.color_element_hover
    property color dltPressColor: root.color_element_press

    property bool dltDarkThemeRefresh: root.dark_theme
    onDltDarkThemeRefreshChanged: {
        // force to refresh because can't find reason why this isn't
        //    allways refreshing after changing dark_theme state
        // i think that is caused by some interaction with containsMouse
        colorOverlay.color = dltIdleColor
    }

    signal userClicked()
    Image{
        id: img
        fillMode: Image.PreserveAspectFit
        anchors{
            fill: parent
            margins: {
                var pw = parent.width * 0.20;
                var ph = parent.height * 0.20;

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
        color: dltIdleColor
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
            colorOverlay.color = msArea.containsPress ? dltPressColor : dltHoverColor
        }
        onExited: {
            // change image to idle
            img.source = dltImageIdle;

            // change color to idle
            colorOverlay.color = dltIdleColor
        }
        onPressed: {
            // change color to press
            colorOverlay.color = dltPressColor
        }
        onReleased: {
            // change color to hover if mouse was relesed on area or if was relesed
            //     outside area to the idle color
            colorOverlay.color = msArea.containsMouse ? dltHoverColor : dltIdleColor

            // if was relesed, still containing the mouse activate click
            if(msArea.containsMouse)
                userClicked()
        }

        ToolTip{
            visible: msArea.containsMouse && Backend.personalization.showTooltips
            text: dltDescription
            delay: 800
        }
    }

}
