#include "SongDetails.h"

SongDetails::SongDetails(QObject *parent)
    : QObject{parent},
    m_id(0),
    m_tags(nullptr)
{

}

const int &SongDetails::get_id() const
{
    return m_id;
}

const TagList *SongDetails::get_tags() const
{
    return m_tags;
}

void SongDetails::set_id(const int &id)
{
    m_id = id;
}

void SongDetails::set_tags(const TagList *tags)
{
    m_tags = tags;
}
