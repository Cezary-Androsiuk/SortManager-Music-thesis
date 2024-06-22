import QtQuick 2.15

import "qrc:/SortManager-Music/qml/components" // ImageButton
import "qrc:/SortManager-Music/qml/delegates/Playlist"

Item {
    width: parent.width - 15 /*scrollbar offset*/
    height: headerHeight

    Item{
        id: headerContent
        anchors{
            fill: parent
            bottomMargin: headerSeparatorSpace
        }

        Item{
            id: refreshField
            anchors{
                top: parent.top
                left: parent.left
                bottom: parent.bottom
            }
            width: height

            ImageButton{
                dltDescription: "Reload Songs"
                dltImageIdle: Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/refresh_36px.png")
                dltImageHover: dltImageIdle
                onUserClicked: {
                    backend.database.refreshPlaylist()
                    reloadPressed()
                }
            }
        }

        Item{
            id: searchField
            anchors{
                left: refreshField.right
                right: shuffleField.left
                top: parent.top
                bottom: parent.bottom
            }
            Rectangle{
                anchors{
                    fill: parent
                    leftMargin: 5
                    rightMargin: 5
                    topMargin: 8
                    bottomMargin: 8
                }
                color: root.color_accent1
                opacity: 0.1
            }

        }

        Item{
            id: shuffleField
            anchors{
                top: parent.top
                bottom: parent.bottom
                right: filtersField.left
            }
            width: height

            ImageButton{
                dltDescription: "Shuffle Songs"
                dltImageIdle: Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/shuffle_36px.png")
                dltImageHover: dltImageIdle
                onUserClicked: backend.playlist.shufflePlaylist()
            }
        }

        Item{
            id: filtersField
            anchors{
                right: parent.right
                top: parent.top
                bottom: parent.bottom
            }
            width: height

            ImageButton{
                dltDescription: "Filters"
                dltImageIdle: Qt.resolvedUrl("qrc:/SortManager-Music/assets/icons/filter_36px.png")
                dltImageHover: dltImageIdle
                onUserClicked: backend.database.loadFiltersModel()
            }
        }
    }

    Item{
        id: headerSeparator
        anchors{
            fill: parent
            topMargin: parent.height - headerSeparatorSpace
        }

        ThinSeparator{}
    }



}
