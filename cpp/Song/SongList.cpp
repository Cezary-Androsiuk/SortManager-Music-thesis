#include "SongList.h"

SongList::SongList(QObject *parent)
    : QObject{parent},
    m_songs({})
{}

QList<Song *> &SongList::songs()
{
    return m_songs;
}

const QList<Song *> &SongList::c_ref_songs() const
{
    return m_songs;
}

QList<Song *> SongList::const_songs() const
{
    return m_songs;
}

qsizetype SongList::songs_count() const
{
    return this->m_songs.count();
}
