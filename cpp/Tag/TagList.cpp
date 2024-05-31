#include "TagList.h"

TagList::TagList(QObject *parent)
    : QObject{parent},
    m_tags({})
{}

QList<Tag *> &TagList::tags()
{
    return m_tags;
}

const QList<Tag *> &TagList::c_ref_tags() const
{
    return m_tags;
}

QList<Tag *> TagList::const_tags() const
{
    return m_tags;
}

qsizetype TagList::tags_count() const
{
    return m_tags.count();
}
