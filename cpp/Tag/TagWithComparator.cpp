#include "TagWithComparator.h"

TagWithComparator::TagWithComparator(QObject *parent)
    : Tag{parent},
    m_comparison_way(0),
    m_comparison_value("")
{}

const int &TagWithComparator::get_comparison_way() const
{
    return m_comparison_way;
}

const QString TagWithComparator::get_comparison_value() const
{
    return m_comparison_value;
}

void TagWithComparator::set_comparison_way(const int &way)
{
    m_comparison_way = way;
}

void TagWithComparator::set_comparison_value(const QString &value)
{
    m_comparison_value = value;
}
