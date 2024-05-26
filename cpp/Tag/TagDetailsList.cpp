#include "TagDetailsList.h"

TagDetailsList::TagDetailsList(QObject *parent)
    : QObject{parent},
    m_tags({})
{}

QList<TagDetails *> &TagDetailsList::tags()
{
    return m_tags;
}

const QList<TagDetails *> &TagDetailsList::c_ref_tags() const
{
    return m_tags;
}

QList<TagDetails *> TagDetailsList::const_tags() const
{
    return m_tags;
}
