#include "Playlist.h"

Playlist::Playlist(QObject *parent)
    : QObject{parent}
{}

Filter *Playlist::get_filter() const
{
    return m_filter;
}

void Playlist::set_filter(Filter *filter)
{
    m_filter = filter;
    emit this->filterChanged();
}
