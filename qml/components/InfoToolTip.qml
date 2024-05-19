import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material
import Qt5Compat.GraphicalEffects

Item{
    id: infoToolTip
    anchors.fill: parent

    required property string dltDescription

    property color dltIdleColor: root.color_element_idle
    property color dltHoverColor: root.color_element_hover

    property bool dltDarkThemeRefresh: root.dark_theme
    onDltDarkThemeRefreshChanged: {
        // force to refresh because can't find reason why this isn't
        //    allways refreshing after changing dark_theme state
        // i think that is caused by some interaction with containsMouse
        colorOverlay.color = dltIdleColor
    }

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

        source: Qt.resolvedUrl("qrc:/Music_directory_player/assets/icons/info.png")
    }

    ColorOverlay {
        id: colorOverlay
        anchors.fill: img
        source: img
        color: dltIdleColor
    }

    MouseArea{
        id: msArea
        anchors.fill: parent
        hoverEnabled: true

        onEntered: {
            // change color to hover
            colorOverlay.color = dltHoverColor
        }
        onExited: {
            // change color to idle
            colorOverlay.color = dltIdleColor
        }

        ToolTip{
            visible: msArea.containsMouse && Backend.personalization.showTooltips
            text: dltDescription
            delay: 600
        }
    }

}
