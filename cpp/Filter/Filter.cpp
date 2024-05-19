#include "Filter.h"

Filter::Filter(QObject *parent)
    : QObject{parent}
{}

const QVariantMap &Filter::get_fields() const
{
    return m_fields;
}

QVariantMap &Filter::get_fields_ref()
{
    return m_fields;
}

void Filter::set_fields(const QVariantMap &qv_map)
{
    m_fields = qv_map;
    emit this->fieldsChanged();
}
