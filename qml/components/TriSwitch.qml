import QtQuick

Item {
    id: triSwitch

    property bool dark_theme: root.dark_theme
    property bool enabled: true // allways enabled, disabled was not implemented

    // colors
    property color colorBackgroundLeft:   Qt.rgba(200/255, 64/255, 64/255, 1.00) // Qt.rgba(0.70, 0.00, 0.00, 1.00)
    property color colorBackgroundMiddle: Qt.rgba(0.50, 0.50, 0.50, 0.20)
    property color colorBackgroundRight:  Qt.rgba(32/255, 200/255, 32/255, 1.00) // Qt.rgba(0.00, 0.70, 0.00, 1.00)

    property color colorBorderLeft:   colorBackgroundLeft
    property color colorBorderMiddle: Qt.rgba(0.45, 0.45, 0.45, 1.00)
    property color colorBorderRight:  colorBackgroundRight

    property color colorDotLeft:   dark_theme ? Qt.rgba((200/3)/255, (64/3)/255, (64/3)/255, 1.00) : "white" // Qt.rgba(1.00, 1.00, 1.00, 1.00) // Qt.rgba(28/255, 27/255, 31/255, 1.00)
    property color colorDotMiddle: Qt.rgba(0.45, 0.45, 0.45, 1.00)
    property color colorDotRight:  dark_theme ? Qt.rgba((32/3)/255, (200/3)/255, (32/3)/255, 1.00) : "white" // Qt.rgba(1.00, 1.00, 1.00, 1.00) // Qt.rgba(28/255, 27/255, 31/255, 1.00)


    property color colorDotShadowLeft:   Qt.rgba(0.50, 0.00, 0.00, 0.20)
    property color colorDotShadowMiddle: Qt.rgba(0.50, 0.50, 0.50, 0.20)
    property color colorDotShadowRight:  Qt.rgba(0.00, 0.50, 0.00, 0.20)


    // switch size
    property int switchHeight: 40
    property int switchWidth: 80 // 60

    // old switch areas:
    //    switch areas      #-1##|#0#|##1##
    // property int lm_border: switchWidth * 5/13 // border between left and middle area
    // property int mr_border: switchWidth * 8/13 // border between middle and right area

    // new switch areas:
    // state: -1    ########0########|##1##  ==  3/4 | 1/4
    // state:  0    ####-1#####|#####1#####  ==  2/4 | 2/4
    // state:  1    #-1##|########0########  ==  1/4 | 3/4
    // this makes switch easier to operate
    readonly property int l_border: switchWidth * 3/4
    readonly property int m_border: switchWidth * 2/4
    readonly property int r_border: switchWidth * 1/4
    property int border: m_border // border between both areas

    readonly property int switchBorderMargin: switchHeight * 0.2


    // dot positions
    readonly property int dotPosLeft: - switchWidth * 0.25
    readonly property int dotPosMiddle: 0
    readonly property int dotPosRight: switchWidth * 0.25
    readonly property int dotPos: (state == 0 ? dotPosMiddle : (state < 0 ? dotPosLeft : dotPosRight))


    // dot size
    readonly property int dotSizeLeft: switchHeight * 0.6
    readonly property int dotSizeMiddle: switchHeight * 0.4
    readonly property int dotSizeRight: switchHeight * 0.6
    readonly property int dotSizeDefault: (state == 0 ? dotSizeMiddle : (state < 0 ? dotSizeLeft : dotSizeRight))
    readonly property int dotSizePressed: switchHeight - switchBorderMargin * 1.5
    property int dotSizeChangable: dotSizeDefault  // changed by mouse area


    // L: -1     M: 0     R: 1
    property int state: 0


    readonly property color colorBackground:
        (state == 0 ? colorBackgroundMiddle : (state < 0 ? colorBackgroundLeft : colorBackgroundRight))
    readonly property color colorBorder:
        (state == 0 ? colorBorderMiddle : (state < 0 ? colorBorderLeft : colorBorderRight))
    readonly property color colorDot:
        (state == 0 ? colorDotMiddle : (state < 0 ? colorDotLeft : colorDotRight))
    readonly property color colorDotShadow:
        (state == 0 ? colorDotShadowMiddle : (state < 0 ? colorDotShadowLeft : colorDotShadowRight))





    Rectangle{
        id: background
        width: switchWidth - switchBorderMargin
        height: switchHeight - switchBorderMargin
        anchors.centerIn: parent
        border.color: colorBorder
        border.width: 2
        color: colorBackground
        radius: height/2
    }
    Rectangle{
        id: switchDot
        anchors.verticalCenter: background.verticalCenter
        width: dotSizeChangable
        height: width
        x: dotPos - width/2
        radius: height/2
        color: colorDot
    }
    Rectangle{
        id: switchDotShadow
        anchors.verticalCenter: background.verticalCenter
        width: switchHeight
        x: dotPos - width/2
        height: width
        radius: height/2
        color: colorDotShadow
        visible: false
    }

    function handleMousePressed(mouse)
    {
        // set pressed dot size
        triSwitch.dotSizeChangable = triSwitch.dotSizePressed
    }

    function handleMouseReleased(mouse)
    {
        // old switch areas:
        // if(mouse.x < triSwitch.lm_border)
        //     triSwitch.state = -1
        // else if(mouse.x > triSwitch.mr_border)
        //     triSwitch.state = 1
        // else
        //     triSwitch.state = 0

        var m_x = mouse.x
        var border = triSwitch.border
        var state = triSwitch.state

        // update state depends on current mouse position
        switch(state)
        {
        case -1:
            if(m_x < border)    state = 0
            else                state = 1
            break;
        case  0:
            if(m_x < border)    state = -1
            else                state = 1
            break;
        case  1:
            if(m_x < border)    state = -1
            else                state = 0
            break;
        }

        // update border depends on current state
        switch(state)
        {
        case -1:
            border = triSwitch.l_border;
            break;
        case  0:
            border = triSwitch.m_border;
            break;
        case  1:
            border = triSwitch.r_border;
            break;
        }

        // set new button state
        triSwitch.state = state
        // set border between mouse areas
        triSwitch.border = border
        // set default dot size
        triSwitch.dotSizeChangable = triSwitch.dotSizeDefault
    }

    MouseArea{
        id: mouseArea
        anchors.fill: background
        hoverEnabled: true
        onEntered: {
            switchDotShadow.visible = true
        }

        onExited: {
            switchDotShadow.visible = false
        }

        onPressed: (mouse)=> handleMousePressed(mouse)
        onReleased: (mouse)=> handleMouseReleased(mouse)
    }
}
