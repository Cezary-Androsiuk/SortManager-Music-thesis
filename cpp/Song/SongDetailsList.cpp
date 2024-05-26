#include "SongDetailsList.h"

SongDetailsList::SongDetailsList(QObject *parent)
    : QObject{parent},
    m_songs({})
{}

QList<SongDetails *> &SongDetailsList::songs()
{
    return m_songs;
}

const QList<SongDetails *> &SongDetailsList::c_ref_songs() const
{
    return m_songs;
}

QList<SongDetails *> SongDetailsList::const_songs() const
{
    return m_songs;
}
