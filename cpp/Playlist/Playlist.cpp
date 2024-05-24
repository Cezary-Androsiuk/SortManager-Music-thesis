#include "Playlist.h"

Playlist::Playlist(QObject *parent)
    : QObject{parent}
{}

void Playlist::loadNewPlaylistList(TagList *list)
{
    DB << "in PLAYLIST";
}
